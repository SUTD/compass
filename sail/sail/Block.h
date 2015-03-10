/*
 * Block.h
 *
 *  Created on: Jul 11, 2008
 *      Author: isil
 *
 *  A block represents either an ordinary basic block
 *  or a super block that has a summary associated with it.
 *  Unsurprisingly, super blocks are often loops.
 */

#ifndef SAIL_BLOCK_H_
#define SAIL_BLOCK_H_

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


#include <set>
#include <string>
#include "Symbol.h"

using namespace std;

namespace sail {
class CfgEdge;

/**
 * \brief An abstract class that BasicBlock and SuperBlock inherit from.
 */
class Block {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & block_id;
        ar & successors;
        ar & predecessors;
    }
protected:
	int block_id;
	set<CfgEdge*> successors;
	set<CfgEdge*> predecessors;
public:


	Block();
	virtual ~Block();

	/**
	 * @return Is this block a basic block?
	 */
	virtual bool is_basicblock();

	/**
	 *
	 * @return Is this block a super block? (i.e. does it represent a loop)
	 */
	virtual bool is_superblock();
	virtual string to_string() const = 0;
	virtual string to_dotty(string prelude, bool pretty_print = true) = 0;

	void add_successor_edge(CfgEdge* edge);
	void add_predecessor_edge(CfgEdge* edge);
	void set_block_id(int id);
	int get_block_id() const;

	/**
	 *
	 * @return set of successors of this block. A block may have more than
	 * two successors, e.g., for switch statements
	 */
	set<CfgEdge*>& get_successors();

	/**
	 *
	 * @return the set of predecessors of this block.
	 */
	set<CfgEdge*>& get_predecessors();

	//haiyan added 4.19.2013
	/**
	 * @return a map of condition->successor
	 */
	map<Symbol*, CfgEdge*>& get_successors_map();
	//haiyan added 4.19.2013

};

}

#endif /* SAIL_BLOCK_H_ */
