/*
 * Verify.cpp
 *
 *  Created on: May 23, 2012
 *      Author: boyang
 */

#include "Verify.h"
#include "ComputePO.h"
#include <dirent.h>
#include <string.h>
#include <fstream>


#define DEBUG false
#define UNKNOWN_STR "unknown"
#define OBSERVED "observed"
#define ASK_USER false
#define SHOW_DETAILS false
#define CHECKING true

//for intraprocedual only
#define CONST_RENAMING true

int Verify::num_tries;

Verify::Verify(char* folder_name, char* entry_name){
	num_tries = 0;

	long size;
	char *the_path = NULL;
	char *buf;

	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL)
		the_path = getcwd(buf, (size_t)size);

	strcat(the_path, "/");
	strcat(the_path, folder_name);
	strcat(the_path, "/");



	char the_path_abs[256];
	the_path_abs[0]=0;
	strcat(the_path_abs, folder_name);
	strcat(the_path_abs, "/");


	char * char_path_rel = the_path;
	char * char_path_abs = the_path_abs;
	char * char_path;

	//cout << "char_path_rel  : " << char_path_rel << endl;
	//cout << "char_path_abs  : " << char_path_abs << endl;

	struct dirent *pent;
	if(DirectoryExists(char_path_rel))
	{
		char_path = char_path_rel;
	} else if (DirectoryExists(char_path_abs))
	{
		char_path = char_path_abs;
	} else
	{
		cout << "[ERROR]Invalid path !!!" << endl;
		exit(EXIT_FAILURE);
	}

	bool main_has_found = false;

	DIR *dir = opendir (char_path);
	while((pent = readdir(dir))!= NULL)
	{
		if(strcmp(pent->d_name, ".")==0 || strcmp(pent->d_name, "..")==0)
		{
			continue;
		}

		/*
		 * ignore files other than .sail file
		 */
		char * pch;
		pch = strstr (pent->d_name,".sail");
		if (!pch)
		{
			//cout << "it's null : " <<   pent->d_name << endl;
			continue;
		}

		char temp_file[256];
		strcpy(temp_file,  char_path);
		sail::Function* temp_f = sail::Serializer::load_file(strcat(temp_file, pent->d_name));
		func_vector.push_back(temp_f);
		function_declaration* declare = (temp_f)->get_original_declaration();
		if(declare == NULL)
			continue;
		string name_declare =  declare->get_name();
		//cout << "name declare : "<< name_declare << endl;
		if(strcmp(name_declare.c_str(), entry_name) == 0)
		{
			main_has_found = true;
			this->entry_func = temp_f;
		}
		//cout << temp_f->to_string() << endl;
	}

	if(!main_has_found)
	{
		cout << "[ERROR] The function " << entry_name
				<< "'s sail has not been found!" << endl;
		exit(EXIT_FAILURE);
	}


	if(CONST_RENAMING){
		this->entry_cr = new ConstRenaming(entry_func);
	}


	this->original_poe = new POE(func_vector, entry_func);
	this->original_poe->setPrecondition(true);
	this->original_poe->init_env();


	//this->original_poe->print_out_po_info();
	computeLT(entry_func->get_entry_block());
	this->level = 1;


}


bool Verify::DirectoryExists( const char* pzPath)
{
	if ( pzPath == NULL) return false;

	DIR *pDir;
	bool bExists = false;

	pDir = opendir (pzPath);

	if (pDir != NULL)
	{
		bExists = true;
		(void) closedir (pDir);
	}

	return bExists;
}



Verify::Verify(Function* func, POE* original_poe, vector<Function *> func_vector,
		int level, LoopTree& loopTree, ConstRenaming*  entry_cr)
{

	this->entry_func = func;
	this->original_poe = original_poe;
	this->func_vector = func_vector;
	this->level = level;
	this->loopTree = loopTree;
	this->entry_cr = entry_cr;
}


Verify::~Verify() {
	// TODO Auto-generated destructor stub
}

int Verify::get_num_tries()
{
	return this->num_tries;
}

int Verify::get_init_po_time()
{
	return this->original_poe->get_init_po_time();
}


int Verify::get_loop_unroll_time()
{
	return this->original_poe->get_loop_unroll_time();
}


