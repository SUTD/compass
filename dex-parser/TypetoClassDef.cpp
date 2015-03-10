#include "TypetoClassDef.h"

ClassDefInfo* type2ClassDef(DexFileReader* dfr, unsigned int typeidx)
{
	ClassDefInfo* classdef = NULL;
	map<int, int>* type2class = dfr->getClassToTypeidx();
	if (type2class == NULL)
		cout << "type2class is NULL" << endl;
	if(!GLOBAL_MAP){
	if (type2class->count(typeidx) > 0) //found
			{
		unsigned classdefidx = (*type2class)[typeidx];
		classdef = (*dfr->getClassDefIdx2Ele())[classdefidx];
	} else {
		classdef = new ClassDefInfo(dfr->getTypename(typeidx));
	}}else{
		string str = dfr->getTypename(typeidx);
		classdef = dfr->getClassDefByName(str);
	}
	//12/20
	//classdef->setTypeIdx(typeidx);
	//12/20
	return classdef;
}

vector<ClassDefInfo*>* ClassdefList(DexFileReader* dfr)
{
	ClassDefInfo* temp = NULL;
	//DexFileReader* dfr = new DexFileReader(in);
	vector<ClassDefInfo*>* templist = new vector<ClassDefInfo*>();
	for(unsigned int i = 0; i < dfr->getTypeIdsSize(); i++)
	{
		temp = type2ClassDef(dfr, i);
		templist->push_back(temp);
	}
	return templist;
}

map<string, ClassDefInfo*>* str2Classdef(DexFileReader* dfr) //given a string, have its ClassDefInfo*
{
	map<string, ClassDefInfo*>* tempmap = new map<string,ClassDefInfo*>();
	vector<ClassDefInfo*>* templist = ClassdefList(dfr);
	if(templist != NULL)
	{
		vector<ClassDefInfo*>::iterator it = templist->begin();
		for(; it != templist->end(); it++)
		{
			string str = (*it)->getTypename();
			(*tempmap)[str] = (*it);
		}
	}
	return tempmap;
}
