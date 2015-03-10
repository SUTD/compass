#ifndef ASSEMBLY_H_
#define ASSEMBLY_H_


#include "instruction.h"
#include <vector>

using namespace std;

namespace il
{
class expression;

/**
 * \brief Inline assembly
 */

class assembly : public il::instruction
{

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::instruction>(*this);
        ar & asm_instruction;
        ar & inputs;
        ar & outputs;
        ar & clobber_registers;
    }


private:
	string asm_instruction;
	vector<expression*> inputs;
	vector<expression*> outputs;
	vector<string> clobber_registers;

public:
	assembly();

	/**
	 *
	 * @param asm_instruction: string representation of a basic inline
	 * assembly instruction
	 * @param inputs input registers
	 * @param outputs output registers
	 * @param clobber_registers clobbered registers
	 */
	assembly(string asm_instruction,
			vector<expression*> & inputs, vector<expression*> &outputs,
			vector<string> & clobber_registers, location loc);
	virtual ~assembly();
	virtual void print();
	virtual string to_string() const;

	string get_instruction();
	vector<expression*>& get_input_registers();
	vector<expression*>& get_output_registers();
	vector<string>& get_clobber_registers();


};

}

#endif /*ASSEMBLY_H_*/
