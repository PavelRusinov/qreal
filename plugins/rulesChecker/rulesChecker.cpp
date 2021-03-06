﻿#include "rulesChecker.h"

using namespace qReal::rulesChecker;

RulesChecker::RulesChecker(qrRepo::GraphicalRepoApi const &graphicalRepoApi
		, qReal::gui::MainWindowInterpretersInterface &interpretersInterface)
	: mGRepoApi(&graphicalRepoApi)
	, mWindowInterface(&interpretersInterface)
	, mNoErrorsOccured(true)
{
	// TODO: get these lists from metamodel somehow
	mLinkTypes << "SequenceFlow";
	mContainerTypes << "Pool" << "Lane" << "BPMN Diagram";
}

bool RulesChecker::makeDetour(Id const &currentNode, IdList &usedNodes)
{
	if (usedNodes.contains(currentNode)) {
		return false; // cannot learn some more here
	}

	if (!mDiagramElements.contains(currentNode)) {
		return true;  // we already have made detour of forward nodes
	}

	mDiagramElements.removeOne(currentNode);
	usedNodes.append(currentNode);

	if (isLink(currentNode)) {
		Id const destinationNode = mGRepoApi->to(currentNode);
		if (destinationNode == Id::rootId()) {
			postError(noEndNode, currentNode); // we've already put info that link is incorrect
			return true; // done end-job for link(50%)
		}
		return makeDetour(destinationNode, usedNodes);
	}

	if (isEndNode(currentNode)) {
		return true; // we found real end-node
	}

	IdList frontNodes =  mGRepoApi->outgoingLinks(currentNode);
	if (frontNodes.isEmpty()) {
		postError(noEndNode, currentNode);
		return true; // done end-job for nodes (now 100%)
	}

	bool foundFinalNode = false; // to catch that we have found end-node anywhere in path
	foreach (Id const &node, frontNodes) {
		if (makeDetour(node, usedNodes)) {
			foundFinalNode = true;
		}
	}
	return foundFinalNode;
}

void RulesChecker::checkLinksRule(qReal::Id const &link)
{
	if (isLink(link)) {
		if (mGRepoApi->from(link) == Id::rootId() || mGRepoApi->to(link) == Id::rootId()) {
			postError(incorrectLink, link);
		}
	}
}

void RulesChecker::checkFinalNodeRule(qReal::Id const &node)
{
	bool isLastNode = isEndNode(node);
	if (!isLastNode && !isStartNode(node)) {
		return;
	}

	IdList incorrectLinks = (isLastNode) ? mGRepoApi->outgoingLinks(node) : mGRepoApi->incomingLinks(node);
	if (!incorrectLinks.isEmpty()) {
		postError((isLastNode) ? linkFromFinalNode : linkToStartNode, node);
		foreach (Id const &key, incorrectLinks) {
			mDiagramElements.removeOne(key);
		}
	}
}

void RulesChecker::checkDiagram()
{
	checkDiagramElements();
	IdList startElements = collectStartNodes();

	// check all paths which have start nodes
	while (!startElements.isEmpty()) {
		Id const currentHead = startElements.first();
		IdList usedNodes;
		if (!makeDetour(currentHead, usedNodes)) {
			postError(noEndNode, startElements.first());
		}
		startElements.removeFirst();
	}

	// check other connected components
	while (!mDiagramElements.isEmpty()) {
		Id const headNode = findFirstNode();
		postError(noStartNode, headNode);
		IdList usedNodes;
		if (!makeDetour(headNode, usedNodes)) {
			postError(noEndNode, headNode);
		}
	}
}

void RulesChecker::prepareOutput()
{
	mNoErrorsOccured = true;
	mWindowInterface->dehighlight();
	mWindowInterface->errorReporter()->clear();
}

void RulesChecker::checkAllDiagrams()
{
	prepareOutput();

	IdList diagrams = mGRepoApi->graphicalElements(Id("BPMNDiagram", "BPMNMetamodel", "BPMNDiagramNode"));

	foreach (Id const &diagram, diagrams) {
		mDiagramElements = elementsOfDiagram(diagram);
		checkDiagram();
	}

	if (mNoErrorsOccured) {
		mWindowInterface->errorReporter()->addInformation(tr("All diagrams compiled without errors"));
	}
}

void RulesChecker::checkCurrentDiagram()
{
	prepareOutput();

	if (mWindowInterface->activeDiagram() != Id()) {
		mDiagramElements = elementsOfDiagram(mWindowInterface->activeDiagram());
		checkDiagram();

		if (mNoErrorsOccured) {
			mWindowInterface->errorReporter()->addInformation(tr("Current diagram compiled without errors"));
		}
	}
}

void RulesChecker::postError(RulesChecker::ErrorsType const error, Id const &badNode)
{
	QString errorMsg("");
	switch (error) {
	case linkToStartNode: {
		errorMsg = tr("There are links to start node");
		break;
	}
	case linkFromFinalNode: {
		errorMsg = tr("There are links from End-event");
		break;
	}
	case noStartNode: {
		errorMsg = tr("There is no start-node in path");
		break;
	}
	case noEndNode: {
		errorMsg = tr("There is no end-node in path");
		break;
	}
	case incorrectLink: {
		errorMsg = tr("Some links are incorrect");
		break;
	}
	default: {
		errorMsg = tr("There are problems");
	}
	}
	mWindowInterface->errorReporter()->addError(errorMsg, badNode);
	mWindowInterface->highlight(badNode, false);
	mNoErrorsOccured = false;
}

bool RulesChecker::isLink(qReal::Id const &node) const
{
	return mLinkTypes.contains(node.element());
}

bool RulesChecker::isContainer(const qReal::Id &node) const
{
	return mContainerTypes.contains(node.element());
}

bool RulesChecker::isStartNode(qReal::Id const &node) const
{
	return (node.element() == "StartEvent");
}

bool RulesChecker::isEndNode(qReal::Id const &node) const
{
	return (node.element() == "EndEvent");
}

qReal::IdList RulesChecker::elementsOfDiagram(qReal::Id const &diagram) const
{
	IdList result = mGRepoApi->children(diagram);
	int const childrenCount = result.size();
	for (int i = 0; i < childrenCount; i++) {
		result.append(elementsOfDiagram(result.at(i)));
	}

	return result;
}

void RulesChecker::checkDiagramElements()
{
	foreach (Id const &id, mDiagramElements) {
		if (isContainer(id)) {
			mDiagramElements.removeOne(id);
		}

		checkLinksRule(id);
		checkFinalNodeRule(id);
	}
}

qReal::IdList RulesChecker::collectStartNodes() const
{
	IdList headNodes;
	foreach (Id const &id, mDiagramElements) {
		if (isStartNode(id)) {
			headNodes << id;
		}
	}
	return headNodes;
}

qReal::Id RulesChecker::findFirstNode() const
{
	Id result = mDiagramElements.first();
	int minIncomingLinks = mGRepoApi->incomingLinks(result).size();

	foreach (Id const &element, mDiagramElements) {
		int incomingLinks = mGRepoApi->incomingLinks(element).size();
		if (incomingLinks < minIncomingLinks) {
			minIncomingLinks = incomingLinks;
			result = element;
		}
	}

	return result;
}
