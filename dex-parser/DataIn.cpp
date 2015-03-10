#include "DataIn.h"
using namespace std;


DataIn::DataIn()
{
	_barray = NULL;
	_pos = 0; 
}

char* DataIn::getByteArray(){
	return _barray;
}

int DataIn::readDexFile(string filename)
{
	//you have to check that file is open first. Otherwise,
	// stat will have no effect and the filesize will be uninitialized.
	ifstream inFile(filename.c_str());
	while(!inFile)
	{
		cerr<<"unable to open input file" << filename << "--bailing out!\n";
		return -1;
	}

	//read file size
	struct stat filestatus;
	stat(filename.c_str(), &filestatus);
	int filesize = filestatus.st_size;
#ifdef DEBUG
	cout << filesize << endl;
	cout << "pay attention" << endl;
#endif
	//assign the space
	_barray = new char[filesize];
	// start to read file


	char ch;
	char *temp = _barray;

	while(inFile.get(ch))
	{
		*(temp++) = ch;
	}

#ifdef DEBUG
	char* end = temp;
	temp = _barray;
	int i = 0;
	while(temp != end)
	{	
		i += 1;
		printf("%.2x\t", (unsigned char)*(temp++));
		if (i%16 == 0)
			cout << endl;
	}
#endif
	//close file
	inFile.close();
	return 0;
}

DataIn::~DataIn()
{
	delete[] _barray;
}

 	
int DataIn::getCurrentPosition()
{
#ifdef 	DEBUG
	cout << "current position is: " << this->_pos <<endl;
#endif
	return this->_pos;
}
	
void DataIn::move(int absoffset)
{
	this->_pos = absoffset;
}

				
void DataIn::pop()
{
	if (!_stack.empty()){
		this->_pos = this->_stack.top();
#ifdef 	DEBUG
		cout << "position after pop's return value is:  " << _pos << endl;
		printf("%x hex \n", _pos);
#endif
		this->_stack.pop();
	}else{
		cout << "cannnot pop. The stack is empty" << endl;
		exit(0);
	}
}


void DataIn::push()
{
	this->_stack.push(this->_pos);
}

	
	// push + move
void DataIn::pushMove(int absoffset)
{
//	cout << "absoffset: " << absoffset << endl;
	push();
	move(absoffset);
}


char DataIn::readByte()
{	

#ifdef DEBUG
	printf("%.2x\t", (unsigned char)*(_barray+_pos));
#endif
       	return _barray[_pos ++];
}
	
int DataIn::readIntx()
{	
	char*p = & _barray[_pos];
	_pos = _pos + 4;
	return	*(int*)p;
}
unsigned int DataIn::readUIntx()
{	
	char*p = & _barray[_pos];
	_pos = _pos + 4;
	return	*(unsigned int*)p;
}



long DataIn::readLongx()
{
	char*p = & _barray[_pos];
	_pos = _pos + 8;
	return *(long*)p;
}


int DataIn::readShortx()
{
	char*p = & _barray[_pos];
	_pos = _pos + 2;
	return *(short*)p;
}

unsigned int DataIn::readUShortx()
{
	char*p = & _barray[_pos];
	_pos = _pos + 2;
	return	*(unsigned short*)p;
}

float DataIn::readFloatx()
{
	char*p = & _barray[_pos];
	 _pos = _pos + 4;
	return *(float*)p;
}
double DataIn::readDoublex()
{
	char*p =& _barray[_pos];
	_pos = _pos+8;
	return *(double*)p;
	
}
////////////////////////////////////////////////////////
//four-byte bit pattern, zero-extended to the right, and interpreted as an IEEE754 32-bit floating point value
float DataIn::readVariantFloatx(unsigned int bytes){
	char*p = &_barray[_pos];
	_pos = _pos + bytes;

	//4 bytes
	char q[4];
	memcpy(&q[4-bytes],p,bytes);
	feedArray(q,bytes, 4);
	return *(float*)q;
}
//eight-byte bit pattern, zero-extended to the right, and interpreted as an IEEE754 32-bit floating point value
double DataIn::readVariantDoublex(unsigned int bytes){
	char*p = &_barray[_pos];
	_pos = _pos + bytes;

	//8 bytes
	char q[8];
	memcpy(&q[8-bytes], p, bytes);
	feedArray(q, bytes, 8);
	return *(double*)p;
}
///////////////////////////////////////////////////////

int DataIn::bigEndian_readShortx()
{	
	unsigned short low = (unsigned short) readByte();
	unsigned short high =(unsigned short) readByte();	
	return (low<< 8) | high;
}
int DataIn::bigEndian_readIntx()
{
	unsigned int first = (unsigned int) readByte(); 
	unsigned int second = (unsigned int) readByte(); 
	unsigned int third = (unsigned int) readByte(); 
	unsigned int forth = (unsigned int) readByte(); 
	
	return (first << 24) | (second << 16)| (third<< 8) | forth;
}

