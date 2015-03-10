/*
 * Semaphore.h
 *
 *  Created on: Aug 15, 2008
 *      Author: tdillig
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <boost/thread.hpp>
using namespace boost;
using namespace std;

class Semaphore {
private:
	mutex m;
	condition_variable_any cv;
	int i;
public:
	Semaphore();
	Semaphore(int count);
	void signal();
	void wait();

	// Wait for i to be greater than 0, but sets it to 0 afterwards.
	void wait_and_clear();
	// Decrements the semaphore value, but doesn't wait on it.
	void down();
	virtual ~Semaphore();
	int count();
};

#endif /* SEMAPHORE_H_ */
