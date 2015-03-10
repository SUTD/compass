#include "Graph.h"

#include <iostream>
#include <assert.h>
using namespace std;

#define PRINT_PATH false
#define PRINT_BUILD_INFO false
/* Node class*/
void Node::addEdge(Edge* edge)
{
    if(this->_edges == NULL)
	this->_edges = new vector<Edge*>();
    this->_edges->push_back(edge);
}


Node::~Node()
{
    delete _edges;
}

void Graph::addNode(unsigned int name, unsigned int start_addr, unsigned int end_addr)
{
    if(this->_nodeList == NULL)
	this->_nodeList = new vector<Node*>();
    NodeData* nd = new NodeData();
    nd->setName(name);
    nd->setStart(start_addr);
    nd->setEnd(end_addr);
    Node* p = new Node();
    p->setData(nd);
    _nodeList->push_back(p);
    this->_nodeNum++;
}

void Graph::addEdge(unsigned int tail, unsigned int head)
{
    //int tail_idx = locateVex(tail);
    int tail_idx = locateVexByEndAddr(tail); 
    int head_idx = locateVex(head);
    if(PRINT_BUILD_INFO)
    {
    	cout << "tail index " << tail_idx << endl;
    	cout << "head index " << head_idx << endl;
    }
    assert((tail_idx != -1)&&(head_idx != -1));
    Edge* e = new Edge();
    e->setVex(head_idx);
    _nodeList->at(tail_idx)->addEdge(e); //Node's addEdge
    this->_edgeNum++;
    if(PRINT_BUILD_INFO)
	cout << "edgeNum ** " << this->_edgeNum << endl;
}


/* Graph class*/
void Graph::display(){
	cout << this->_nodeNum << " nodes: ";
	for (int i=0; i<_nodeNum; i++)
		cout << i << ", ";
	cout << endl;
	cout << this->_edgeNum << " edges: " << endl;
	for (int i=0; i<_nodeNum; i++){
		vector<Edge*>* edges = _nodeList->at(i)->getEdges();
		if(edges != NULL){
		    for(unsigned int j = 0; j < edges->size(); j++)
			cout << _nodeList->at(i)->getData()->getName() << "->" << _nodeList->at(edges->at(j)->getAdjVex())->getData()->getName() << endl;
		}
	}
}

//find the node index according to the node name
int Graph::locateVex(unsigned int nodename){
	for (int i=0; i<this->_nodeNum; i++){
		if (nodename == this->_nodeList->at(i)->getData()->getName())
			return i;
	}
	return -1;
}

int Graph::locateVexByEndAddr(unsigned int endaddr){
	for (int i=0; i<this->_nodeNum; i++){
		if (endaddr == this->_nodeList->at(i)->getData()->getEnd())
			return i;
	}
	return -1;
}


void Graph::print_path(vector<int>* path){
	cout << "[ ";
	for (unsigned int i=0; i<path->size(); i++)
		cout << _nodeList->at(path->at(i))->getData()->getName()<< " ";

	cout << " ]" << endl;
}

void Graph::findNodesInAllPaths(vector<int>* path){
    set<int>* cur_nodes_in_path = new set<int>();
    for(unsigned int i = 0; i< path->size(); i++)
    {
    	//cout << "path->at(i)" << path->at(i) << endl;
    	cur_nodes_in_path->insert(path->at(i));
	
	//_nodesInPaths->insert(path->at(i));
    }
    _nodesInPaths->push_back(cur_nodes_in_path);
}
bool Graph::isadjacent_node_not_in_current_path(int node, vector<int>* path){

	for (unsigned int i=0; i<path->size(); i++){
		if (path->at(i) == node)
			return false;
	}
	return true;
}


// source and target are the index in the _nodeList table
void Graph::findAllPaths(int source, int target){
	// create a vector
	vector<int>*	tmp_path = new vector<int>();

	// put the source in a vector
	tmp_path->push_back(source);

	//create a queue of vecotrs pointers
	queue<vector<int>*>* q = new queue<vector<int>*>();
	//add the first vector into queue
	q->push(tmp_path);
	
	int i = 0;
	while(!q->empty()){
		// access and pop the first vector
		//cout << i++ << " round !!" << endl;
		//if(i++ >10000000)
		//	assert(false);
		tmp_path = q->front();
		q->pop();

		// get the last Node in the vector
		int last_nodeof_path = tmp_path->at(tmp_path->size() - 1);
		// compare the last node of the path with the target path
		// if hit target, print out
		if ( last_nodeof_path == target){
			if(PRINT_PATH)
			{
			    cout << "The requried path is:: ";
			    print_path(tmp_path);
			}
			findNodesInAllPaths(tmp_path);
		}
		else
		    _nodesInPaths->push_back(NULL);
		//otherwise	
		Node* last_node = this->_nodeList->at(last_nodeof_path);
		if (last_node->getEdges() !=NULL){
			for (unsigned int i = 0; i< last_node->getEdges()->size(); i++){
				//for each adjacent edges
				int adjvex = last_node->getEdges()->at(i)->getAdjVex();
				// adjacent node is not in the current path (in other words, no circle gurantee)
				if (isadjacent_node_not_in_current_path (adjvex, tmp_path)){
					// create a new path with adjacent node as the last node
					vector<int>* new_path = new vector<int>( tmp_path->begin(), tmp_path->end());
					new_path->push_back(adjvex);
					//push into the queue
					q->push(new_path);
				}
			}
		}
		delete tmp_path;
	}
	delete q;
}

vector<Node*>* Graph::getNodeList()
{
    return _nodeList;
}
/*
int main(){
	Graph g;
	g.createGraph();
	g.display();
}
*/