bool Verify::do_verification(list<POEKey*>& list_loop_id,
		map<POEKey* , map<int, Constraint*>* > & invariant_map)
{


	if(level>8) return false;

	num_tries++;


	if(ASK_USER||SHOW_DETAILS)
	{
		cout << "\n\n********************************************************"
				<< "******************************" << endl;
		cout << "*************************  verification level " << this->level
				<<  " **************************************" << endl;
	}

	if(ASK_USER||SHOW_DETAILS)
	{
		cout << "********************************************************"
				<< "******************************" << endl;
	}

	if(ASK_USER)
	{
		list<POEKey*>::iterator list_riter = list_loop_id.begin();
		for(; list_riter != list_loop_id.end(); list_riter++)
		{
			cout << "verification while loop list :" << (*list_riter)->to_string() << endl;
		}

		cout << "\n"<<  endl;
	}

	Constraint t(true);
	map<POEKey*, ProofObligation*> map_POEKey_po;
	original_poe->get_map_POEKey_po(map_POEKey_po);

	if(DEBUG)
	{
		//this->original_poe->print_out_po_info();
	}

	/*
	 * check if wp is not valid on function entry
	 */
	CallingContext * cc = new CallingContext();
	POEKey* poekey_entry = new POEKey(entry_func->get_entry_block(), cc);
	ProofObligation* entry_PO = original_poe->find_ProofObligation(poekey_entry);

	Constraint wp_entry = entry_PO->getWp_before();
	//if(!wp_entry.valid()){
	if(!valid(wp_entry)){
		if(SHOW_DETAILS)
		{
			cout << "!wp_entry.valid()  " << endl;
			cout << "WP: " << wp_entry << endl;
		}
		return false;
	}


	list<POEKey*>::iterator list_iter = list_loop_id.begin();
	for(; list_iter != list_loop_id.end(); list_iter++)
	{
		map<POEKey*, ProofObligation*>::reverse_iterator mapKey_riter = map_POEKey_po.rbegin();
		for(; mapKey_riter != map_POEKey_po.rend(); mapKey_riter++)
		{
			if(mapKey_riter->first->is_superblock() && *mapKey_riter->first == *(*list_iter))
			{
				ProofObligation * po_super_cur = mapKey_riter->second;
				Constraint p = po_super_cur->getPrecondition();
				Constraint q = po_super_cur->getPostcondition();
				Constraint i = po_super_cur->getKnown_inv();
				Constraint r = po_super_cur->getInv_to_show();
				Constraint c = po_super_cur->getWhile_condition();
				Constraint t(true);

				if(ASK_USER||SHOW_DETAILS)
				{
					cout << "Block ID " << mapKey_riter->first->to_string();
					cout << "\nCalling Level " << this->level << endl;
					cout << "P:  " << p << "\nQ:  " << q
							<< "\nI:  " << i << "\nR:  " << r<< endl;
					cout << "C:  " << c << endl;
				}

				set<Constraint> obs;
				po_super_cur->getObserved(obs);
				if(obs.size() > 0)
				{
					set<Constraint>::iterator iter;
					for(iter = obs.begin(); iter!=obs.end(); iter++)
					{
						if(ASK_USER)
						{
							cout << "Observed: " << (*iter).to_string() << endl;
						}
					}
				}
				//cout << "============compute  c_wp_sr start ==============" << endl;
				Constraint c_wp_sr = ComputePO::compute_superBlock_wp(mapKey_riter->first,
						r, func_vector, this->entry_func, true);

				//cout << "============compute  c_wp_sr start ==============" << endl;
				//cout << "c_wp_sr : " <<  c_wp_sr << endl;

				if(ASK_USER||SHOW_DETAILS)
				{
					cout << "c_wp_sr :" << c_wp_sr <<"\n"<< endl;
				}

				//case 1
				//if(!(p&i).implies(r))
				if(!implies(p&i, r))
				{
					if(ASK_USER||SHOW_DETAILS)
					{
						cout << "Invoke Case 1 ";
						cout << mapKey_riter->first->to_string()<< endl;
					}
					//cout << "Calling Level " << this->level << endl;

					if(!strengthen_pre(mapKey_riter->first, po_super_cur, invariant_map))
					{
						if(ASK_USER||SHOW_DETAILS)
						{
							cout << "************************* end verification level"
									<< this->level  <<  " *************"
									<<"**********************\n\n" << endl;
						}
						return false;
					}
				} else
				{
					if(ASK_USER||SHOW_DETAILS)
					{
						cout << "case 1 passed!";
						cout << mapKey_riter->first->to_string()<< endl;
					}
					//cout << "Calling Level " << this->level<<"\n" << endl;
				}

				//case 2
				//if(!((i&r&c).implies(c_wp_sr)))
				if(!implies((i&r&c), c_wp_sr))
				{
					if(ASK_USER||SHOW_DETAILS)
					{
						cout << "Invoke Case 2 ";
						cout <<  mapKey_riter->first->to_string() << endl;
					}
					//cout << "Calling Level " << this->level<< endl;
					if(!show_inv(mapKey_riter->first, po_super_cur, c_wp_sr, invariant_map))
					{
						if(ASK_USER||SHOW_DETAILS)
						{
							cout << "************************* end verification level"
									<< this->level  <<  " *************"
									<<"**********************\n\n" << endl;
						}
						return false;
					}
				}else
				{
					if(ASK_USER||SHOW_DETAILS)
					{
						cout << "case 2 passed!";
						cout << mapKey_riter->first->to_string()<< endl;
					}
					//cout << "Calling Level " << this->level<<"\n" << endl;
				}


				//case 3
				//if(!((i&r&!c).implies(q)))
				if(!implies((i&r&!c), q))
				{
					if(ASK_USER||SHOW_DETAILS)
					{
						cout << "Invoke Case 3 ";
						cout << mapKey_riter->first->to_string()<< endl;
					}
					//cout << "Calling Level " << this->level << endl;
					if(!strengthen_inv(mapKey_riter->first, po_super_cur, invariant_map))
					{
						if(ASK_USER||SHOW_DETAILS)
						{
							cout << "************************* end verification level"
									<< this->level  <<  " *************"
									<<"**********************\n\n" << endl;
						}
						return false;
					}
				} else
				{
					if(ASK_USER||SHOW_DETAILS)
					{
						cout << "case 3 passed!";
						cout << mapKey_riter->first->to_string() << endl;
					}
					//cout << "Calling Level " << this->level<<"\n" << endl;
				}
				if(ASK_USER||SHOW_DETAILS)
				{
					cout << "\n" << endl;
				}
			}
		}
	}


	map<POEKey* , map<int, Constraint*>* >::iterator map_iter = invariant_map.begin();
	for(; map_iter != invariant_map.end(); map_iter++)
	{
		map<POEKey *, ProofObligation *>::reverse_iterator mapKey_riter;
		for(mapKey_riter = map_POEKey_po.rbegin();
				mapKey_riter != map_POEKey_po.rend(); mapKey_riter++)
		{
			if(!mapKey_riter->first->is_superblock())
			{
				continue;
			}
			list<POEKey*>::iterator list_riter = list_loop_id.begin();
			for(; list_riter != list_loop_id.end(); list_riter++)
			{
				if(!(*(*list_riter)== *((*map_iter).first)))
				{
					continue;
				}

				if(*mapKey_riter->first == *((*map_iter).first))
				{
					ProofObligation * po_super_cur = mapKey_riter->second;
					Constraint i = po_super_cur->getKnown_inv();
					Constraint r = po_super_cur->getInv_to_show();
					//Constraint w = po_super_cur->getWhile_condition();
					//cout << "abd r : " << r.to_string() << endl;
					Constraint invariant = i&r;

					if(valid(invariant))
						continue;
					//cout << "invariant  : " << invariant << endl;
					//Constraint c =  (*((*map_iter).second))&invariant;
					Constraint c =  invariant;
					map<int, Constraint*>::iterator sec = (*map_iter).second->begin();
					if(level >= sec->first)
					{
						map<int, Constraint*>* temp_map = new map<int, Constraint*> ();
						temp_map->insert(pair<int , Constraint*>(level , new Constraint(c)));
						(*map_iter).second = temp_map;
					}
					else
					{
						continue;
					}
					//(*map_iter).second = new Constraint(c);
					//				cout << "Block ID :" << mapKey_riter->first->to_string();
					//				cout << "\n[<< loop invariant >>] in verification level " << this->level<< " : " << invariant << endl;
					//				cout << "\n"<< endl;
				}
			}
		}
	}
	if(ASK_USER||SHOW_DETAILS)
	{
		cout << "************************* end verification level"
				<< this->level  <<  " *************"
				<<"**********************\n\n" << endl;
	} else
	{
		//		cout << "************************* end verification level"
		//				<< this->level  <<  " *************"
		//				<<"**********************\n\n" << endl;
	}
	if(this->level == 1 && CONST_RENAMING == true)
	{
		cout << "===== constant renaming map  ===== " << endl;
		entry_cr->print_map();
		cout << "\n\n" << endl;
	}
	return true;
}


