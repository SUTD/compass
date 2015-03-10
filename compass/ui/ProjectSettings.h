/*
 * ProjectSettings.h
 *
 *  Created on: Apr 8, 2010
 *      Author: tdillig
 */

#ifndef PROJECTSETTINGS_H_
#define PROJECTSETTINGS_H_

#include "Identifier.h"
#include <set>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

using namespace std;


class ProjectSettings
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & excluded_units;
		ar & entry_point;
	}

public:
	set<Identifier> excluded_units;
	Identifier entry_point;
public:
	ProjectSettings();
	virtual ~ProjectSettings();
};

#endif /* PROJECTSETTINGS_H_ */
