/*
 * SailGenerator.cpp
 *
 *  Created on: Feb 25, 2010
 *      Author: tdillig
 */

#include "SailGenerator.h"
#include "compass_ui.h"
#include "util.h"

#include "FileIdentifier.h"
#include "fileops.h"




string SailGenerator::runtime_include_dir =
		FileIdentifier(COMPASS_INCLUDE_DIR).to_string();



SailGenerator::SailGenerator(const string & compile_dir,
		const string & sail_out_dir, set<Error*> & errors,
		bool compile_all):errors(errors),
				compile_dir(compile_dir), sail_out_dir(sail_out_dir),
				compile_all(compile_all)
{

	string path = compile_dir;
	if(path[path.size()-1] != '/') path+='/';
	string makefile = path + "Makefile";
	string android = path + "AndroidManifest.xml";
	ifstream in_android(android.c_str());
	if(in_android.is_open()) {
		//We are looking at an Android project
		compile_android_project();
		return;
	}

	string dir = compile_dir;
	if(dir[dir.size()-1]!='/') dir+='/';
	set<string> java_files;
	collect_all_java_files(java_files, dir);
	if(java_files.size() > 0)
	{
		//We are looking at a java project
		compile_java_project(java_files);
		return;
	}




	ifstream in(makefile.c_str());
	if(in.is_open())
		compile_with_makefile();
	else compile_without_makefile();


}

void SailGenerator::compile_without_makefile()
{
	num_lines = 0;

	set<string> files;
	string dir = compile_dir;
	if(dir[dir.size()-1]!='/') dir+='/';
	collect_all_c_files(files, dir);

	set<string>::iterator it = files.begin();
	for(; it!= files.end(); it++)
	{
		const string & cur_file = *it;

		/*
		 * Count lines
		 */
		{
			ifstream in(cur_file.c_str());
			if(in.is_open())
			{
				while(!in.eof())
				{
					string temp;
					std::getline(in, temp);
					num_lines++;
				}
			}
			in.close();
		}

		/*
		 * skip .h files in compilation.
		 */
		if(is_header_file(cur_file)) continue;




		compiled_files.insert(cur_file);


		string gcc;
		if(is_c_file(cur_file)) gcc = GCC_PATH;
		else gcc = GPP_PATH;
		gcc += " ";
		gcc += compass_macro_defs;
		gcc += " -include \"" + string(COMPASS_INCLUDE_FILE) + "\" ";
		gcc += " -c -I. -I"+ runtime_include_dir
				+" --sail=" + sail_out_dir + " ";
		gcc +=  "\""+cur_file+"\"";
		gcc += " 2> out.txt";

		cout << "Calling: " << gcc << endl;
		int r = system(gcc.c_str());
		bool compilation_error = false;
		if(r != 0) compilation_error = true;
		ifstream in;
		in.open("out.txt");
		while(compilation_error && in.is_open() && !in.eof()) {
			string temp;
			std::getline(in, temp);
			cout << temp << endl;
			string temp2;
			if(!in.eof()){
				std::getline(in, temp2);
				temp+=temp2;
			}
			if(is_empty_string(temp)) continue;
			if(temp.find("error: (") != string::npos) continue;
			if(temp.find("warning:")!=string::npos &&
					temp.find("error:")==string::npos) continue;


			errors.insert(make_compilation_error(temp));
		}
		in.close();
		int s = system("rm out.txt");
		if(s == -1){
			cout << "Error: Call to SAIL GCC failed. (Did you install SAIL gcc"
					" in the expected folder?)" << endl;
		}

	}
}


