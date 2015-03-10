/*
 * util.h
 *
 *  Created on: Sep 1, 2008
 *      Author: tdillig
 */

#ifndef MISTRAL_UTIL_H_
#define MISTRAL_UTIL_H_

#include <string>
#include <vector>
#include <stdlib.h>
using namespace std;

#include "util.h"



inline bool have_same_sign(long int a, long int b)
{
	long int temp = a ^ b;
	return !(temp & (1L << 63));

}


inline long int gcd(long int _a, long int _b)
{
	long int a = labs(_a);
	long int b = labs(_b);

	int t;
	while(b!=0){
		t = a;
		a = b;
		b = t % b;
	}
	return a;
}

inline long int lcm(long int a, long int b)
{
	long int d = gcd(a, b);
	return labs(a*b/d);
}



#endif /* MISTRAL_UTIL_H_ */
