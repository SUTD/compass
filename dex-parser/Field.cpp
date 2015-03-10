#include"Field.h"
Field::Field (	const string& classowner,
		unsigned int classowneridx, 
		const string& name, 
		const string& type,
		unsigned int typeidx,
		string flag,
		unsigned int byteoff, 
		unsigned int nextoff ): _name(name), _class_owner(classowner), _class_owner_idx(classowneridx), _type(type), _type_idx(typeidx),_flag(flag), _byte_off(byteoff), _next_off(nextoff)
{}

string Field::getName()
{
	return _name;
}

string Field::getClassOwner()
{
	return _class_owner;
}

string Field::getType()
{	
	return _type;
}

string Field::toString()
{
	return getClassOwner()+"->"+getName()+": "+getType();
}


unsigned int Field::setNextOff(string type) //to make our life easy, most of the primitive type int,long,double,short, byte all maps to int
{
	//if ((type == "Z") ||(type == "B")) {//boolean or byte
	if (type == "B") {//boolean or byte
		_next_off = 4; 
		return 4;
	} else if (type == "C"){ // short or char
		_next_off = 2; 
		return 2;
	}else if ( (type == "I") ||(type == "F") ||(type == "Z")||(type == "S")) { // int or float and compass deal with bool as an interger
		_next_off = 4; 
		return 4;
	}else if ( (type == "J") ||(type == "D") ) { //long or double
		//_next_off = 8; 
		_next_off = 4; 
		return 4;
	}else {
		_next_off = 8; 
		return 8;
	}
}

unsigned int Field::getOffset() //set it as a bit offset
{
	return _byte_off*8;
}

//added for DebugInfo
void Field::setFlag(string flag)
{
	_flag = flag;
}

string Field::getFlag()
{
	return _flag;
}

void Field::setCurrOff(unsigned int cur)
{
	_byte_off = cur;
}



//hzhu added 4.25.2012
unsigned int Field::getClassOwnerIdx()
{
	return _class_owner_idx;
}
unsigned int Field::getTypeIdx()
{
	return _type_idx;
}
        //hzhu end 4.25.2012 

string Field::getFieldName(){
	return getClassOwner()+"->"+getName();
}
