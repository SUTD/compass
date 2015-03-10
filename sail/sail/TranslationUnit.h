/*
 * TranslationUnit.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#ifndef SAIL_TRANSLATIONUNIT_H_
#define SAIL_TRANSLATIONUNIT_H_

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>


#include<string>
#include<vector>
#include "util.h"
#include <typeinfo>


namespace il{class translation_unit;}
using namespace std;

namespace sail {

class Function;


/**
 * \brief Represents a single translation unit
 */
class TranslationUnit {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & functions;
        ar & original;
        ar & file_name;
    }
private:
	vector<Function*> functions;
	il::translation_unit* original;
	string file_name;


public:

	TranslationUnit(il::translation_unit* original, string asm_name);
	TranslationUnit(vector<Function*> & functions,
			il::translation_unit* original, string asm_name);
	TranslationUnit(){};
	vector<Function*>& get_functions();
	void add_function(Function* f);
	il::translation_unit* get_original_translation_unit();
	virtual ~TranslationUnit();
	string get_file_name();




};

}

#endif /* TRANSLATIONUNIT_H_ */