bool Verify::strengthen_pre(POEKey* while_poekey, ProofObligation* po_while,
		map<POEKey* , map<int, Constraint*>* > & invariant_map)
{
	Constraint p = po_while->getPrecondition();
	Constraint i = po_while->getKnown_inv();
	Constraint r = po_while->getInv_to_show();
	set<Constraint> set_observed;
	po_while->getObserved(set_observed);


	Constraint psi;
	set<Constraint> theta;
	//add dynamic information
	set<Constraint>::iterator obs_iter;
	for(obs_iter=set_observed.begin(); obs_iter!= set_observed.end(); obs_iter++)
	{
		theta.insert(*obs_iter);
	}
	theta.insert(p&i);
	if(CONST_RENAMING == true)
	{
		theta.insert(entry_cr->get_knowledge());
	}

	while(true)
	{
		psi = abduce(p&i, r, theta);


		if(ASK_USER||SHOW_DETAILS)
		{
			cout << "psi :" << psi  << endl;
		}

		//if(psi.unsat()){
		if(unsat(psi)){
			return false;
		}


		POE* newPOE = this->original_poe->updatePre(while_poekey, psi, this->loopTree);

		list<POEKey*> list_loop_id;
		get_previous_sb_id(while_poekey, list_loop_id);


		Verify new_verify(this->entry_func, newPOE, func_vector, this->level+1,
				this->loopTree, entry_cr);
		if(new_verify.do_verification(list_loop_id, invariant_map))
		{
			return true;
		}
		theta.insert((!psi)&p&i);
	}
}


Constraint Verify::get_unrolled_psi(Constraint psi,
		const set<map<VariableTerm*, VariableTerm*> >& renamings)
{
	Constraint res(true);
	for(auto it = renamings.begin(); it!= renamings.end(); it++)
	{
		const map<VariableTerm*, VariableTerm*> & cur_map = *it;
		Constraint cur_psi = psi;
		for(auto it = cur_map.begin(); it!= cur_map.end(); it++) {
			VariableTerm* key = it->first;
			VariableTerm* val = it->second;

			cur_psi.replace_term(key, val);

		}
		res &= cur_psi;
	}

	return res;
}

