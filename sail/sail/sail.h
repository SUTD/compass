/*
 * sail.h
 *
 *  Created on: Jul 22, 2008
 *      Author: isil
 */

#include "AddressLabel.h"
#include "AddressVar.h"
#include "ArrayRefRead.h"
#include "ArrayRefWrite.h"
#include "Assembly.h"
#include "Assignment.h"
#include "BasicBlock.h"
#include "Binop.h"
#include "Block.h"
#include "Branch.h"
#include "Cast.h"
#include "Cfg.h"
#include "CfgEdge.h"
#include "Constant.h"
#include "FieldRefRead.h"
#include "FieldRefWrite.h"
#include "Function.h"
#include "FunctionCall.h"
#include "FunctionPointerCall.h"
#include "Instruction.h"
#include "Jump.h"
#include "Label.h"
#include "Load.h"
#include "Loop.h"
#include "LoopInvocationInstruction.h"
#include "SaveInstruction.h"
#include "Store.h"
#include "SuperBlock.h"
#include "Symbol.h"
#include "Serializer.h"
#include "T1T2Block.h"
#include "TranslationUnit.h"
#include "Unop.h"
#include "Variable.h"
#include "AddressString.h"
#include "DropVariable.h"
#include "StaticAssert.h"
#include "Assume.h"
#include "AssumeSize.h"
#include "InstanceOf.h"
#include "ExceptionReturn.h"

#include "Serializable.h"


