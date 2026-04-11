#include "pythongraphreader.h"
#include <assert.h>
#include <iostream>


namespace dpct {

PythonGraphReader::PythonGraphReader(pybind11::dict &graphDict, pybind11::dict &weightsDict,
                                     GraphBuilder *graphBuilder)
    : GraphReader(graphBuilder), graphDict_(graphDict), weightsDict_(weightsDict) {}

size_t PythonGraphReader::getNumWeights(pybind11::dict &hypothesis, GraphReader::JsonTypes type,
                                        bool statesShareWeights) {
    size_t numWeights;
    StateFeatureVector stateFeatVec = extractFeatures(hypothesis, type);

    if (statesShareWeights)
        numWeights = stateFeatVec[0].size();
    else {
        numWeights = 0;
        for (auto stateFeats : stateFeatVec)
            numWeights += stateFeats.size();
    }
    return numWeights;
}

void PythonGraphReader::createGraphFromPython() {

    // get flag whether states should share weights or not
    bool statesShareWeights = false;
    if (graphDict_.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Settings])) {
        pybind11::dict settings = graphDict_[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Settings])];
        if (settings.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::StatesShareWeights]))
            statesShareWeights = settings[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::StatesShareWeights])].cast<bool>();
    }

    // ------------------------------------------------------------------------------
    // get weight vector and number of weights needed for each different variable type
    FeatureVector weights = readWeightsFromPython(weightsDict_);
    size_t numDetWeights = 0;
    size_t numDivWeights = 0;
    size_t numAppWeights = 0;
    size_t numDisWeights = 0;
    size_t numLinkWeights = 0;

    pybind11::list segmentationHypotheses = graphDict_[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Segmentations])];
    for (size_t i = 0; i < segmentationHypotheses.size(); i++) {
        pybind11::dict jsonHyp = segmentationHypotheses[i];
        numDetWeights = getNumWeights(jsonHyp, GraphReader::JsonTypes::Features, statesShareWeights);

        if (jsonHyp.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::DivisionFeatures]))
            numDivWeights = getNumWeights(jsonHyp, GraphReader::JsonTypes::DivisionFeatures, statesShareWeights);

        if (jsonHyp.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::AppearanceFeatures]))
            numAppWeights = getNumWeights(jsonHyp, GraphReader::JsonTypes::AppearanceFeatures, statesShareWeights);

        if (jsonHyp.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::DisappearanceFeatures]))
            numDisWeights = getNumWeights(jsonHyp, GraphReader::JsonTypes::DisappearanceFeatures, statesShareWeights);
    }

    pybind11::list linkingHypotheses = graphDict_[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Links])];
    for (size_t i = 0; i < linkingHypotheses.size(); i++) {
        pybind11::dict jsonHyp = linkingHypotheses[i];
        if (jsonHyp.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Features]))
            numLinkWeights = getNumWeights(jsonHyp, GraphReader::JsonTypes::Features, statesShareWeights);
    }

    if (weights.size() != numDetWeights + numDivWeights + numAppWeights + numDisWeights + numLinkWeights) {
        std::stringstream s;
        s << "Loaded weights do not meet model requirements! Got " << weights.size() << ", need "
          << numDetWeights + numDivWeights + numAppWeights + numDisWeights + numLinkWeights;
        throw std::runtime_error(s.str());
    }

    size_t linkWeightOffset = 0;
    size_t detWeightOffset = linkWeightOffset + numLinkWeights;
    size_t divWeightOffset = detWeightOffset + numDetWeights;
    size_t appWeightOffset = divWeightOffset + numDivWeights;
    size_t disWeightOffset = appWeightOffset + numAppWeights;

    // ------------------------------------------------------------------------------
    // read segmentation hypotheses and add to flowgraph
    std::cout << "\tcontains " << len(segmentationHypotheses) << " segmentation hypotheses" << std::endl;

    for (size_t i = 0; (int)i < segmentationHypotheses.size(); i++) {
        pybind11::dict jsonHyp = segmentationHypotheses[i];

        if (!jsonHyp.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Id]))
            throw std::runtime_error("Cannot read detection hypothesis without Id!");
        size_t id = jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Id])].cast<size_t>();

        if (!jsonHyp.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Features]))
            throw std::runtime_error("Cannot read detection hypothesis without features!");

        if (jsonHyp.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Timestep])) {
            pybind11::list timeJson = jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Timestep])];
            if (len(timeJson) != 2)
                throw std::runtime_error("Node's Timestep is supposed to be a 2-element array");
            std::pair<int, int> timeRange = std::make_pair(timeJson[0].cast<int>(), timeJson[1].cast<int>());
            graphBuilder_->setNodeTimesteps(id, timeRange);
        }

        FeatureVector detCosts = weightedSumOfFeatures(extractFeatures(jsonHyp, GraphReader::JsonTypes::Features),
                                                       weights, detWeightOffset, statesShareWeights);
        FeatureVector detCostDeltas = costsToScoreDeltas(detCosts);
        FeatureVector appearanceCostDeltas;
        FeatureVector disappearanceCostDeltas;
        if (jsonHyp.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::AppearanceFeatures]))
            appearanceCostDeltas = costsToScoreDeltas(
                weightedSumOfFeatures(extractFeatures(jsonHyp, GraphReader::JsonTypes::AppearanceFeatures), weights,
                                      appWeightOffset, statesShareWeights));

        if (jsonHyp.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::DisappearanceFeatures]))
            disappearanceCostDeltas = costsToScoreDeltas(
                weightedSumOfFeatures(extractFeatures(jsonHyp, GraphReader::JsonTypes::DisappearanceFeatures), weights,
                                      disWeightOffset, statesShareWeights));

        size_t targetIdx = 0;
        if (jsonHyp.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::DisappearanceTarget])) {
            targetIdx = jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::DisappearanceTarget])].cast<size_t>();
        }

        graphBuilder_->addNode(id, detCosts, detCostDeltas, appearanceCostDeltas, disappearanceCostDeltas, targetIdx);
    }

    // read linking hypotheses
    std::cout << "\tcontains " << len(linkingHypotheses) << " linking hypotheses" << std::endl;
    for (size_t i = 0; i < linkingHypotheses.size(); i++) {
        pybind11::dict jsonHyp = linkingHypotheses[i];

        size_t srcId = jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::SrcId])].cast<size_t>();
        size_t destId = jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::DestId])].cast<size_t>();
        graphBuilder_->addArc(
            srcId, destId,
            costsToScoreDeltas(weightedSumOfFeatures(extractFeatures(jsonHyp, GraphReader::JsonTypes::Features),
                                                     weights, linkWeightOffset, statesShareWeights)));
    }

    // read divisions
    for (size_t i = 0; i < segmentationHypotheses.size(); i++) {
        pybind11::dict jsonHyp = segmentationHypotheses[i];
        size_t id = jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Id])].cast<size_t>();

        if (jsonHyp.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::DivisionFeatures])) {
            graphBuilder_->allowMitosis(id, costsToScoreDelta(weightedSumOfFeatures(
                                                extractFeatures(jsonHyp, GraphReader::JsonTypes::DivisionFeatures),
                                                weights, divWeightOffset, statesShareWeights)));
        }
    }

    // read exclusion constraints between detections
    // const Json::Value exclusions = graphDict_[GraphReader::JsonTypeNames[GraphReader::JsonTypes::Exclusions]];
    // std::cout << "\tcontains " << exclusions.size() << " exclusions" << std::endl;
    // for(size_t i = 0; i < (int)exclusions.size(); i++)
    // {
    // 	const Json::Value jsonExc = exclusions[i];
    // 	// TODO: implement some API for those?!
    // }
    if (graphDict_.contains(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Exclusions]) &&
        pybind11::len(graphDict_[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Exclusions])]) > 0)
        throw std::runtime_error("FlowSolver cannot deal with exclusion constraints yet!");
}

