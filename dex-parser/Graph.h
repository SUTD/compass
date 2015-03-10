#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <string>
#include <vector>
#include "CodeHeader.h"
using namespace std;

//const MAX_VERTEX_NUM = 20;

class NodeData {
private:
	unsigned int 	_nodeName;
	unsigned int	_startAddr;
	unsigned int 	_endAddr;	
public:
	NodeData(): _nodeName(0), _startAddr(0), _endAddr(0){}
	void setName(unsigned int name)		{this->_nodeName = name;}
	void setStart(unsigned int start) 	{this->_startAddr = start;}
	void setEnd(unsigned int end) 	{this->_endAddr = end;}
	unsigned int  getName()	 		{return this->_nodeName;}
	unsigned int getStart() 		{return this->_startAddr;}
	unsigned int getEnd() 		{return this->_endAddr;}
};

class Edge {
private:
	int 	_adjvex;		// pointing vertex index
	//Edge*	_nextEdge;	// next edge
	int	_weight;
public:
	Edge(): _adjvex(-1), _weight(1) {}
	void setVex(int vex) 		{ this->_adjvex = vex; }
	//void setNextEdge( Edge* next) 	{ this->_nextEdge = next; }
	//Edge* getNextEdge()		{ return this->_nextEdge; }
	int getAdjVex()			{ return this->_adjvex; }
};


class Node {
private:
	NodeData*	_data;
	//Edge*		_firstEdge;	// the first edge
	vector<Edge*>*  _edges;    
public:
	Node(): _data(NULL), _edges(NULL){}
	void setData(NodeData* data)	{this->_data = data;}
	vector<Edge*>* getEdges(){return this->_edges;}
	//void setEdge(Edge* edge)	{this->_firstEdge = edge;}
	NodeData* getData()		{return this->_data;}
	//Edge* getFirstEdge()		{return this->_firstEdge;}
	//void insertEdge(Edge* edge);
	void addEdge(Edge* edge);
	vector<Edge*>* getEdge(int index) {return this->_edges;}
	~Node();
};


class Graph {
private:
	int 		_nodeNum;
	int 		_edgeNum;
	vector<Node*>*	_nodeList;
	vector<set<int>*>* _nodesInPaths; //nodes in all paths
public:
	Graph(): _nodeNum(0), _edgeNum(0){_nodeList = new vector<Node*>(); _nodesInPaths = new vector<set<int>*>();}
	vector<Node*>* getNodeList();
	void addNode(unsigned int name, unsigned int start, unsigned int end);
	void addEdge(unsigned int tail, unsigned int head);

	///those three methods can be outside the graph class
	//void createGraph(CodeHeader* ch);
	//void createNodes(CodeHeader* ch);
	//void createEdges(CodeHeader* ch);
	///

	void setNodeNum(int node)	{ this->_nodeNum = node; }
	void setEdgeNum(int edge)	{ this->_edgeNum = edge; }
	int locateVex(unsigned int nodename);
	int locateVexByEndAddr(unsigned int endaddr);


	void display();
	void findNodesInAllPaths(vector<int>* path);
	vector<set<int>*>* getNodesInAllPaths() {return _nodesInPaths;}
	void findAllPaths(int source, int target);
	void print_path(vector<int>* path);
	bool isadjacent_node_not_in_current_path(int node, vector<int>* path);
	~Graph()			{ delete _nodeList; delete _nodesInPaths; }
};


#endif
