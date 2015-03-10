#ifndef TYPE_H_
#define TYPE_H_

#include <assert.h>




#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "../types.h"
#include "util.h"
#include "./hash.h"
#include "./namespace_context.h"
using namespace std;


#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

namespace il
{

class expression;
class function_declaration;
class type;
class function_type;

enum value_type { INTEGER, IEEE_FLOAT, VOID, COMPLEX, BOOL };
//static unordered_map<string, type *> name_to_type;

enum type_attribute_kind {
	NO_TYPE_ATTRIBUTE,
	POSITION_DEPENDENT_ADT,
	SINGLE_VALUED_ADT,
	MULTI_VALUED_ADT

};


/*
 * Places in the private segment of shared type classes.
 * Prevents construction, destruction, and copying.
 */
#define SHARED_PRIVATE(classname)                                   \
    classname() {}                                                  \
    virtual ~classname() {}                                         \
    classname(const classname &rhs) {}                              \
    classname &operator=(const classname &rhs) { return *this; }

#define SHARED_PRIVATE_CUSTOM_DTOR(classname)                       \
    classname() {}                                                  \
    virtual ~classname();                                           \
    classname(const classname &rhs) {}                              \
    classname &operator=(const classname &rhs) { return *this; }

#define SHARED_DEFINE_MAKE_METHOD(classname, formals, ctorargs) \
    static classname *make formals                              \
    {                                                           \
        classname *__t = new classname ctorargs ;               \
        return static_cast<classname *>(get_instance(__t));     \
    }

/*
 * Abstract class type
 */
/**
 * \brief Parent class of all the concrete types.
 */
class type
{
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & size;
        ar & asize;
        ar & typedef_name;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & size;
        ar & asize;
        ar & typedef_name;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    static bool _init;
    static bool init();
    static set<type*> unused_types;

    static type* uniquify_type(type* t);


private:
    type(const type &rhs) {}
    type &operator=(const type &rhs) { return *this; }

public:
    static void uniquify_loaded_types();
    struct type_hash {
        size_t operator()(type *t) const
        {
            return t->hash();
        }
    };

    struct type_eq {
        bool operator()(type *a, type *b) const
        {
            return *a == *b;
        }
    };

    static void clear();





protected:
    type() : hash_c(0) {}
    virtual ~type() {}

    /**
     * Helper to make() methods that returns the unique instance
     * of the given type instance.
     */
    static type *get_instance(type *t);

    static unordered_set<type *, type_hash, type_eq> instances;


    static map<string, type*> type_names;




    void rehash_begin();
    void rehash_end();

    int size;
    int asize;
    string typedef_name;

    /* convenience cache for the hash value */
    size_t hash_c;


    static set<type**> unresolved_types;

public:

    static type *get_type_from_name(string name)
    {
        return type_names[name];
    }



	void set_attribute( type_attribute_kind attrib, type* k, type* v);

	type_attribute_kind get_attribute();
	type* get_adt_key_type();
	type* get_adt_value_type();

	bool is_abstract_data_type();

    /*
     * The set of type* references to be uniquified once
     * loading is complete.
     */
    static void register_loaded_typeref(type** t);



	/*
	static void delete_types()
	{
		hash_map<string, type*>::iterator it = name_to_type.begin();
		for(; it!= name_to_type.end(); it++)
			delete it->second;

		name_to_type.clear();
	}*/

    virtual bool operator==(const type& other) = 0;
    inline size_t hash()
    {
    	return hash_c;
    }

    virtual void print() {};

    virtual string to_string() const {return "";} ;


    friend ostream& operator <<(ostream &os, const type &obj);

    /*
     * The type when dereferencing t, integer type if not defined.
     */
    virtual type *get_deref_type() const;

    virtual type* get_inner_type()
    {
        return NULL;
    }

    bool is_base_or_enum_type() const
    {
        return is_base_type() || is_enum_type();
    };

    bool is_char_type() const;

    bool is_array_type() const
    {
        return is_constarray_type() || is_vararray_type();
    }

    virtual bool is_void_type() const { return false; }

    bool is_void_star_type() const
    {
        return this->is_pointer_type() &&
               this->get_deref_type()->is_void_type();
    };

