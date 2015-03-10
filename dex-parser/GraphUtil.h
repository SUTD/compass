#ifndef _GRAPH_UTIL_H
#define _GRAPH_UTIL_H

#include "Graph.h"
#include "CodeHeader.h"
using namespace std;

class GraphUtil
{
private:
    Graph* _g;
    CodeHeader* _ch;
public:
    GraphUtil(Graph* g,CodeHeader* ch) {this->_g = g; this->_ch = ch;}
    ~GraphUtil() {delete _g;}
    /*
    void createGraph(Graph* g,CodeHeader* ch);
    void createNodes(Graph* g,CodeHeader* ch);
    void createEdges(Graph* g,CodeHeader* ch);
    int findNodeIdx(Graph* g, unsigned int addr);
    */
    void createGraph();
    void createNodes();
    void createEdges();
    int findNodeIdx(unsigned int addr);
    void findAllPaths(int, int);
};
#endif

