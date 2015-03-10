/*
 * allocator.h
 *
 *  Created on: Aug 18, 2008
 *      Author: tdillig
 */

#include <new>

/*
//scalar, throwing new and it matching delete
void* operator new (std::size_t size) throw(std::bad_alloc);
void operator delete (void* ptr) throw();

//scalar, nothrow new and it matching delete
void* operator new (std::size_t size,const std::nothrow_t&) throw();
void operator delete (void* ptr, const std::nothrow_t&) throw();

//array throwing new and matching delete[]
void* operator new  [](std::size_t size) throw(std::bad_alloc);
void operator delete[](void* ptr) throw();

//array, nothrow new and matching delete[]
void* operator new [](std::size_t size, const std::nothrow_t&) throw();
void operator delete[](void* ptr, const std::nothrow_t&) throw();
*/
