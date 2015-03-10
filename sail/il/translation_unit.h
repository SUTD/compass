#ifndef TRANSLATION_UNIT_H_
#define TRANSLATION_UNIT_H_


#include <map>
using namespace std;

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>


namespace il
{
class file;

/**
 * \brief A translation unit consists of one or more files.
 */
class translation_unit
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & files;
    }
private:
	map<string, file*> files;
public:
	translation_unit();
	virtual ~translation_unit();
	map<string, file*> & get_files();
	file *get_file_from_name(string name);
	virtual string to_string() const;
};

}

#endif /*TRANSLATION_UNIT_H_*/
