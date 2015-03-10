/*
 * Serializer.h
 *
 *  Created on: Jul 19, 2008
 *      Author: isil
 */

#ifndef SERIALIZER_H_
#define SERIALIZER_H_



#include <string>
#include <vector>
using namespace std;
namespace sail {

class TranslationUnit;
class Cfg;
class Function;


string get_legal_filename(string  directory, const string & identifier);

/**
 * \brief This class can be used to serialize a translation unit.
 */


class Serializer {
public:
	Serializer(TranslationUnit* tu, string output_dir);
	virtual ~Serializer();
	static Function* load_file(string file);


};

}

#endif /* SERIALIZER_H_ */
