/*
 * compass_assert.h
 *
 *  Created on: Mar 9, 2010
 *      Author: tdillig
 */

#ifndef COMPASS_ASSERT_H_
#define COMPASS_ASSERT_H_

#include <assert.h>
#include <string>

using namespace std;

//#define ENABLE_EXTRA_CONTEXT 1



#define c_assert(expr)							\
  ((expr)								\
   ? __ASSERT_VOID_CAST (0)						\
   : __compass_assert_fail (__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION))

#define c_warn(expr)							\
  ((expr)								\
   ? __ASSERT_VOID_CAST (0)						\
   : __compass_assert_warn (__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION))



#define JOIN(x, y)  JOIN_AGAIN(x, y)
#define JOIN_AGAIN(x, y)  x ## y
#define UNIQUENAME(prefix)  JOIN(prefix, __COUNTER__)



#define assert_context(ctx) _assert_context UNIQUENAME(_ac_)(ctx)

#ifdef ENABLE_EXTRA_CONTEXT
#define assert_context_opt(ctx) (assert_context(ctx));
#endif

#ifndef ENABLE_EXTRA_CONTEXT
#define assert_context_opt(ctx)
#endif






void push_context(const std::string & ctx);
extern void pop_context();
extern void clear_context();

class _assert_context
{
public:
	inline _assert_context(const std::string & ctx)
	{
		push_context(ctx);
	}
	inline ~_assert_context()
	{
		pop_context();
	}

};


extern void __compass_assert_fail(const char* exp, const char* file, int line,
		const char* fn)
 __attribute__ ((__noreturn__));

extern void __compass_assert_warn(const char* exp, const char* file, int line,
		const char* fn);




#endif /* COMPASS_ASSERT_H_ */