    virtual string get_typedef_name()
    {
        return typedef_name;
    }

    virtual bool is_pointer_type() const { return false; }
    virtual bool is_constarray_type() const { return false; }
    virtual bool is_vararray_type() const { return false; }
    virtual bool is_enum_type() const { return false; }
    virtual bool is_record_type() const { return false; }
    virtual bool is_vector_type() const { return false; }
    virtual bool is_complex_type() const { return false; }
    virtual bool is_base_type() const { return false; }
    virtual bool is_function_type() const { return false; }
    virtual bool is_invalid_type() const { return false; }

    virtual int get_size() { return size; }
    virtual int get_asize() { return asize; }
    virtual bool is_signed_type() const { return false; }
    virtual void print_kind() const
    {
        if(is_pointer_type())
            cout << "pointer type" << endl;
        if(is_constarray_type())
            cout << "constarray_type" << endl;
        if(is_vararray_type())
            cout << "vararray_type" << endl;
        if(is_enum_type())
            cout << "enum_type" << endl;
        if(is_record_type())
            cout << "record_type" << endl;
        if(is_vector_type())
            cout << "vector_type" << endl;
        if(is_complex_type())
			cout << "complex_type" << endl;
        if(is_base_type())
            cout << "base_type" << endl;
        if(is_function_type())
            cout << "function_type" << endl;
    }

};


/**
 * \brief Pointer to type elem_type
 */
class pointer_type : public type
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<type>(*this);
        ar & elem_type;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<type>(*this);
        ar & elem_type;
        type::register_loaded_typeref(&elem_type);
        hash_c = 0;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    SHARED_PRIVATE(pointer_type)

    pointer_type(type *t, const string &typedef_name);

public:
    SHARED_DEFINE_MAKE_METHOD(pointer_type,
                              (type *t, const string &typedef_name),
                              (t, typedef_name))

    type *elem_type;

    static const int SIZE = 64;
    static const int ASIZE = 64;

    virtual bool operator==(const type& _other);
    void compute_hash();

    virtual bool is_pointer_type() const;
    virtual string to_string() const;

    /**
     * @return the type of the element obtained if a pointer of this type
     * is dereferenced
     */
    virtual type *get_deref_type() const;
    virtual type* get_inner_type();

};

/**
 * \brief Abstract class representing a non-pointer array type
 */
class array_type : public type
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<type>(*this);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<type>(*this);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

public:
    virtual ~array_type(){ };
    virtual string to_string() const = 0;
    virtual type *get_elem_type() = 0;
};


/**
 * \brief The size of a const_array is known at compile time.
 */
class constarray_type : public array_type
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<array_type>(*this);
        ar & elem_type;
        ar & num_elems;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<array_type>(*this);
        ar & elem_type;
        type::register_loaded_typeref(&elem_type);
        ar & num_elems;
        hash_c = 0;


    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    SHARED_PRIVATE(constarray_type)

    constarray_type(type *t, int64 num_elems, const string &typedef_name);
    void compute_hash();


protected:
    type* elem_type;
    int64 num_elems;

public:
    SHARED_DEFINE_MAKE_METHOD(constarray_type,
                              (type *t, int64 num_elems, const string &typedef_name),
                              (t, num_elems, typedef_name))

    virtual bool operator==(const type& _other);

    virtual type* get_inner_type();

    virtual bool is_constarray_type() const;
    virtual string to_string() const;

    virtual type *get_elem_type();
    virtual int64 get_num_elems();
    virtual int get_size();
    virtual int get_asize();

};

/**
 * \brief Size of a vararray may not be known at compile time.
 */
class vararray_type : public array_type
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<array_type>(*this);
        ar & elem_type;
        ar & num_elems;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<array_type>(*this);
        ar & elem_type;
        type::register_loaded_typeref(&elem_type);
        ar & num_elems;
        hash_c = 0;

    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    SHARED_PRIVATE(vararray_type)

    vararray_type(type *t, expression *num_elems, const string &typedef_name);
    void compute_hash();

protected:
    type *elem_type;
    expression *num_elems;

