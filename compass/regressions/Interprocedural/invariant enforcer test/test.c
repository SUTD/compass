// Coreutils/rmdir final 


/* rmdir -- remove directories

   Copyright (C) 90, 91, 1995-2002, 2004-2008 Free Software
   Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Options:
   -p, --parent		Remove any parent dirs that are explicitly mentioned
			in an argument, if they become empty after the
			argument file is removed.

   David MacKenzie <djm@ai.mit.edu>  */

#include <stdlib.h>


typedef unsigned int bool;
#define true 1
#define false 0

bool verbose = false;
#define stdout NULL
#define stderr NULL

int x;
int errno;


//--------------------------------------
//Library stubs

char* strrchr(char* s, char c)
{
	assume(x>=0);
	assume(x<buffer_size(s)-1);
	buffer_safe(s, x+1);
	if(rand()) return NULL;
	return &s[x];
}





int errno;
int opterr;
int optind = 0;
char* optarg;


//---------------------------------------
/* If true, don't treat failure to remove a nonempty directory
   as an error.  */
static bool ignore_fail_on_non_empty;



/* Return true if an rmdir failure with errno == error_number
   for DIR is ignorable.  */
static bool
ignorable_failure ()
{
  return (ignore_fail_on_non_empty && (foo()));
}



/* Remove any empty parent directories of DIR.
   If DIR contains slash characters, at least one of them
   (beginning with the rightmost) is replaced with a NUL byte.
   Return true if successful.  */

static bool
remove_parents (char *dir)
{
  
  char *slash;
  bool ok = true;

  while (1)
    {
      slash = strrchr (dir, '/');
      if (slash == NULL)
	break;


      ok = foo();

      if (!ok)
	{
	  // Stop quietly if --ignore-fail-on-non-empty. 
	  if (ignorable_failure ())
	    {
	      ok = true;
	      static_assert(0); // shold fail
	    }
	  break;
	}
    }
  return ok;
}