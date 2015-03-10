#ifndef _TYPE_MAPPING_H
#define _TYPE_MAPPING_H
#include "ClassDefInfo.h"
#include "sail.h"
#include "type.h"
#include "DataIn.h"
#include "TypetoClassDef.h"
#include "StringSplit.h"
class DexFileReader;


il::type* makeLibraryType(DexFileReader*,string t_name, map<string, ClassDefInfo*>* typemap);
il::type* makePrimitiveType(DexFileReader*,string str);


il::type* TypeMapping(unsigned int,DexFileReader*, ClassDefInfo*, map<string, ClassDefInfo*>*);
void updateRecordType(DexFileReader*, il::type*, ClassDefInfo*, map<string, ClassDefInfo*>*); //the parameter type just come from the return value of TypeMapping(); and classDefInfo is the same as TypeMapping()
il::type* getType(unsigned int, DexFileReader*, ClassDefInfo*, map<string,ClassDefInfo*>*);
bool alreadyInDerivatives(il::record_type* base, il::record_type* deriv);
#endif
