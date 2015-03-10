/*
 * Identifier.h
 *
 *  Created on: Feb 16, 2010
 *      Author: tdillig
 */

#ifndef IDENTIFIER_H_
#define IDENTIFIER_H_


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

#include "FileIdentifier.h"
#include "type.h"
#include "namespace_context.h"
#include "language.h"

#include <string>

using namespace std;

namespace il{
	class function_type;
};

/*
 * Identifiers are used to uniquely represent summary units.
 */
class Identifier
{
	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & dir;
		ar & function_name;
		ar & loop_id;
		ar & signature;
		ar & ns;
		ar & language;

	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & dir;
		ar & function_name;
		ar & loop_id;
		ar & signature;
		il::type::register_loaded_typeref((il::type**)&signature);
		ar & ns;
		ar & language;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()




private:
	FileIdentifier dir;
	string function_name;
	int loop_id;
	il::function_type* signature;
	il::namespace_context ns;
	language_type language;
protected:

public:
	Identifier()
	{
		loop_id = 0;
		signature = NULL;
		language = LANG_UNSUPPORTED;
	};
	/*
	 * Constructor for functions
	 */
	Identifier(const FileIdentifier& dir, const string & function_name,
			const il::namespace_context & ns, il::function_type* signature,
			language_type language);

	/*
	 * Constructor for loops
	 */
	Identifier(const Identifier& enclosing_fun_id, int loop_id);

	Identifier(const Identifier& other);

	string to_string(char dir_separator = '/') const;
	/*
	 * Returns a short, human readable name for the identifier.
	 * Note: This may not be unique among all identifiers.
	 */
	string to_short_string() const;
	bool operator==(const Identifier & other) const;
	bool operator<(const Identifier & other) const;
	const string& get_function_name() const;
	const il::namespace_context& get_namespace() const;
	il::function_type* get_function_signature() const;
	Identifier get_function_identifier() const;
	const FileIdentifier& get_file_id() const;
	language_type get_language() const;
	bool is_loop_id() const;
	bool is_function_id() const;
	int get_loop_id() const;
	bool is_empty() const;



	friend ostream& operator <<(ostream &os, const Identifier &obj);

	virtual ~Identifier();
};

ostream& operator <<(ostream &os, const Identifier &obj);

#endif /* IDENTIFIER_H_ */
