#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "DexFileReader.h"
#include "DebugInfoReader.h"
#include <stdio.h>
#include <math.h>
#include "DexOpcode.h"
#include "OpcodeFormat.h"
#include "InstructionProcess.h"
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include "Ins2Debug.h"
#include "Graph.h"
#include "GraphUtil.h"

using namespace std;


#include "sail.h"
#include "type.h"
#include "TypeMapping.h"
#include "InstructionMapping.h"
#include "function_declaration.h"
#define TRIES_CHECK false
#define METHOD_THROW_EXCEPTIONS false
#define SELF_DEFINED_METHODS false
#define PRINT_SYNC_INFO false
#define PRINT_TRIES true
#define CLASS_HIERARCHY false
#define PRINT_FUNSIG false
#define CHECK_SELF_DEFINE_CLASS false
#define CHECK_CLASS_FIELD false
#define CHECK_CLASSDEF false
#define DEBUG false
#define DEBUG_MD true
#define CHECK_UNRESOLVED_TYPE false
#define CLINIT_PRINT false
#define PRINT_CFG true
#define DELETE_CODEH true

void printCFG(sail::Function* f);
sail::TranslationUnit* dex_to_sail(const string & dex_file);
void SerializeToSailFile(sail::TranslationUnit* transliation_unit, const string & out_put);