bool Verify::show_inv(POEKey* w_poekey, ProofObligation* po_while,
		Constraint body_wp, map<POEKey* , map<int, Constraint*>* > & invariant_map)
{
	Block* w_b = w_poekey->getBlock();
	SuperBlock* w_sb = static_cast<SuperBlock *> (w_b);
	Constraint p = po_while->getPrecondition();
	Constraint i = po_while->getKnown_inv();
	Constraint r = po_while->getInv_to_show();
	Constraint q = po_while->getPostcondition();
	Constraint c = po_while->getWhile_condition();
	set<Constraint> set_observed;
	po_while->getObserved(set_observed);

	Constraint unroll_c = po_while->get_unrolled_sp();
	const set<map<VariableTerm*, VariableTerm*> >& renamings =
			po_while->get_unroll_renamings();

	//cout << "UNROLL CONSTRAINT: " << po_while->get_unrolled_sp()  << endl;

	Constraint t(true);

	Constraint psi;
	set<Constraint> theta;

	if(ASK_USER)
	{
		if(query_client(w_sb, po_while))
		{
			return true;
		}
	}

	//add dynamic info
	set<Constraint>::iterator obs_iter;
	for(obs_iter=set_observed.begin(); obs_iter!= set_observed.end(); obs_iter++)
	{
		theta.insert(*obs_iter);
	}


	theta.insert(p);
	theta.insert(q);
	theta.insert(r&i&c);
	theta.insert(!r);
	if(CONST_RENAMING == true)
	{
		theta.insert(entry_cr->get_knowledge());
	}

	while(true)
	{

		psi = abduce(r&i&c, body_wp, theta);


		if(ASK_USER||SHOW_DETAILS)
		{
			cout << "psi :" << psi  << endl;
		}

		//if(psi.unsat()){
		if(unsat(psi)){
			return false;
		}


		Constraint unrolled_psi = get_unrolled_psi(i&psi, renamings);
		Constraint test_c = unrolled_psi & unroll_c;
		if(test_c.unsat_discard()) {
			theta.insert((!psi)&i);
			//assert(false);
			continue;
		}



		list<POEKey*> list_inner;
		this->loopTree.getInnerLoopList(w_poekey, list_inner);

		POE* newPOE;
		if(list_inner.size() > 0)
		{
			POE* tempPOE = original_poe->updateInnerPO(w_poekey, psi, list_inner);
			newPOE = tempPOE->updateInv(w_poekey, psi);
		}
		else
		{
			newPOE = original_poe->updateInv(w_poekey, psi);
		}


		//updated loop id + looptree. getInnerLoopList()
		list<POEKey*> list_loop_id;
		list_loop_id.push_front(w_poekey);
		list_loop_id.splice(list_loop_id.end(), list_inner);

		if(DEBUG)
		{
			list<POEKey*>::iterator list_iter = list_loop_id.begin();
			cout << "strengthen_inv list  : " << endl;
			for(; list_iter != list_loop_id.end(); list_iter++)
			{
				cout << (*list_iter)->to_string() << endl;
			}
		}


		Verify* new_verify = new Verify(this->entry_func, newPOE, func_vector,
				this->level+1, this->loopTree, entry_cr );
		if(new_verify->do_verification(list_loop_id, invariant_map))
		{
			return true;
		}
		//break;
		theta.insert((!psi)&i);
	}
}


bool Verify::strengthen_inv(POEKey * w_poekey, ProofObligation* po_while,
		map<POEKey* , map<int, Constraint*>* > & invariant_map)
{

	Constraint p = po_while->getPrecondition();
	Constraint q = po_while->getPostcondition();
	Constraint i = po_while->getKnown_inv();
	Constraint r = po_while->getInv_to_show();
	set<Constraint> set_observed;
	po_while->getObserved(set_observed);


	Constraint psi;
	set<Constraint> theta;
	Constraint c = po_while->getWhile_condition();
	Constraint t(true);

	//add dynamic information
	set<Constraint>::iterator obs_iter;
	for(obs_iter=set_observed.begin(); obs_iter!= set_observed.end(); obs_iter++)
	{
		theta.insert(*obs_iter);
	}
	theta.insert(p);
	theta.insert(r&i&!c);
	if(CONST_RENAMING == true)
	{
		theta.insert(entry_cr->get_knowledge());
	}

	while(true)
	{
		psi = abduce(r&i&!c, q, theta);
		//psi = (c | q);
		if(ASK_USER||SHOW_DETAILS)
		{
			cout << "psi :" << psi  << endl;
		}

		//if(psi.unsat()){
		if(unsat(psi)){
			return false;
		}


		list<POEKey*> list_inner;
		this->loopTree.getInnerLoopList(w_poekey, list_inner);

		POE* newPOE;
		if(list_inner.size() > 0)
		{
			POE* tempPOE = original_poe->updateInnerPO(w_poekey, psi, list_inner);
			newPOE = tempPOE->updateInv(w_poekey, psi);
		}
		else
		{
			newPOE = original_poe->updateInv(w_poekey, psi);
		}

		//updated loop id + looptree. getInnerLoopList()
		list<POEKey*> list_loop_id;
		list_loop_id.push_front(w_poekey);
		list_loop_id.splice(list_loop_id.end(), list_inner);

		if(DEBUG)
		{
			list<POEKey*>::iterator list_iter = list_loop_id.begin();
			cout << "strengthen_inv list  : " << endl;
			for(; list_iter != list_loop_id.end(); list_iter++)
			{
				cout << (*list_iter)->to_string() << endl;
			}
		}


		Verify* new_verify = new Verify(this->entry_func, newPOE, func_vector,
				this->level+1, this->loopTree, entry_cr );
		if(new_verify->do_verification(list_loop_id, invariant_map))
		{
			return true;
		}
		Constraint block_c = (!psi)&i;
		//cout << "adding to theta: " << block_c << endl;
		theta.insert(block_c);
	}
}


