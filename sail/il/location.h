#ifndef LOCATION_H_
#define LOCATION_H_


#include <string>
#include <iostream>
#include "../types.h"


using namespace std;


#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/set.hpp>

/*
 * Represents one location in the source file. Nice and small
 * since it is everywhere.
 */

namespace il
{

/**
 * \brief A location in the source file is identified by its starting and
 *  ending lines and byte offsets for the start and end lines.
 */
class location
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & byte_start;
    	ar & byte_end;
    	ar & line_start;
    	ar & line_end;
    }

public:
	/*
	 * Bytes are relative to the starting and ending line, e.g.,
	 * they always start at 0 for every new line.
	 */
	uint16 byte_start;
	uint16 byte_end;
	uint32 line_start;
	uint32 line_end;

	static const uint32 INVALID = ~(0);
public:
	friend ostream& operator <<(ostream &os, const location &obj);
	string to_string() const;
	location(uint16 start_byte, uint32 start_line, uint16 end_byte,
			uint32 end_line);

	location(uint32 start_line, uint16 start_byte)
	{
		line_start = start_line;
		byte_start = start_byte;
		byte_end = line_end = INVALID;
	};

	void set_end_loc(uint32 line, uint16 byte)
	{
		line_end = line;
		byte_end = byte;
	};

	string to_string(bool pp);

	location();
	bool is_valid();
	void print();

	~location();
};

}

#endif /*LOCATION_H_*/
