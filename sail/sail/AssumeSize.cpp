/*
 * AssumeSize.cpp
 *
 *  Created on: Apr 10, 2009
 *      Author: tdillig
 */

#include "AssumeSize.h"
#include "Symbol.h"
#include "node.h"
#include "type.h"
#include "expression.h"

namespace sail {

AssumeSize::AssumeSize()
{
	original = NULL;
	inst_id = ASSUME_SIZE;
	line = -1;
}

AssumeSize::AssumeSize(Symbol*  buffer, Symbol*  size, il::node* original,
		int line)
{
	this->original = original;
	inst_id = ASSUME_SIZE;
	this->buffer = buffer;
	this->size = size;
	this->line = line;
}

AssumeSize::~AssumeSize()
{

}


Symbol* AssumeSize::get_buffer()
{
	return buffer;
}

Symbol* AssumeSize::get_size()
{
	return size;
}


string AssumeSize::to_string() const
{
	return "assume_size(" + buffer->to_string()+ ", " + size->to_string() + ")";
}
string AssumeSize::to_string(bool pretty_print) const
{
	return "assume_size(" + buffer->to_string(pretty_print)+ ", " +
	size->to_string(pretty_print) + ")";
}
il::node* AssumeSize::get_original_node()
{
	return original;
}

bool AssumeSize::is_save_instruction()
{
	return false;
}


bool AssumeSize::is_synthetic()
{
	return true;
}


bool AssumeSize::is_removable()
{
	return false;
}


}
