#include <deque>
#include <assert.h>
#include <functional>
#include <sstream>
#include <algorithm>

#include "trackingalgorithm.h"
#include "graph.h"
#include "node.h"
#include "arc.h"
#include "log.h"

namespace dpct
{

TrackingAlgorithm::TrackingAlgorithm(Graph* graph):
	graph_(graph)
{

}

void TrackingAlgorithm::breadthFirstSearchVisitor(Node* begin, VisitorFunction func)
{
    assert(begin != nullptr);
	std::deque<Node*> queue;
	queue.push_back(begin);

	while(queue.size() > 0)
	{
        Node* node = queue.front();
		queue.pop_front();
        assert(node != nullptr);
        func(node);

		// add all nodes following outgoing arcs to queue
		for(Node::ArcIt outArc = node->getOutArcsBegin(); outArc != node->getOutArcsEnd(); ++outArc)
		{
            Node* next = (*outArc)->getTargetNode();
            assert(next != nullptr);

            // only add if not yet present in queue (or should we remove it and only update at the end?)
            if(std::find(queue.begin(), queue.end(), next) == queue.end())
            {
                queue.push_back(next);
            }
		}
	}
}

void TrackingAlgorithm::findNonintersectingBackwardPaths(Node* begin, Node* end, Solution& paths)
{
    // flow along the used arcs in the paths, where "forward flow" means from end to begin
    std::map<const Arc*, uint8_t> forward_flows;

    // init flows to zero
    VisitorFunction vis = [&](Node* n)
    {
        forward_flows[n->getBestInArc()] = 0;
    };
    breadthFirstSearchVisitor(begin, vis);

    // function to find a path with positive residual flow (recursively)
    std::function<bool(const Node*, const Node*, Path&)> findGoodPath = [&](const Node* begin, const Node* end, Path& p)
    {
        if(begin == end)
            return true;

        Arc* a = end->getBestInArc();
        int residual_capacity = 1 - forward_flows[a];
        if(residual_capacity > 0)
        {
            p.insert(p.begin(), a);
            return findGoodPath(begin, a->getSourceNode(), p);
        }
        else
            return false;
    };

    std::function<bool(const Node*, const Node*, Path&)> findGoodPathFromSource = [&](const Node* begin, const Node* end, Path& p)
    {
        for(Node::ConstArcIt a = end->getInArcsBegin(); a != end->getInArcsEnd(); ++a)
        {
            p.clear();
            p.push_back(*a);
            if(findGoodPath(begin, (*a)->getSourceNode(), p))
                return true;
        }
        return false;
    };  

    paths.clear();
    Path currentPath;
    while(findGoodPathFromSource(begin, end, currentPath))
    {
        // if we found a path, we know that its residual flow == 1
        for(const Arc* a : currentPath)
        {
            forward_flows[a] += 1;
        }
        paths.push_back(currentPath);
    }
}

void TrackingAlgorithm::printPath(TrackingAlgorithm::Path& p)
{
#ifdef DEBUG_LOG
    std::function<std::string(Node*)> nodeName = [](Node* n)
    {
        std::stringstream s;
        if(n->getUserData() != nullptr)
        {
            s << n->getUserData() << " = " << n;
        }
        else
        {
            s << n;
        }
        return s.str();
    };

    if(p.size() > 0)
        DEBUG_MSG("Path starts at node " << nodeName(p.front()->getSourceNode())
                  << " with cellCount " << p.front()->getSourceNode()->getCellCount()
                  << " and score " << p.front()->getSourceNode()->getCurrentScore());
    else
        return;

    for(Path::iterator it = p.begin(); it != p.end(); ++it)
    {
        DEBUG_MSG("\t follows " << (*it)->typeAsString() << " arc to node " <<  nodeName((*it)->getTargetNode())
                  << " enabled= " << ((*it)->isEnabled()?"yes":"no")
                  << " used= " << (*it)->getUseCount()
                  << " with cellCount " <<  (*it)->getTargetNode()->getCellCount()
                  << " and score " <<  (*it)->getTargetNode()->getCurrentScore());
    }
#endif
}

void TrackingAlgorithm::tic()
{
    startTime_ = std::chrono::high_resolution_clock::now();
}

double TrackingAlgorithm::toc() // return time in seconds
{
    endTime_ = std::chrono::high_resolution_clock::now();
    return getElapsedSeconds();
}

double TrackingAlgorithm::getElapsedSeconds()
{
    std::chrono::duration<double> elapsed_seconds = endTime_ - startTime_;
    DEBUG_MSG("Elapsed time: " << elapsed_seconds.count() << "sec");
    return elapsed_seconds.count();
}

} // namespace dpct
