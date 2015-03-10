/*
 * Directory.cpp
 *
 *  Created on: Feb 16, 2010
 *      Author: tdillig
 */

#include "FileIdentifier.h"
#include <pwd.h>

FileIdentifier::FileIdentifier(string path)
{
	this->file = construct_absolute_path(path, dirs);
}
FileIdentifier::FileIdentifier(const FileIdentifier& other)
{
	this->dirs = other.dirs;
	this->file = other.file;
}
string FileIdentifier::to_string(char separator) const
{
	string res;
	for(unsigned int i=0; i<dirs.size(); i++)
	{
		res += separator;
		string cur = dirs[i];
		res += cur;
	}
	res +=   separator + file;
	return res;
}
const string& FileIdentifier::get_filename() const
{
	return this->file;
}
int FileIdentifier::num_dirs()
{
	return dirs.size();
}

void FileIdentifier::add_dir(const string& dir)
{
	dirs.push_back(dir);
}
void FileIdentifier::pop_dir()
{
	dirs.pop_back();
}
const string & FileIdentifier::get_ith_dir(int i)
{

	assert(i>=0 && ((unsigned int)i)<dirs.size());
	return dirs[i];
}
bool FileIdentifier::operator==(const FileIdentifier& other) const
{
	if(dirs.size() != other.dirs.size()) return false;
	for(unsigned int i=0; i<dirs.size(); i++)
	{
		if(dirs[i] != other.dirs[i]) return false;
	}
	return this->file == other.file;

}

bool FileIdentifier::operator!=(const FileIdentifier& other) const
{
	return !(*this == other);
}
bool FileIdentifier::operator<(const FileIdentifier& other) const
{
	if( dirs > other.dirs) return false;
	if( dirs < other.dirs) return true;
	return file < other.file;

}

bool FileIdentifier::operator>(const FileIdentifier& other) const
{
	if(dirs < other.dirs) return false;
	if(dirs > other.dirs) return true;
	return file > other.file;

}
FileIdentifier::~FileIdentifier()
{

}

void FileIdentifier::chop_into_directories(const string & path, vector<string>& dirs)
{
	string cur;
	for(unsigned int i=0; i<path.size(); i++)
	{
		if(path[i] == '/') {
			if(cur != "")  {
				dirs.push_back(cur);
				cur = "";
			}
		}
		else cur += path[i];
	}
	if(cur != "") dirs.push_back(cur);

}

string FileIdentifier::construct_absolute_path(const string& file,
		vector<string>& result)
{
	string cur_dir = string(get_current_dir_name());
	passwd *pw = getpwuid(getuid());
	assert(pw != NULL);
	string home_dir = pw->pw_dir;

	/*
	 * Make a vector of directories from cur_dir and home_dir
	 */
	vector<string> cur_dir_vector;
	chop_into_directories(cur_dir, cur_dir_vector);
	vector<string> home_dir_vector;
	chop_into_directories(home_dir, home_dir_vector);


	/*
	 * Find file name without any directories
	 */
	string file_name;
	size_t pos = file.rfind('/');
	if(pos == string::npos) file_name = file;
	else file_name  = file.substr(pos+1);

	/*
	 * Directory name
	 */
	string dir;
	if(pos != string::npos) {
		dir = file.substr(0, pos+1);
	}


	bool second_dot = false;
	if(dir.size() >= 2 && dir[1] == '.') second_dot = true;


	if(dir == "" || (dir[0]!='/' && (second_dot ||
			(dir[0] != '.' && dir[0] != '~')))) {
		result.insert(result.end(), cur_dir_vector.begin(), cur_dir_vector.end());
	}



	string cur;
	for(unsigned int i=0; i < dir.size(); i++)
	{
		// If we see .., pop off the last directory
		if(i != dir.size()-1 && dir[i] == '.' && dir[i+1] == '.') {
			assert(result.size() > 0);
			result.pop_back();
			i++;
			continue;
		}
		else if(dir[i] == '/') {
			if(cur!= ""){
				result.push_back(cur);
				cur = "";
			}
		}

		else if(dir[i] == '.' && i<dir.size()-1 && dir[i+1] == '/')
			result.insert(result.end(), cur_dir_vector.begin(),
					cur_dir_vector.end());
		else if (dir[i] == '~'){
			result.insert(result.end(), home_dir_vector.begin(),
					home_dir_vector.end());
		}
		else cur+=dir[i];
	}
	if(cur != ""){
		result.push_back(cur);
	}

	return file_name;
}

ostream& operator <<(ostream &os, const FileIdentifier &obj)
{
	 os  << obj.to_string();
	 return os;
}