void SailGenerator::compile_with_makefile()
{
	num_lines = 0;
	string dir = compile_dir;
	if(dir[dir.size()-1]!='/') dir+='/';
	collect_all_c_files(compiled_files, dir);

	set<string>::iterator it = compiled_files.begin();
	for(; it!= compiled_files.end(); it++)
	{
		const string & cur_file = *it;

		/*
		 * Count lines
		 */

		ifstream in(cur_file.c_str());
		if(in.is_open())
		{
			while(!in.eof())
			{
				string temp;
				std::getline(in, temp);
				num_lines++;
			}
		}
		in.close();
	}



	string escaped_macros;
	for(unsigned int i=0; i < compass_macro_defs.size(); i++)
	{
		if(compass_macro_defs[i] == '"')
			escaped_macros +="\\\"";
		else escaped_macros += compass_macro_defs[i];
	}

	string command;
	if(compile_all) command += "make clean; ";
	command+= "make SAIL_GCC=\"" + string(GCC_PATH) + "\"  SAIL_CFLAGS=\"" +
			"--sail=\\\"" + sail_out_dir + "\\\"  " + escaped_macros +
			+ " -I" + runtime_include_dir +
			" -include \\\"" + COMPASS_INCLUDE_FILE + "\\\" " + "\" 2> out.txt";
	if(compile_all) command += "; make clean";

	char* orig_dir = get_current_dir_name();
	int x = chdir(compile_dir.c_str());
	assert(x == 0);
	int status = system(command.c_str());
	if(status < 0) {
		cout << "Compilation problem" << endl;
	}

	ifstream in;
	in.open("out.txt");
	while(in.is_open() && !in.eof()) {
		string temp;
		std::getline(in, temp);
		cout << temp << endl;
		string temp2;
		if(!in.eof()){
			std::getline(in, temp2);
			temp+=temp2;
		}
		if(is_empty_string(temp)) continue;
		if(temp.find("error:") == string::npos) continue;
		if(temp.find("error: (") != string::npos) continue;
		if(temp.find("warning:")!=string::npos &&
				temp.find("error:")==string::npos) continue;

		cout << "Making cc error: " << temp << endl;
		errors.insert(make_compilation_error(temp));
	}
	in.close();
	int s = system("rm out.txt");
	assert(s == s);

	x = chdir(orig_dir);
	assert(x == 0);
	free(orig_dir);
}

string escape_dollar(const string & s)
{
	string res;
	for(unsigned int i=0; i<s.size(); i++) {
		if(s[i] == '$') res+="\\$";
		else res += s[i];
	}
	return res;
}

void SailGenerator::compile_java_project(set<string> & compiled_files)
{
	cout << "*** compiling JAVA project *** " << endl;

	//remove all .class files
	delete_files_with_extension(compile_dir, ".class");
	int ignore = system("rm -rf classes.dex 2> /dev/null");
	assert(ignore == ignore);

	num_lines = 0;
	set<string>::iterator it = compiled_files.begin();
	bool compilation_error = false;

	string java_files = COMPASS_JAVA_RUNTIME;

	for(; it!= compiled_files.end(); it++)
	{
		const string & cur_file = *it;

		/*
		 * Count lines
		 */

		ifstream in2(cur_file.c_str());
		if(in2.is_open())
		{
			while(!in2.eof())
			{
				string temp;
				std::getline(in2, temp);
				num_lines++;
			}
		}
		in2.close();


		compiled_files.insert(cur_file);



		cout << "******** current file: " <<cur_file << endl;
		java_files += " ";
		java_files += "\"" + cur_file + "\"";
	}

	string javac = "javac -g " + java_files + " 2> out.txt";

	cout << "Calling: " << javac << endl;
	int r = system(javac.c_str());
	assert(r==r);
	ifstream in;
	in.open("out.txt");
	while(in.is_open() && !in.eof()) {
		string temp;
		std::getline(in, temp);
		cout << temp << endl;

		if(temp.find(":")==string::npos) continue;
		int index = temp.find(":");
		string t2 = temp.substr(index+1);
		if(t2.find(":") ==string::npos) continue;
		compilation_error = true;
		errors.insert(make_compilation_error(temp));


	}
	in.close();
	int s = system("rm out.txt");
	s = system("rm eo.txt 2> /dev/null ");
	assert(s == s);

	if(compilation_error){
		delete_files_with_extension(compile_dir, ".class");
		return;
	}

	set<string> class_files;
	string dir = compile_dir;
	if(dir[dir.size()-1]!='/') dir+='/';
	collect_all_class_files(class_files, dir);


	string files;
	for(auto it = class_files.begin(); it != class_files.end(); it++) {
		//skip inner classes
		//if(files.find('$')!= string::npos) continue;
		files += " ";
		files += "\"" + escape_dollar(*it) + "\"";
	}
	cout << "files: " << files << endl;
	string dx = string(ANDROID_PATH) +
			"/dx --dex --debug --no-strict --output classes.dex "
			+ files;
	cout << dx << endl;
	s = system(dx.c_str());
	assert(s == s);

	//string cmd = string(DEX_PARSER) + " \"" + compile_dir
	//		+ "/classes.dex\" ./sail/";
	string cmd = string(DEX_PARSER) + " classes.dex ./sail/";
	ignore = system(cmd.c_str());
	assert(ignore == ignore);

	//delete_files_with_extension(compile_dir, ".class");
	ignore = system("rm -rf classes.dex 2> /dev/null");

	delete_files_with_extension(compile_dir, ".class");

	string del = "rm " + string(COMPASS_JAVA_RUNTIME_DIR)+ "/*.class";
	ignore = system(del.c_str());

}


