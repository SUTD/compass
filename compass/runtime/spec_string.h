/*
 * spec_string.h
 *
 *  Created on: Jun 13, 2010
 *      Author: isil
 */

#ifndef SPEC_STRING_H_
#define SPEC_STRING_H_



namespace spec
{

	typedef char* string;


/*	class string
	{
	private:

		char* c;

	public:
		string()
		{
			c = "";
		}

		string(const char* c)
		{
			this->c = (char*)c;
		}

		string(const string& s)
		{
			this->c = s.c;
		}

	    string& operator=(const string& str)
		{
	    	string&  s = (string&) str;
			c = s.c;
			return *this;
		}

	    string& operator=(const char* s)
		{
			c = (char*)s;
			return *this;
		}

	    bool operator==(const string& other)
		{
	    	if(this->c == other.c) return true;
	    	if(static_choice()) return true;
	    	return false;

		}





	};
*/
}


#endif /* SPEC_STRING_H_ */
