/*
 * Type.cpp
 *
 *  Created on: Oct 16, 2008
 *      Author: tdillig
 */

#include "Type.h"
#include "compass_assert.h"



bool Type::is_type_compatible(il::type* t1, il::type* t2,
		bool supertype)
{
	assert_context("Checking type compatibility: " + (t1 ? t1->to_string() : "null") +
			", " + (t2 ? t2->to_string() : "null") );
	if(t1 == NULL && t2 == NULL) return true;
	if(t1 == NULL || t2 == NULL) return false;

	if(supertype && t1->is_void_type()) return true;
	if(t2->is_void_type()) return true;



	if(t1->is_base_or_enum_type() && t2->is_base_or_enum_type() )
		return true;
	if(*t1==*t2) return true;

	if(t1->is_array_type() && t2->is_array_type()){
		il::array_type* at = (il::array_type*) t1;
		il::array_type* at2 = (il::array_type*) t2;
		return at2->get_elem_type()->get_size() == at->get_elem_type()->get_size();
	}
	if(t1->is_array_type() || t2->is_array_type()){
		return false;
	}



	if(t1->is_record_type() && !t2->is_record_type()){
		il::record_type* rt = (il::record_type*) t1;
		il::type* first_field = rt->get_field_from_offset(0)->t;
		return is_type_compatible(first_field, t2, supertype);
	}
	if(t2->is_record_type() && !t1->is_record_type()){
		il::record_type* rt = (il::record_type*) t2;
		il::type* first_field = rt->get_field_from_offset(0)->t;
		return is_type_compatible(t1, first_field,supertype);
	}

	if(t1->is_base_or_enum_type() && !t2->is_base_or_enum_type()) return false;
	if(t2->is_base_or_enum_type() && !t1->is_base_or_enum_type()) return false;
	if(t1->is_base_or_enum_type() && t2->is_base_or_enum_type())
	{
		if(!supertype)
		{
			return t1->get_size() == t2->get_size() &&
				t1->is_signed_type() == t2->is_signed_type();
		}
		else {
			return t1->get_size() >= t2->get_size() &&
							t1->is_signed_type() == t2->is_signed_type();
		}
	}

	if(t1->is_pointer_type() && !t2->is_pointer_type()) return false;
	if(t2->is_pointer_type() && !t1->is_pointer_type()) return false;
	if(t1->is_pointer_type() && t2->is_pointer_type()){
		il::pointer_type* pt1 = (il::pointer_type*) t1;
		il::pointer_type* pt2 = (il::pointer_type*) t2;

		/*
		 * Work around GCC's fanciful type system
		 */
		il::type* pt1_inner = pt1->get_inner_type();
		while(pt1_inner->is_array_type()) {
			pt1_inner = pt1_inner->get_inner_type();
		}
		il::type* pt2_inner = pt2->get_inner_type();
		while(pt2_inner->is_array_type()) {
			pt2_inner = pt2_inner->get_inner_type();
		}



		return is_type_compatible(pt1_inner, pt2_inner, supertype);
	}

	if(t1->is_record_type() && t2->is_record_type()){
		il::record_type* rt1 = (il::record_type*) t1;
		il::record_type* rt2 = (il::record_type*) t2;

		if(rt1->is_subtype_of(rt2) || rt2->is_subtype_of(rt1))
			return true;

		vector<il::record_info*>& fields1 = rt1->get_fields();
		vector<il::record_info*>& fields2 = rt2->get_fields();

		if(!supertype && fields1.size() != fields2.size()) return false;
		if(supertype && fields1.size() > fields2.size()) return false;
		for(unsigned int i=0; i<fields1.size(); i++){

			if(!is_type_compatible(fields1[i]->t,fields2[i]->t, false))
				return false;
		}
		return true;
	}


	if(t1->is_function_type() && !t2->is_function_type()) return false;
	if(t2->is_function_type() && !t1->is_function_type()) return false;
	if(t1->is_function_type() && t2->is_function_type()){
		return true;
		il::function_type* ft1 = (il::function_type*) t1;
		il::function_type* ft2 = (il::function_type*) t2;


		if(!is_type_compatible(ft1->get_return_type(), ft2->get_return_type(),
				supertype))
			return false;
		if(ft1->get_arg_types().size() != ft2->get_arg_types().size())
			return false;

		const vector<il::type*>& args1 = ft1->get_arg_types();
		const vector<il::type*>& args2 = ft2->get_arg_types();
		for(unsigned int i=0; i<args1.size() ;i++){
			if(!is_type_compatible(args1[i], args2[i],false)) return false;
		}
		return true;
	}

	c_assert(false);
}

il::type* Type::get_type_of_first_field(il::type* t)
{
	if(!t->is_record_type()) return t;
	il::record_type* rt = (il::record_type*) t;
	if(rt->get_field_from_offset(0) == NULL) return t;
	return get_type_of_first_field(rt->get_field_from_offset(0)->t);
}

bool Type::is_supertype(il::type* t1, il::type* t2)
{
	return is_type_compatible(t1, t2, true);
}
