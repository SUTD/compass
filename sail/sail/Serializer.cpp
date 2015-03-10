/*
 * Serializer.cpp
 *
 *  Created on: Jul 19, 2008
 *      Author: isil
 */

#include "sail-serialization.h"

#include "serialization-hooks.h"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "Serializer.h"
#include "TranslationUnit.h"
#include "Function.h"
#include "Cfg.h"
#include "function_declaration.h"
#include <dirent.h>
#include <errno.h>

#include <dirent.h>
#include <string>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>



#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>


#include <fstream>

#include <assert.h>
#include <iostream>


/*
 * Maximum length of any file created.
 */
//haiyan changed it to 266
//#define MAX_FILENAME_SIZE 110
#define MAX_FILENAME_SIZE 110

/*
 * The minimum size of any "chunk" created by splitting a file
 * into folder/.../file
 */
#define MIN_FILNAME_SIZE 10


#define SAIL_EXTENSION ".sail"


using namespace std;


namespace sail {

static void _mkdir(const char *dir)
{
        char tmp[5560];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
                tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
                if(*p == '/') {
                        *p = 0;
                        mkdir(tmp, S_IRWXU | S_IRWXG | S_IRWXO);
                        *p = '/';
                }
        mkdir(tmp, S_IRWXU);
}

string replace_illegal_characters(const string& s)
{
	string res;
	for(unsigned int i=0; i < s.size(); i++)
	{
		char c = s[i];
		if(c=='/') res += "#";
		else res += c;
	}
	return res;

}



string get_legal_filename(string  directory, const string & _identifier)
{
	if(directory.size() == 0 || directory[directory.size()-1] != '/')
		directory+='/';
	string identifier = replace_illegal_characters(_identifier);

	if(identifier.size() >(MAX_FILENAME_SIZE-MIN_FILNAME_SIZE))
	{
		string cur;
		while(identifier.size() > (MAX_FILENAME_SIZE-MIN_FILNAME_SIZE))
		{
			cur = identifier.substr(0, (MAX_FILENAME_SIZE-MIN_FILNAME_SIZE));
			identifier = identifier.substr((MAX_FILENAME_SIZE-MIN_FILNAME_SIZE));

			if(identifier.size() < MIN_FILNAME_SIZE) {
				cur += identifier;
				identifier = "";
			}
			directory+=cur + "/";
			if(identifier != "") _mkdir(directory.c_str());



		}
		directory+= identifier;
		if(directory[directory.size()-1] == '/')
			return directory.substr(0, directory.size()-1);
		return directory;


	}
	return directory + identifier;


}



Serializer::Serializer(TranslationUnit* tu, string output_dir)
{



	vector<Function*> & fv = tu->get_functions();
	if(output_dir.size() == 0 || output_dir[output_dir.size()-1] != '/')
		output_dir += "/";

	FileIdentifier out_fi(output_dir);

	string expanded_dir = out_fi.to_string('/');




	map<string, Function*> checks;

	_mkdir(expanded_dir.c_str());
	for(unsigned int i=0; i< fv.size(); i++)
	{
		Function* f = fv[i];


		string identifier = f->get_identifier().to_string('#');
		identifier+= SAIL_EXTENSION;

		string file_name;



		file_name = get_legal_filename(expanded_dir, identifier);
		//cout << "file_name:: " << file_name << endl;
		//haiyan added for test the size of filename
		/*
		std::stringstream out;
		out << i;
		std::string s = out.str();
		file_name = "/home/scratch/hzhu/f" + s;
		cout << "file_name :: " << file_name << endl;
		//haiyan ended
		*/


		ofstream myfile;
		myfile.open (file_name.c_str(), ios::binary | ios::trunc);
		if(!myfile.is_open())
		{
			cout << "error: Cannot create file \"" << file_name
					<< "\" to write SAIL." << endl;
			continue;
		}
		OUT_ARCHIEVE oa(myfile);
		
		oa << f;
		myfile.close();
	}







}


Function * Serializer::load_file(string file)
{

	ifstream infile;
	infile.open(file.c_str(), ios::binary);
	assert(infile.is_open());
	begin_serialization();
	Function * f;
	IN_ARCHIEVE ia(infile);
	ia >> f;
	end_serialization();
	assert(f != NULL);
	return f;
}




Serializer::~Serializer() {

}



}
