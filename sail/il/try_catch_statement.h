#ifndef TRY_CATCH_STATEMENT_H_
#define TRY_CATCH_STATEMENT_H_

#include "control_statement.h"

#include <vector>

namespace il
{

/**
 * \brief The representation of an try-catch statement
 */
class try_catch_statement : public il::control_statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<il::control_statement>(*this);
        ar & try_statement;
        ar & catch_types;
        ar & catch_bodies;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<il::control_statement>(*this);
        ar & try_statement;
        ar & catch_types;
        for(unsigned int i=0; i<catch_types.size(); i++) {
        	type::register_loaded_typeref(&catch_types[i]);
        }

        ar & catch_bodies;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
private:
	statement *try_statement;
	
	/**
	 * NULL catch_type indicates a catch-all handler (i.e. catch (...))
	 */
	vector<type *> catch_types;
	vector<statement *> catch_bodies;

public:
	try_catch_statement(statement *try_stmt, location loc);
	try_catch_statement(statement *try_stmt, location loc,
			vector<type *> catch_types, vector<statement *> catch_bodies);
	try_catch_statement();
	virtual ~try_catch_statement();
	void print();
	statement *get_try_statement();
	vector<type *> &get_catch_types();
	vector<statement *> &get_catch_bodies();
	void add_handler(type *catch_type, statement *catch_body);
	virtual string to_string() const;

};

}

#endif /* TRY_CATCH_STATEMENT_H_ */
