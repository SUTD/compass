/*
 * language.h
 *
 *  Created on: Apr 19, 2010
 *      Author: tdillig
 */

#ifndef LANGUAGE_H_
#define LANGUAGE_H_

enum language_type {
	LANG_C = 0,
	LANG_CPP = 1,
	LANG_JAVA = 2,
	LANG_UNSUPPORTED = 3,
	
};
typedef enum language_type lang_type;


#endif /* LANGUAGE_H_ */
