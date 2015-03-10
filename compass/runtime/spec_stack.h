/*
 * stack.h
 *
 *  Created on: Jun 18, 2010
 *      Author: isil
 */

#ifndef SPEC_STACK_H_
#define SPEC_STACK_H_


namespace spec
{
	template<class T>
	class stack
	{
	private:
		vector<T>* elems;

	public:
		stack()
		{
			elems = new vector<T>();
		}


		int size()
		{
			return elems->size();

		}

		bool empty()
		{
			return elems->size() <= 0;
		}


		void push(const T& t)
		{
			elems->push_back(t);
		}
		void pop()
		{
			elems->pop_back();
		}

		T& top()
		{
			return elems->at(((int)elems->size())-1);
		}




		};




}


#endif /* SPEC_STACK_H_ */