Constraint Verify::abduce(Constraint f, Constraint phi, set<Constraint>& theta)
{

	//int t1 = clock();

	Constraint cons_abduce = !f|phi;
	if(DEBUG)
	{
		cout << "f : " << f.to_string() << endl;
		cout << "phi : " << phi.to_string() << endl;
		cout << "cons_abduce : "  <<cons_abduce.to_string() << endl;
	}


	set<Term*> all_terms;
	cons_abduce.get_terms(all_terms, false);

	set<Term*>::iterator t_iter;
	map<VariableTerm*, int> costs;
	set<VariableTerm*> abd_vterms_not_msa;


	/*
	 * set costs
	 */
	for(t_iter = all_terms.begin(); t_iter!= all_terms.end(); t_iter++)
	{
		if((*t_iter)->type == VARIABLE_TERM){
			VariableTerm* vt = static_cast<VariableTerm*>(*t_iter);
			string vt_name = vt->get_name();
			size_t found;
			found = vt_name.find("__" UNKNOWN_STR);
			if (found!=string::npos)
			{
				costs.insert(pair<VariableTerm*, int>(vt, 100));
			}
			else
			{
				size_t found2;
				found2 = vt_name.find("$cn");
				if(found2!=string::npos)
					costs.insert(pair<VariableTerm*, int>(vt, 1));
				else
					costs.insert(pair<VariableTerm*, int>(vt, 2));
			}
		}
	}


	/*
	 * compute msa
	 */
	set<VariableTerm*> abd_msa;
	cons_abduce.msa(abd_msa, theta, costs);


	if(DEBUG)
	{
		cout <<  "############### abduce start#############" << endl;
		set<Constraint>::iterator iter_theta;
		for(iter_theta = theta.begin(); iter_theta != theta.end();   iter_theta++)
		{
			cout << "theta  :" << (*iter_theta).to_string()  << endl;
		}
		set<VariableTerm*>::iterator iter_abd_msa = abd_msa.begin();
		for(; iter_abd_msa != abd_msa.end();   iter_abd_msa++)
		{
			string vt_name = (*iter_abd_msa)->get_name();
			cout << "abd_msa  :" << (*iter_abd_msa)->to_string()  << endl;
		}
		cout <<  "############### abduce end #############" << endl;
	}

	set<VariableTerm*>::iterator iter_abd_msa = abd_msa.begin();
	for(; iter_abd_msa != abd_msa.end();   iter_abd_msa++)
	{
		string vt_name = (*iter_abd_msa)->get_name();
		size_t found = vt_name.find("__" UNKNOWN_STR);
		if (found!=string::npos)
		{
			//if unknown exists
			return false;
		}
	}



	set<Term*> abd_terms;
	cons_abduce.get_terms(abd_terms, false);

	/*
	 * find \barV
	 */
	for(t_iter = abd_terms.begin(); t_iter!= abd_terms.end(); t_iter++)
	{
		if((*t_iter)->type == VARIABLE_TERM){
			VariableTerm* vt = static_cast<VariableTerm*>(*t_iter);
			if(abd_msa.find(vt) == abd_msa.end())
			{
				abd_vterms_not_msa.insert(vt);
			}
		}
	}

	if(DEBUG)
	{
		set<VariableTerm*>::iterator iter_vt = abd_vterms_not_msa.begin();
		for(;iter_vt != abd_vterms_not_msa.end(); iter_vt++)
		{
			cout << "abduce eliminate_evars:   " << (*iter_vt)->to_string() << endl;
		}
		cout << "before elimination  :" << cons_abduce.to_string()   << endl;
	}
	cons_abduce.eliminate_uvars(abd_vterms_not_msa);

	if(DEBUG)
	{
		cout << "after elimination  :" << cons_abduce.to_string()   << endl;
	}

	/*
	 * checking
	 */
	if(CHECKING){
		if(!((f & cons_abduce).implies(phi)))
			//if(!implies((f & cons_abduce), phi))
		{
			cout << "Wrong abductive solution!" << endl;
			cout << "LHS: " << (f&cons_abduce) << endl;
			cout << "RHS: " << phi << endl;
			assert(false);
		}
		set<Constraint>::iterator iter_theta = theta.begin();
		for(; iter_theta != theta.end();   iter_theta++)
		{
			if(!cons_abduce.unsat_discard() && ((*iter_theta)&cons_abduce).unsat_discard())
			{
				assert(false);
			}
		}
	}
	//double abd_time = to_time(clock()-t1);
	//record abduce information
//	ofstream myfile;
//	myfile.open ("abduceInfo.txt", fstream::in | fstream::out | fstream::app);
//	if (myfile.is_open())
//	{
//		//a) Size of I
//		//b) Size of phi
//		//c) Size of abductive solutioon
//		//d) Time
//		myfile << f.nc_size() << ",\t";
//		myfile << phi.nc_size() << ",\t";
//		myfile << cons_abduce.nc_size() << ",\t";
//		myfile << abd_time << "\t" << endl;
//		myfile.close();
//	}
	return cons_abduce;
}


