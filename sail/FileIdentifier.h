/*
 * Directory.h
 *
 *  Created on: Feb 16, 2010
 *      Author: tdillig
 */

#ifndef FILE_IDENTIFIER_H_
#define FILE_IDENTIFIER_H_

#include <string>
#include <vector>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/set.hpp>

using namespace std;

/**
 * Represents a directory on the file system.
 */
class FileIdentifier
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & dirs;
		ar & file;
	}

private:
	vector<string> dirs;
	string file;


public:
	FileIdentifier(){};
	FileIdentifier(string path);
	FileIdentifier(const FileIdentifier& other);
	string to_string(char separator = '/') const;
	const string& get_filename() const;
	int num_dirs();
	const string & get_ith_dir(int i);
	bool operator==(const FileIdentifier& other) const;
	bool operator!=(const FileIdentifier& other) const;
	bool operator<(const FileIdentifier& other) const;
	bool operator>(const FileIdentifier& other) const;
	void add_dir(const string& dir);
	void pop_dir();

	virtual ~FileIdentifier();


	/*
	 * Static methods
	 */
	static string construct_absolute_path(const string& path,
			vector<string>& path_vec);

	static void chop_into_directories(const string& path, vector<string>& dirs);

	friend ostream& operator <<(ostream &os, const FileIdentifier &obj);

};

ostream& operator <<(ostream &os, const FileIdentifier &obj);

#endif /* FILE_IDENTIFIER_H_ */
