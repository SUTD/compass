#include "translation_unit.h"
#include "file.h"


namespace il
{

translation_unit::translation_unit()
{
}

translation_unit::~translation_unit()
{
}

map<string, file*> & translation_unit::get_files()
{
	return files;
}
file *translation_unit::get_file_from_name(string name)
{
	return files[name];
}

string translation_unit::to_string() const
{
	map<string, file*>::const_iterator it = files.begin();
	string res;
	for(; it != files.end(); it++)
		res += it->second->to_string();

	return res;
}


}
