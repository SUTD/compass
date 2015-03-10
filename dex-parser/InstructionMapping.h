#ifndef _INSTRUCTION_MAPPING_H
#define _INSTRUCTION_MAPPING_H
#include <fstream>
#include "sail.h"
#include "type.h"
#include "TypeMapping.h"
#include "string_const_exp.h"
#include "CodeHeader.h"
#include "StringSplit.h"
//#include "Ins2Debug.h"
#include "Int2Str.h"
using namespace std;

#define SAIL_INS_PRINT true
namespace il {
class string_const_exp;
class node;
}
class Ins2Debug;
class InstructionMapping
{
public:
    InstructionMapping(DexFileReader* dfr, map<string,ClassDefInfo*>*, il::type* ret_type);
    ~InstructionMapping();
    //1/5
    //void setsailInstruction(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int); //vector because of one dex instruction may have more than 1instructions
    void setsailInstruction(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int, bool);
    //1/5
    vector<sail::Instruction*>* getSailInstruction();
    sail::Label* get_label(const string& name);
    void initFields(vector<sail::Instruction*>* init_ins);



private:
    void InstructionMapping_constantAssignment(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void InstructionMapping_constantStringAddr(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void InstructionMapping_varAssignment(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void InstructionMapping_F10t(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void InstructionMapping_F10x(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void InstructionMapping_F11n(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void InstructionMapping_F11x(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void InstructionMapping_F12x_unop_neg(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, il::unop_type, unsigned int);
    void cast(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int,il::type*);
    //haiyan added 10.10
    void cast_assignment(sail::Variable* lhs, sail::Variable* rhs, il::string_const_exp* original, unsigned int line);
    //haiyan ended 10.10
    void InstructionMapping_F12x(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void InstructionMapping_F20t(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    //1/5 add the last parameter indicating if it is a clinit

    void InstructionMapping_F21c(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int, bool);
    //1/5
    void InstructionMapping_F21h(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F21s(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void branch(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void InstructionMapping_F21t(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F22b(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void load(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    //1/5 last parameter is clinit flag

    void store(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int, bool);
    //1/5
    void InstructionMapping_F22c(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void InstructionMapping_F22s(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F22t(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F22x(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void binop(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, il::binop_type, bool,bool,unsigned int);
    void cmp_branch(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F23x(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F30t(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F31c(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F31i(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void switch_branch(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void InstructionMapping_F31t(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F32x(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void New_functioncall(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    void storeField(Ins2Debug*, map<ASMInstruction*,DebugInfo*>* , ASMInstruction*, unsigned int);
    void store2array(Ins2Debug*, map<ASMInstruction*,DebugInfo*>* , ASMInstruction*, unsigned int);
    void loadFromArray(Ins2Debug*, map<ASMInstruction*,DebugInfo*>* , ASMInstruction*, unsigned int);
    void loadField(Ins2Debug*, map<ASMInstruction*,DebugInfo*>* , ASMInstruction*,unsigned int);
    void fillarray(Ins2Debug*, map<ASMInstruction*,DebugInfo*>* , ASMInstruction*, unsigned int);
    void functioncall(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F35c(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F3rc(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void InstructionMapping_F51l(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*,unsigned int);
    void setConstant(unsigned int reg_n ,int value);
    int getConstant(unsigned int);
    unsigned int getRandom();
    void add_labels(ASMInstruction*);
    sail::Instruction* getLastIns();
    void updateFunctionCallExps(unsigned int, ASMInstruction*);
    void printIns(ASMInstruction* asm_ins, unsigned int addr);
    void processExceptions(Ins2Debug*,ASMInstruction*, unsigned int addr,unsigned int line );
    map<string,ClassDefInfo*>* getTypeMap();
    void processAnnotationThrows(Ins2Debug*, ASMInstruction* asm_ins, unsigned int addr, unsigned int line,bool is_lib);
    void processTryCatch(Ins2Debug*, ASMInstruction*, unsigned int, unsigned int, bool has_annotation);
    //added by haiyan 9.27
    void processTryFinally(Ins2Debug* ins_debug, ASMInstruction* asm_ins, unsigned int addr, unsigned int line);
    void processTryFinallyExceptions(Ins2Debug* ins_debug, ASMInstruction* asm_ins, unsigned int addr, unsigned int line, bool is_lib);
    //ended by haiyan 9.27
    void processTryThrows(Ins2Debug*, ASMInstruction*, unsigned int, unsigned int, bool is_lib, bool has_annotation);
    vector<sail::Label*>* createLabelListForCatch(vector<unsigned int>*, bool has_finally, bool has_annotation);
    vector<pair<sail::Symbol*, sail::Label*>>* createTargets(vector<sail::Symbol*>*, vector<sail::Label*>*);
    void extendCatchAddrs(vector<unsigned int>*);
    void updateSailInstruction(); //for the line number
    void processInvariant(sail::Variable* in_def, sail::Variable* in_use, vector<sail::Symbol*>* in_uses, unsigned int startline); //process i = i+1, should be t = i+1; i =t;
    //void processLoadStoreInvariant(sail::Variable* in_def, sail::Variable* in_use, unsigned int startline, bool load, unsigned int offset);
    void InstructionMapping_arrayLength(Ins2Debug* ins_debug, map<ASMInstruction*,DebugInfo*>* instruction_debug, ASMInstruction* asm_ins,unsigned int addr);
    //haiyan added 9.30
    void updateAnotherBranchReturnType(unsigned int addr, unsigned int reg_num, il::type* type , sail::Variable* var);
    void addingOneAssignmentBeforeReturn(unsigned int addr, unsigned int line); //for jump instruction jumps to return ins;
    //haiyan ended 9.30

    //haiyan added 10.1 for const-class instruction
    //1/5 add last parameter indicating if it is a clinit
    void InstructionMapping_constClass(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    //1.5
    //haiyan ended 10.1 for const-class instruction
    //haiyan added 10.1 instance-of instruction
    void InstanceOf(Ins2Debug*, map<ASMInstruction*,DebugInfo*>*, ASMInstruction*, unsigned int);
    //haiyan ended 10.1 for instance-of instruction

    
    
    DexFileReader* _dfr;
    vector<sail::Instruction*>* _sail_ins;
    map<string, ClassDefInfo*>* _type_map; //function type not existing inside this mapping, we need to build them
    map<unsigned int,sail::Label*>* _label_list;
    map<string, unsigned int>* _static_field_addr; //for sget and sput ,string is the string for field, and unsigned int for the fake global addr
    //haiyan 5.26 added
    map<unsigned int, sail::Variable*>* _static_var;
    //haiyan 5.26 ended
    unsigned int _newest_addr;
    map<string,sail::Label*> *_str_label;
    //map<unsigned int, int>* _constant;
    unsigned int _random;
    int _backward_latest_line;


    sail::Variable* _tempvar_for_invariant;

    map<pair<sail::Variable*, unsigned int>, sail::Variable*>* _fields_temp_var_map; //exmaple<v0+4>->tempvar


    //haiyan added 9.30
    pair<unsigned int, il::type*>* _fun_declared_return_var;
    bool _return_ins_has_label;
    unsigned int _return_ins_addr;
    il::type* _different_branch_return_type;
    sail::Variable* _different_branch_return_var;
    sail::Assignment* _return_ins;
    //haiyan ended 9.30

    sail::Label* _label_for_annotation_throw;
    bool _annotation_labelled;
};
#endif
