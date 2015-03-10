Installation instructions:
---------------------------

- First install all the required dependencies. For Ubunut/Kubuntu, they are all listed in the file apt-get-list.txt
- Then, go to the build folder and type:
cmake ..
make

- Go to the mail directory and type:
./install-sail-gcc CUSTOM_GCC_LOCATION 
where CUSTOM_GCC_LOCATION indicates the directory in which you want to have our modified version of gcc.

The abductor binary will be in build/abductor/abductor


Running Abductor:
-------------------

To run abductor, you first generate a .sail intermediate file from a .c file. To do this, type
CUSTOM_GCC_LOCATION/bin/gcc --sail=. myfile.c

For each function in myfile.c, this generates fun_name.sail file. Then, in this folder you call abductor as:

PATH_TO_ABDUCTOR_BIN/abductor . main


where the first argument specifies the location of the .sail files and the second argument is the name of the main function.


For your convenience, here is a script that analyzes all functions starting from the main function in a file given as argument:
----------------------
rm -rf *.sail
CUSTOM_GCC_LOCATION/bin/gcc --sail=. $1
PATH_TO_ABDUCTOR_BIN/abductor/abductor .
----------------------