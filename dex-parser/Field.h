#ifndef _FIELD_H
#define _FIELD_H

#include <string>
using namespace std;

class Field
{
public:
	//Field(const string& classowner, const string& name, const string& type,string flag = "", unsigned int curroff= 0, unsigned int nextoff = 0);
	Field(){}
	Field(const string& classowner, unsigned int classowneridx, const string& name, const string& type, unsigned int typeidx,string flag = "", unsigned int curroff= 0, unsigned int nextoff = 0);
	~Field();
	string getName();
	string getClassOwner();
	string getType();
	string toString();
	void setCurrOff(unsigned int);
	unsigned int setNextOff(string ); //added for DebugInfo
	//getOffset() designed for sail
	unsigned int getOffset();
	//end of getOffset();
	void setFlag(string flag); //added for DebugInfo
	string getFlag();
	
	//hzhu added 4.25.2012
	unsigned int getClassOwnerIdx();
	unsigned int getTypeIdx();
	//hzhu end 4.25.2012
	void setClassOwner(string classowner) {this->_class_owner = classowner;}

	//1/9
	string getFieldName();
	//1/9
private:
	string _name;
	string _class_owner;

	//hzhu added 4.25.2012
	unsigned int  _class_owner_idx; 
	//hzhu end 4.25.2012
	
	string _type; // the type of the field

	//hzhu added 4.25.2012
        unsigned int  _type_idx; 
        //hzhu end 4.25.2012

	string _flag; // Added for DebugInfo
	unsigned int _byte_off; //added for DebugInfo
	unsigned int _next_off;
};
#endif