pybind11::object PythonGraphReader::saveResult() {
    pybind11::list detectionResults;
    pybind11::list linkResults;
    pybind11::list divisionResults;

    // save links
    GraphBuilder::ArcValueMap arcValues = graphBuilder_->getArcValues();
    pybind11::list linkingHypotheses = graphDict_[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Links])];
    for (size_t i = 0; i < linkingHypotheses.size(); i++) {
        // store in graph
        pybind11::dict jsonHyp = linkingHypotheses[i];
        size_t srcId = jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::SrcId])].cast<size_t>();
        size_t destId = jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::DestId])].cast<size_t>();
        size_t value = arcValues[std::make_pair(srcId, destId)];
        jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Value])] = value;

        // store in extra list
        pybind11::dict linkRes;
        linkRes[pybind11::str(JsonTypeNames[JsonTypes::SrcId])] = srcId;
        linkRes[pybind11::str(JsonTypeNames[JsonTypes::DestId])] = destId;
        linkRes[pybind11::str(JsonTypeNames[JsonTypes::Value])] = value;
        linkResults.append(linkRes);
    }

    // save divisions and detections
    GraphBuilder::DivisionValueMap divisionValues = graphBuilder_->getDivisionValues();
    GraphBuilder::NodeValueMap nodeValues = graphBuilder_->getNodeValues();

    pybind11::list segmentationHypotheses = graphDict_[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Segmentations])];
    for (size_t i = 0; i < segmentationHypotheses.size(); i++) {
        // store in graph
        pybind11::dict jsonHyp = segmentationHypotheses[i];
        size_t id = jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Id])].cast<size_t>();
        bool division = divisionValues[id];
        size_t value = nodeValues[id];
        jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::Value])] = value;
        jsonHyp[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::DivisionValue])] = division;

        // store in extra list
        pybind11::dict detRes;
        detRes[pybind11::str(JsonTypeNames[JsonTypes::Id])] = id;
        detRes[pybind11::str(JsonTypeNames[JsonTypes::Value])] = value;
        detectionResults.append(detRes);

        pybind11::dict divRes;
        divRes[pybind11::str(JsonTypeNames[JsonTypes::Id])] = id;
        divRes[pybind11::str(JsonTypeNames[JsonTypes::Value])] = division;
        divisionResults.append(divRes);
    }

    pybind11::dict result;
    result[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::DetectionResults])] = detectionResults;
    result[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::LinkResults])] = linkResults;
    result[pybind11::str(GraphReader::JsonTypeNames[GraphReader::JsonTypes::DivisionResults])] = divisionResults;
    return result;
}