void Verify::print_out_block(Block * b_p)
{
	cout << "block number : " << b_p->get_block_id() << endl;
	if(b_p->is_basicblock())
	{
		BasicBlock* bb = static_cast<BasicBlock *>(b_p);
		vector<Instruction*> statements = bb->get_statements();

		vector<Instruction*>::iterator iter_inst = statements.begin();
		for (; iter_inst != statements.end(); iter_inst++) {
			cout << (*iter_inst)->line << ": " << (*iter_inst)->to_string()<< endl;
		}
	}
}


void Verify::get_previous_sb_id(POEKey* sb_bottom, list<POEKey*>& list_sb_id)
{
	list<POEKey*>::iterator list_sb_iter;
	queue<POEKey*> q_poeKey;
	q_poeKey.push(sb_bottom);
	while(!q_poeKey.empty()){
		POEKey* pk_cur = q_poeKey.front();
		q_poeKey.pop();

		if(pk_cur->is_superblock())
		{
			//SuperBlock* spb_cur = static_cast<SuperBlock *> (b_cur);
			bool find_b = false;

			for(list_sb_iter = list_sb_id.begin();
					list_sb_iter != list_sb_id.end(); list_sb_iter++)
			{
				if(*(*list_sb_iter) == *pk_cur)
				{
					find_b = true;
					break;
				}
			}

			if(!find_b && !(*pk_cur == *sb_bottom))
			{
				list_sb_id.push_front(pk_cur);
			}

			//continue if the current block is not the bottom block
			if(!(*pk_cur == *sb_bottom))
			{
				//list_sb_id.push_front(sb_bottom);
				continue;
			}
		}


		set<CfgEdge*> set_Edges = pk_cur->get_predecessors();
		set <CfgEdge*>::iterator iter_Edges = set_Edges.begin();

		for (; iter_Edges!=set_Edges.end(); iter_Edges++)
		{
			Block * b_next = (*iter_Edges)->get_source();
			q_poeKey.push(new POEKey(b_next, pk_cur->getcallingcontext()));
		}


		/*
		 * over outer loop
		 */
		Function* func_caller  = this->entry_func;
		//Function* func_callee;
		POEKey* pk_bb = NULL;
		if(q_poeKey.empty())
		{
			//-----------------------------------
			/*
			 * Interprocedural analysis update
			 */
			CallingContext* cur_cc = pk_cur->getcallingcontext();
			if(set_Edges.empty() && cur_cc->get_context_size() != 0)
			{
				/*
				 * find the caller
				 */
				string cur_name = cur_cc->get_curfunc_name();
				int cur_line = cur_cc->get_back_line_num();
				// caller's cc
				CallingContext* copy_cc = new CallingContext(*cur_cc);
				copy_cc->pop_back_context();
				map<POEKey*, ProofObligation*>::iterator map_key_iter;
				map<POEKey*, ProofObligation*> map_POEKey_po;
				original_poe->get_map_POEKey_po(map_POEKey_po);
				for(map_key_iter = map_POEKey_po.begin(); map_key_iter != map_POEKey_po.end(); map_key_iter++)
				{
					Block* b = (*map_key_iter).first->getBlock();
					CallingContext* cc = (*map_key_iter).first->getcallingcontext();
					if(!(*cc == *copy_cc))
					{
						continue;
					}
					if(b->is_basicblock())
					{
						BasicBlock *bb_cur = static_cast<BasicBlock *> (b);
						vector<Instruction*> s = bb_cur->get_statements();
						vector<Instruction*>::iterator inst_iter;
						for (inst_iter = s.begin(); inst_iter != s.end(); ++inst_iter) {
							if((*inst_iter)->get_instruction_id() == FUNCTION_CALL)
							{
								FunctionCall* functionCall = static_cast<sail::FunctionCall*> (*inst_iter);
								string name = functionCall->get_function_name();
								if(functionCall->line == cur_line && strcmp(name.c_str(), cur_name.c_str()) == 0)
								{
									string callee_func_name = cur_cc->get_curfunc_name();
									//func_callee = function_finder(callee_func_name);
									string func_caller_name; //= function_finder(copy_cc->get_caller_name());

									if(copy_cc->get_context_size() > 0)
									{
										func_caller_name = copy_cc->get_caller_name();
										func_caller = function_finder(func_caller_name);

									}else
									{
										//function_declaration* declare_caller =  org_func->get_original_declaration();
										//func_caller_name = declare_caller->get_name();
										func_caller = this->entry_func;
									}

									pk_bb = new POEKey(bb_cur, copy_cc);
									//cout <<"bb_cur  :" <<  bb_cur->get_block_id() <<endl;
									//cout << "pk_bb 1 : " << pk_bb->to_string();
								}
							}
						}
					}
				}
			}

			CallingContext * cc_program_entry = new CallingContext();
			POEKey* pk_program_entry = 	 new POEKey(entry_func->get_entry_block(), cc_program_entry);


			POEKey* pk_function_entry =  new POEKey(func_caller->get_entry_block(), cur_cc);



			//			cout << "\npk_program_entry  : " << pk_program_entry->to_string();
			//			cout << "\npk_function_entry  : " << pk_function_entry->to_string();
			//			cout << "\npk_cur : " << pk_cur->to_string();
			//			cout << "\npk_bb : " << pk_bb->to_string();


			if(*pk_cur == *pk_program_entry)
			{
				continue;
			}
			else if(*pk_cur == *pk_function_entry)
			{
				assert(pk_bb != NULL);
				q_poeKey.push(pk_bb);
			}
			else
			{
				//outter loop
				BasicBlock *bb_func_entry = static_cast<BasicBlock *> (pk_function_entry->getBlock());
				SuperBlock* sb_p = find_superblock(pk_cur->get_block_id() ,bb_func_entry);
				POEKey* pk_outter_sb = new POEKey(sb_p, pk_function_entry->getcallingcontext());
				/*
				 * outer while loop id
				 */
				list_sb_id.push_front(pk_outter_sb);
				list<POEKey*> list_inner;
				this->loopTree.getInnerLoopList(pk_outter_sb, list_inner);
				list_sb_id.splice(list_sb_id.end(), list_inner);
				//				set <CfgEdge*> set_Edges = sb_p->get_predecessors();
				//				set <CfgEdge*>::iterator iter_Edges;
				//				for (iter_Edges=set_Edges.begin(); iter_Edges!=set_Edges.end(); iter_Edges++)
				//				{
				//					Block * b_next =  (*iter_Edges)->get_source();
				//					q_b.push(b_next);
				//				}
			}
		}
	}
}




