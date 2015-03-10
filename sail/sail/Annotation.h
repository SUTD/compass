/*
 * Annotation.h
 *
 *  Created on: Jan 16, 2013
 *      Author: tdillig
 */

#ifndef ANNOTATION_H_
#define ANNOTATION_H_
#include "Serializable.h"
#include <string>

namespace sail {

class Annotation {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{

	}
public:
	Annotation();
	virtual ~Annotation();
	virtual string to_string() const = 0;
};

class FlowAnnotation:public Annotation
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<Annotation>(*this);
		ar & to;
		ar & from;
		ar & through;
	}
private:
	string to;
	string from;
	string through;
public:
	FlowAnnotation(const string & to, const string & from, const string & through);
	FlowAnnotation(){};
	const string & get_to() const;
	const string & get_from() const;
	const string & get_through() const;
	virtual string to_string() const;
};

} /* namespace sail */
#endif /* ANNOTATION_H_ */