long DataIn::bigEndian_readLongx()
{
	unsigned long first = (unsigned long) readByte();//56
        unsigned long second = (unsigned long) readByte();//48
        unsigned long third = (unsigned long) readByte(); //40
        unsigned long forth = (unsigned long) readByte(); //32
	unsigned long fifth = (unsigned long) readByte(); //24
        unsigned long sixth = (unsigned long) readByte(); //16
        unsigned long seventh = (unsigned long) readByte(); //8
        unsigned long eighth = (unsigned long) readByte();

        return (first << 56) | (second << 48)| (third<< 40) | (forth << 32) | (fifth << 24) |(sixth <<16) |(seventh << 8)|eighth;
}
/*	
long DataIn::readLeb128()
{
	int bitpos = 0;
	long vln = 0L;
	do	
	{
		int inp = readByte(); // caution: this converstion might be wrong!
		vln |= ((long)(inp & 0x7F)) << bitpos;
		bitpos += 7;
		if((inp&0x80)== 0){
			break;
		}
	}while(true);
	if ((( 1L << (bitpos - 1)) & vln ) != 0){
		vln -= (1L << bitpos);
	}
	  return vln;
}
*/

int DataIn::readLeb128()
{
	int end = _pos;
        int currentByteValue;
        int result;
        result = _barray[end++] & 0xff;
        if (result <= 0x7f)
	{
        	result = (result << 25) >> 25;
        }
	else 
	{
        	currentByteValue = _barray[end++] & 0xff;
            	result = (result & 0x7f) | ((currentByteValue & 0x7f) << 7);
            	if (currentByteValue <= 0x7f) 
		{
                	result = (result << 18) >> 18;
            	} 
		else
		{
                	currentByteValue = _barray[end++] & 0xff;
                	result |= (currentByteValue & 0x7f) << 14;
                	if (currentByteValue <= 0x7f)
			{
                    		result = (result << 11) >> 11;
                	}
			else 
			{
                    		currentByteValue = _barray[end++] & 0xff;
                    		result |= (currentByteValue & 0x7f) << 21;
                    		if (currentByteValue <= 0x7f)
				{
                        		result = (result << 4) >> 4;
                    		}
				else
				{
                        		currentByteValue = _barray[end++] & 0xff;
                        		if (currentByteValue > 0x0f) 
					{
                            			cout<<"Invalid Leb!" <<endl;
						exit(1);
                        		}
                        		result |= currentByteValue << 28;
                    		}
                	}
            	}
        }
        _pos = end;
        return result;
}


/*int DataIn::readULeb128()
{
	int end = _pos;
        int currentByteValue;
        int result;
	result = _barray[end++] & 0xff;
        if (result > 0x7f) 
	{
        	currentByteValue = _barray[end++] & 0xff;
        	result = (result & 0x7f) | ((currentByteValue & 0x7f) << 7);
            	if (currentByteValue > 0x7f) 
		{
                	currentByteValue = _barray[end++] & 0xff;
                	result |= (currentByteValue & 0x7f) << 14;
                	if (currentByteValue > 0x7f)
			{
                    		currentByteValue = _barray[end++] & 0xff;
                    		result |= (currentByteValue & 0x7f) << 21;
                    		if (currentByteValue > 0x7f)
				{
                        		currentByteValue = _barray[end++] & 0xff;
                        		if (currentByteValue > 0x0f)
				 	{
                            			cout<<"Invalid LEB!!"<<endl;
						exit(1);
                       			}
                        		result |= currentByteValue << 28;
                    		}
                	}
            }
        }
	_pos = end;
        return result;
}*/


/*
unsigned int DataIn::readULeb128()
{
	unsigned int value = 0;
	int count = 0;
	char b = readByte(); 
	while((b & 0x80)!=0)
	{
		value |= (b & 0x7f) << count;
		count += 7;

		b = readByte();
       	}
	value |= (b & 0x7f) << count;
	return value;
}
*/



unsigned int DataIn::readULeb128()
{
	unsigned int result = 0;
	int shift = 0;
	unsigned char byte;
	while (true){
		byte = readByte();
		//printf("%.2x\t", byte);
		result |= ((byte & 0x7f) << shift);
		if ((byte & 0x80) == 0)
			break;
		shift += 7;
	}
	return result;
}


void DataIn::skip(int byte)
{
	_pos = _pos + byte;
}	

int DataIn::getStackSize()
{
	return _stack.size();
}


void DataIn::feedArray(char* a,unsigned int bytes, unsigned int total_size){
	for(int i = 0; i < total_size -bytes; i++){
		a[i] =0;
	}
}
