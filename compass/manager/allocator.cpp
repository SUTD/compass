/*
 * allocator.c
 *
 *  Created on: Aug 18, 2008
 *      Author: tdillig
 */

#include "allocator.h"
#include <iostream>

using namespace std;
#include "malloc.h"
/*

//scalar, throwing new and it matching delete
void* operator new (std::size_t size) throw(std::bad_alloc)
{
	return malloc(size);

}
void operator delete (void* ptr) throw()
{
	free(ptr);
}

//scalar, nothrow new and it matching delete
void* operator new (std::size_t size,const std::nothrow_t&) throw()
{
	return malloc(size);
}
void operator delete (void* ptr, const std::nothrow_t&) throw()
{
	free(ptr);
}

//array throwing new and matching delete[]
void* operator new  [](std::size_t size) throw(std::bad_alloc)
{
	return malloc(size);
}
void operator delete[](void* ptr) throw()
{
		free(ptr);
}

//array, nothrow new and matching delete[]
void* operator new [](std::size_t size, const std::nothrow_t&) throw()
{
	return malloc(size);
}
void operator delete[](void* ptr, const std::nothrow_t&) throw()
{
		free(ptr);
}
*/
