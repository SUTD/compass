#include "Graph.h"
#include "GraphUtil.h"
#define _G_DBG false
#define _TRY_BLOCK true
#define _SYNC_BLOCK false
//void createNodes(Graph* g,CodeHeader* ch)
void GraphUtil::createNodes()
{
    map<unsigned int,  ASMInstruction*>* addr_ins = _ch->getAdd2InsMap();
    map<unsigned int, ASMInstruction*>::iterator it = addr_ins->begin();
    //NodeData* nd;
	bool lastfinished = false;
	int i = 0;
	unsigned int temp_name = 0;
	unsigned int temp_start = 0;
	unsigned int temp_end = 0;
	bool pos_catchall;
	if(_SYNC_BLOCK)
		pos_catchall = false;
    //bool pre_throw = true;
    for(; it != addr_ins->end(); it++)
    {
		ASMInstruction* asm_ins = (*it).second;
		//cout << "asm_ins  + addr " << asm_ins->toString() << " + " <<it->first << endl;
		DexOpcode opcode = asm_ins->getOpcode();
		if (_TRY_BLOCK) {
			if ((asm_ins->realTryStart()) || (asm_ins->realTryEnd())) {
				//if it is not first ins(first ins has no last ins, so we don't need to track if lastfinished or not)
				if (it != addr_ins->begin()) {
					if (!lastfinished) {
						it--;
						temp_end = it->first;
						i++;
						if (_G_DBG) {
							cout
									<< "(closure the last node because of try block)[ nd->end ]:"
									<< temp_end << endl;
							cout << "----" << endl;
						}
						_g->addNode(temp_name, temp_start, temp_end);
						lastfinished = true;
						it++;
					}
				}
				_g->addNode(it->first, it->first, it->first);
				if (_G_DBG) {
					cout << "encounter real try " << it->first
							<< "  create node -> " << i++ << endl;
					cout << "(closure of real try ) [nd->end ] " << it->first
							<< endl;
				}
				lastfinished = true;
				//create another ins's begin
				it++; //next ins;
				ASMInstruction* next_ins = it->second;
				it--;
				if ((it != (--addr_ins->end())) && (!next_ins->realTryStart())
						&& (!next_ins->realTryEnd())) //not the last ins, so need to begin another new node
						{
					it++;
					temp_name = it->first;
					temp_start = it->first;
					lastfinished = false;
					if (_G_DBG)
						cout << "next ins of try block " << temp_start
								<< "  create node -> " << i << endl;
				}
				continue;
			}
		}
		if (_SYNC_BLOCK) {
			if (asm_ins->is_label()) {
				if (asm_ins->getLabel().find("catchall") != string::npos)
					pos_catchall = true;
			}

			if (opcode == OP_THROW)
				pos_catchall = false;

			if (((opcode == OP_MONITOR_ENTER) || (opcode == OP_MONITOR_EXIT))
					&& (!pos_catchall)) {
				if (it != addr_ins->begin()) //not the first ins, so may need to closure the last node
						{
					if (!lastfinished) {
						it--;
						temp_end = it->first;
						i++;
						if (_G_DBG) {
							cout
									<< "(closure the last node because of monitor)[ nd->end ]:"
									<< temp_end << endl;
							cout << "----" << endl;
						}
						_g->addNode(temp_name, temp_start, temp_end);
						lastfinished = true;
						it++;
					}
				}

				_g->addNode(it->first, it->first, it->first);
				if (_G_DBG) {
					cout << "encounter monitor " << it->first
							<< "  create node -> " << i++ << endl;
					cout << "(closure of the monitor ) [nd->end ] " << it->first
							<< endl;
				}
				lastfinished = true;
				//create another ins's begin
				if (it != (--addr_ins->end())) //not the last ins, so need to begin another new node
						{
					it++;
					temp_name = it->first;
					temp_start = it->first;
					lastfinished = false;
					if (_G_DBG)
						cout << "next ins of monitor " << temp_start
								<< "  create node -> " << i << endl;
				}
				continue;
			}
		}
		if (it == addr_ins->begin()) // the first ins that should be the begin of a block
				{
			temp_name = it->first;
			temp_start = it->first;
			lastfinished = false;
			if (_G_DBG) {
				cout << "first ins " << it->first << "   create node ->" << i
						<< endl;
			}

		} else // not the first ins
		{
			it--;
			ASMInstruction* last_ins = (*it).second;
			it++;
			if ((last_ins->isCondition())
					&& ((!(asm_ins->is_label()))
							|| ((asm_ins->is_label())
									&& (asm_ins->onlyTrylabels())))) //next ins of condition
					{
				if (!lastfinished) //finished the last node
				{
					it--;
					temp_end = it->first;
					i++;
					//cout << "(closure the last node)[ nd->end ]:" << nd->getEnd()<<endl;
					if (_G_DBG) {
						cout << "(closure the last node)[ nd->end ]:"
								<< temp_end << endl;
						cout << "----" << endl;
					}
					_g->addNode(temp_name, temp_start, temp_end);
					lastfinished = true;
					it++;
				}
				if (_G_DBG)
					cout << "next ins of condition : " << it->first
							<< " create node->" << i << endl;
				temp_name = it->first;
				temp_start = it->first;
				lastfinished = false;
			}

			if ((asm_ins->is_label()) && (asm_ins->isJump())) //first jump to the
					{
				if (!asm_ins->onlyTrylabels()) {
					it--;
					temp_end = it->first;
					i++;
					it++;
					if (_G_DBG) {
						cout << "( label and jump ) [ nd->end ] " << temp_end
								<< endl;
						cout << "----" << endl;
					}
					_g->addNode(temp_name, temp_start, temp_end);
					lastfinished = true;
				}
			}
			if (asm_ins->is_label()) //labels of if/switch conditions
			{
				if (_G_DBG)
					cout << "ins label " << asm_ins->getLabel() << endl;
				if (!asm_ins->onlyTrylabels()) {
					if (!lastfinished) //finished the last block
					{
						it--;
						temp_end = it->first;
						i++;
						//cout << "(closure the last node)[ nd->end ]:" << nd->getEnd()<<endl;
						if (_G_DBG) {
							cout << "(closure the last node !)[ nd->end ]:"
									<< temp_end << endl;
							cout << "----" << endl;
						}
						_g->addNode(temp_name, temp_start, temp_end);
						it++;
						lastfinished = true;

					}
					temp_name = it->first;
					temp_start = it->first;
					lastfinished = false;
					if (_G_DBG) {
						cout << "label ins " << it->first << "   create node-> "
								<< i << endl;
					}
				} else {
					if (_G_DBG)
						cout << "try_label " << endl;
				}
			}
			if (asm_ins->isJump()) //set as end
			{
				//assert(!lastfinished);
				if (!lastfinished) {
					temp_end = it->first;
					i++;
					if (_G_DBG) {
						cout << "(closure of the jump) [ nd->end ] " << temp_end
								<< endl;
						cout << "----" << endl;
					}
					_g->addNode(temp_name, temp_start, temp_end);
					lastfinished = true;
				}
			}
		}
	}

    if(!lastfinished)
    {
	temp_end = ((--(addr_ins->end()))->first);
	if(_G_DBG)
	    cout << "closure of the last node [ nd->end ] " << temp_end <<endl;
	i++;
	_g->addNode(temp_name, temp_start, temp_end);
	lastfinished = true;
    }
}

