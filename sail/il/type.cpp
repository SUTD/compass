#include "type.h"
#include "expression.h"
#include <sstream>
#include "util.h"
#include "function_declaration.h"
#include "serialization-hooks.h"
#include <deque>

#define DEBUG_TYPE_UNIQUIFY false
#define CHECK_UNIQUENESS false
#define INVALID_OFFSET (-99999)


namespace il
{
ostream& operator <<(ostream &os, const type &obj)
{
      os  << obj.to_string();
      return os;
}

ostream& operator <<(ostream &os, const enum_info &obj)
{
      os  << obj.fname << "(" << obj.value << ")";
      return os;
}


ostream& operator <<(ostream &os, const record_info &obj)
{
      os  << (*obj.t) << " " << obj.fname << " (" << obj.offset << ")";
      return os;
}
struct ta_data
{
	type_attribute_kind attrib;
	type* key_t;
	type* val_t;
};

map<string, ta_data> type_attribute_map;



void type::set_attribute( type_attribute_kind attrib, type* k, type* v)
{
	ta_data d;
	d.attrib = attrib;
	d.key_t = k;
	d.val_t = v;
	type_attribute_map[this->to_string()] = d;
}

type* type::get_adt_key_type()
{
	if(type_attribute_map.count(to_string()) == 0)
		return NULL;
	return uniquify_type(type_attribute_map[to_string()].key_t);
}

type* type::get_adt_value_type()
{
	if(type_attribute_map.count(to_string()) == 0)
		return NULL;
	return uniquify_type(type_attribute_map[to_string()].val_t);
}

type_attribute_kind type::get_attribute()
{
	if(type_attribute_map.count(to_string()) == 0)
		return NO_TYPE_ATTRIBUTE;
	return type_attribute_map[to_string()].attrib;
}

bool type::is_abstract_data_type()
	{
		if(type_attribute_map.count(to_string()) == 0) {
			return false;
		}
		type_attribute_kind t = type_attribute_map[to_string()].attrib;
		return(t >= POSITION_DEPENDENT_ADT  &&
				t <= MULTI_VALUED_ADT);
	}



///////////////////////////////////////////////////////////////////////
// type

bool type::_init = type::init();
set<type**> type::unresolved_types;
set<type*> type::unused_types;

bool type::init()
{
	register_end_callback(type::uniquify_loaded_types);
	return true;
}

#define TEMP_DEBUG false
unordered_set<type *, type::type_hash, type::type_eq> type::instances;
map<string, type*> type::type_names;

void type::register_loaded_typeref(type** t)
{
	unresolved_types.insert(t);
}


type *type::get_instance(type *t)
{

	if(t == NULL) return NULL;
	//assert(t->to_string()!="");
    unordered_set<type *>::iterator it = instances.find(t);
    if (it != instances.end()) {
        if(!in_serialization()) delete t;
        else unused_types.insert(t);
        return *it;
    }


    if(CHECK_UNIQUENESS && !in_serialization()) {
    	if(type_names.count(t->to_string()) != 0)
    	{
    		cerr << "Type with name " << t->to_string() <<
    				" already exists!" << endl;
    		cerr << "Existing type: " << t->to_string() << " address: " <<
    				((long int) type_names[t->to_string()]) << endl;
    		cerr << "New type: " << t->to_string() << " address: " <<
    				((long int) t) << endl;
    		assert(false);

    	}

    }
    type_names[t->to_string()] = t;

    instances.insert(t);
    return t;
}

void type::clear()
{
	  set<type*> to_delete;
	  to_delete.insert(instances.begin(), instances.end());
	  instances.clear();
	  set<type*>::iterator it = to_delete.begin();
	  //for(; it != to_delete.end(); it++)
	//	  delete *it;
}

type* type::uniquify_type(type* t)
{
	if(t == NULL) return NULL;

	unordered_set<type *>::iterator it = instances.find(t);
	if(it != instances.end()) return *it;


	type* res = NULL;

	if(t->is_base_type())
	{
		base_type* bt = (base_type*)t;

		res = base_type::make(bt->get_name(), bt->get_size(), bt->get_asize(),
				bt->is_signed_type(), bt->vt, bt->get_typedef_name());
	}
	else if(t->is_invalid_type())
	{
		res = invalid_type::make();
	}
	else if(t->is_enum_type())
	{
		enum_type* et = (enum_type*)t;
		vector<enum_info> ve;
		for(unsigned int i=0; i < et->elems.size(); i++)
		{
			ve.push_back(*et->elems[i]);
		}
		res = enum_type::make(et->name, et->ns, et->size, et->asize,
				et->is_signed, ve, et->get_typedef_name());
	}
	else if(t->is_pointer_type())
	{
		il::pointer_type* pt = (il::pointer_type*) t;
		type* new_elem_t = uniquify_type(pt->get_inner_type());
		res = il::pointer_type::make(new_elem_t, pt->get_typedef_name());
	}
	else if(t->is_constarray_type())
	{
		il::constarray_type* ct = (il::constarray_type*) t;
		type* new_elem_t = uniquify_type(ct->get_inner_type());
		res = il::constarray_type::make(new_elem_t, ct->get_num_elems(),
				ct->get_typedef_name());
	}
	else if(t->is_vararray_type())
	{
		il::vararray_type* vt = (il::vararray_type*) t;
		type* new_elem_t = uniquify_type(vt->get_inner_type());
		res= il::vararray_type::make(new_elem_t, vt->get_num_elems(),
				vt->get_typedef_name());
	}
	else if(t->is_record_type())
	{
		il::record_type* rt = (il::record_type* )t;
		il::record_type* new_rt = il::record_type::make(rt->name,
				rt->ns, rt->size, rt->asize, rt->is_union(), rt->is_class(),
				rt->elems, rt->typedef_name, rt->abstract);

		/*
		 * Add all the member functions
		 */
		{
			map<string, method_info*>& member_functions =
					rt->get_member_functions();
			map<string, method_info*>::iterator it =
					member_functions.begin();
			for(; it!= member_functions.end(); it++) {
				new_rt->add_member_function(it->first, it->second->ns,
						(function_type*)uniquify_type(it->second->get_signature()),
						it->second->virtual_method,
						it->second->static_method, it->second->abstract_method);
			}
		}

		{
			vector<record_info*>::const_iterator it = rt->elems.begin();
			for(; it!= rt->elems.end(); it++) {
				record_info* ri = *it;
				type* nested_t = uniquify_type(ri->t);
				ri->t = nested_t;

			}
		}
		{
			map<int, record_type*>::iterator it = rt->bases.begin();
			for(; it != rt->bases.end(); it++)
			{
				int offset = it->first;
				record_type* base_t = (record_type*)uniquify_type(it->second);
				new_rt->add_base(base_t, offset);
			}
		}


		vector<record_type*>::iterator it2 = rt->derivatives.begin();
		for(; it2 != rt->derivatives.end(); it2++)
		{
			new_rt->add_derivative((record_type*)uniquify_type(*it2));
		}


		res = new_rt;



	}
	else if(t->is_vector_type())
	{
		il::vector_type* vt = (vector_type*)t;
		res = vector_type::make(uniquify_type(vt->get_elem_type()),
				vt->get_num_elems());
	}
	else if(t->is_complex_type())
	{
		il::complex_type* ct = (complex_type*)t;
		res= complex_type::make(uniquify_type(ct->get_elem_type()));
	}
	else if(t->is_function_type())
	{
		function_type* ft = (function_type*)t;
		type* ret = uniquify_type(ft->get_return_type());
		vector<type*> args;
		for(unsigned int i=0; i < ft->get_arg_types().size(); i++)
		{
			args.push_back(uniquify_type(ft->get_arg_types()[i]));
		}
		res= function_type::make(ret, args, ft->is_vararg);
	}

	assert(res != NULL);
	return res;


}

void type::uniquify_loaded_types()
{
	if(DEBUG_TYPE_UNIQUIFY)
		cout << "********* Uniquifing loaded types ******" << endl;
	map<type*, set<type**> > links;



	set<type**>::iterator it = type::unresolved_types.begin();
	for(; it!= type::unresolved_types.end(); it++)
	{
		if(**it == NULL) continue;
		links[**it].insert(*it);
	}
	map<type*, set<type**> >::iterator it2 = links.begin();
	for(; it2 != links.end(); it2++)
	{
		type* t = it2->first;
		if(DEBUG_TYPE_UNIQUIFY){
			cout << "Uniquifying type " << *t << " cur addr: " << t << endl;
		}
		t = uniquify_type(t);
		if(DEBUG_TYPE_UNIQUIFY){
			cout << " ---> new addr: " << t << endl;
		}
		set<type**>::iterator it3 = it2->second.begin();
		for(; it3 != it2->second.end(); it3++)
		{
			if(DEBUG_TYPE_UNIQUIFY){
				cout << " updating address " << *it3 << endl;
			}
			**it3 = t;
		}
	}

	unresolved_types.clear();
	unused_types.clear();

}

void type::rehash_begin()
{
	unordered_set<type *>::iterator it = type::instances.find(this);
	if (it != type::instances.end())
		type::instances.erase(it);
}

void type::rehash_end()
{
	unordered_set<type *>::iterator it = type::instances.find(this);
	if (it == type::instances.end()) {
		hash_c = 0;
		type::instances.insert(this);
	}
}

type *type::get_deref_type() const
{
	return il::get_integer_type();
}

bool type::is_char_type() const
{
	if (!is_base_type())
		return false;

	const base_type *bt = (base_type *) this;
	return bt->get_name().find("char") != string::npos;
}

///////////////////////////////////////////////////////////////////////
// pointer_type

pointer_type::pointer_type(type *t, const string &typedef_name)
	: elem_type(t)
{
	assert(t != NULL);
	size = SIZE;
	asize = ASIZE;
	this->typedef_name = typedef_name;
	compute_hash();
}

bool pointer_type::operator==(const type &other)
{
	if (!other.is_pointer_type())
		return false;

	pointer_type &pt = (pointer_type &) (other);
	return *pt.elem_type == *elem_type;
}

void pointer_type::compute_hash()
{
	hash_c = elem_type->hash() * 17;
}


bool pointer_type::is_pointer_type() const { return true; }

string pointer_type::to_string() const
{
	if(elem_type == NULL)
		return "<INVALID>*";

	string res =   elem_type->to_string() + "*";
	//res += "(" + int_to_string((long int) this ) + ")";
	return res;
}



type *pointer_type::get_deref_type() const { return elem_type; }


type *pointer_type::get_inner_type() { return elem_type; }


///////////////////////////////////////////////////////////////////////
// constarray_type

constarray_type::constarray_type(type *et,
                                 int64 num_elems,
                                 const string& typedef_name)
 : elem_type(et),
   num_elems(num_elems)
{
	assert(et!=NULL);
	this->typedef_name = typedef_name;
	compute_hash();
}

bool constarray_type::operator==(const type &other)
{
	if (!other.is_constarray_type())
		return false;

	constarray_type &ct = (constarray_type &) other;
	return *ct.elem_type == *elem_type && ct.num_elems == num_elems;
}

void constarray_type::compute_hash()
{

	hash_c = elem_type->hash() * 5 + num_elems*47;

}


type *constarray_type::get_inner_type() { return elem_type; }

bool constarray_type::is_constarray_type() const { return true; }

string constarray_type::to_string() const
{
	string s;
	stringstream out;
	out << num_elems;
	s = out.str();
	if(elem_type == NULL)
			return "<INVALID>[" + s + "]";
	return  elem_type->to_string() + "[" + s + "]";
}

type *constarray_type::get_elem_type() { return elem_type; }

int64 constarray_type::get_num_elems() { return num_elems; }


int constarray_type::get_size()
{
    int size = num_elems*elem_type->get_size();
    if(size<0) size = 0;
    return size;
};

int constarray_type::get_asize()
{
    int asize = num_elems*elem_type->get_size();
    if(asize<0) asize = 0;
    return asize;
};


///////////////////////////////////////////////////////////////////////
// vararray_type

vararray_type::vararray_type(type *t,
                             expression *num_elems,
                             const string &typedef_name)
	: elem_type(t),
	  num_elems(num_elems)
{
	assert(t != NULL);
	assert(num_elems != NULL);
	size = asize = 0;
	this->typedef_name = typedef_name;
	compute_hash();
}

bool vararray_type::operator==(const type &other)
{
	if (!other.is_vararray_type())
		return false;

	vararray_type &vt = (vararray_type &) other;
	return *vt.elem_type == *elem_type && num_elems == vt.num_elems;
}


bool vararray_type::is_vararray_type() const { return true; }

string vararray_type::to_string() const
{

	return elem_type->to_string() + "[" + num_elems->to_string() +"]";
}

type *vararray_type::get_inner_type() { return elem_type; }

type *vararray_type::get_elem_type() { return elem_type; }

expression *vararray_type::get_num_elems() { return num_elems; }


void vararray_type::compute_hash()
{
	hash_c = elem_type->hash() * 29 + (size_t)num_elems;
}

///////////////////////////////////////////////////////////////////////
// enum_type

enum_type::enum_type(string name,
	                 namespace_context ctx,
	                 int size,
	                 int asize,
	                 bool is_signed,
	                 vector<enum_info> &elems,
	                 const string &typedef_name)
	: name(name),
	  ns(ctx),
	  is_signed(is_signed)
{
	this->size = size;
	this->asize = asize;
	this->typedef_name = typedef_name;
	for (unsigned int i = 0; i < elems.size(); i++){
		this->elems.push_back(new enum_info(elems[i]));
	}

	compute_hash_code();
}

void enum_type::compute_hash_code()
{
	hash_c = 19 * string_hash(name) + ns.get_hash_code()*7;
}

bool enum_type::operator==(const type &other)
{
    if (!other.is_enum_type())
    	return false;

    enum_type &et = (enum_type &) other;
    return et.name == name && et.ns == ns;
}

string enum_type::to_string() const
{
	return  "enum " + ns.to_string() + "::" + name;
}

bool enum_type::is_enum_type() const { return true; }

vector<enum_info *> &enum_type::get_fields() { return elems; }

bool enum_type::is_signed_type() const
{ return is_signed; }


///////////////////////////////////////////////////////////////////////
// record_type

type** record_type::get_typeref_from_index(int i)
{
	if(i >= elems.size()) return NULL;
	assert(i >=0 && i < (int)elems.size());
	return &elems[i]->t;
}

record_type::record_type(string name,
                         namespace_context ctx,
                         int size,
                         int asize,
                         bool is_union,
                         bool is_class,
                         const vector<record_info *> &elems,
                         const string& typedef_name, bool is_abstract)
    : elems(elems),
      name(name),
      ns(ctx)
{

	assert(name != "");
	this->compare_all_fields = false;
    this->typedef_name = typedef_name;
    if (is_class) kind = CLASS;
    else if (is_union) kind = UNION;
    else kind = STRUCT;

    this->size = size;
    this->asize = asize;
    this->abstract = is_abstract;

    for(unsigned int i = 0; i < elems.size(); i++) {
	
    	//Note: From gcc, it is possible that
    	//elems[i]->t is NULL, so we cannot assert this here

	
        if(name_to_record_info.count(elems[i]->fname) != 0)
        {
        	continue;
        	cout << "Processing type " << name << endl;
        	cout << "Field already exists: " << elems[i]->fname << " address: " <<
        			(long int) elems[i] << endl;
        	cout << "Existing field: " << elems[i]->fname << " address: " <<
        			(long int) name_to_record_info[elems[i]->fname] << endl;
        	assert(false);
        }
        name_to_record_info[elems[i]->fname] = elems[i];
        offset_to_elem[elems[i]->offset] = elems[i];
    }

    compute_hash();


}

void record_type::update_incomplete_record(int size, int asize,
		const vector<record_info*> &elems, bool is_abstract)
{
	this->size = size;
	this->asize = asize;
	this->abstract = is_abstract;
	this->elems = elems;
	for(unsigned int i = 0; i < elems.size(); i++) {
	  if(name_to_record_info.count(elems[i]->fname) != 0)
	   {
	        	cout << "Processing type " << name << endl;
	        	cout << "Field already exists: " << elems[i]->fname << " address: " <<
	        			(long int) elems[i] << endl;
	        	cout << "Existing field: " << elems[i]->fname << " address: " <<
	        			(long int) name_to_record_info[elems[i]->fname] << endl;
	        	assert(false);

	   }
	   name_to_record_info[elems[i]->fname] = elems[i];
	   offset_to_elem[elems[i]->offset] = elems[i];
	}

}


record_type::~record_type()
{
}

void record_type::compute_recursive_fields()
{
	for(int i = 0; i < (int)elems.size(); i++) {
		record_info* ri = elems[i];
		//cout << "record_type = " <<this->to_string()<< endl;
		//cout << "index = " << i << endl;
		if(ri == NULL) 
		    cout << "ri is NULL"<< endl;
		
		//cout << "ri.offset = " << ri->offset << endl;
		//cout << "ri.fname " << ri->fname << endl;
		//cout << "ok here ? " << endl;
		if(ri->t == NULL) 
		    cout << "type is NULL" << endl;
		
		if(!ri->t->is_pointer_type()) continue;
		pointer_type* pt = (pointer_type*) ri->t;
		type* elem_t = pt->get_deref_type();
		if(elem_t == this) {
			recursive_fields.push_back(ri);
		}
	}
}



bool record_type::operator==(const type &other)
{


	//haiyan added
	if(!other.is_record_type())
	    return false;
	//haiyan ended
	record_type &rt = (record_type &) other;


	if(!(ns == rt.ns)){
		return false;
	}
	if(name != rt.name) {
		return false;
	}
	if(kind != rt.kind) {
		return false;
	}
	if(elems.size() != rt.elems.size()){
		return false;
	}

	return true;


}

void record_type::compute_hash()
{
	hash_c = ns.get_hash_code() + 19 * string_hash(name);
}

void record_type::add_member_function(const string & name,
		const namespace_context & ns,
		function_type *fd, bool virtual_method,
		bool static_method, bool abstract_method)
{
	method_info* mi = new method_info;
	mi->name = name;
	mi->ns = ns;
	mi->set_signature(fd);
	mi->virtual_method = virtual_method;
	mi->static_method = static_method;
	mi->abstract_method = abstract_method;

	member_functions[name] = mi;
}

map<string, method_info*> &record_type::get_member_functions()
{
	return member_functions;
}


void record_type::add_base(record_type *base, long int offset)
{
	if(bases.count(offset) > 0) return;
	bases[offset] = base;
}

const map<int, record_type*> & record_type::get_bases()
{
	return bases;
}

void record_type::add_derivative(record_type *sub)
{
	derivatives.push_back(sub);
}

vector<record_type *> &record_type::get_derivatives()
{
	return derivatives;
}

void record_type::get_transitive_subclasses(set<record_type*>& subclasses)
{
	set<record_type*> worklist;
	set<record_type*> visited;

	worklist.insert(this);

	while(worklist.size() > 0) {
		record_type* rt = *worklist.begin();
		worklist.erase(rt);
		if(visited.count(rt) > 0) continue;
		subclasses.insert(rt);
		visited.insert(rt);


		vector<record_type*>::iterator it = rt->derivatives.begin();
		for(; it!= rt->derivatives.end(); it++) {
			worklist.insert(*it);
		}

	}



}

bool record_type::contains_method(string fun_name, type* _sig)
{
	cout << "Contains method called on type " << *this << endl;
	if(member_functions.count(fun_name) == 0 ){
		return false;
	}
	//signatures should always be function types
	if(!_sig->is_function_type()){
		return false;
	}
	il::function_type* fi = static_cast<il::function_type*>(_sig);

	il::function_type* my_sig = member_functions[fun_name]->get_signature();

	//We do not want to compare the this pointer type, since it
	//will clearly be different
	return my_sig->get_method_signature()==fi->get_method_signature();
}

void record_type::get_classes_inheriting_method(string fun_name,
		il::type* signature, set<il::record_type*>& inheriting_classes)
{

	inheriting_classes.insert(this);


	deque<record_type*> worklist;
	worklist.push_back(this);
	set<record_type*> visited;
	while(worklist.size() >0)
	{
		record_type* rt = worklist.front();
		worklist.pop_front();
		if(visited.count(rt) > 0) continue;
		visited.insert(rt);

		/*
		 * Do I override this method?
		 */
		if(this != rt && rt->contains_method(fun_name, signature)){
			continue;
		}

		inheriting_classes.insert(rt);


		vector<il::record_type*>::iterator it = derivatives.begin();
		for(; it!= derivatives.end(); it++)
		{
			record_type* subclass = *it;
			worklist.push_back(subclass);
		}

	}


}

bool record_type::is_recursive()
{
	return recursive_fields.size() != 0;
}

vector<record_info *> &record_type::get_recursive_fields()
{
	return recursive_fields;
}




bool record_type::has_virtual_methods()
{
	auto it = member_functions.begin();
	for(; it != member_functions.end(); it++) {
		if(!it->second->virtual_method) return true;
	}
	return false;
}

bool record_type::is_abstract()
{
	return abstract;
}

string record_type::to_string() const
{
	string res;

	/*if (kind == CLASS)
		res += "class ";
	else if (kind == UNION)
		res += "union ";
	else
		res += "struct "; */

	string ns_string = ns.to_string();
	res += (ns_string == "" ? "" : "::") + name;
	//res += "(" + int_to_string((long int) this) + ")";

	return res;
}

bool record_type::is_record_type() const { return true; }

vector<record_info *> &record_type::get_fields() { return elems; }

record_info *record_type::get_field_from_name(string name)
{
	if(name_to_record_info.count(name) == 0) {
		return NULL;
	}
	return name_to_record_info[name];
}

/*
 * This can return null if the index does
 * not denote the start of a field
 */
record_info *record_type::get_field_from_offset(int offset)
{
	if(offset_to_elem.count(offset) == 0) return NULL;
	return offset_to_elem[offset];
}

il::type* record_type::get_type_from_offset(int offset)
{

	//cout << "GETTING TYPE OF OFFSET: "

	if(offset_to_elem.count(offset) > 0){
		if(!offset_to_elem[offset]->t->is_record_type())
			return offset_to_elem[offset]->t;
	}
	int last_offset_before = INVALID_OFFSET;
	for (auto it = offset_to_elem.begin(); it != offset_to_elem.end(); it++)
	{
		int cur_offset = it->first;
		if(cur_offset>offset) break;
		last_offset_before = cur_offset;
	}
	if(last_offset_before == INVALID_OFFSET) return NULL;

	il::type* t = offset_to_elem[last_offset_before]->t;
	if(!t->is_record_type()) return NULL;
	il::record_type* new_rt = static_cast<record_type*>(t);
	return new_rt->get_type_from_offset(offset - last_offset_before);
}


bool record_type::is_struct()
{
	return kind == STRUCT;
}

bool record_type::is_class()
{
	return kind == CLASS;
}

bool record_type::is_union()
{
	return kind == UNION;
}

bool record_type::is_supertype_of(il::record_type* t)
{
	set<record_type*> subclasses;
	this->get_transitive_subclasses(subclasses);
	return subclasses.count(t) > 0;
}

bool record_type::is_subtype_of(il::record_type* t)
{
	set<record_type*> subclasses;
	t->get_transitive_subclasses(subclasses);
	return subclasses.count(this) > 0;
}


///////////////////////////////////////////////////////////////////////
// vector_type

vector_type::vector_type(type *elem_type, int num_elems)
	: elem_type(elem_type),
	  num_elems(num_elems)
{
	assert(elem_type != NULL);
	compute_hash();
}

bool vector_type::operator==(const type &other)
{
	if (!other.is_vector_type())
		return false;

	vector_type &pt = (vector_type &) other;
	return *pt.elem_type == *elem_type && pt.num_elems == num_elems;
}

void vector_type::compute_hash()
{
	hash_c = (size_t)elem_type* 43 + num_elems;

}

int vector_type::get_num_elems() { return num_elems; }

type *vector_type::get_elem_type() { return elem_type; }

string vector_type::to_string() const
{
	string s;
	stringstream out;
	out << num_elems;
	s = out.str();
	if(elem_type == NULL)
		return "VEC(<INVALID>)";
	return "VEC(" + elem_type->to_string() + ", " + s + ")";
}

bool vector_type::is_vector_type() const { return true; }


int vector_type::get_size()
{
	int size = num_elems*elem_type->get_size();
	if(size<0) size = 0;
	return size;
}

int vector_type::get_asize()
{
	int asize = num_elems*elem_type->get_size();
	if(asize<0) asize = 0;
	return asize;
}


///////////////////////////////////////////////////////////////////////
// complex_type

complex_type::complex_type(type *elem_type) : elem_type(elem_type)
{
	assert(elem_type != NULL);
	compute_hash();
}

bool complex_type::operator==(const type &other)
{
	if (!other.is_complex_type())
		return false;

	complex_type &pt = (complex_type &) other;
	return *pt.elem_type == *elem_type;
}

void complex_type::compute_hash()
{
	hash_c = ((size_t) elem_type) *41;
}

type *complex_type::get_elem_type() { return elem_type; }

string complex_type::to_string() const
{
	if(elem_type == NULL)
		return "COMPLEX(<INVALID>)";
	return "COMPLEX(" + elem_type->to_string()+ ")";
}

bool complex_type::is_complex_type() const { return true; }



//TODO: Check me
int complex_type::get_size()
{
	return 2*elem_type->get_size();
}

int complex_type::get_asize()
{
	return 2*elem_type->get_size();
}

///////////////////////////////////////////////////////////////////////
// function_type

function_type::function_type(type *ret_type, const vector<type *> &arg_types, bool is_vararg)
    : ret_type(ret_type),
      arg_types(arg_types),
      is_vararg(is_vararg)
{
	for(unsigned int i=0; i<arg_types.size(); i++) {
		assert(arg_types[i] != NULL);
	}
	size = 0;
	asize = 0;
	compute_hash();

}


bool function_type::operator==(const type &other)
{
	if (!other.is_function_type())
		return false;


	bool check = TEMP_DEBUG && to_string() == other.to_string();
	function_type& ft = (function_type&) other;
	if(ft.ret_type == NULL && ret_type != NULL) return false;
	if(ret_type == NULL && ft.ret_type != NULL) return false;
	if( ret_type != NULL && !(*ft.ret_type == *ret_type)) return false;
	if(ft.is_vararg != is_vararg) return false;
	if(arg_types.size() != ft.arg_types.size()) return false;
	for(unsigned int i=0; i<arg_types.size(); i++)
	{
		il::type* t1 = arg_types[i];
		il::type* t2 = ft.arg_types[i];
		if(!(*t1 == *t2)) return false;
	}

	return true;

}

void function_type::compute_hash()
{

	hash_c = ((size_t) ret_type) * 53;
	for(unsigned int i=0; i<arg_types.size(); i++) {
		hash_c*=17;
		hash_c += (size_t)arg_types[i];
	}
	hash_c += (is_vararg? 1 : 0);


}

type *function_type::get_return_type()
{
	return ret_type;
}
const vector<type *> &function_type::get_arg_types()
{
	return arg_types;
}

string function_type::to_string() const
{
	string res =  (ret_type != NULL ? ret_type->to_string(): "[nonret]") + " (";
	for(unsigned int i=0; i < arg_types.size(); i++)
	{
		res += arg_types[i]->to_string();
		if(!(i == arg_types.size() - 1))
			res += ", ";
	}
	res += ")";
	//res += "(" + int_to_string((long int) this) +")";
	return res;
}

string function_type::to_byte_string() const
{
	string res;

	if(ret_type == NULL)
		res += "[nonret]";
	else if(ret_type->is_base_type())
		res+= int_to_string(ret_type->get_size());
	else res += ret_type->to_string();

	res+= " (";
	for(unsigned int i=0; i < arg_types.size(); i++)
	{
		if(arg_types[i]->is_base_type())
		{
			res+=int_to_string(arg_types[i]->get_size());
		}
		else
		{
			res += arg_types[i]->to_string();
		}
		if(!(i == arg_types.size() - 1))
			res += ", ";
	}
	res += ")";
	return res;
}

bool function_type::is_function_type() const { return true; }

function_type* function_type::get_method_signature()
{


	assert(arg_types.size() > 0);
	vector<type*> new_arg_types;
	for(unsigned int i=1; i<arg_types.size(); i++)
	{
		type* t = arg_types[i];
		new_arg_types.push_back(t);
	}

	type* res = function_type::make(ret_type, new_arg_types, is_vararg);
	return static_cast<function_type*>(res);
}



///////////////////////////////////////////////////////////////////////
// base_type

base_type::base_type(string name,
          int size,
          int asize,
          bool is_signed,
          value_type vt,
          string typedef_name)
    : name(name), is_signed(is_signed), vt(vt)
{
    this->size = size;
    this->asize = asize;
    this->typedef_name = typedef_name;
    compute_hash();
}

bool base_type::operator==(const type &other)
{
	if (!other.is_base_type())
		return false;


	base_type &bt = (base_type &) other;
	return bt.name == name && bt.size == size &&
	       bt.is_signed == is_signed &&
	       bt.vt == vt;
}

void base_type::compute_hash()
{
     hash_c = string_hash(name) *7 + size * 19 + (is_signed ? 1 : 0) * 51 + vt;

}

string base_type::get_name() const
{
	return name;
}

bool base_type::is_void_type() const { return vt == VOID; }
bool base_type::is_signed_type() const { return is_signed; }

string base_type::to_string() const
{
	return name;
}

bool base_type::is_base_type() const { return true; }
//---------------------------------------
invalid_type::invalid_type()
{
	compute_hash();
}

void invalid_type::compute_hash()
{
     hash_c = 0;
}


bool invalid_type::operator==(const type &other)
{
	if (!other.is_invalid_type())
		return false;
	return true;
}
int invalid_type::get_size()
{
	return 0;
}
int invalid_type::get_asize()
{
	return 0;
}

string invalid_type::to_string() const
{
	return "[invalid]";
}


//----------------------------------------

type * get_integer_type()
{
    if (type::get_type_from_name("int") != NULL)
        return type::get_type_from_name("int");
    type *t = base_type::make("int", 32, 32, true, INTEGER, "");
    return t;
}

type *get_unsigned_integer_type()
{
    if(type::get_type_from_name("unsigned int") != NULL)
        return type::get_type_from_name("unsigned int");
    type *t = base_type::make("unsigned int", 32, 32, false, INTEGER, "");
    return t;
}

type *get_char_type()
{
    if (type::get_type_from_name("char") != NULL)
        return type::get_type_from_name("char");
    type *t = base_type::make("char", 8, 8, false, INTEGER, "");
    return t;
}

type* get_void_type()
{
	return base_type::make("void", 0, 0, true, VOID, "");
}

type* get_void_ptr_type()
{

	type* t = base_type::make("void", 0, 0, true, VOID, "");
	t = pointer_type::make(t, "");
	return t;
}

function_type* get_void_function_type()
{
	type* ret_type = get_void_type();
	vector<type*> args;
	return function_type::make(ret_type, args, false);
}

//--------------------------------------------------




}


