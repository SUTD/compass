
#ifndef HASH_H_
#define HASH_H_

#include <string>
using namespace std;

namespace il {

/**
 * Utility string hash function, just like OpenJDK's java.lang.String
 */
size_t string_hash(const string &s);

}

#endif /* HASH_H_ */
