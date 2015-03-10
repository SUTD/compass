/*
 * spec_list.h
 *
 *  Created on: Jun 17, 2010
 *      Author: isil
 */

#ifndef SPEC_LIST_H_
#define SPEC_LIST_H_

namespace spec
{
	template<class T>
	class list
	{
	private:
		vector<T>* elems;
		int start;
		int _end;

	public:
		list()
		{
			elems = new vector<T>();
			start = -1;
			_end = 0;
		}

		T at(int i)
		{
			return elems->at(i);
		}

		int size()
		{
			return _end-start-1;

		}
		void push_front(const T& t)
		{
			elems->at(start) = t;
			start--;
		}
		void pop_front()
		{
			start++;
		}

		T& front()
		{
			return elems->at(start+1);

		}
		T& back()
		{
			return elems->at(_end-1);
		}

		void push_back(const T& t)
		{
			elems->at(_end) = t;
			_end++;
		}
		void pop_back()
		{
			_end--;
		}

		class iterator
		{
			friend class list;
		private:

			list<T>* l;
			int pos;
			iterator(list<T> * l, int pos)
			{
				this->l = l;
				this->pos = pos;
			}
		public:
			const T& operator*()
			{
				return l->at(pos);
			}

			bool operator==(const iterator& other) const
			{
				return pos == other.pos;
			}
			bool operator!=(const iterator& other) const
			{
				return pos != other.pos;
			}
			void operator++(int ignore)
			{
				pos++;
			}
			void operator++()
			{
				pos++;
			}
			void operator--(int ignore)
			{
				pos--;
			}
			void operator--()
			{
				pos--;
			}


		};

		iterator begin()
		{
			iterator it(this, start+1);
			return it;
		}

		iterator end()
		{
			iterator it(this, _end);
			return it;
		}

	};


}


#endif /* SPEC_LIST_H_ */
