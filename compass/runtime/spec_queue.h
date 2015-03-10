/*
 * spec_queue.h
 *
 *  Created on: Jun 18, 2010
 *      Author: isil
 */

#ifndef SPEC_QUEUE_H_
#define SPEC_QUEUE_H_



namespace spec
{
	template<class T>
	class queue
	{
	private:
		list<T>* elems;

	public:
		queue()
		{
			elems = new list<T>();
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
			elems->pop_front();
		}

		T& front()
		{
			return elems->front();
		}




		};




}



#endif /* SPEC_QUEUE_H_ */
