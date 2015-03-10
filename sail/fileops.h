/*
 * fileops.h
 *
 *  Created on: Jan 4, 2010
 *      Author: tdillig
 */

#ifndef FILEOPS_H_
#define FILEOPS_H_

bool remove_directory(string path);

void make_directory(string path);
string ee_escape(string e);

void copy_directory(string source, string dest);
void copy_android_directory(string source, string dest);
void delete_files_with_extension(string path, string extension);
void collect_all_files_with_extension(set<string> & reg_paths, string prefix,
                const string & ext);


/*
 * Finds the path for file, if there are multiple files with this name it
 * returns the first one.
 */
bool find_path_for_file(string cur_path, const string & file, string & res);

string strip(string s);

#endif /* FILEOPS_H_ */
