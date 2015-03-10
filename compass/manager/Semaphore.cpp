/*
 * Semaphore.cpp
 *
 *  Created on: Aug 15, 2008
 *      Author: tdillig
 */

#include "Semaphore.h"

Semaphore::Semaphore() {
	i=0;

}

Semaphore::Semaphore(int count) {
	i=count;

}
void Semaphore::signal()
{
	unique_lock<mutex> lock(m);
	i++;
	if(i>0) cv.notify_one();
}

void Semaphore::wait()
{
	unique_lock<mutex> lock(m);
	if(i>0) {
		i--;
		return;
	}
	do{
		cv.wait(lock);
	} while(i<=0);
	assert(i>0);
	i--;
}

void Semaphore::wait_and_clear()
{
	unique_lock<mutex> lock(m);
	if(i>0) {
		i = 0;
		return;
	}
	cv.wait(lock);
	assert(i>0);
	i = 0;
}

void Semaphore::down()
{
	unique_lock<mutex> lock(m);
	i--;
}

int Semaphore::count()
{
	unique_lock<mutex> lock(m);
	return i;
}

Semaphore::~Semaphore() {
	// TODO Auto-generated destructor stub
}
