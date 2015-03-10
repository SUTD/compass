/*
 * Serializable.h
 *
 *  Created on: Apr 12, 2010
 *      Author: tdillig
 */

#ifndef SERIALIZABLE_H_
#define SERIALIZABLE_H_

#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/set.hpp>
#include <iostream>
using namespace std;


class Serializable
{

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {

    }

public:
	Serializable();
	virtual ~Serializable();
};

#endif /* SERIALIZABLE_H_ */
