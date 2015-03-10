/*
 * serializable_types.h
 *
 *  Created on: Apr 12, 2010
 *      Author: tdillig
 */

#ifndef SERIALIZABLE_TYPES_H_
#define SERIALIZABLE_TYPES_H_

enum sum_data_type {
	NO_DATA = -1,
	SAIL_FUNCTION,
	COMPASS_SUMMARY,
	SUM_END
};

static const char* file_extensions[] = {
		".sail",
		".compass"
};


#endif /* SERIALIZABLE_TYPES_H_ */
