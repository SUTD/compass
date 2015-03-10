/*
 * POE.cpp
 *
 *  Created on: May 18, 2012
 *      Author: boyang
 */

#include "POE.h"
#include "Abductor.h"
#include "ComputePO.h"
#include <typeinfo>
#include "util.h"
#include "il/binop_expression.h"
#include <map>

#define UNKNOWN_STR "unknown"


POE::POE()
{
	// TODO Auto-generated constructor stub
}

POE::~POE()
{
	// TODO Auto-generated destructor stub
}


POE::POE(Function* f){
	this->entry_func = f;
}


POE::POE(vector<Function *> func_vector, Function *entry_func)
{
	this->func_vector = func_vector;
	this->entry_func = entry_func;
}


POE::POE(const POE& other)
{
	this->entry_func = other.entry_func;
	precondition = other.precondition;
	this->func_vector = other.func_vector;


	map<POEKey*, ProofObligation*>::iterator mapPOEKey_iter;
	map<POEKey*, ProofObligation*> omap_poekey_po = other.map_poekey_po;

	for(mapPOEKey_iter = omap_poekey_po.begin(); mapPOEKey_iter != omap_poekey_po.end(); mapPOEKey_iter++)
	{
		//should be OK
		POEKey* first =  mapPOEKey_iter->first;
		ProofObligation * po_temp = new ProofObligation(*(mapPOEKey_iter->second));
		map_poekey_po.insert(pair<POEKey* , ProofObligation*>(first, po_temp));
	}
}


void POE::setPrecondition(Constraint precondition)
{
	this->precondition = precondition;
}

void POE::print_out_po_info()
{
	cout <<endl << "%%%%%%%%%%%%%%%%%%  print_out  POE::map_block_po info"  << endl;

	map<POEKey *, ProofObligation*>::iterator mapKey_iter;

	for(mapKey_iter = map_poekey_po.begin(); mapKey_iter != map_poekey_po.end(); mapKey_iter++)
	{
		if(mapKey_iter->first->is_superblock())
		{
			cout << "super block!!" << endl;
			cout << "BLOCK INFO "  << mapKey_iter->first->to_string() <<endl;
			cout << "------------------------" << endl;
			cout << "precondition  :"<< mapKey_iter->second->getPrecondition().to_string() << endl;
			cout << "postcondition  :"<< mapKey_iter->second->getPostcondition().to_string() << endl;
			cout << "known_inv (I)  :" << mapKey_iter->second->getKnown_inv().to_string() << endl;
			cout << "inv_to_show (R)  :" << mapKey_iter->second->getInv_to_show().to_string() << endl;
			cout << "While Condition (C)  :" << mapKey_iter->second->getWhile_condition().to_string() << endl;
			cout << "wp_before  :" << mapKey_iter->second->getWp_before() << endl;
			set<Constraint> obs;
			mapKey_iter->second->getObserved(obs);
			if(obs.size() > 0)
			{
				set<Constraint>::iterator iter;
				for(iter = obs.begin(); iter!=obs.end(); iter++)
				{
					cout << "Observed :" << (*iter).to_string() << endl;
				}
			}
		}
		else
		{
			continue;
			cout << "BLOCK INFO " << mapKey_iter->first->to_string() <<endl;
			cout << "------------------------" << endl;
			cout << "precondition  :"<< mapKey_iter->second->getPrecondition().to_string() << endl;
			cout << "postcondition  :"<< mapKey_iter->second->getPostcondition().to_string() << endl;
			cout << "known_inv (I)  :" << mapKey_iter->second->getKnown_inv().to_string() << endl;
			cout << "inv_to_show (R)  :" << mapKey_iter->second->getInv_to_show().to_string() << endl;
			cout << "wp_before  :" << mapKey_iter->second->getWp_before() << endl;
		}

		cout << "#############################################"  << endl;
		//      ---------------------   For test -----------------
		//				set<CfgEdge*> edges = mapBlock_iter->first->get_predecessors();
		//				set<CfgEdge*>::iterator edge_iter;
		//				for(edge_iter = edges.begin(); edge_iter != edges.end(); edge_iter++)
		//				{
		//					Block* b_source = (*edge_iter)->get_source();
		//					cout << "predecessor block : " << b_source->get_block_id();
		//					cout << "   and superblock? " << b_source->is_superblock() << endl;
		//
		//				}
		//      ---------------------  end ----------------------
	}
}


void POE::init_env()
{
	CallingContext * calling_seq = new CallingContext();
	POEKey * entry_key = new POEKey(entry_func->get_entry_block(), calling_seq);
	POEKey * exit_key = new POEKey(entry_func->get_exit_block(), calling_seq);
	ComputePO cpo(entry_key, entry_func, func_vector);  //generate map
	//cpo.set_entry_func(entry_func);     //for top function
	cpo.get_map_poekey_po(this->map_poekey_po);
	cpo.get_init_evironment(entry_key, precondition, exit_key, true,  map_poekey_po);
	eliminate_temp_for_map();
	po_time = cpo.get_time();
	unroll_time = cpo.get_unroll_time();
}

int POE::get_init_po_time()
{
	return po_time;
}


int POE::get_loop_unroll_time()
{
	return unroll_time;
}




POE* POE::updatePre(POEKey* w, Constraint added_pre, LoopTree& lt) //, set<SuperBlock*>& set_sb)
{

	//copy POE to new POE
	POE* new_poe = new POE(*this);
	map<POEKey*, ProofObligation*> map_poekey_po;
	new_poe->get_map_POEKey_po(map_poekey_po);
	ComputePO cpo = ComputePO(this->entry_func, func_vector);
	cpo.updatePre(w, added_pre, map_poekey_po, lt);
	return new_poe;
}




