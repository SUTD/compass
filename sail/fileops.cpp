/*
 * fileops.cpp
 *
 *  Created on: Jan 4, 2010
 *      Author: tdillig
 */


#include <dirent.h>
#include <string>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>

#include <dirent.h>
#include <string>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <set>



#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>


#include <fstream>

#include <assert.h>
#include <iostream>


using std::string;

using namespace std;

bool find_path_for_file(string cur_path, const string & file, string & res)
{
	if(cur_path.size()>=1 && cur_path[cur_path.size()-1] == '/')
	cur_path = cur_path.substr(0, cur_path.size()-1);
	DIR *dp =  opendir(cur_path.c_str());
	 if(dp  == NULL) return false;
	 struct dirent *dirp;
	 while ((dirp = readdir(dp)) != NULL) {
			string name = string(dirp->d_name);
			if(name == "." || name == "..")
				continue;
			if(name == file) {
				res = cur_path;
				closedir(dp);
				return true;
			}
			string new_prefix = cur_path + "/" + name;
			DIR *cur_dp = opendir(new_prefix.c_str());
			if(cur_dp  != NULL)
			{
				bool found = find_path_for_file(new_prefix, file, res);
				closedir(cur_dp);
				if(found) {
					 closedir(dp);
					 return true;
				}

			}



	 }
	 closedir(dp);
	 return false;
}



void remove_directory(string path)
{
	if(path[path.size()-1] == '/')
		path = path.substr(0, path.size()-1);

	 DIR *dp =  opendir(path.c_str());
	 if(dp  == NULL) return;
	 struct dirent *dirp;
	 while ((dirp = readdir(dp)) != NULL) {
			string name = string(dirp->d_name);
			if(name == "." || name == "..")
				continue;
			string new_prefix = path + "/" + name;
			DIR *cur_dp = opendir(new_prefix.c_str());
			if(cur_dp  != NULL)
			{
				closedir(cur_dp);
				remove_directory(new_prefix);
			}
			//cout << "DELETING: " << new_prefix << endl;
			remove(new_prefix.c_str());


	 }
	 closedir(dp);
}

static void internal_mkdir(const char *dir)
{
        char tmp[2560];
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

bool _ends_with(const string& s, const string & ending)
{
        if(s.size() < ending.size()) return false;
        size_t pos = s.rfind(ending);
        bool res = (pos == (s.size()-ending.size()));
        return res;
}


void collect_all_files_with_extension(set<string> & reg_paths, string prefix,
                const string & ext)
{
         DIR *dp =  opendir(prefix.c_str());
         if(dp  == NULL) return;
         struct dirent *dirp;
         while ((dirp = readdir(dp)) != NULL) {
                string name = string(dirp->d_name);
                if(name == "." || name == "..")
                        continue;
                string new_prefix = prefix + name  + "/";
                DIR *cur_dp = opendir(new_prefix.c_str());
                if(cur_dp  != NULL)
                {
                        closedir(cur_dp);
                        collect_all_files_with_extension(reg_paths, new_prefix, ext);
                        continue;
                }
                if(_ends_with(name, ext))
                {
                        reg_paths.insert(prefix + name);
                }


         }
         closedir(dp);
}



void delete_files_with_extension(string path, string extension)
{


	if(path[path.size()-1] == '/')
		path = path.substr(0, path.size()-1);

	 DIR *dp =  opendir(path.c_str());
	 if(dp  == NULL) return;
	 struct dirent *dirp;
	 int ext_size = extension.size();
	 while ((dirp = readdir(dp)) != NULL) {
			string name = string(dirp->d_name);

			if(name == "." || name == "..")
				continue;
			string new_prefix = path + "/" + name;
			DIR *cur_dp = opendir(new_prefix.c_str());
			if(cur_dp  != NULL)
			{
				closedir(cur_dp);
				delete_files_with_extension(new_prefix, extension);
			}
			else
			{
				if((int)new_prefix.size()>=ext_size){
					string rest = new_prefix.substr(new_prefix.size()-ext_size);
					cout << "Rest: " << rest << " ext: " << extension << endl;
					if(rest == extension) {
						cout << "deleting: " << new_prefix << endl;
						remove(new_prefix.c_str());
					}
				}

			}
			//cout << "DELETING: " << new_prefix << endl;
			//remove(new_prefix.c_str());


	 }
	 closedir(dp);
}


void make_directory(string path)
{
	internal_mkdir(path.c_str());
}

string ee_escape(string e)
{
	string res;
	for(int  i=0; i < e.size(); i++)
	{
		if(e[i] == ' ') res += "\\ ";
		else res += e[i];
	}
	return res;
}

void copy_android_directory(string source, string dest)
{
	make_directory(dest);
	string s = source + "/gen";
	string cmd = "cp -r " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());

	s = source + "/libs";
	cmd = "cp -r " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());

	s = source + "/res";
	cmd = "cp -r " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());

	s = source + "/src";
	cmd = "cp -r " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());

	s = source + "/AndroidManifest.xml";
	cmd = "cp  " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());

	s = source + "/build.xml";
	cmd = "cp  " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());

	s = source + "/*.properties";
	cmd = "cp  " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());


}


void copy_directory(string source, string dest)
{
	make_directory(dest);

	string s = source + "/*.c";
	string cmd = "cp -r " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());
	//cerr << "cmd22: " << cmd << endl;


	s = source + "/*.h";
	cmd = "cp -r " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());
	//cerr << "cmd22: " << cmd << endl;


	s = source + "/*.hpp";
	cmd = "cp -r " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());
	//cerr << "cmd22: " << cmd << endl;

	s = source + "/*.cpp";
	cmd = "cp -r " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());
	//cerr << "cmd22: " << cmd << endl;


	s = source + "/*.cc";
	cmd = "cp -r " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());

	s = source + "/*.java";
	cmd = "cp -r " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());
	//cerr << "cmd22: " << cmd << endl;

	s = source + "/Makefile";
	cmd = "cp -r " + ee_escape(s) + " " + ee_escape(dest) +
			" 2>/dev/null";
	//cerr << "cmd: " << cmd << endl;
	system(cmd.c_str());

	copy_android_directory(source, dest);
}


string strip(string s)
{
	string res;
	int i = 0;
	while(i < s.size() && s[i] == ' ') i++;

	res = s.substr(i);

	for(i = res.size()-1; i>=0; i--)
	{
		if(res[i]!=' ') break;
	}
	return res.substr(0, i+1);

}

