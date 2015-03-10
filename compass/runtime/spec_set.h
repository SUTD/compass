/*
 * spec_set.h
 *
 *  Created on: Jun 16, 2010
 *      Author: isil
 */

#ifndef SPEC_SET_H_
#define SPEC_SET_H_

#include "spec_adt.h"

template<class T>
class set
{

	private:
		void __mark_single_valued_adt_(T* t, T* x)
		{
		}
	public:

		set(const set<T> & other)
		{

			_compass_adt_set_size(this, 0);
		}


		set()
		{
			T* t = 0;
			__mark_single_valued_adt_(t, t);
			_compass_adt_set_size(this, 0);
		}

		T* get_ith_position(int i)
		{
			//static_assert(i < size());
			T* val = (T*) _compass_adt_read_pos(this, i);
			assume(!is_nil<T>(*val));
			return val;
		}


		void _insert(const T& t)
		{
			int old_size = _compass_adt_get_size(this);
			T* n = new T(t);
			_compass_adt_insert(this, t,n);
			_compass_adt_set_size(this, old_size+1);

		}

		void insert(const T& val)
		{
			bool in_set = _compass_adt_contains(this, val);
			if(!in_set){
				_insert(val);

			}
		}

		unsigned int count(const T& val)
		{
			bool in_set = _compass_adt_contains(this, val);
			return (unsigned int) in_set;
		}

		unsigned int erase(const T& val)
		{
			bool in_set = _compass_adt_contains(this, val);
			_compass_adt_remove(this, val);
			if(in_set){
				int old_size = _compass_adt_get_size(this);
				_compass_adt_set_size(this, old_size-1);
			}
			return (unsigned int) in_set;
		}

		unsigned int size() const
		{
			return _compass_adt_get_size(this);
		}


		class iterator
		{
			friend class set;
		private:
			set<T> *data;
			int cur;
			iterator(set<T> *data, int pos)
			{
				this->data = data;
				this->cur = pos;
			}

		public:
			iterator()
			{
				data = 0;
				cur = -1;
			}


			T& operator*()
			{
				return *data->get_ith_position(cur);
			}
			T* operator->()
			{
				return data->get_ith_position(cur);
			}
			bool operator==(const iterator& other) const
			{
				return other.data == data &&
						cur == other.cur;
			}
			bool operator!=(const iterator& other) const
			{
				return other.data!= data || cur != other.cur;
			}
			void operator++(int ignore)
			{
				cur++;
			}
			void operator++()
			{
				cur++;
			}
			void operator--()
			{
				cur--;
			}
			void operator--(int ignore)
			{
				cur--;
			}


		};


		iterator begin()
		{
			iterator it(this, 0);
			return it;
		}

		iterator end()
		{
			iterator it(this, _compass_adt_get_size(this));
			return it;
		}


};

#endif /* SPEC_SET_H_ */