public:
    SHARED_DEFINE_MAKE_METHOD(vararray_type,
                              (type *t, expression *num_elems, const string &typedef_name),
                              (t, num_elems, typedef_name))

    virtual bool operator==(const type& _other);

    virtual bool is_vararray_type() const;
    virtual string to_string() const;

    virtual type* get_inner_type();
    virtual type *get_elem_type();
    virtual expression * get_num_elems();
};
//---------------------------------
struct enum_info
{
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & fname;
		ar & value;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & fname;
		ar & value;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
	string fname;
	int64 value;
	virtual ~enum_info(){};
	friend ostream& operator <<(ostream &os, const type &obj);



};

/**
 * \brief Represents an enum type
 */
class enum_type : public type
{
public:
    vector<enum_info*> elems;
    string name;
    namespace_context ns;
    bool is_signed;

    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<type>(*this);
        ar & elems;
        ar & name;
        ar & ns;
        ar & is_signed;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<type>(*this);
        ar & elems;
        ar & name;
        ar & ns;
        ar & is_signed;
        compute_hash_code();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    SHARED_PRIVATE(enum_type)

    enum_type(string name, namespace_context ctx, int size, int asize,
    		bool is_signed, vector<enum_info> & elems,
    		const string& typedef_name);
    void compute_hash_code();

public:
    SHARED_DEFINE_MAKE_METHOD(enum_type,
    		(string name, namespace_context ctx, int size, int asize,
            bool is_signed, vector<enum_info> &elems, const string &typedef_name),
            (name, ctx, size, asize, is_signed, elems, typedef_name))

    virtual bool operator==(const type& _other);
    virtual string to_string() const;
    virtual bool is_enum_type() const;
    vector<enum_info*> & get_fields();
    virtual bool is_signed_type() const;
};




/**
 * \brief Represents a function type
 */
class function_type: public type
{
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<type>(*this);
		ar & ret_type;
		ar & arg_types;
		ar & is_vararg;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<type>(*this);
		ar & ret_type;
		if(ret_type != NULL) {
			type::register_loaded_typeref(&ret_type);
		}
		ar & arg_types;
		for(unsigned int i=0; i<arg_types.size(); i++) {
			type::register_loaded_typeref(&arg_types[i]);
		}
		ar & is_vararg;
		hash_c = 0;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    SHARED_PRIVATE(function_type)

    function_type(type *ret_type, const vector<type*> &arg_types, bool is_vararg);
    void compute_hash();

public:
	type *ret_type;
	vector<type*> arg_types;
	bool is_vararg;

public:
    SHARED_DEFINE_MAKE_METHOD(function_type,
                              (type *ret_type, const vector<type *> &arg_types, bool is_vararg),
                              (ret_type, arg_types, is_vararg))

	virtual bool operator==(const type& _other);

	virtual type *get_return_type();
	const vector<type*> & get_arg_types();
	virtual string to_string() const;
	string to_byte_string() const;
	virtual bool is_function_type() const;

	/**
	 * \brief This function is only useful for function types associated
	 * with methods.
	 * @return Returns the signature of the method, diregarding the type of the
	 * this pointer.
	 */
	function_type* get_method_signature();

};

/**
 * \brief Represents struct fields (name,offset etc.)
 */
struct record_info
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & fname;
        ar & offset;
        ar & t;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & fname;
        ar & offset;
        ar & t;
        type::register_loaded_typeref(&t);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    string fname;
    int offset;
    type *t;
    virtual ~record_info(){ };
    friend ostream& operator <<(ostream &os, const type &obj);

};


enum record_kind {
    STRUCT,
    UNION,
    CLASS
};

/**
 * \brief Represents methods in classes/structs
 */
struct method_info
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & name;
        ar & ns;
        ar & signature;
        ar & virtual_method;
        ar & static_method;
        ar & abstract_method;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
    	il::function_type* temp;
    	ar & name;
		ar & ns;
		ar & temp;
		ar & virtual_method;
		ar & static_method;
		ar & abstract_method;
		signature = temp;
		type::register_loaded_typeref((type**)&signature);



    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    string name;
    namespace_context ns;
private:
    il::function_type* signature;
public:
    il::function_type* get_signature()
    {
    	return signature;
    }
    void  set_signature(il::function_type* fd)
    {
    	signature = fd;;
    }
    bool virtual_method;
    bool static_method;
    bool abstract_method;
    virtual ~method_info(){ };


};

