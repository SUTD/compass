/*
 * spec_vector.h
 *
 *  Created on: May 2, 2010
 *      Author: isil
 */

#ifndef SPEC_VECTOR_H_
#define SPEC_VECTOR_H_
#include "spec_adt.h"


/*
namespace spec
{
	template <class K, class V>
	struct pos_adt
	{

		K* key;
		V* value;

		void __mark_position_dependent_adt_(K* k, V* v)
		{

		}
		static pos_adt make(K* k, V* v);

	};
}*/

#define MAX_SIZE 1073741823





namespace spec
{




	template<class T>
	class vector
	{
	private:

		void __mark_position_dependent_adt_(int* k, T* v)
		{

		}



	public:
		vector()
		{
			T* x = 0;
			int k;
			//adt = pos_adt<int, T>::make(&k, x);
			__mark_position_dependent_adt_(&k, x);
			_compass_adt_set_size(this, 0);
		}
		vector(const vector<T> & other)
		{
			T* x = 0;
			int k;
			//adt = pos_adt<int, T>::make(&k, x);
			__mark_position_dependent_adt_(&k, x);
			_compass_adt_set_size(this, 0);
			for(int i = 0; i < other.size(); i++)
			{
				push_back(other[i]);
			}
		}
		~vector()
		{

		}


		//---------------------------------

		unsigned int size() const
		{
			return _compass_adt_get_size(this);
		}

		unsigned int max_size() const
		{
			return MAX_SIZE;
		}

		void reserve(int s)
		{

		}

		//void resize (unsigned int sz, T c = T() )
		//{
		//
		//}

		T& operator[](int n)
		{
			T* x = (T*)_compass_adt_read(this, n);
			return *x;
		}

		const T& operator[](int n) const
		{
			T* x = (T*)_compass_adt_read(this, n);
			return *x;
		}


		T& at(int n)
		{
			T* x = (T*)_compass_adt_read(this, n);
			return *x;
		}

		const T& at(int n) const
		{
			T* x = (T*)_compass_adt_read(this, n);
			return *x;
		}

		void push_back( const T & _x)
		{
			T& x = (T&)_x;

			//T t = x;
			T* t = new T(x);
			int old_size = _compass_adt_get_size((vector<T>*)this);
			_compass_adt_insert(this, old_size, t);
			_compass_adt_set_size((vector<T>*)this, old_size+1);
		}

		void pop_back()
		{
			int size = _compass_adt_get_size((vector<T>*)this);
			_compass_adt_remove(this, size-1);
			_compass_adt_set_size((vector<T>*)this, size-1);

		}


		class iterator
		{
			friend class vector;
		private:
			vector<T> *vec;
			int pos;
			iterator(vector<T> * vec, int pos)
			{
				this->vec = vec;
				this->pos = pos;
			}

		public:
			iterator()
			{
				vec = 0;
				pos = -1;
			}


			T& operator*()
			{
				return (*vec)[pos];
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
			iterator it(this, 0);
			return it;
		}

		iterator end()
		{
			iterator it(this, _compass_adt_get_size(this));
			return it;
		}






	};

}





//template <class K, class V, class ADT>
//void _compass_adt_insert(ADT* adt, K key, V  val);


//typedef std::vector spec::vector;



/*
 *
 * operator=	 Copy vector content (public member function)


Iterators:
begin	 Return iterator to beginning (public member type)
end	 Return iterator to end (public member function)
rbegin	 Return reverse iterator to reverse beginning (public member function)
rend	 Return reverse iterator to reverse end (public member function)


Capacity:
size	 Return size (public member function)
max_size	 Return maximum size (public member function)
resize	 Change size (public member function)
capacity	 Return size of allocated storage capacity (public member function)
empty	 Test whether vector is empty (public member function)
reserve	 Request a change in capacity (public member function)


Element access:
operator[]	 Access element (public member function)
at	 Access element (public member function)
front	 Access first element (public member function)
back	 Access last element (public member function)


Modifiers:
assign	 Assign vector content (public member function)
push_back	 Add element at the end (public member function)
pop_back	 Delete last element (public member function)
insert	 Insert elements (public member function)
erase	 Erase elements (public member function)
swap	 Swap content (public member function)
clear	 Clear content (public member function)


Allocator:
get_allocator	 Get allocator (public member function)
 */


#endif /* SPEC_VECTOR_H_ */
