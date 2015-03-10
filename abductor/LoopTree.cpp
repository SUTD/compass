/*
 * LoopTree.cpp
 *
 *  Created on: Jul 25, 2012
 *      Author: boyang
 */

#include "LoopTree.h"

LoopTree::LoopTree() {
	// TODO Auto-generated constructor stub

}

LoopTree::~LoopTree() {
	// TODO Auto-generated destructor stub
}



void LoopTree::insert(POEKey* outer, POEKey* inner)
{
	map<POEKey*,set<POEKey*> >::iterator  tm_iter;
	for(tm_iter = tm.begin(); tm_iter != tm.end(); tm_iter++)
	{
		if(tm_iter->first == outer)
			break;
	}
	if(tm_iter == tm.end())
	{
		set<POEKey*> newSet;   // = new set<int>();
		newSet.insert(inner);
		tm.insert(pair<POEKey*, set<POEKey*> >(outer, newSet));
	}
	else
	{
		tm_iter->second.insert(inner);
	}
}


void LoopTree::getInnerLoopList(POEKey* outer, list<POEKey*>& list_inner)
{
	map<POEKey*,set<POEKey*> >::iterator  tm_iter;
	for(tm_iter = tm.begin(); tm_iter != tm.end(); tm_iter++)
	{
		if(*(tm_iter->first) == *outer)
		{
			set<POEKey*>::iterator list_riter;
			for(list_riter = tm_iter->second.begin(); list_riter != tm_iter->second.end(); list_riter++)
			{
				list_inner.push_front(*list_riter);
			}
		}
	}
}

void LoopTree::getAllLoopList(list<POEKey*>& list_POEKey)
{
	map<POEKey*,set<POEKey*> >::iterator  tm_iter;
	for(tm_iter = tm.begin(); tm_iter != tm.end(); tm_iter++)
	{
		set<POEKey*>::iterator list_riter;
		for(list_riter = tm_iter->second.begin(); list_riter != tm_iter->second.end(); list_riter++)
		{
			bool find = false;
			list<POEKey*>::iterator it;
			for(it =  list_POEKey.begin(); it != list_POEKey.end(); it++)
			{
				if(*it == *list_riter)
					find = true;
			}
			if(!find)
				list_POEKey.push_front(*list_riter);
		}
	}
}




string LoopTree::to_string() const
{

	string res = "";
	map<POEKey*,set<POEKey*> >::const_iterator  tm_iter;
	for(tm_iter = tm.begin(); tm_iter != tm.end(); tm_iter++)
	{
		res += tm_iter->first->to_string();
		res += ", ";
		set<POEKey*>::const_iterator sec_iter;
		for(sec_iter = tm_iter->second.begin(); sec_iter != tm_iter->second.end(); sec_iter++)
		{
			res += (*sec_iter)->to_string() + "  ";
		}
		res += "\n";
	}
	return res;
}

