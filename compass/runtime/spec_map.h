/*
 * spec_map.h
 *
 *  Created on: May 16, 2010
 *      Author: tdillig
 */

#ifndef SPEC_MAP_H_
#define SPEC_MAP_H_

#include "spec_adt.h"
#include <map>
/*
namespace spec
{
	template <class K, class V>
	struct val_adt
	{

		K* key;
		V* value;

		void __mark_single_valued_adt_(K* k, V* v)
		{

		}
	};


}*/

namespace spec
{

template<class T1, class T2>
  struct pair
  {
    T1 first;
    T2 second;

    pair()
    : first(), second() { }


    pair(const T1& a, const T2& b)
    : first(a), second(b)
  {

  }

  };



//------------------------------------------

template<class K, class V>
	class map
	{

	private:


		void __mark_single_valued_adt_(K* k, pair<const K, V>* v)
		{

		}
	public:

		map(const map<K, V> & other)
		{
			_compass_adt_set_size(this, 0);
		}


		map()
		{
			K* k = 0;
			pair<const K, V>* v = 0;
			__mark_single_valued_adt_(k, v);
			_compass_adt_set_size(this, 0);
		}

		pair<const K, V>* get_ith_position(int i)
		{
			//static_assert(i < size());
			pair<const K, V>* val = (pair<const K, V>*)
					_compass_adt_read_pos(this, i);
			assume(!is_nil<V>(val->second));
			return val;
		}


		void _insert(const K& key, const V& val)
		{
			int old_size = _compass_adt_get_size(this);
		//	pair<const K, V> anonymous(key, val);
			pair<const K, V>* p = new pair<const K, V>(key, val);

			_compass_adt_insert(this, key, p);
			_compass_adt_set_size(this, old_size+1);

		}
		V& operator[](const K& key)
		{
			bool in_map = _compass_adt_contains(this, key);
			if(!in_map){
				V val= V();
				_insert(key, val);

			}
			pair<const K, V>* val = (pair<const K, V>*)
					_compass_adt_read(this, key);
			return val->second;
		}

		unsigned int count(const K& key)
		{
			bool in_map = _compass_adt_contains(this, key);
			return (unsigned int) in_map;
		}

		unsigned int erase(const K& key)
		{
			bool in_map = _compass_adt_contains(this, key);
			_compass_adt_remove(this, key);
			if(in_map){
				int old_size = _compass_adt_get_size(this);
				_compass_adt_set_size(this, old_size-1);
			}
			return (unsigned int) in_map;
		}

		unsigned int size() const
		{
			return _compass_adt_get_size(this);;
		}

		class iterator
		{
			friend class map;
		private:
			map<K, V> *data;
			int cur;
			iterator(map<K, V> *data, int pos)
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


			pair<const K, V>& operator*()
			{
				return *data->get_ith_position(cur);
			}
			pair<const K, V>* operator->()
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





}






#endif /* SPEC_MAP_H_ */
