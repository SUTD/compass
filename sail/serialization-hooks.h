/*
 * serialization-hooks.h
 *
 *  Created on: Apr 10, 2010
 *      Author: tdillig
 */

#ifndef SERIALIZATIONHOOKS_H_
#define SERIALIZATIONHOOKS_H_

#include <set>
using namespace std;

typedef void (*callback_t)();



void register_begin_callback(callback_t fn);
void register_end_callback(callback_t fn);

bool in_serialization();


void begin_serialization();
void end_serialization();


#endif /* SERIALIZATIONHOOKS_H_ */
