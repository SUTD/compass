/*
 * spec_adt.h
 *
 *  Created on: May 16, 2010
 *      Author: tdillig
 */

#ifndef SPEC_ADT_H_
#define SPEC_ADT_H_


template <class K, class V, class ADT>
void _compass_adt_insert(ADT* adt, K key, V*  val);

template <typename K,  typename ADT>
void* _compass_adt_read_ref(ADT* adt, K key);

template <typename K,  typename ADT>
void* _compass_adt_read_pos_ref(ADT* adt, K key);

template <typename K,  typename ADT>
void* _compass_adt_read_pos(ADT* adt, K key);

template <typename K,  typename ADT>
void* _compass_adt_read(ADT* adt, K key);

template <typename K,  typename ADT>
void* _compass_adt_resize(ADT* adt, K key);

template <typename K,  typename ADT>
bool _compass_adt_contains(ADT* adt, K key);

template <typename K,  typename ADT>
void _compass_adt_remove(ADT* adt, K key);

template <typename ADT>
int _compass_adt_get_size(ADT* adt);

template <typename ADT>
void _compass_adt_set_size(ADT* adt, int size);

template <typename T>
T* get_fresh(T* t);


template <typename T>
T get_nil(T* t);


template <typename T>
bool is_nil(T t);


void* get_fresh2();

#endif /* SPEC_ADT_H_ */
