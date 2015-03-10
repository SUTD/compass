/*
 * hash.cpp
 *
 *  Created on: Apr 11, 2010
 *      Author: tdillig
 */
#include "hash.h"

namespace il {
size_t string_hash(const string &s)
{
    size_t h = 0;
    for (unsigned int i = 0; i < s.length(); ++i)
        h = 31 * h + s[i];
    return h;
}

}
