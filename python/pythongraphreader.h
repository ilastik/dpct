#ifndef PYTHON_GRAPH_READER
#define PYTHON_GRAPH_READER

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <dpct_core/graphbuilder.h>
#include <dpct_core/graphreader.h>

namespace dpct {

// ----------------------------------------------------------------------------------------
/**
 * @brief A python graph reader provides functions to read a flow/magnusson graph from a python dict,
 * and stores a mapping from ids to graph nodes, which allows to write back a result after tracking.
 *
 */
class PythonGraphReader : public GraphReader {
  public:
    /**
     * @brief Construct a python graph reader that reads from the specified files
     * @param graphDict the python dictionary containing the graph description
     * @param weightsDict the python dictionary containing the weights
     * @param graphBuilder magnusson or flow graph builder
     */
    PythonGraphReader(pybind11::dict &graphDict, pybind11::dict &weightsDict, GraphBuilder *graphBuilder);

    /**
     * @brief Add nodes and arcs to the graph builder according to the model file.
     * Costs are computed from features times weights
     */
    void createGraphFromPython();

    /**
     * @brief Save a resulting flow map back to the python dict, the flow is extracted by the graph builder.
     *
     * @returns Another python dictionary that looks exactly like the resulting JSON files.
     */
    pybind11::object saveResult();

  private:
    StateFeatureVector extractFeatures(pybind11::dict &entry, GraphReader::JsonTypes type);
    FeatureVector readWeightsFromPython(pybind11::dict &weightsDict);
    size_t getNumWeights(pybind11::dict &hypothesis, GraphReader::JsonTypes type, bool statesShareWeights);

  private:
    /// python graph dictionary in the same style as if it was stored in a json file
    pybind11::dict &graphDict_;

    /// python dictionary containing the weights
    pybind11::dict &weightsDict_;

    /// the weight vector loaded from file
    FeatureVector weights_;
};

} // end namespace dpct

#endif // PYTHON_GRAPH_READER
