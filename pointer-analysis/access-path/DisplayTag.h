/*
 * DisplayTag.h
 *
 *  Created on: Sep 14, 2011
 *      Author: isil
 */

#ifndef DISPLAYTAG_H_
#define DISPLAYTAG_H_

#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

#include <string>
using namespace std;

class DisplayTag;

/**
 * Records user-friendly information for synthetic variables
 * introduced by the analysis, such as imprecise or unmodeled values.
 */
class DisplayTag {
	friend class boost::serialization::access;
protected:
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & description;
		ar & line;
		ar & file;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & description;
		ar & line;
		ar & file;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
private:
	string description;
	int line;
	string file;


public:
	DisplayTag(string readable_exp, int line = -1, string file = "");

	DisplayTag(const DisplayTag& other);
	DisplayTag();
	string to_string();
	string get_description() const;
	int get_line() const;
	string get_file() const;
	bool inline operator==(const DisplayTag & dt) const
	{
		return this->line == dt.line &&
				this->file == dt.file &&
				this->description == dt.description;
	}
	~DisplayTag();
};

#endif /* DISPLAYTAG_H_ */