void SailGenerator::compile_android_project()
{
	/*
	 * Count number of lines
	 */
	num_lines = 0;
	string dir = compile_dir;
	if(dir[dir.size()-1]!='/') dir+='/';
	collect_all_java_files(compiled_files, dir);

	set<string>::iterator it = compiled_files.begin();
	for(; it!= compiled_files.end(); it++)
	{
		const string & cur_file = *it;

		/*
		 * Count lines
		 */

		ifstream in(cur_file.c_str());
		if(in.is_open())
		{
			while(!in.eof())
			{
				string temp;
				std::getline(in, temp);
				num_lines++;
			}
		}
		in.close();
	}


	/*
	 * Call the ant build script with the debug target
	 */
	string command;
	if(compile_all) command += "ant clean; ";
	command+= "ant debug > out.txt 2> eo.txt";


	char* orig_dir = get_current_dir_name();
	int x = chdir(compile_dir.c_str());
	assert(x == 0);
	int ignore = system(command.c_str());
	assert(ignore == ignore);

	/*
	 * Check if there are any errors. There are errors if
	 * eo.txt is non-empty.
	 */
	bool has_errors = false;
	{
		ifstream in;
		in.open("eo.txt");
		while(in.is_open() && !in.eof()) {
			string temp;
			std::getline(in, temp);
			if(temp.find("BUILD FAILED")!=string::npos) {
				has_errors = true;
				break;
			}
		}
	}
	cout << "Errors? " << has_errors << endl;

	ifstream in;
	in.open("out.txt");
	while(has_errors && in.is_open() && !in.eof()) {
		string temp;
		std::getline(in, temp);
		//cout << "current line: " <<  temp << endl;
		if(temp.find("[javac]")==string::npos) continue;
		cout << temp << endl;
		if(temp.find(":")==string::npos) continue;
		int index = temp.find(":");
		string t2 = temp.substr(index+1);
		if(t2.find(":") ==string::npos) continue;
		//cout << temp << endl;
		temp = temp.substr(12);
		//cout << temp << endl;
		//Every error has a second line with more details, skip
		string temp2;
		std::getline(in, temp2);

		//cout << "Making cc error: " << temp << endl;
		errors.insert(make_compilation_error(temp));
	}
	in.close();
	int s = system("rm out.txt");
	s = system("rm eo.txt");
	assert(s == s);



	/*
	 * If there are no errors, now compile the dex file into
	 * SAIL.
	 */
	if(!has_errors) {
		string cmd = string(DEX_PARSER) + " ./bin/classes.dex ./sail/";
		int ignore = system(cmd.c_str());
		assert(ignore == ignore);
	}
	if(compile_all) command += "ant clean; ";
	ignore = system(command.c_str());
	assert(ignore == ignore);

	x = chdir(orig_dir);
	assert(x == 0);
	free(orig_dir);

}


int SailGenerator::get_total_lines()
{
	return num_lines;
}


bool SailGenerator::has_makefile()
{
	return false;
}


const set<string> & SailGenerator::get_compiled_files()
{
	return compiled_files;
}


bool SailGenerator::is_number(string s)
{
	for(unsigned int i=0; i < s.size(); i++)
	{
		char c = s[i];
		if(c>='0' && c<='9') continue;
		return false;
	}
	return true;

}

bool SailGenerator::is_empty_string(string s)
{
	for(unsigned int i = 0; i < s.size(); i++){
		if(s[i] == ' ' || s[i] == '\t' || s[i] == '\n') continue;
		return false;
	}
	return true;
}

Error* SailGenerator::make_compilation_error(const string & message)
{

	/*
	 * Format is file:line:message
	 */

	int first_col_pos = message.find(":");
	if(first_col_pos == (int)string::npos) return new Error(-1, "",
			ERROR_COMPILATION, "", Identifier());
	string file = message.substr(0, first_col_pos);
	string rest = message.substr(first_col_pos+1);
	int second_col_pos = rest.find(":");
	if(second_col_pos == (int)string::npos) return new Error(-1, "",
			ERROR_COMPILATION, "", Identifier());
	string line = rest.substr(0, second_col_pos);


	string msg = rest.substr(second_col_pos+1);
	if(!is_number(line)) return make_compilation_error(msg);
	int line_int = string_to_int(line);

	return new Error(line_int, file, ERROR_COMPILATION, msg, Identifier());
}


SailGenerator::~SailGenerator()
{

}
