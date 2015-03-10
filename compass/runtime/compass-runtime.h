/*
 * compass-runtime.h
 *
 *  Created on: Feb 26, 2010
 *      Author: tdillig
 *
 *  This file is included on the top of every file compiled through the
 *  SailGenerator.
 */

#ifndef COMPASSRUNTIME_H_
#define COMPASSRUNTIME_H_

static void static_assert(int assert_exp)
{

}

static void observed(int assert_exp)
{

}


static void check_buffer()
{

}

static void check_null()
{


}

static void check_uninit()
{


}

static void check_cast()
{


}

static void check_memory_leak()
{

}

static void check_double_delete()
{

}

static void check_deleted_access()
{

}

static void assume(int pred)
{


}



static int static_choice()
{
	// This is irrelevant; we just place it here so gcc does not
	// warn us.
	return 0;

}

#ifdef __cplusplus
#include "spec_vector.h"
#include "spec_map.h"
#include "spec_string.h"
#include "spec_set.h"
#include "spec_list.h"
#include "spec_stack.h"
#include "spec_queue.h"

#endif

#endif /* COMPASSRUNTIME_H_ */
