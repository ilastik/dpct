#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#include "flowgraph.h"
#include "flowgraphbuilder.h"
#include "graph.h"
#include "magnusson.h"
#include "magnussongraphbuilder.h"
#include "pythongraphreader.h"

using namespace dpct;
/**
 * @brief Helper class to release / lock the Python GIL
 */
class ScopedGILRelease {
  public:
    ScopedGILRelease() : state_() {}
  private:
    pybind11::gil_scoped_release state_;
};

pybind11::object flowBasedTracking(pybind11::object &graphDict, pybind11::object &weightsDict) {
    pybind11::dict graph = graphDict;
    pybind11::dict weights = weightsDict;

    FlowGraph flowGraph;
    FlowGraphBuilder graphBuilder(&flowGraph);
    PythonGraphReader pyGraphReader(graph, weights, &graphBuilder);
    pyGraphReader.createGraphFromPython();

    {
        ScopedGILRelease gilLock;
        flowGraph.maxFlowMinCostTracking();
    }

    return pyGraphReader.saveResult();
}

pybind11::object maxFlowTracking(pybind11::object &graphDict, pybind11::object &weightsDict) {
    pybind11::dict graph = graphDict;
    pybind11::dict weights = weightsDict;

    FlowGraph flowGraph;
    FlowGraphBuilder graphBuilder(&flowGraph);
    PythonGraphReader pyGraphReader(graph, weights, &graphBuilder);
    pyGraphReader.createGraphFromPython();

    {
        ScopedGILRelease gilLock;
        flowGraph.maxFlow();
    }

    return pyGraphReader.saveResult();
}

pybind11::object magnussonTracking(pybind11::object &graphDict, pybind11::object &weightsDict) {
    pybind11::dict graph = graphDict;
    pybind11::dict weights = weightsDict;

    Graph::Configuration config(true, true, true);
    Graph magnussonGraph(config);
    MagnussonGraphBuilder graphBuilder(&magnussonGraph);
    PythonGraphReader pyGraphReader(graph, weights, &graphBuilder);
    pyGraphReader.createGraphFromPython();
    std::vector<TrackingAlgorithm::Path> paths;

    {
        ScopedGILRelease gilLock;
        Magnusson tracker(&magnussonGraph, true, true, false);
        double score = tracker.track(paths);
        std::cout << "\nTracking finished in " << tracker.getElapsedSeconds() << " secs with energy " << -score
                  << std::endl;
    }

    graphBuilder.getSolutionFromPaths(paths);
    return pyGraphReader.saveResult();
}

/**
 * @brief Python interface of 'dpct' module
 */
PYBIND11_MODULE(dpct, m) {
    m.def("trackFlowBased", flowBasedTracking, pybind11::arg("graph"), pybind11::arg("weights"),
        "Use the flow-based tracker on a graph specified as a dictionary,"
        "in the same structure as the supported JSON format. Similarly, the weights are also given as dict.\n\n"
        "Returns a python dictionary similar to the result.json file, but also stores 'value' or 'divisionValue'"
        "for each detection and link.");
    m.def("trackMaxFlow", maxFlowTracking, pybind11::arg("graph"), pybind11::arg("weights"),
        "Run min-cost max-flow tracking on a graph specified as a dictionary,"
        "in the same structure as the supported JSON format. Similarly, the weights are also given as dict.\n\n"
        "The max-flow disregards division constraints and simply pushes as much flow through the net as possible.\n\n"
        "Returns a python dictionary similar to the result.json file, but also stores 'value' or 'divisionValue'"
        "for each detection and link.");
    m.def("trackMagnusson", magnussonTracking, pybind11::arg("graph"), pybind11::arg("weights"),
        "Use Magnusson's tracker on a graph specified as a dictionary,"
        "in the same structure as the supported JSON format. Similarly, the weights are also given as dict.\n\n"
        "Magnusson only approximates the residual graph and is thus much faster but not as close to the optimum, "
        "but still always feasible.\n\n"
        "Returns a python dictionary similar to the result.json file, but also stores 'value' or 'divisionValue'"
        "for each detection and link.");
}