void GraphUtil::createEdges()
{
    map<unsigned int, ASMInstruction*>* addr_ins = _ch->getAdd2InsMap();
    if(_G_DBG)
	cout << "size!!! " << addr_ins->size()<< endl;
    map<unsigned int, ASMInstruction*>::iterator it = addr_ins->begin();
    bool pos_catchall;
    if(_SYNC_BLOCK)
	pos_catchall = false;
    for(; it != addr_ins->end(); it ++)
    {
	unsigned int addr = (*it).first;
	ASMInstruction* asm_ins = (*it).second;
	DexOpcode opcode = asm_ins->getOpcode();
	
	if(_G_DBG)
	{
	    cout << "opcode " << opcode << endl;
	    cout << "addr " << addr << endl;
	    cout << "ins " << asm_ins->toString()<<endl;
	}
	if(_SYNC_BLOCK)
	{
	    if(asm_ins->is_label())
	    {
		if(asm_ins->getLabel().find("catchall") != string::npos)
		    pos_catchall = true;
	    }
	    if(opcode == OP_THROW)
		pos_catchall = false;
	}
	if(_TRY_BLOCK)
	{
	    if(((asm_ins->realTryStart())||(asm_ins->realTryEnd()))&&(!asm_ins->isReturn()))
	    {
		if(it != (--addr_ins->end()))
		{
		    it ++; //next ins
		    ASMInstruction* next_ins = it->second;
		    unsigned int next_addr = it->first;
		    it --;
		    if(!next_ins->onlyCatches())
		    {
			_g->addEdge(it->first, next_addr);
		    }
		}
		continue;
	    }

	}
	switch(opcode)
	{
	    /*
	    if(_SYNC_BLOCK)
	    {
		
		case OP_MONITOR_ENTER:
		case OP_MONITOR_EXIT:
		{
		   if(_G_DBG)
		   {
			cout << "in monitor~ " << endl;
			cout << "pos_catchall " << pos_catchall << endl;
		    }
		    if(!pos_catchall)
		    {
			if(it != (--addr_ins->end()))
			{
			    it ++; //next ins
			    ASMInstruction* next_ins = it->second;
			    unsigned int next_addr = it->first;
			    it --;
			    if(!next_ins->onlyCatches())
			    {
				_g->addEdge(it->first, next_addr);
			    }
			}
		    }
		}
		break;
	    }
	    */
	    case OP_GOTO:
	    case OP_GOTO_16:
	    case OP_GOTO_32:
	    {
		//find self located node
		if(_G_DBG)
		    cout << "in goto " << endl;
		//int tail_idx = locateVexByEndAddr(addr);
		//find the dest node
		string dest  = asm_ins->getOperand1();
		dest = dest.substr(1,dest.size()-1);
		unsigned int dest_addr = atoi(dest.c_str());
		_g->addEdge(addr, dest_addr);
		//int head_idx = locateVex(dest_addr);
		//create edge and related to node
	    }
	    break;
	    case OP_IF_EQ:
	    case OP_IF_NE:
	    case OP_IF_LT:
	    case OP_IF_GE:
	    case OP_IF_GT:
	    case OP_IF_LE:
	    {
		//self located node idx
		if(_G_DBG)
		{
		    cout << "--"<<endl;
		    cout <<"in condition " << endl;
		}
		//int tail_idx = locateVexByEndAddr(addr);
		//dest node idx 1
		it ++;
		unsigned int dest1 = it->first;
		//int head_idx_1 = locateVex(dest1);
		it--;
		_g->addEdge(addr,dest1);
		//dest node idx 2
		string dest = asm_ins->getOperand3();
		dest = dest.substr(1, dest.size()-1);
		unsigned int dest_2 = atoi(dest.c_str());
		if(_G_DBG)
		    cout << "dest_2 :: " << dest_2 <<endl;
		_g->addEdge(addr,dest_2);

	    }
	    break;
	    case OP_IF_EQZ:
	    case OP_IF_NEZ:
	    case OP_IF_LTZ:
	    case OP_IF_GEZ:
	    case OP_IF_GTZ:
	    case OP_IF_LEZ:
	    {
		//self located node idx
		if(_G_DBG)
		{
		    cout << "--"<<endl;
		    cout <<"in condition " << endl;
		}
		//int tail_idx = locateVexByEndAddr(addr);
		//dest node idx 1
		it ++;
		unsigned int dest1 = it->first;
		//int head_idx_1 = locateVex(dest1);
		it--;
		_g->addEdge(addr, dest1);
		
		//dest node idx 2
		string dest = asm_ins->getOperand2();
		dest = dest.substr(1, dest.size()-1);
		unsigned int dest_2 = atoi(dest.c_str());
		if(_G_DBG)
		    cout << "dest_2 :: " << dest_2 <<endl;
		_g->addEdge(addr, dest_2);
		
		
	    }
	    break;
	    case OP_PACKED_SWITCH:
	    case OP_SPARSE_SWITCH:
	    {
		if(_G_DBG)
		{
		    cout << "--"<<endl;
		    cout << "in switch"<< endl;
		}
		assert(asm_ins->hasSwitch());
		Switch* swi = asm_ins->getSwitch();
		vector<unsigned int>* target = swi->getTarget();
		assert(target->size()>0);
		for(unsigned int i =0; i != target->size(); i++)
		{
		    unsigned int head_addr = target->at(i);
		    _g->addEdge(addr,head_addr);
		   
		}
		//default is the next ins
		it ++;
		unsigned int dest_default = it->first;
		//int head_idx_default = locateVex(dest_default);
		it--;
		_g->addEdge(addr,dest_default);
		
	    }
	    break;
	    default: //doesn't belong to jump but whose next is a label
	    {
		if(_G_DBG)
		{
		    cout <<"--" <<endl;
		    cout << "in default " << endl;
		}
		ASMInstruction* cur_ins = it->second;
		unsigned int cur_addr = it->first;
	
		if(++it != addr_ins->end() )
		{    
		    ASMInstruction* next_ins = (it)->second;
		    //DexOpcode next_opc = next_ins->getOpcode();
		    bool tag = false;
		    if((_G_DBG) && (_SYNC_BLOCK))
			cout << "pos_catchall " << pos_catchall << endl;
		    //cout << "next ins " << next_ins->toString()<<endl;
		    if((next_ins->is_label())&&(!cur_ins->isReturn()))
		    {
			if(_G_DBG)
			    cout << "satisfying------------- " << endl;
			string  label_str = next_ins->getLabel();
			//cout << "label " << label_str << endl;
			//if((label_str.substr(0,5)!= "catch")&&(!next_ins->onlyTrylabels()))
			if(!next_ins->onlyTryorCatch())
			{
			    unsigned int next_addr = it->first;
			    _g->addEdge(cur_addr,next_addr);
			    tag = true;
			}

		    }
		    //else if(((next_opc == OP_MONITOR_ENTER)||(next_opc == OP_MONITOR_EXIT))&&(!cur_ins->isReturn())&&(tag == false)&&(!pos_catchall))
		    //{
		//	if(_G_DBG)
		//	    cout << "add edge because of monitor " << endl;
		//	_g->addEdge(cur_addr, it->first);
		 //   }
		    if(((next_ins->realTryStart())||(next_ins->realTryEnd()))&&(!cur_ins->isReturn()) && (tag == false))
		    {
			if(_G_DBG)
			    cout << "add edge because of try block " << endl;
			_g->addEdge(cur_addr, it->first);
		    }

		}
		it--;
	    }
	}
    }
}
/* Graph class*/
//void createGraph(Graph* g, CodeHeader* ch){
void GraphUtil::createGraph(){
	//get the numbers
	if(_G_DBG)
	    cout << "in create nodes =================" << endl;
	createNodes();
	if(_G_DBG)
	    cout << "out create nodes ===================" << endl;
	// now create all the edges
	if(_G_DBG)
	    cout << "in create edges ------------------" << endl;
	createEdges();	
	if(_G_DBG)
	    cout << "out create edges -------------------" << endl;
}

//int findNodeIdx(Graph* g, unsigned int addr)
int GraphUtil::findNodeIdx(unsigned int addr)
{
    vector<Node*>* nodel = _g->getNodeList();
    for(unsigned int i = 0 ; i < nodel->size(); i++)
    {
	if((addr >= nodel->at(i)->getData()->getStart())&&(addr <= nodel->at(i)->getData()->getEnd()))
	    return i;
    }
    return -1;
}


void GraphUtil::findAllPaths(int source, int target)
{
    _g->findAllPaths(source, target);
}