/**
 * \brief Struct, class, union
 */
class record_type : public type
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
    	// this is done here since all child types are guaranteed
    	// to be resolved
    	((record_type*)this)->compute_recursive_fields();
        ar & boost::serialization::base_object<type>(*this);

        ar & name;
        ar & ns;
        ar & kind;
        ar & abstract;

        ar & elems;
        ar & name_to_record_info;
        ar & offset_to_elem;

        ar & bases;
        ar & derivatives;

        ar & recursive_fields;

        ar & member_functions;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<type>(*this);


        ar & name;
        ar & ns;
        ar & kind;
        ar & abstract;

        ar & elems;
        ar & name_to_record_info;
        ar & offset_to_elem;

        ar & bases;
        map<int, record_type*>::iterator it = bases.begin();
        for(; it!= bases.end(); it++)
        {
        	type::register_loaded_typeref((type**)&it->second);


        }
        ar & derivatives;
        for(unsigned int i=0; i<derivatives.size(); i++)
        {
        	type::register_loaded_typeref((type**)&derivatives[i]);
        }


        ar & recursive_fields;
        compare_all_fields = true;
        ar & member_functions;
        compute_hash();

    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

public:
    vector<record_info*> elems;
    map<string, record_info*> name_to_record_info;
    map<int, record_info*> offset_to_elem;
    map<string, method_info*> member_functions;

    map<int, record_type*> bases;
    vector<record_type *> derivatives;

    string name;
    namespace_context ns;
    record_kind kind;
    vector<record_info*> recursive_fields;

    /*
     * This is initially false sine we need to be able to
     * "patch up" inner types while parsing a function.
     * However, after loading its always true;
     */
    bool compare_all_fields;

    /*
     * Is this class abstract?
     */
    bool abstract;


    void compute_recursive_fields();

private:
    SHARED_PRIVATE_CUSTOM_DTOR(record_type)

	record_type(string name,
	            namespace_context ctx,
	            int size,
	            int asize,
	            bool is_union,
	            bool is_class,
	            const vector<record_info *> &elems,
	            const string& typedef_name, bool is_abstract);
    void compute_hash();




public:
    SHARED_DEFINE_MAKE_METHOD(record_type,
                              (string name,
                               namespace_context ctx,
                               int size,
                               int asize,
                               bool is_union,
                               bool is_class,
                               const vector<record_info*> &elems,
                               const string &typedef_name, bool is_abstract),
                              (name, ctx, size, asize, is_union, is_class, elems,
                            		  typedef_name, is_abstract))

    virtual bool operator==(const type& _other);

    void add_member_function(const string & name,const namespace_context & ns,
    		function_type *fd, bool virtual_method,
    		bool static_method, bool abstract_method);
    map<string, method_info*> &get_member_functions();

    void update_incomplete_record(int size, int asize,
    		const vector<record_info*> &elems, bool is_abstract);


    void add_base(record_type *base, long int offset);
    const map<int, record_type*> &get_bases();
    void add_derivative(record_type *sub);
    vector<record_type *> &get_derivatives();

    /**
     * @param The set of all subtypes T such that T is a subtype of
     * this class.
     */
    void get_transitive_subclasses(set<record_type*>& subclasses);

    type** get_typeref_from_index(int i);

    bool is_recursive();
    vector<record_info *> &get_recursive_fields();

    /**
     * @return Does this class have any virtual methods?
     */
    bool has_virtual_methods();

    /**
     * @return Is this an abstract class?
     */
    bool is_abstract();





    virtual string to_string() const;
    virtual bool is_record_type() const;

    /**
     * @return Is this type a supertype of t?
     */
    bool is_supertype_of(il::record_type* t);

    /**
     * @return Is this type a subtype of t?
     */
    bool is_subtype_of(il::record_type* t);

    /**
     * \brief Given a method identified by fun_name and its signature,
     * populates the set of classes that inherit
     * (i.e., do not override this method).
     */
    void get_classes_inheriting_method(string fun_name, il::type* signature,
    		set<il::record_type*>& inheriting_classes);

    /**
     * @return Does the given class contain a method with
     * the given name and signature?
     */
    bool contains_method(string fun_name, type* sig);

    vector<record_info *> &get_fields();

    record_info *get_field_from_name(string name);

    /*
     * This can return null if the index does
     * not denote the start of a field
     */
    record_info *get_field_from_offset(int offset);

    il::type* get_type_from_offset(int offset);

    bool is_struct();
    bool is_class();
    bool is_union();


};


