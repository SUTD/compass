/*
 * TranslationUnit.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "TranslationUnit.h"
#include "Function.h"
#include "translation_unit.h"
#include "node.h"

namespace sail {

TranslationUnit::TranslationUnit(vector<Function*> &functions,
		il::translation_unit* original, string asm_name) {
	this->functions = functions;
	this->original = original;
	this->file_name = asm_name.substr(0, asm_name.size()-2);
}

TranslationUnit::TranslationUnit(il::translation_unit* original,
		string asm_name)
{
	this->original = original;
	this->file_name = asm_name.substr(0, asm_name.size()-2);
}


vector<Function*>& TranslationUnit::get_functions()
{
	return functions;
}
void TranslationUnit::add_function(Function* f)
{
	functions.push_back(f);
}

il::translation_unit* TranslationUnit::get_original_translation_unit()
{
	return original;
}



string TranslationUnit::get_file_name()
{
	return file_name;
}

TranslationUnit::~TranslationUnit() {

}






}