SuperBlock* Verify::find_superblock(int block_id , BasicBlock* func_entry)
{
	set<sail::Block*, sail::CompareBlock> f_worklist;

	//generate map
	f_worklist.insert(func_entry);

	while(f_worklist.size() > 0){
		sail::Block* b_cur = *f_worklist.begin();
		f_worklist.erase(b_cur);

		if(b_cur->is_basicblock()){
			// do nothing
		}else{
			SuperBlock* spb_cur = static_cast<SuperBlock *> (b_cur);
			if(spb_cur->get_block_id() == block_id)
			{
				return spb_cur;
			}
			BasicBlock* bb_sb_entry = spb_cur->get_entry_block();
			//Add entry of nested while loop into the worklist
			f_worklist.insert(bb_sb_entry);
		}

		set<CfgEdge*>& set_Edges = b_cur->get_successors();
		set <CfgEdge*>::iterator iter_Edges;

		for (iter_Edges=set_Edges.begin(); iter_Edges!=set_Edges.end(); iter_Edges++)
		{
			f_worklist.insert((*iter_Edges)->get_target());
		}
	}
	assert(false);
	return NULL;
}




bool Verify::query_client(SuperBlock* b_while, ProofObligation* po_while)
{
	Constraint p = po_while->getPrecondition();
	//Constraint q = po_while->getPostcondition();
	Constraint i = po_while->getKnown_inv();
	Constraint r = po_while->getInv_to_show();
	Constraint c = po_while->getWhile_condition();

	cout << "++++++++++++++++++ query client Start ++++++++++++++++++" <<endl;
	cout << "assume(out of loop) p&r&i :  " << (p&r&i).to_string() << endl;
	cout << "assume(in the loop) i :  " << i.to_string() << endl;
	cout << "while condition :" << c.to_string() << endl;

	set <Block*> set_b = b_while->get_body();
	set <Block*>::iterator iter_set_b;
	for(iter_set_b = set_b.begin(); iter_set_b != set_b.end(); iter_set_b++)
	{
		if (b_while->get_exit_block()->get_block_id() == (*iter_set_b)->get_block_id()
				|| b_while->get_entry_block()->get_block_id() == (*iter_set_b)->get_block_id())
		{
			continue;
		}
		print_out_block(*iter_set_b);
	}

	cout << "assert : " << r << endl;
	cout << "++++++++++++++++++ query client End ++++++++++++++++++" <<endl;
	cout << "can be verified? yes(1) or no(0) :" ;
	int proved;
	cin >> proved;
	if(proved == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void Verify::getLoopTree(LoopTree& reLT)
{
	reLT = this->loopTree;
}


void Verify::computeLT(BasicBlock * b_entry)
{
	set<sail::Block*, sail::CompareBlock> worklist;
	worklist.insert(b_entry);


	stack <POEKey*> stack_loop_l;
	stack <POEKey*> stack_loop_r;

	queue <Function*> q_func;
	queue <Function*> q_callingfunc;
	queue <int> q_line;
	queue <POEKey*> q_outter_POEKey;
	queue <CallingContext*> q_cur_cc;
	//queue <POEKey*> q_cur_POEKey;


	Block* cur_outter_Loop = b_entry;
	CallingContext* outter_cc = new CallingContext();
	CallingContext* cur_cc = new CallingContext();
	POEKey* cur_outter_POEKey = new POEKey(cur_outter_Loop, outter_cc);

	POEKey* curRight = NULL;
	Function* cur_func = this->entry_func;

	do{
		while(worklist.size() > 0){
			sail::Block* b_cur = *worklist.begin();
			worklist.erase(b_cur);
			POEKey* cur_POEKey = new POEKey(b_cur, cur_cc);

			//			cout << "cur_POEKey  :  " << cur_POEKey->to_string() << endl;
			//			cout << "cur_outter_POEKey :" << cur_outter_POEKey->to_string() << endl;
			//			cout << "********************************************" << endl;

			if(curRight != NULL && *cur_POEKey == *curRight)
			{
				cur_outter_POEKey = stack_loop_l.top();
				stack_loop_l.pop();
				curRight = stack_loop_r.top();
				stack_loop_r.pop();
			}

			if(cur_POEKey->is_basicblock()){
				BasicBlock *bb = static_cast<BasicBlock *> (cur_POEKey->getBlock());
				vector<Instruction*> cur_statements = bb->get_statements();
				vector<Instruction*> ::iterator iter = cur_statements.begin();
				for (; iter != cur_statements.end(); ++iter) {
					if((*iter)->get_instruction_id() == FUNCTION_CALL)
					{
						FunctionCall* functionCall = static_cast<sail::FunctionCall*> (*iter);
						string name = functionCall->get_function_name();
						size_t found;
						found = name.find(UNKNOWN_STR);
						if (found!=string::npos)
						{
							continue;
						}

						found = name.find(OBSERVED);
						if (found!=string::npos)
						{
							continue;
						}

						Function *callee_func = function_finder(name);
						//CallingContext* new_cc = new CallingContext(*inner_cc);
						//q_cc.push(new_cc);
						q_func.push(callee_func);
						q_line.push(functionCall->line);
						q_outter_POEKey.push(cur_outter_POEKey);
						q_cur_cc.push(cur_POEKey->getcallingcontext());
					}
				}
			}else{

				loopTree.insert(cur_outter_POEKey, cur_POEKey);

				//find loop end.
				set<CfgEdge*> set_Edges = cur_POEKey->get_successors();
				set <CfgEdge*>::iterator iter_Edges=set_Edges.begin();
				for (; iter_Edges!=set_Edges.end(); iter_Edges++)
				{
					stack_loop_r.push(curRight);
					//curRight = new POEKey((*iter_Edges)->get_target(), cur_outter_POEKey->getcallingcontext());
					curRight = new POEKey((*iter_Edges)->get_target(), cur_POEKey->getcallingcontext());
				}

				SuperBlock* spb_cur = static_cast<SuperBlock *> (cur_POEKey->getBlock());
				BasicBlock* bb_sb_entry = spb_cur->get_entry_block();
				//Add entry of nested while loop into the worklist
				worklist.insert(bb_sb_entry);

				stack_loop_l.push(cur_outter_POEKey);
				cur_outter_POEKey = cur_POEKey;

			}

			set<CfgEdge*>& set_Edges = b_cur->get_successors();
			set <CfgEdge*>::iterator iter_Edges=set_Edges.begin();

			for(; iter_Edges!=set_Edges.end(); iter_Edges++)
			{
				worklist.insert((*iter_Edges)->get_target());
			}
		}


		if(q_func.empty())
		{
			break;
		}else
		{

			Function* callee_func = q_func.front();
			//function_declaration* declare = callee_func->get_original_declaration();
			q_func.pop();
			int line = q_line.front();
			q_line.pop();

			cur_outter_POEKey = q_outter_POEKey.front();
			q_outter_POEKey.pop();

			CallingContext* cur_cc_temp = q_cur_cc.front();
			q_cur_cc.pop();
			//function_declaration* declare = cur_func->get_original_declaration();
			cur_cc = new CallingContext(*cur_cc_temp);
			cur_cc->add_context(cur_func, line);
			cur_func = callee_func;
			worklist.insert(callee_func->get_entry_block());
		}
	}while(true);

}

Function * Verify::function_finder(string name)//, vector<Term*> args)
{
	/*
	 * only compare function name and number of args
	 */
	vector<Function *>::iterator iter;
	for(iter = func_vector.begin(); iter != func_vector.end(); iter++)
	{
		function_declaration* declare = (*iter)->get_original_declaration();
		string name_declare =  declare->get_name();
		vector<variable_declaration*> args_declare = declare->get_arguments();
		if(std::strcmp(name_declare.c_str(), name.c_str()) == 0)// && args.size() == args_declare.size())
		{
			return (*iter);
		}
	}
	cout << "did not find function " << name << endl;
	assert(false);
}

double Verify::to_time(int ticks)
{
	return ((double)(ticks))/((double)CLOCKS_PER_SEC);
}


bool Verify::implies(const Constraint& c1, const Constraint& c2){
	if(CONST_RENAMING)
		return (entry_cr->get_knowledge()&c1).implies(c2);
	else
		return c1.implies(c2);
}

bool Verify::unsat(const Constraint& c){
	if(CONST_RENAMING)
	{
		//cout << "c : " << c.to_string() << endl;
		return (entry_cr->get_knowledge()&c).unsat_discard();
	}
	else
		return c.unsat_discard();
}

bool Verify::valid(const Constraint& c){
	if(CONST_RENAMING)
		return (!entry_cr->get_knowledge()|c).valid();
	else
		return c.valid();
}