/**
 ** \brief GCC vector type.
 */
/**
 * This is *not* a vararray, but num_elems items
 * of type elem_type in parallel with +-*  etc performed through
 * parallel assembly instructions (if available). Should only see this
 * in high-performance numerical computing, graphics applications etc.
 */
class vector_type : public type
{
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<type>(*this);
		ar & elem_type;
		ar & num_elems;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<type>(*this);
		ar & elem_type;
		type::register_loaded_typeref(&elem_type);
		ar & num_elems;
		hash_c = 0;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    SHARED_PRIVATE(vector_type)

    vector_type(type *elem_type, int num_elems);
    void compute_hash();

protected:
	type *elem_type;
	int num_elems;

public:

    SHARED_DEFINE_MAKE_METHOD(vector_type,
                              (type *elem_type, int num_elems),
                              (elem_type, num_elems))

    virtual bool operator==(const type& _other);
	virtual int get_num_elems();
	virtual type *get_elem_type();
	virtual string to_string() const;
	virtual bool is_vector_type() const;

	virtual int get_size();
	virtual int get_asize();

};

/**
 * \brief Type of a complex number
 */
class complex_type: public type
{
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<type>(*this);
		ar & elem_type;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<type>(*this);
		ar & elem_type;
		il::type::register_loaded_typeref(&elem_type);
		hash_c = 0;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
        ;
    SHARED_PRIVATE(complex_type)

    complex_type(type *elem_type);
    void compute_hash();

protected:
	type *elem_type;
public:

    SHARED_DEFINE_MAKE_METHOD(complex_type,
                              (type *elem_type),
                              (elem_type))

	virtual bool operator==(const type& _other);

    virtual type *get_elem_type();
	virtual string to_string() const;
	virtual bool is_complex_type() const;

	virtual int get_size();
	virtual int get_asize();

};



/**
 * \brief Represents base types like integer and float.
 */
class base_type: public type
{
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<type>(*this);
		ar & name;
		ar & is_signed;
		ar & vt;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<type>(*this);
		ar & name;
		ar & is_signed;
		ar & vt;
		compute_hash();
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    SHARED_PRIVATE(base_type)

    base_type(string name,
              int size,
              int asize,
              bool is_signed,
              value_type vt,
              string typedef_name);

    void compute_hash();

public:
	string name;
	bool is_signed;
	value_type vt;

public:
    SHARED_DEFINE_MAKE_METHOD(base_type,
                              (string name,
                               int size,
                               int asize,
                               bool is_signed,
                               value_type vt,
                               string typedef_name),
                              (name, size, asize, is_signed, vt, typedef_name))

	virtual bool operator==(const type& _other);

	virtual string get_name() const;

	virtual bool is_void_type() const;
	virtual bool is_signed_type() const;
	virtual string to_string() const;
	virtual bool is_base_type() const;

};




/**
 * \brief Invalid type
 */

class invalid_type: public type
{
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<type>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<type>(*this);
		hash_c = 0;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
	invalid_type();
	virtual ~invalid_type() {}
	invalid_type(const invalid_type &rhs) {}
	invalid_type &operator=(const invalid_type &rhs) { return *this; }

    void compute_hash();


public:

    SHARED_DEFINE_MAKE_METHOD(invalid_type,
                             (),
                              ())

	virtual bool operator==(const type& _other);


	virtual string to_string() const;
	virtual bool is_invalid_type() const { return true; }

	virtual int get_size();
	virtual int get_asize();

};

/*
 * ostream operators
 */

ostream& operator <<(ostream &os, const type &obj);


type * get_integer_type();
type *get_unsigned_integer_type();
type *get_char_type();
type* get_void_type();
type* get_void_ptr_type();
function_type* get_void_function_type();


}

#endif /*TYPE_H_*/


