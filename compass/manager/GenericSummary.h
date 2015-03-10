/*
 * GenericSummary.h
 *
 *  Created on: Aug 15, 2008
 *      Author: tdillig
 */

#ifndef GENERICSUMMARY_H_
#define GENERICSUMMARY_H_
#include "Summary.h"




class GenericSummary: public Summary {
public:
	GenericSummary();
	virtual bool operator==(const Summary& other);
	virtual ~GenericSummary();
	string data;
private:


};

#endif /* GENERICSUMMARY_H_ */
