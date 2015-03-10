/*
 * CfgEdge.h
 *
 *  Created on: Jul 6, 2008
 *      Author: isil
 *
 *   An edge in the control flow graph, represented by
 *   non-null source and target basic blocks along with an edge condition.
 *   The edge condition is a Symbol such that the edge is taken if Symbol != 0.
 *   Note that the symbol representing the condition can be NULL, meaning that
 *   the edge is taken unconditionally.
 */

#ifndef CFGEDGE_H_
#define CFGEDGE_H_

#include <string>
using namespace std;

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

namespace sail {

class BasicBlock;
class Symbol;
class Block;

/**
 * \brief Represents an edge from a source block to a target block possibly labeled
 * with some condition.
 */
class CfgEdge {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & source;
        ar & target;
        ar & cond;
        ar & backedge;
    }
private:
	Block* source;
	Block* target;
	Symbol* cond;
	bool backedge;
public:
	CfgEdge(Block* source, Block* target, Symbol* cond);
	CfgEdge(const CfgEdge& other);
	CfgEdge(){};

	/**
	 *
	 * @return source of the edge
	 */
	Block* get_source();

	/**
	 *
	 * @return target of the edge
	 */
	Block* get_target();

	/**
	 *
	 * @return Is this edge a backedge or not?
	 */
	bool is_backedge();
	void mark_backedge();
	void unmark_backedge();

	void set_target(Block* b);
	void set_source(Block* b);
	void set_cond(Symbol* cond);

	/**
	 *
	 * @return the condition labelling this edge; NULL if the target of the
	 * edge has exactly one predecessor.
	 */
	Symbol* get_cond();
	string to_string() const;
	string to_string(bool pretty_print) const;
	virtual ~CfgEdge();
};

}

#endif /* CFGEDGE_H_ */
