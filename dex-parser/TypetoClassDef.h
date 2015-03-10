#ifndef TYPE_TO_CLASSDEF_H
#define TYPE_TO_CLASSDEF_H
#include "ClassDefInfo.h"
#include "DataIn.h"
#include "DexFileReader.h"
class DexFileReader;


ClassDefInfo* type2ClassDef(DexFileReader* dfr, unsigned int typeidx);

vector<ClassDefInfo*>* ClassdefList(DexFileReader* dfr);

map<string,ClassDefInfo*>* str2Classdef(DexFileReader* dfr);


#endif
