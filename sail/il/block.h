#ifndef BLOCK_H_
#define BLOCK_H_

#include "statement.h"
#include <vector>
using namespace std;




namespace il
{

class variable_declaration;
class statement;

/**
 * \brief Represents a block statement {decls; S1; ... ; S_k}
 */
class block : public il::statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::statement>(*this);
        ar & var_decls;
        ar & statements;
    }


private:
	vector<variable_declaration*> var_decls;
	vector<statement*> statements;

public:
	friend ostream& operator <<(ostream &os, const block &obj);
	block();
	block(vector<variable_declaration*> & var_decls,
				vector<statement*> & statements);
	vector<variable_declaration*> & get_var_declarations();
	vector<statement*> & get_statements();
	void print();
	virtual ~block();
	virtual string to_string() const;

};

}

#endif /*BLOCK_H_*/
