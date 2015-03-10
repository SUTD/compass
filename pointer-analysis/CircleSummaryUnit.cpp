#include "CircleSummaryUnit.h"



void CircleSummaryUnit::add_summary_unit(SummaryUnit* sum)
{
	su.push_back(sum);
	int inst_size = sum->get_equivalent_insts().size();
	inst_size_list.push_back(inst_size);
}

CircleSummaryUnit& CircleSummaryUnit::operator= (
		const CircleSummaryUnit& other)
{
	cout << "calling assignment overload function ! " << endl;
	this->su.clear();
	this->su.insert(this->su.begin(),
			other.su.begin(),
			other.su.end());

	this->inst_size_list.clear();
	this->inst_size_list.insert(this->inst_size_list.begin(),
			other.inst_size_list.begin(),
			other.inst_size_list.end());
	return *this;
}


bool CircleSummaryUnit::operator==(
		const CircleSummaryUnit& other) const
{

	if(this->inst_size_list.size() != other.inst_size_list.size()){
		cout << "CircleSummaryUnit::operator == " << endl;
		cout << "Size 1 " << this->inst_size_list.size() << endl;
		cout << "Size 2 " << other.inst_size_list.size() << endl;
		assert(false);
	}
	assert(this->inst_size_list.size() == other.inst_size_list.size());

	for(int i = 0; i < (int) this->inst_size_list.size(); i++){
		int self_size = this->inst_size_list.at(i);
		int other_size = other.inst_size_list.at(i);

		if(self_size == other_size)
			continue;
		else
			return false;
	}
	return true;

}



string CircleSummaryUnit::to_string() {
	string str = "ALL circle fix point :: ";
	str += "\n";

	for (int i = 0; i < (int) su.size(); i++) {
		str += "	== Element ";
		str += int_to_string(i);
		str += "	Total insts :: ";
		str += int_to_string(su.at(i)->get_equivalent_insts().size());
		str += "\n";
		str += su.at(i)->to_string();
		str += "//------------------ ";
		str += "\n";
	}

	return str;
}