/**
 * \mainpage SAIL: Static Analysis Intermediate Language
 * \image html sailboat-small.png
 *
 * \section intro_sec Introduction
 * SAIL is a front-end for program analysis systems that provides a two-level
 * representation, consisting of both a language-specific high-level intermediate
 * language as well as a  low-level, language-independent representation. SAIL
 * provides a precise mapping from the low-level instructions to the high-level
 * expressions and statements used in the source code, greatly simplifying tasks
 * such as error reporting and relating reasoning on the low-level representation
 * back to the original source code. SAIL also provides support for control
 * flow graph construction as well as compact and efficient serialization.
 * SAIL currently parses C and  C++ code and uses
 * GCC 4.3.4 to generate its high-level intermediate representation.
 *
 * \section req_section Requirements
 * SAIL has only been tested on 64-bit Linux. Since SAIL is based on GCC 4.3.4,
 * you must be able to build GCC to build SAIL. GCC requires a few unusual dependencies
 * whose installation will vary depending on your Linux distribution. Assuming that basic
 * development support is already installed, on Ubuntu/Kubuntu 9.10
 * the following command should install the required extra dependencies:
\verbatim
sudo apt-get install libc6-dev-i386 gettext gawk flex libmpfr-dev cmake
\endverbatim

In addition to these GCC requirements, SAIL uses the C++ library
Boost Serialization to read and write information to disk. On Ubuntu/Kubuntu
the following command should install the required library (lboost_serialization):
\verbatim
sudo apt-get install libboost-serialization-dev
\endverbatim


\section sail Getting SAIL
You can download SAIL from http://www.cs.wm.edu/~idillig/sail/sail-0.4.tar.gz . SAIL
is released under the BSD license; however note that GCC (and the modified GCC obtained
be linking SAIL with GCC) is still covered under GPL. All files in the
gcc folder are licensed under the GPL license as well.

 \section install_sec Installation
 SAIL is based on GCC 4.3.4. You can obtain this version of gcc, for example,
 from ftp://mirrors.kernel.org/gnu/gcc/gcc-4.3.4/gcc-core-4.3.4.tar.gz .
 You must also add the language component for C++.

To download and set-up GCC, do the following:

\verbatim

wget ftp://mirrors.kernel.org/gnu/gcc/gcc-4.3.4/gcc-core-4.3.4.tar.gz
wget ftp://mirrors.kernel.org/gnu/gcc/gcc-4.3.4/gcc-g++-4.3.4.tar.gz

#unzip gcc-core
tar xf gcc-core-4.3.4.tar.gz

#unzip g++
tar xf gcc-g++-4.3.4.tar.gz
\endverbatim

Being based on GCC makes the build process just a little longer, but all the steps
are listed below.

 The tar.gz file contains the following:
 \li Folder \p framework/gcc. This folder will replace some files in the
 \p gcc subfolder of GCC 4.3.4.
 \li Folder \p framework/sail. This is the project building \p libsail.a, the static library
 that both GCC and the application using SAIL link against.

SAIL uses cmake to build, which you might need to install on your system.
 First, build \p sail by issuing \p the following commands in the
 \p framework folder.
 \verbatim
 mkdir build
 cd build
 cmake ../sail/
 make
\endverbatim

 Now, go into gcc.
\verbatim
cd gcc-4.3.4
cd gcc
\endverbatim

 First, delete all the files we will be replacing in the \p gcc
 subfolder
\verbatim
rm c-decl.c c-gimplify.c common.opt Makefile.in
cd cp
rm Make-lang.in
rm decl2.c
rm decl.c
\endverbatim

Now, link all files in the gcc subfolder:
\verbatim
ln -s ../../../framework/gcc/cp/Make-lang.in Make-lang.in
ln -s ../../../framework/gcc/cp/decl2.c decl2.c
ln -s ../../../framework/gcc/cp/decl.c decl.c
cd ..
ln -s ../../framework/gcc/c-decl.c c-decl.c
ln -s ../../framework/gcc/c-gimplify.c c-gimplify.c
ln -s ../../framework/gcc/common.opt common.opt
ln -s ../../framework/gcc/Makefile.in Makefile.in

ln -s ../../framework/gcc/il-c.h il-c.h
ln -s ../../framework/gcc/il-c.c il-c.c
ln -s ../../framework/gcc/il-main.h il-main.h
ln -s ../../framework/gcc/il-main.cpp il-main.cpp
\endverbatim

and continue with symlinks for the \p libsail.a and the \p sail folder:
\verbatim
ln -s ../../framework/build/libsail.a libsail.a
ln -s ../../framework/sail/ sail
cd ..
\endverbatim


Assuming that the modified \p gcc that can optionally emit SAIL instruction
should live in \p /abspath/gcc, configure \p  gcc with the following options:
\verbatim
./configure --enable-stage1-languages=all --disable-bootstrap --prefix=/abspath/gcc --enable-checking=no --disable-stage1-checking
\endverbatim

Then, build \p gcc with
\verbatim
make
\endverbatim
and install into your directory with
\verbatim
make install
\endverbatim

\section running-section Running GCC with SAIL
To compile any program using the modified \p gcc, simply add the flag
\p --sail to \p gcc if you want to emit SAIL files. One \p .sail file
will be generated per funcion in a folder called \p sail. Note that SAIL does
not interfere with the normal build process; any standard makefile will still
work. There will only be a filled \p sail folder afterwards.

To test if SAIL-gcc works, go to the folder \p /abspath/gcc/bin and create a
file \p test.c with the following content:
\verbatim
void foo()
{
	int x = 3;
}
\endverbatim

Now run
\verbatim
 ./gcc -c --sail=./sail/ test.c
\endverbatim
You should see no error message and a \p sail folder should have been created
in the current directory. This directory contains one (binary) \p .sail file with the high and low-level
intermediate language and CFG representation per function.

We recommend that no optimizations are enabled when invoking SAIL's
version of gcc; otherwise, GCC performs various transformation during its
 parsing phase that cause SAIL's high-level language to resemble the original
 source code less closely.


\section using-section Using SAIL
This section describes how to include SAIL into your project and read the serialized
\p .sail files. Since SAIL is written in C++, it can only be included from C++ code.
For this, your project must include the following folders as
include directories (\o-I option on GCC):
\verbatim
sail, sail/il, sail/sail
\endverbatim
Furthermore, you must link against \p libsail.a and \p lboost_serialization.
How to do this will depend on your application/build system. A typical example
command will look somthing like this:
\verbatim
g++ -lboost_serialization -I.. -I../sail -I../il  mysail.cpp libsail.a -o mysail
\endverbatim

Here is the layout of a simple example application that opens the serialized file
\p test.c/test.c:foo.sail that was created using \p test.c from above:

\verbatim
 // All of the high-level intermediate language headers are combined in sail.h
#include "sail.h"

// All of the low-level intermediate language headers are combined in sail.h
#include "il.h"

using namespace sail; //namespace for high-level il
using namespace il; //namespace for low-levl il
#include <iostream>

#include "Serializer.h"
#include "Function.h"
#include "Cfg.h"

using namespace std;

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		cout << "No .sail file provided. " << endl;
		exit(1);
	}
	sail::Function* f = sail::Serializer::load_file(argv[1]);
	assert(f != NULL);
	cout << "CFG" << endl << f->get_cfg()->to_dotty(false) << endl;

}
\endverbatim

All the classes implemented inside the namespace il are associated with SAIL's
high-level representation while all the classes implemented in namespace SAIL
correspond to the low-level representation. For any instruction in the low-level
language, it is possible to obtain the corresponding construct in the high-level
representation by calling the get_original_node() function implemented by every
instruction in the low-level language.

By default, Load, Store, ArrayRefRead, and ArrayRefWrite instructions all include
field offsets; however, it is possible to generate low-level instructions where
field offsets are only allowed to appear in FieldRefRead and FieldRefWrite
instructions. To disable offsets in Load, Store, ArrayRefRead, and ArrayRefWrite
instructions, just set the flag ALLOW_OFFSETS defined in Function.cpp to false.
As an example, consider the C code:
\verbatim
x->f = 2;
\endverbatim
With the default setting where ALLOW_OFFSETS is true, SAIL's low-level language
includes a single Store instruction that specifies an offset
 associated with the byte offset of field f in struct x. If ALLOW_OFFSETS is disabled,
SAIL's low-level language will first generate a pointer arithmetic (Binop) instruction
 followed by a Store instruction.

While SAIL's high-level language includes different looping constructs, such as
for or while loop, GCC replaces different looping constructs with goto statements
during parsing. As a result, SAIL currently does not
generate any syntactic looping constructs in its high-level representation.


\section notes Further notes
SAIL currently has been tested on OpenSSH, OpenSSL and SAMBA as well as
many smaller applications. However, it has not yet been used on larger applications
and there are more likely still some bugs left in SAIL. If you run into any issues,
 feel free to contact us at the email addresses listed below.
Any bug report accompanied by a patch is especially appreciated!

\section contact Contact
SAIL is developed by
Thomas Dillig (http://www.stanford.edu/~tdillig) and
Isil Dillig (http://www.stanford.edu/~isil). You can reach us at tdillig@stanford.edu
and isil@stanford.edu.









 */