PythonGraphReader::StateFeatureVector PythonGraphReader::extractFeatures(pybind11::dict &entry,
                                                                         GraphReader::JsonTypes type) {
    StateFeatureVector stateFeatVec;
    if (!entry.contains(GraphReader::JsonTypeNames[type]))
        throw std::runtime_error("Could not find dict entry for " + GraphReader::JsonTypeNames[type]);

    pybind11::list featuresPerState = entry[pybind11::str(GraphReader::JsonTypeNames[type])];

    if (len(featuresPerState) == 0)
        throw std::runtime_error("Features may not be empty for " + GraphReader::JsonTypeNames[type]);

    // std::cout << "\tReading features for: " << GraphReader::JsonTypeNames[type] << std::endl;

    // get the features per state
    for (size_t i = 0; i < featuresPerState.size(); i++) {
        // get features for the specific state
        FeatureVector featVec;
        pybind11::list featuresForState = featuresPerState[i];

        if (len(featuresForState) == 0)
            throw std::runtime_error("Features for state may not be empty for " + GraphReader::JsonTypeNames[type]);

        for (size_t j = 0; (int)j < len(featuresForState); j++) {
            featVec.push_back(featuresForState[j].cast<double>());
        }

        // std::cout << "\t\tfound " << featVec.size() << " features for state " << i << std::endl;

        stateFeatVec.push_back(featVec);
    }

    return stateFeatVec;
}

PythonGraphReader::FeatureVector PythonGraphReader::readWeightsFromPython(pybind11::dict &weightsDict) {
    pybind11::list weightsList = weightsDict[pybind11::str(GraphReader::GraphReader::JsonTypeNames[GraphReader::GraphReader::JsonTypes::Weights])];
    FeatureVector weights;
    for (size_t i = 0; i < weightsList.size(); i++) {
        // weights.push_back(extract<GraphBuilder::ValueType>(weightsList[i]));
        weights.push_back(weightsList[i].cast<GraphBuilder::ValueType>());
    }
    return weights;
}

} // end namespace dpct