POE* POE::updateInv(POEKey* w, Constraint new_inv)
{
	//cout << "updateInv has been invoked " << endl;
	POE* new_poe = new POE(*this);
	map<POEKey*, ProofObligation*> map_poekey_po;
	new_poe->get_map_POEKey_po(map_poekey_po);

	map<POEKey*, ProofObligation*>::iterator rc_m_iter;
	rc_m_iter = map_poekey_po.find(w);
	rc_m_iter->second->setKnown_inv(rc_m_iter->second->getKnown_inv()& rc_m_iter->second->getInv_to_show());
	//rc_m_iter->second->setKnown_inv(rc_m_iter->second->getKnown_inv()&
	//		(!rc_m_iter->second->getWhile_condition()|rc_m_iter->second->getInv_to_show()));
	rc_m_iter->second->setInv_to_show(new_inv);

	return new_poe;
}



POE* POE::updateInnerPO(POEKey* pk_w, Constraint r, list<POEKey*>& list_loop_id)
{
	POE* new_poe = new POE(*this);
	map<POEKey*, ProofObligation*> map_block_po;
	new_poe->get_map_POEKey_po(map_block_po);

	Block* b = pk_w->getBlock();
	SuperBlock* spb = NULL;
	if(b->is_superblock())
	{
		spb = static_cast<SuperBlock *> (b);

	}else
	{
		assert(false);
	}

	POEKey* pk_entry = new POEKey(spb->get_entry_block(), pk_w->getcallingcontext());
	ComputePO cpo = ComputePO(pk_entry, entry_func, func_vector);
	cpo.sandwitch_loop_sp(pk_w, r);
	cpo.sandwitch_loop_wp(pk_w, r);
	cpo.updateInnerPO(map_block_po, list_loop_id);

	return new_poe;
}




void POE::get_map_POEKey_po(map<POEKey*, ProofObligation*> & map_return)
{
	map_return = this->map_poekey_po;
}


void POE::eliminate_temp_for_map()
{
	map<POEKey*, ProofObligation* >::iterator mapPOEKey_iter;
	for(mapPOEKey_iter = map_poekey_po.begin(); mapPOEKey_iter != map_poekey_po.end(); mapPOEKey_iter++)
	{
		//POEKey* first =  mapPOEKey_iter->first;
		ProofObligation* po_cur = mapPOEKey_iter->second;
		//if(first->is_superblock())
		//{

		//P
		Constraint c = po_cur->getPrecondition();
		eliminate_temp(c);
		set<Term*> all_terms;
		c.get_terms(all_terms, false);

		set<Term*>::iterator t_iter;
		for(t_iter = all_terms.begin(); t_iter!= all_terms.end(); t_iter++)
		{
			if((*t_iter)->type == FUNCTION_TERM){
				FunctionTerm* ft = static_cast<FunctionTerm*>(*t_iter);
				string fun_name = ft->get_name();

				size_t found;
				found = fun_name.find(UNKNOWN_STR);
				if (found!=string::npos)
				{
					Term* term_func = (*t_iter);
					Term* new_term = VariableTerm::make("__" + fun_name);
					c.replace_term(term_func, new_term);
				}
			}
		}
		po_cur->setPrecondition(c);

		//Q
		c = po_cur->getPostcondition();
		eliminate_temp(c);
		po_cur->setPostcondition(c);

		//R
		c = po_cur->getInv_to_show();
		eliminate_temp(c);
		po_cur->setInv_to_show(c);

		//C
		c = po_cur->getWhile_condition();
		eliminate_temp(c);
		po_cur->setWhile_condition(c);

		//I
		c = po_cur->getKnown_inv();
		eliminate_temp(c);
		po_cur->setKnown_inv(c);

		//wp before  unnecessary
		c = po_cur->getWp_before();
		eliminate_temp(c);
		po_cur->setWp_before(c);


	}
}


void POE::eliminate_temp(Constraint & cons)
{
	set<Term*> all_terms;
	cons.get_terms(all_terms, false);
	set<Term*>::iterator iter_terms;
	set<VariableTerm*> eleminate_terms;
	for(iter_terms = all_terms.begin(); iter_terms != all_terms.end(); iter_terms++)
	{
		set<string> string_vars;
		set<string>::iterator iter_string;

		if((*iter_terms)->type == VARIABLE_TERM)
		{
			//Does term have multiple variables?
			(*iter_terms)->get_vars(string_vars);
			for(iter_string = string_vars.begin(); iter_string != string_vars.end(); iter_string++)
			{
				size_t found;

				found = (*iter_string).find("__temp");
				if (found!=string::npos)
				{
					VariableTerm* vt = static_cast<VariableTerm*>(*iter_terms);
					eleminate_terms.insert(vt);
				}
			}
		}
	}

	set<VariableTerm*>::iterator iter_vt;

	//	for(iter_vt = eleminate_terms.begin(); iter_vt != eleminate_terms.end(); iter_vt++)
	//	{
	//		cout << (*iter_vt)->to_string() << endl;
	//	}

	cons.eliminate_evars(eleminate_terms);
}



ProofObligation * POE::find_ProofObligation(POEKey*key){

	map<POEKey *, ProofObligation *>::iterator mapPOEKey_iter;

	for(mapPOEKey_iter = map_poekey_po.begin(); mapPOEKey_iter != map_poekey_po.end(); mapPOEKey_iter++)
	{
		if(*mapPOEKey_iter->first == *key){
			return mapPOEKey_iter->second;
		}
	}
	return NULL;

}

