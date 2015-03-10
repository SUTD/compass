#include "MemGraph.h"
#include "StaticAssertElement.h"
#include "Type.h"
#include "MemNode.h"


#define WIDENING_CONDITION 10
#define GET_MEMORY_NODE_CHECK false
#define CONSIDER_TYPE_ASSERT false
#define TESTING_SOURCE false

//inside ap also can get its node
MemGraph::MemGraph() {
	_mem_nodes = new set<MemNode*>();
	_edges = new set<MemEdge*>();
	_ap_map = new map<AccessPath*, MemNode*>();
	_edge_counter = 0;
	_node_counter = 0;
	return_ap = NULL;
	unmodeled_val_node = NULL;
	_init_node = NULL;
	_init_target_node = NULL;
}

MemGraph::MemGraph(const MemGraph& other){
	_mem_nodes = other._mem_nodes;
	_edges = other._edges;
	_ap_map = other._ap_map;
	_edge_counter = other._edge_counter;
	_node_counter = other._node_counter;
	return_ap = other.return_ap;
	_arg_locs = other._arg_locs;
	_global_locs = other._global_locs;
	_root_locs = other._root_locs;
	unmodeled_val_node = other.unmodeled_val_node;
	_init_node = other._init_node;
	_init_target_node = other._init_target_node;
}

MemNode* MemGraph::get_init_node(){

	if(_init_node != NULL)
		return _init_node;

	set_init_node();
	return _init_node;
}

MemNode* MemGraph::set_init_node(){
	sail::Variable* var = new sail::Variable("init", il::get_integer_type(), true);
	//calling this one is OK, because same purpose of using "init" variable;
	AccessPath* ap = AccessPath::get_ap_from_symbol(var);
	_init_node = this->get_memory_node(ap);

	//zero constant calling AccessPath::get_ap_from_symbol() also is safe;
	sail::Constant* zero = new sail::Constant(0, true, 32);
	AccessPath* zero_ap = AccessPath::get_ap_from_symbol(zero);
	MemNode* zero_node = get_memory_node(zero_ap);
	add_edge_to_next_layer(zero_node, zero_ap);



	pair<MemNode*, int> p(_init_node, 0);
	MemNode* target = *(zero_node->get_targets_from_offset(0)->begin());

	add_edge_for_single_source_and_single_target(
			p,  target, false);
	assert(_init_node != NULL);

	_init_target_node = target;
}

MemNode* MemGraph::get_init_deref_node(){
	if (_init_node == NULL)
		set_init_node();

	return _init_target_node;

}


MemNode* MemGraph::get_memory_node(AccessPath* represent_ap) {
	assert(represent_ap != NULL);

	if (GET_MEMORY_NODE_CHECK) {
		cout << "need to find node for " << represent_ap->to_string() << endl;
		if (represent_ap->get_ap_type() == AP_VARIABLE) {
			cout << "Is Variable Type whose sail variable is "
					<< ((Variable*) (represent_ap))->get_original_var()->to_string()
					<< endl;
		}
	}
	assert(_ap_map != NULL);

	if (GET_MEMORY_NODE_CHECK) {
		if (_ap_map->size() != 0) {
			map<AccessPath*, MemNode*>::iterator it = _ap_map->begin();
			for (; it != _ap_map->end(); it++) {
				cout << "AP == " << it->first->to_string() << "[Node ]"
						<< it->second->to_string() << endl;
			}
		}
	}

	if (has_location(represent_ap)) { //get its reprentive's MemNode;
		if(GET_MEMORY_NODE_CHECK)
			cout << "Found the corresponding node "<< ((*_ap_map)[represent_ap])->to_string() <<endl;
		assert((*_ap_map)[represent_ap] != NULL);
		return (*_ap_map)[represent_ap];
	}

	//build the MemNode, it is a representative;
	MemNode* mem_node = NULL;
	if(represent_ap->get_ap_type() == AP_UNMODELED)
		mem_node = new MemNode((UnmodeledValue*)represent_ap);
	else
		mem_node  = new MemNode(this->_node_counter++, represent_ap);

	(*_ap_map)[represent_ap] = mem_node;


	if(GET_MEMORY_NODE_CHECK)
		cout << endl<<"<<<< >>>>>>********* inserting node for  " << represent_ap->to_string()
			<< "   whose node infor is "<< mem_node->to_string()<< endl;

	_mem_nodes->insert(mem_node);

	if (mem_node->get_representative()->to_string().find("static_assert")
			!= string::npos) {
		if (GET_MEMORY_NODE_CHECK) {
			cout << "set as assert node " << endl;
			cout << mem_node->to_string(true) << endl;
		}
		mem_node->set_as_assert_node();
		if(GET_MEMORY_NODE_CHECK)
			cout << "mem_node static_assert ?? " << mem_node->is_assert_node()
				<< endl;
	}


	//some node type may need to init
	if (GET_MEMORY_NODE_CHECK) {
		cout << "Memnode representative type  ?? "
				<< mem_node->get_representative()->get_ap_type() << endl;
		if (mem_node->get_representative()->get_inner() != NULL)
			cout << "whose inner type is "
					<< mem_node->get_representative()->get_inner()->get_ap_type()
					<< endl;
	}

	mem_node->one_field_node_init();

	if (represent_ap->get_ap_type() == AP_VARIABLE) {
		_root_locs.insert(mem_node);

		sail::Variable* sail_var =
				((Variable*) represent_ap)->get_original_var();

		if (sail_var->is_argument()) {
			_arg_locs[represent_ap] = mem_node;
		}else if (sail_var->is_global()) {
			_global_locs[represent_ap] = mem_node;
		}else if(sail_var->is_return_variable()){
			return_ap = represent_ap;
		}
		//what if global variable?
		if(GET_MEMORY_NODE_CHECK)
			cout << mem_node->to_string()
				<< " is root location !!!!!!!!!!!!!!!!!!!!!!!! " << endl;
	}
	assert(mem_node != NULL);
	return mem_node;
}


map<AccessPath*, MemNode*>& MemGraph::get_argument_location_map(){
	return this->_arg_locs;
}

AccessPath* MemGraph::get_return_var_ap(){

	return this->return_ap;
}

MemNode* MemGraph::get_return_var_node(){

	if(return_ap != NULL)
		return this->get_memory_node(return_ap);
	return NULL;
}

bool MemGraph::has_location(AccessPath* _ap) {
	assert(_ap_map != NULL);
	return (_ap_map->count(_ap) > 0);
}


MemEdge* MemGraph::get_edge(MemNode* source, MemNode* target, int s_off,
		int t_offset, bool init) {
	//assert(source->get_offset_2_ap_map()->count(s_off) > 0);

	//in the case that all virtual function is called by base type,
	//but base type doesn't contain some fields of derivative;
	if(source->get_offset_2_ap_map()->count(s_off) == 0){
		cerr << "Node " << source->to_string() << endl;
		cerr << "	does not contain offset  " << s_off << "!!!" << endl;
		return NULL;
	}

	MemEdge* edge = new MemEdge(this->_edge_counter++, source, target, s_off,
			t_offset);

	if (!edge_existing(edge)) {
		update_graph_for_adding_edge(edge);

		//the init of size of dereference of alloc ;
		if (init) {
			if ((source->get_representative()->get_ap_type() == AP_DEREF)
					&& (source->get_representative()->get_inner()->get_ap_type()
							== AP_ALLOC)) {
				edge->set_as_init_edge();
				cout << "set as init edge " << endl;
				//edge->set_as_default_edge();
//			target->set_as_default_node();
//			target->set_default_edge(edge);
//			source->insert_to_default_map(s_off, target);
			}
		}
		if(0){
			cout << endl << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% " << endl;
			cout << "making edge NO.  " << edge->get_time_stamp() << endl;
			cout << "\t "<<" content is   " << edge->to_string() << endl;
			cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% " << endl;
		}
		return edge;

	} else { //already existing

		cout << "already existed, delete it !" << endl;
		delete edge;

		this->_edge_counter--;

		return NULL;
	}
}



MemGraph::~MemGraph() {
	//cout << " free space of MemGraph called here ! " << endl;
}



void MemGraph::add_edge(MemEdge* edge) {
	assert(edge!= NULL);
	assert(_edges != NULL);
	_edges->insert(edge);
}



void MemGraph::add_node(MemNode* node) {
	assert(_mem_nodes != NULL);
	_mem_nodes->insert(node);
}



//adding edge for ap inside node (out edges for this node);
//ap inside node, for eg, ap could be FieldSelection for some deref node;
//differentiate representative and ap, a calling of this function adding one level of edge
void MemGraph::add_edge_to_next_layer(MemNode* node, AccessPath* ap) {

	if(0)
		cout << "\t" << "inside add Edge To Next Layer whose node is :: "
			<< node->to_string() << endl;

	AccessPath* ap_rep = node->get_representative();

	MemNode* deref_node = NULL;


	if (ap->get_ap_type() == AP_ALLOC) {

		assert(ap_rep == ap);
		making_deref_node_for_next_level(node, ap, deref_node, 0, 0);
		assert(deref_node != NULL);

		il::type* t = ap->get_type();
		//cout << "type is " << t->to_string() << endl;
		assert(t->is_pointer_type());

		//fill in different fields of deref of alloc
		if (t->get_inner_type()->is_record_type()) { //alloc record_type

			//rtti field
			FieldSelection* rtti_f = FieldSelection::make("rtti",
					RECORT_RIIT_FIELD_OFFSET, il::get_integer_type(),
					deref_node->get_representative(), NULL);

			deref_node->add_mem_node_info(RECORT_RIIT_FIELD_OFFSET, rtti_f);

			add_edge_to_next_layer(deref_node, rtti_f);

			//size field
			FieldSelection* size_f = FieldSelection::make("size",
					SIZE_OFFSET, il::get_unsigned_integer_type(),
					deref_node->get_representative(), NULL);
			deref_node->add_mem_node_info(SIZE_OFFSET, size_f);
			//leaving it update among the update for ins;
			vector<il::record_info*> f =
					((il::record_type*) (t->get_inner_type()))->get_fields();
			vector<il::record_info*>::iterator it = f.begin();

			//updating when calling init
			for (; it != f.end(); it++) {
				FieldSelection* f_s = FieldSelection::make((*it)->fname,
						(*it)->offset, (*it)->t,
						deref_node->get_representative(), NULL);
				deref_node->add_mem_node_info((*it)->offset, f_s);
				assert(deref_node != NULL);
			}

			//collect the location;


			return;

		} else { //alloc array type
			//leaving the f_size updating target node in the update for ins step
			string array_size = "size";
			FieldSelection* f_size = FieldSelection::make(array_size,
					SIZE_OFFSET, il::get_unsigned_integer_type(),
					deref_node->get_representative(), NULL);
			deref_node->add_mem_node_info(SIZE_OFFSET, f_size);
			FieldSelection* eles = FieldSelection::make("array_ele", 0,
					t->get_inner_type(), deref_node->get_representative(),
					NULL);
			deref_node->add_mem_node_info(0, eles);


			//collect the location;
			return;
		}

	}

	if (ap->get_ap_type() == AP_STRING) { //string node will point to the char node;

		StringLiteral* str_literal_ap = static_cast<StringLiteral*>(ap);

		string s = str_literal_ap->get_string_constant();

		char first_char;
		if (s.size() > 0)
			first_char = s[0];
		else
			first_char = '\0';

		ConstantValue* first_c = ConstantValue::make(first_char);

		get_memory_node(first_c);

		Deref* const_deref = Deref::make(first_c);

		MemNode* t_node = get_memory_node(const_deref);

		pair<MemNode*, int> p(node, 0);

		this->add_edge_for_single_source_and_single_target(p, t_node, true);

		return;
	}

	if (ap_rep->get_ap_type() == AP_ARITHMETIC) {

		ArithmeticValue* ari = static_cast<ArithmeticValue*>(ap);

		making_deref_node_for_next_level(node, ari, deref_node, 0, 0);
		return;
	}

	if(ap_rep->get_ap_type() == AP_FUNCTION){

		FunctionValue* fv = static_cast<FunctionValue*>(ap);

		making_deref_node_for_next_level(node, fv, deref_node, 0, 0);

		return;
	}

	if(ap_rep->get_ap_type() == AP_UNMODELED){
		UnmodeledValue* unmodel_val = static_cast<UnmodeledValue*>(ap);
		making_deref_node_for_next_level(node, unmodel_val, deref_node, 0, 0);
		return;
	}

	if (ap->get_ap_type() == AP_CONSTANT) {

		//cout << "!! constant ap " << endl;
		assert(ap_rep == ap);

		//making a constantvalue ap(the inner ap)
		ConstantValue* constant_value = static_cast<ConstantValue*>(ap);

		making_deref_node_for_next_level(node, constant_value, deref_node, 0,
				0);

		assert(deref_node != NULL);

		return;

	} else if (ap->get_ap_type() == AP_VARIABLE) {

		//cout << ap->to_string() <<" is  variable ap !!! " << endl;

		assert(ap_rep == ap);

		Variable* var = static_cast<Variable*>(ap_rep);

		il::type* il_type = ((Variable*)var)->get_type();

//		sail::Variable* sail_var = var->get_original_var();


		//cout << "check type  " << il_type->to_string() << endl;

		making_deref_node_for_next_level(node, var, deref_node, 0, 0);

		assert(deref_node != NULL);
		assert(il_type != NULL);

		//done of the deref of variable;
//		if (!(sail_var->get_type()->is_pointer_type())) //base type (one level of deref)
//			return;

		if(!il_type->is_pointer_type())
			return;

		else { //update the fields of the new created Node and generate a bunch of next level edges for each fields;
//			cout
//					<< " ==> making Edges for deref (who has fields or array element) "
//					<< endl;
			il::pointer_type* p_t =
					static_cast<il::pointer_type*>(il_type);
			assert(p_t->get_inner_type() != NULL);


			if (p_t->get_inner_type()->is_record_type()) { //update the created nodes and build next level of nodes;

				//cout << " should be record type !!" << endl;
				il::record_type* record_t =
						static_cast<il::record_type*>(p_t->get_inner_type());
				vector<il::record_info*> fields = record_t->get_fields();
				vector<il::record_info*>::iterator it = fields.begin();
				vector<FieldSelection*> field_selections;
				//rtti field
				FieldSelection* rtti_f = FieldSelection::make("rtti",
						RECORT_RIIT_FIELD_OFFSET,
						il::get_unsigned_integer_type(),
						deref_node->get_representative(), NULL);
				field_selections.push_back(rtti_f);
				deref_node->add_mem_node_info(RECORT_RIIT_FIELD_OFFSET, rtti_f);
				for (; it != fields.end(); it++) {
					FieldSelection* f_s = FieldSelection::make((*it)->fname,
							(*it)->offset, (*it)->t,
							deref_node->get_representative(), NULL);
					field_selections.push_back(f_s);
					deref_node->add_mem_node_info((*it)->offset, f_s);
					assert(deref_node != NULL);
				}
				for (vector<FieldSelection*>::iterator it1 =
						field_selections.begin(); it1 != field_selections.end();
						it1++) {
					add_edge_to_next_layer(deref_node, *it1);
				}


			} else { //added for array

				//cout << "should be array type, check !!! " << endl;
				string array_size = "array_size";

				FieldSelection* f_size = FieldSelection::make(array_size,
						SIZE_OFFSET, il::get_unsigned_integer_type(),
						deref_node->get_representative(), NULL);
				deref_node->add_mem_node_info(SIZE_OFFSET, f_size);

				FieldSelection* eles = FieldSelection::make("array_ele", 0,
						p_t->get_inner_type(), deref_node->get_representative(),
						NULL);
				deref_node->add_mem_node_info(0, eles);

				add_edge_to_next_layer(deref_node, f_size);
				add_edge_to_next_layer(deref_node, eles);

			}
		}

	} else if (ap_rep->get_ap_type() == AP_DEREF) { //Once inside this, last level to process,so return immediately

		//make sure that we init argument deref field;

		assert(ap->get_ap_type() == AP_FIELD);
		if(ap->get_ap_type() != AP_FIELD){
			cerr << "deref of whom ? " << endl;
			print_ap_type(ap);
			assert(false);
		}

		FieldSelection* f_s = static_cast<FieldSelection*>(ap);

		making_deref_node_for_next_level(node, f_s, deref_node,
				f_s->get_field_offset(), 0);

		assert(deref_node != NULL);

		return;

	} else {

		cout << "ap type " << ap_rep->get_ap_type() << endl;
		assert(false);
	}
}



//adding deref nodes for one next level(for inner ap),
//given content to "deref_node", including adding edge and node
void MemGraph::making_deref_node_for_next_level(MemNode* source,
		AccessPath* inner, MemNode*& deref_node, int source_offset,
		int target_offset) {

	Deref* deref = Deref::make(inner);

	deref_node = get_memory_node(deref); //make sure its unique

	if (0) {
		cout << "source " << source->to_string() << "offset " << source_offset
				<< endl;

		cout << "target " << deref_node->to_string() << "offset "
				<< target_offset << endl;
	}

	MemEdge* edge = this->get_edge(source, deref_node, source_offset,
			target_offset);

	if (edge != NULL) {
		edge->set_as_default_edge();
		edge->get_target_node()->set_as_default_node();
		edge->get_target_node()->set_default_edge(edge);
		source->insert_to_default_map(source_offset, deref_node);
	}
	//cout << "out makingDerefNodeForNextLevel function!" << endl;

	assert(deref_node != NULL);
}


// if we are doing flow insensitive analysis, we do not need to consider the order, so the assertion is not necessary to hold;
//all variables should have nodes already;

//source_node and its corresponding offset needs to be updated
void MemGraph::find_source_nodes(AccessPath* lhs_ap_variable,
		sail::Instruction* ins, vector<pair<MemNode*, int>>& s_nodes) {

	MemNode* source_node = NULL;
	AccessPath* lhs_ap_representative = lhs_ap_variable;
	int offset = 0;

	switch (ins->get_instruction_id()) {

	case sail::ADDRESS_STRING:
	case sail::CAST:
	case sail::ASSIGNMENT: //v1= v2;
	case sail::BINOP: //v1 = v2 op v3;
	case sail::UNOP:
	case sail::LOAD: { //v1= v2.b;

		assert(has_location(lhs_ap_representative));
		assert((lhs_ap_representative->get_ap_type() == AP_VARIABLE));
		source_node = get_memory_node(lhs_ap_representative);

		//should find the generated nodes for it
		assert(source_node->get_offset_2_ap_map()->size() ==1);
		assert(source_node->get_offset_2_ap_map()->count(0) > 0);

		pair<MemNode*, int> p(source_node, offset);
		s_nodes.push_back(p);

	}
		break;
	case sail::STORE: { //v1.a = v2; sail instruction (vi+ offset) = vj;
		//set representative ap;
		assert(lhs_ap_variable->get_ap_type() == AP_VARIABLE);

		sail::Store* store = static_cast<sail::Store*>(ins);

		il::type* p_t = ((Variable*)lhs_ap_variable)->get_type();

		if(!store->get_lhs()->get_type()->is_pointer_type()){
			cerr << store->to_string() << endl;
			cerr << "store lhs =>" << store->get_lhs()->to_string()
			<< "  type => " << store->get_lhs()->get_type()->to_string() << endl;
		}


		if (CONSIDER_TYPE_ASSERT) {
			assert(store->get_lhs()->get_type()->is_pointer_type());
		}


		MemNode* cur_node = get_memory_node(lhs_ap_variable);



		//v1 could point to different nodes, so the targets may more than one
		set<MemNode*>* tars = cur_node->get_targets_from_offset(0);

		for (set<MemNode*>::iterator it = tars->begin(); it != tars->end();
				it++) {

			MemNode* s_node = *it;


			//if it is the default node of field selection, we do not need to consider it;
			if(s_node->is_default_node()){
				MemEdge* e = s_node->get_default_edge();

				if(e->get_source_ap(e->get_source_offset())->get_ap_type() == AP_FIELD){
					cout << " {{{{{ skip the node }}} " <<  s_node->to_string()
							<< "   which is born for the argument's default field !"
							<< endl;
					continue;
				}

				//what if the field is initialized as NULL (deref of zero)
				if(e->get_source_ap(e->get_source_offset())->get_ap_type() == AP_CONSTANT){
					cout << " {{{{{ skip the node }}} " <<  s_node->to_string()
					<< "   which is born for the default initialization of NULL !"
					<< endl;
					continue;
				}
			}


			//what if the field is initialized as NULL (deref of zero)
			if (!p_t->is_pointer_type()) {
				cerr << "check instruction's lhs type! " << store->to_string() << endl;
				cerr << "type of p_t " << p_t->to_string() << endl;
				assert(false);
				assert(p_t->is_pointer_type());
			}

			if (p_t->get_inner_type()->is_record_type())
				offset = store->get_offset();
			pair<MemNode*, int> p(s_node, offset);
			s_nodes.push_back(p);
		}


		if (TESTING_SOURCE) {
			if (s_nodes.size() == 0) {
				cerr << "ins  " << ins->to_string() << endl;
				cerr << "cur node is " << cur_node->to_string() << endl;
				cerr << "ap is " << cur_node->get_representative()->to_string()
						<< endl;
				cerr << "ap  type "
						<< cur_node->get_representative()->get_ap_type()
						<< endl;
				if (cur_node->get_representative()->get_ap_type()
						== AP_VARIABLE) {
					sail::Variable* var =
							((Variable*) (cur_node->get_representative()))->get_original_var();
					cout << "sail Variable is " << var->to_string() << endl;
				}

				//assert(false);
			}
		}
	}
		break;

	case sail::FUNCTION_CALL: { // vi = alloc(); or vi = foo()

		if (lhs_ap_variable->get_ap_type() == AP_ALLOC) {
			Deref* der = Deref::make(lhs_ap_variable);
			lhs_ap_representative = der;
			source_node = get_memory_node(lhs_ap_representative);
			il::type* t = lhs_ap_variable->get_type();
			assert(t->is_pointer_type());

			map<int, AccessPath*>* offset_2_source = source_node->get_offset_2_ap_map();

			for(auto it = offset_2_source->begin();
					it != offset_2_source->end(); it ++)
			{
				pair<MemNode*, int> p(source_node, it->first);
				s_nodes.push_back(p);
			}

//			if (t->get_inner_type()->is_record_type())
//				offset = SIZE_OFFSET;
//			else
//				offset = SIZE_OFFSET;

		} else if (lhs_ap_variable->get_ap_type() == AP_VARIABLE) {
			//real function call not only alloc space
			source_node = get_memory_node(lhs_ap_representative);

			pair<MemNode*, int> p(source_node, offset);
			s_nodes.push_back(p);
		} else {

			cout << "check here " << endl;
			assert(false);

		}


	}
		break;
	default: {
		cout << "type of instruction is " << ins->get_instruction_id() << endl;
		assert(false);
	}
		break;
	}
}



//find target nodes and its corresponding offset //target offset should always be 0,rhs
void MemGraph::find_target_node(AccessPath* rhs_ap_variable,
		sail::Instruction* ins, set<MemNode*>*& targets) {

	AccessPath* rhs_ap_representative = rhs_ap_variable;
	targets = new set<MemNode*>();
	bool from_ap_to_memnode = true;
	int offset = 0;
	if(0)
		cout << "check ap representative in find_target_node :: "
			<< rhs_ap_representative->to_string() << "  " << ">>>> ap type is "
			<< rhs_ap_variable->get_ap_type() << rhs_ap_representative->get_type()->to_string()<<endl;
	//assert(has_location(rhs_ap_representative));

	//get its node;
	set<MemNode*>* temp_source_n = new set<MemNode*>();

	//MemNode* temp_source_node = NULL;

	switch (ins->get_instruction_id()) {
	case sail::FUNCTION_CALL: { //vi = alloc();vi = foo(); target could be size ap and alloc ap
		assert(
				(rhs_ap_representative->get_ap_type() == AP_VARIABLE) ||
				(rhs_ap_representative->get_ap_type() == AP_CONSTANT)
				||(rhs_ap_representative->get_ap_type() == AP_ALLOC));
	}
		break;
	case sail::ASSIGNMENT: { //v1= v2;
		assert(
				(rhs_ap_representative->get_ap_type() == AP_VARIABLE) || (rhs_ap_representative->get_ap_type() == AP_CONSTANT));
	}
		break;
	case sail::ADDRESS_STRING: {
		assert(rhs_ap_representative->get_ap_type() == AP_STRING);
		targets->insert(this->get_memory_node(rhs_ap_variable));
		return;
	}
		break;
	case sail::LOAD: { //v1= v2->b;. or it could be vi = array+ variable(offset)

		sail::Load* ld = static_cast<sail::Load*>(ins);
		//cout << "load instruction is " << ld->to_string() << endl;
		if(0)
			cout << "rhs of load instruction " << ld->get_rhs()->to_string()
				<<endl <<endl;
		assert(ld->get_rhs()->get_type()->is_pointer_type());
		il::pointer_type* p_t =
				static_cast<il::pointer_type*>(ld->get_rhs()->get_type());
		if (p_t->get_inner_type()->is_record_type()) {
			Deref* deref = Deref::make(rhs_ap_variable, NULL);
			rhs_ap_representative = deref;
			assert(rhs_ap_representative->get_ap_type() == AP_DEREF);

		} else { //added for array
			from_ap_to_memnode = false;

			MemNode* temp_node = get_memory_node(rhs_ap_variable);
			set<MemNode*>* t = temp_node->get_targets_from_offset(0);

			//ONLY FOR TEST
			//cout << "target size is " << t->size() << endl;
			if(t->size() == 0){
				return;
			}

			//assert(t->size() == 1);
			//temp_source_node = *(t->begin());
			temp_source_n = t;
		}

		//cout << "rhs type " << ld->get_rhs()->get_type()->to_string() << endl;
		if (ld->get_rhs()->get_type()->is_pointer_type()) {
			il::pointer_type* p_t =
					static_cast<il::pointer_type*>(ld->get_rhs()->get_type());
			if (p_t->get_inner_type()->is_record_type())
				offset = ld->get_offset();
		}
	}
		break;
	case sail::STORE: { //v3->a2 = v4
		assert(
				(rhs_ap_representative->get_ap_type() == AP_VARIABLE)
				|| (rhs_ap_representative->get_ap_type() == AP_CONSTANT));
	}
		break;
	case sail::UNOP:
	case sail::BINOP: { //v1 = v2 binop v3; (where v2 could be array type)
		assert(
				(rhs_ap_representative->get_ap_type() == AP_VARIABLE)
				|| (rhs_ap_representative->get_ap_type() == AP_CONSTANT));
	}
		break;
	default:
		break;
	}


	if(from_ap_to_memnode)
		temp_source_n->insert(get_memory_node(rhs_ap_representative));





	set<MemNode*>::iterator it_tar = temp_source_n->begin();

	//cout << " = = = = = size of temp_source_n is  " << temp_source_n->size() << endl;
	for (; it_tar != temp_source_n->end(); it_tar++) {


		//cout << "	>>>>>>>>>>>> offset is " << offset << endl;
		set<MemNode*>* target_nodes = (*it_tar)->get_targets_from_offset(offset);

		for (set<MemNode*>::iterator it = target_nodes->begin(); it != target_nodes->end(); it++) {
			if(0)
				cout << "inserting as target node ^^^^  " << (*it)->to_string()
					<< "representative type is "
					<< (*it)->get_representative()->get_type()->to_string()
					<< endl;

			targets->insert(*it);
		}
		cout<< "------------------- end " << endl;
	}
	//delete target_nodes;
}



void MemGraph::process_bitwise_not(set<MemNode*>*& target,
		set<MemNode*>*& new_target, pair<MemNode*, int>& lhs_node) {
	for (set<MemNode*>::iterator it = target->begin(); it != target->end();
			it++) {
		MemNode* n = *it;
		MemNode* inner_node = n->get_inner_ap_node();
		AccessPath* ap = inner_node->get_representative();
		if ((ap->get_ap_type() == AP_VARIABLE)
				|| (ap->get_ap_type() == AP_ARITHMETIC)
				|| (ap->get_ap_type() == AP_CONSTANT)) {
			AccessPath* av = ap->multiply_ap(-1);
			//MemNode* n_node = this->get_memory_node(av);
			//add_edge_to_next_layer(n_node, av);

			AccessPath* av_arith = ArithmeticValue::make_minus(av,
					ConstantValue::make(1));
			MemNode* av_arith_node = this->get_memory_node(av_arith);
			add_edge_to_next_layer(av_arith_node, av_arith);
			MemNode* deref_node = this->get_memory_node(Deref::make(av_arith));
			new_target->insert(deref_node);
			//d = Deref::make(av, il::get_integer_type());
		} else {
			cout << "ap type is   :: " << ap->get_ap_type() << endl;
			assert(false);
		}
	}
	assert(new_target->size() != 0);

	add_edge_for_source_and_targets(lhs_node, new_target);
}



void MemGraph::process_negation(set<MemNode*>*& target,
		set<MemNode*>*& new_target, pair<MemNode*, int>& lhs_node) {

	//if target contain UNCERTIANTY, we just saying it is uncertainty;

	bool flag = false;
	for(set<MemNode*>::iterator it = target->begin();
			it != target->end(); it++){

			MemNode* n = *it;
			MemNode* inner_node = n->get_inner_ap_node();

			AccessPath* ap = inner_node->get_representative();

			//once found, set target as umodeled_val_node, then return;
			if(ap->get_ap_type() == AP_UNMODELED){
				new_target->insert(unmodeled_val_node);
				flag = true;
				break;
			}
	}

	//otherwise doing the calculation of negation;
	if (!flag) {
		for (set<MemNode*>::iterator it = target->begin(); it != target->end();
				it++) {

			MemNode* n = *it;

			MemNode* inner_node = n->get_inner_ap_node();

			AccessPath* ap = inner_node->get_representative();

			if ((ap->get_ap_type() == AP_VARIABLE)
					|| (ap->get_ap_type() == AP_ARITHMETIC)
					||(ap->get_ap_type() == AP_DEREF)
					|| (ap->get_ap_type() == AP_FUNCTION)) {

				AccessPath* av = ap->multiply_ap(-1);
				MemNode* n_node = get_memory_node(av);
				add_edge_to_next_layer(n_node, av);
				MemNode* deref_node = get_memory_node(Deref::make(av));
				new_target->insert(deref_node);

			} else if (ap->get_ap_type() == AP_CONSTANT) {

				ConstantValue* cc = (ConstantValue*) ap;
				ConstantValue* n_cc = ConstantValue::make(-cc->get_constant());
				MemNode* const_node = get_memory_node(n_cc);
				add_edge_to_next_layer(const_node, n_cc);
				MemNode* deref_const = get_memory_node(Deref::make(n_cc));
				new_target->insert(deref_const);

			} else {

				cout << ap->to_string() << endl;
				cout << "shouldn't be the ap type of " << ap->get_ap_type()
						<< endl;
				cerr << "Target node is " << n->to_string() << endl;
				print_ap_type(ap);
				assert(false);
			}
		}
	}
	//assert(new_target->size() != 0);
	if (new_target->size() != 0)
		add_edge_for_source_and_targets(lhs_node, new_target);

	delete target;

}



void MemGraph::update_for_unop_ins(AccessPath* lhs_variable,
		AccessPath* rhs_variable, sail::Unop* ins) {
	assert(lhs_variable != NULL);
	assert(rhs_variable != NULL);
	il::unop_type unop_t = ins->get_unop();
	vector<pair<MemNode*, int>> s1;
	find_source_nodes(lhs_variable, ins, s1);
	assert(s1.size() == 1);
	pair<MemNode*, int> lhs_node = s1.at(0);

	set<MemNode*>* t;
	find_target_node(rhs_variable, ins, t);
	set<MemNode*>* new_target = new set<MemNode*>();

	switch (unop_t) {
	case il::_NEGATE: {
		cout << "processing negation ins  "<< ins->to_string() << endl;
		process_negation(t, new_target, lhs_node);
	}
		break;
	case il::_BITWISE_NOT: {
		process_bitwise_not(t, new_target, lhs_node);
	}
		break;
	default: {
		cout << "Type of UNOP " << unop_t << endl;
		assert(false);
	}
	}
}



void MemGraph::update_for_cast_ins(AccessPath* lhs_variable,
		AccessPath* rhs_variable_1, sail::Cast* cast) {
	assert(lhs_variable != NULL);
	assert(rhs_variable_1 != NULL);

	if(1){
		cout << "))))))))) lhs_variable type " <<
				lhs_variable->to_string()
					<<"  " << lhs_variable->get_type()->to_string() << endl;

		cout << "(((((((( rhs_variable type "<<
				rhs_variable_1->to_string()
					<<"  " << rhs_variable_1->get_type()->to_string() << endl;
	}

	//first check it type
	if (lhs_variable->get_type()->is_pointer_type()) {
		il::pointer_type* lhs_t =
				static_cast<il::pointer_type*>(lhs_variable->get_type());
		il::type* lhs_inner = lhs_t->get_inner_type();

		il::pointer_type* rhs_t = NULL;
		il::type* rhs_inner = NULL;

		if (!rhs_variable_1->get_type()->is_pointer_type()) {
			cerr << "INVALID CAST !" << endl;
			assert(false);
		} else {
			//cout << "Check cast instruction " << endl;
			rhs_t = static_cast<il::pointer_type*>(rhs_variable_1->get_type());
			rhs_inner = rhs_t->get_inner_type();

			if (!Type::is_supertype(lhs_inner, rhs_inner)
					&& !Type::is_supertype(rhs_inner, lhs_inner)) {
				if (0) {
					cerr << endl << "Cast instruction " << cast->to_string()
							<< endl;
					cerr << "NOT COMPATIBAL TYPE " << "  lhs :: "
							<< lhs_inner->to_string() << "   rhs:: "
							<< rhs_inner->to_string() << endl;

					cout << "HHHHHHHHHHHHHHHHHHHHH " << endl;
				}
				//assert(false);
			}else{

			}
		}
	}

	vector<pair<MemNode*, int>> s1;
	find_source_nodes(lhs_variable, cast, s1);
	assert(s1.size() == 1);
	pair<MemNode*, int> lhs_node = s1.at(0);
	set<MemNode*>* t;

	find_target_node(rhs_variable_1, cast, t);

	add_edge_for_source_and_targets_based_on_types(lhs_node, t);
}



void MemGraph::update_for_address_string_ins(AccessPath* lhs_variable,
		AccessPath* rhs_variable_1, sail::AddressString* ins) {
	vector<pair<MemNode*, int> > s1;
	find_source_nodes(lhs_variable, ins, s1);
	assert(s1.size() == 1);
	pair<MemNode*, int> lhs_node = s1.at(0);

	set<MemNode*>* t;
	find_target_node(rhs_variable_1, ins, t);
	assert(t->size() == 1);
	add_edge_for_single_source_and_single_target(lhs_node, *(t->begin()));
}


void MemGraph::update_for_ap_alloc(AccessPath* lhs_variable,
		AccessPath* rhs_variable_1, AccessPath* rhs_variable_2,
		sail::Instruction* ins) {
	if (0) {
		cout << " ////  update_for_ap_alloc == " << ins->to_string() << endl;
		cout << "//// 	lhs_variable " << lhs_variable->to_string() << endl;
		cout << "\\\\ 	rhs_variable_1 " << rhs_variable_1->to_string() << endl;
		cout << "\\\\ 	rhs_variable_2 " << rhs_variable_2->to_string() << endl;
	}

	vector<pair<MemNode*, int>> source_1;
	find_source_nodes(lhs_variable, ins, source_1);
	assert(source_1.size() == 1);
	pair<MemNode*, int> ret_source = source_1.at(0);

	//alloc ap
	vector<pair<MemNode*, int>> source_2;
	find_source_nodes(rhs_variable_1, ins, source_2);
	//assert(source_2.size() == 1);




	set<MemNode*>* target1;
	set<MemNode*>* target2;

	//alloc targets;
	assert(rhs_variable_1 != NULL);
	find_target_node(rhs_variable_1, ins, target1);
	assert(target1->size() == 1);

	//size target;
	assert(rhs_variable_2 != NULL);
	find_target_node(rhs_variable_2, ins, target2);


	//it may not hold;
	il::type* t = lhs_variable->get_type();
	assert(t->is_pointer_type());
	if (((il::pointer_type*) t)->is_record_type()){
		assert(target2->size() == 1);
	}


	add_edge_for_source_and_targets(ret_source, target1);


	//init of fields;
	set<MemNode*>* target3 = new set<MemNode*>();
	MemNode* deref_init = get_init_deref_node();
	//cout << "deref_init " << deref_init->to_string() << endl;
	target3->insert(deref_init);

	pair<MemNode*, int > size_field_source;
	auto it = source_2.begin();
	for (; it != source_2.end(); it++) {
		if ((*it).second != SIZE_OFFSET) { //point to the target of
			add_edge_for_source_and_targets(*it, target3, true);
		}else{
			size_field_source = *it;
		}

		//pair<MemNode*, int> allo_source = source_2.at(0);
	}
	add_edge_for_source_and_targets(size_field_source, target2, true);
}


//sth. should be done inside find_source/target_node;
void MemGraph::update_graph_for_ins(AccessPath* lhs_variable,
		AccessPath* rhs_variable_1, AccessPath* rhs_variable_2,
		sail::Instruction* ins) {

	cout << "UUUpdate graph for ins  ============= "
			<< ins->to_string() << endl;

	if (ins->get_instruction_id() == sail::CAST) {
		update_for_cast_ins(lhs_variable, rhs_variable_1, (sail::Cast*) ins);
		return;
	}


	if (ins->get_instruction_id() == sail::UNOP) {
		update_for_unop_ins(lhs_variable, rhs_variable_1, (sail::Unop*) ins);
		return;
	}



	if ((ins->get_instruction_id() == sail::ADDRESS_STRING)
			&& (rhs_variable_1->get_ap_type() == AP_STRING)) {
		update_for_address_string_ins(lhs_variable, rhs_variable_1,
				(sail::AddressString*) ins);
		return;
	}



	if (rhs_variable_1->get_ap_type() == AP_ALLOC) {
		update_for_ap_alloc(lhs_variable, rhs_variable_1, rhs_variable_2, ins);
		return;
	}





	//otherwise
	vector<pair<MemNode*, int>> sources;

	find_source_nodes(lhs_variable, ins, sources);
	//cout << "source size ? " << sources.size() << endl;
	//assert(sources.size() != 0);

	//if we do not find source, then just return;
	if(sources.size() == 0)
		return;

	set<MemNode*>* targets = NULL;


	if (rhs_variable_2 == NULL) {
		//cout << "rhs_variable_2  is empty! " << endl;
//		cout << "second operand is not considered, in inst "
//				<< ins->to_string() << endl;

		find_target_node(rhs_variable_1, ins, targets);



	} else {
		//this should be binop!
		//cout << "binop instruction ! " << endl;
		set<MemNode*>* targets_op1;
		find_target_node(rhs_variable_1, ins, targets_op1);

		set<MemNode*>* targets_op2;
		find_target_node(rhs_variable_2, ins, targets_op2);

		sail::Binop* binop = static_cast<sail::Binop*>(ins);
		il::binop_type bt = binop->get_binop();

		targets = new set<MemNode*>();
		build_combination_arithmetic_nodes(*targets_op1, *targets_op2, bt,
				targets);
	}

	if(targets->size() == 0)
		return;


	assert(sources.size() != 0);
	assert(targets->size() != 0);

	if (0) {
		cout << endl << "!!!!!!!!!!! check target !!!!!!!!!! " << endl;

		for (set<MemNode*>::iterator it = targets->begin();
				it != targets->end(); it++) {

			cout << "Target node  " << (*it)->to_string() << endl;

		}
	}

	if (targets->size() != 0) {
		for (unsigned int i = 0; i < sources.size(); i++) {
			pair<MemNode*, int> source_pair = sources.at(i);
		cout << "source ==> NO." << source_pair.first->get_time_stamp()<<
				"     "<<source_pair.first->to_string() << endl;
		cout << "source offset ==> " << source_pair.second << endl;
			add_edge_for_source_and_targets(source_pair, targets);
		}
	}
}



bool MemGraph::edge_existing(MemEdge* edge) {
	set<MemEdge*>::iterator it = this->_edges->begin();
	for (; it != this->_edges->end(); it++) {
		if ((*it)->get_source_node() == edge->get_source_node())
			if ((*it)->get_source_offset() == edge->get_source_offset())
				if ((*it)->get_target_node() == edge->get_target_node())
					if ((*it)->get_target_offset() == edge->get_target_offset())
						return true;
	}
	return false;
}



void MemGraph::add_edge_for_single_source_and_single_target(
		pair<MemNode*, int>& source_pair, MemNode* t, bool default_flag, bool init_flag) {

	MemEdge* e = get_edge(source_pair.first, t, source_pair.second, 0, init_flag);

	if((default_flag)&&(e != NULL)){
		e->set_as_default_edge();
		e->get_target_node()->set_as_default_node();
		e->get_target_node()->set_default_edge(e);
	}
}




void MemGraph::add_edge_for_source_and_targets_based_on_types(
		pair<MemNode*, int>& source_pair, set<MemNode*>* targets) {

	MemNode* s_n = source_pair.first;

	il::type* s_t = s_n->get_type();


	//il::type* tar_t = NULL;

	for (set<MemNode*>::iterator it = targets->begin(); it != targets->end();
			it++) {




		if (s_t->is_pointer_type()) {
			il::type* s_t_inner = ((il::pointer_type*) s_t)->get_inner_type();

			if (s_t_inner->is_record_type()) {

				//assert(tar_t != NULL);

				this->get_edge(s_n, *it, source_pair.second, 0);

			}
		} else {

			this->get_edge(s_n, *it, source_pair.second, 0);
		}
	}
}



void MemGraph::add_edge_for_source_and_targets(
		pair<MemNode*, int> & source_pair,
		set<MemNode*>* targets, bool init) {

	//add for test purpose
	if(targets->size() == 0)
		return;

	//if the original target of source has already contained unmodeled_var_node, then it needn't to be updated!
	set<MemNode*>* orig_target = source_pair.first->get_targets_from_offset(source_pair.second);
	if(orig_target->count(unmodeled_val_node))
		return;


	set<MemNode*>::iterator it = targets->begin();

	//update graph and corresponding nodes;
	//only when nodes that target point to is not inside the orig_target of source, it needs update
	for (; it != targets->end(); it++) {

		if(orig_target->count(*it) > 0){
//			cout << "edge  " << (*it)->to_string() << "  already exits for source original target!" << endl;
//			cout << "skip adding edge ! " << endl;
			continue;
		}

		MemEdge* edge = get_edge(source_pair.first, *it, source_pair.second, 0, init);
	}
}



void MemGraph::build_combination_arithmetic_nodes(set<MemNode*>& op1_targets,
		set<MemNode*>& op2_targets, il::binop_type bt,
		set<MemNode*>* combi_targets) {
	if(1)
		cout << "inside build_combination_arithmetic_nodes " << get_time() << endl;

	if( (op1_targets.count(unmodeled_val_node))
			||(op2_targets.count(unmodeled_val_node)) ){
		combi_targets->insert(unmodeled_val_node);
		return;
	}



	int i = 0;
	for (set<MemNode*>::iterator it = op1_targets.begin();
			it != op1_targets.end(); it++) {

		for (set<MemNode*>::iterator it_inner = op2_targets.begin();
				it_inner != op2_targets.end(); it_inner++) {

			AccessPath* out_ap = (*it)->get_representative();

			if(0)
				cout <<"-- -- -- ||| " <<out_ap->to_string() <<
				"   ap_type"  << out_ap->get_ap_type() << endl;

			assert(out_ap->get_ap_type() == AP_DEREF);

			AccessPath* inner_ap = (*it_inner)->get_representative();
			if(0)
				cout << "-- --- -- ||| " << inner_ap->to_string() <<
				"  ap_type " << inner_ap->get_ap_type() << endl;

			assert(inner_ap->get_ap_type() == AP_DEREF);

			AccessPath* oper1_ap = out_ap->get_inner();
			AccessPath* oper2_ap = inner_ap->get_inner();


			AccessPath* ari_ap = AccessPath::make_access_path_from_binop(
					oper1_ap, oper2_ap, bt);


			//somehow division could be illegal
			if(ari_ap == NULL)
				return;

			//if it needs widening
			if (i >= WIDENING_CONDITION) {

				if (this->unmodeled_val_node == NULL) {

					UnmodeledValue* unmodeled_val =
							get_UnmodeledValue_access_path(
									il::get_integer_type());

					MemNode* unmodeled_val_node = get_memory_node(
							unmodeled_val);

					//this->unmodeled_val_node = unmodeled_val_node;

					add_edge_to_next_layer(unmodeled_val_node, unmodeled_val);

					MemNode* target =
							*(unmodeled_val_node->get_targets_from_offset(0)->begin());
					//the derefence of UNCERTIANTY NODE;
					this->unmodeled_val_node = target;
					cout << "IIIIIIIIIIIIInserting unmodeled node " << target->to_string() << endl;
				}

				//combi_targets->insert(target);
				combi_targets->insert(unmodeled_val_node);


				return;

			}


			//it is possible that second operand is constant 0, then binary ap could be any type
			if(0)
				cout << "-- -- -- || binary ap is " << ari_ap->to_string()
					<<  " ap type is "<< ari_ap->get_ap_type() <<endl;



			//didn't change anything, but use the existing target
			int offset = 0;
			if (ari_ap->get_ap_type() == AP_FIELD) {

				offset = ((FieldSelection*) ari_ap)->get_field_offset();

				AccessPath* inner_ap = ((FieldSelection*) ari_ap)->get_inner();
				assert(inner_ap->get_ap_type() == AP_DEREF);
				assert(has_location(inner_ap));

				MemNode* reside_node = get_memory_node(inner_ap);

				set<MemNode*>* target = reside_node->get_targets_from_offset(
						offset);

				for (set<MemNode*>::iterator it_tar = target->begin();
						it_tar != target->end(); it_tar++) {
						combi_targets->insert(*it_tar);
				}

			} else if (has_location(ari_ap)) {

				MemNode* reside_node = get_memory_node(ari_ap);

				set<MemNode*>* target = reside_node->get_targets_from_offset(
						offset);

				for (set<MemNode*>::iterator it_tar = target->begin();
						it_tar != target->end(); it_tar++) {
					combi_targets->insert(*it_tar);
				}


			} else {
				if(0){
					cout << "need to create a new node for arithmetic ap!!! " << endl;
					cout << "ari_ap type  " << ari_ap->get_ap_type() << endl;
				}
				assert((ari_ap->get_ap_type() == AP_ARITHMETIC)
						||(ari_ap->get_ap_type() == AP_FUNCTION) || (ari_ap->get_ap_type() == AP_CONSTANT));

				MemNode* avn = this->get_memory_node(ari_ap);
				if(0)
					cout << "GET binary node  " << avn->to_string() << endl;

				add_edge_to_next_layer(avn, ari_ap);

				Deref* arithmetic_deref = Deref::make(ari_ap, NULL);
				MemNode* target = get_memory_node(arithmetic_deref);
				combi_targets->insert(target);
			}

			i ++;
//			MemNode* avn = this->get_memory_node(ari_ap);
//
//			cout << "GET binary node  " << avn->to_string() <<endl;
//
//			add_edge_to_next_layer(avn, ari_ap);
//
//			Deref* arithmetic_deref = Deref::make(ari_ap, NULL);
//			MemNode* target = this->get_memory_node(arithmetic_deref);
//			combi_targets->insert(target);
		}
	}

	cout << "outside the build_combination_arithmetic_nodes " << get_time() << endl;
}



void MemGraph::update_graph_for_adding_edge(MemEdge* e) {
	e->add_edge_to_nodes();
	this->add_edge(e);
}



//a set of nodes that reachable
void MemGraph::collect_reachable_locations(MemNode* cur, set<MemNode*>* res) const {
	if (res->count(cur) > 0)
		return;
	//cout << "LOL inserting " << cur->to_string() << endl;
	res->insert(cur);
	map<int, set<MemEdge*>*>* succs = cur->get_succs();
	map<int, set<MemEdge*>*>::iterator it = succs->begin();
	for (; it != succs->end(); it++) {
		set<MemEdge*>* suc = it->second;
		set<MemEdge*>::iterator it2 = suc->begin();
		for (; it2 != suc->end(); it2++) {
			MemEdge* e = *it2;
			collect_reachable_locations(e->get_target_node(), res);

		}
	}
}



const string MemGraph::to_dotty() const {

	set<MemNode*>* to_print = new set<MemNode*>();
	set<MemNode*>::iterator it_loc = _root_locs.begin();
	for (; it_loc != _root_locs.end(); it_loc++) {
		MemNode* root = *it_loc;
		collect_reachable_locations(root, to_print);
	}

	//add those missing one (those not reachable by root, we ignore it right now)
//	map<AccessPath*, MemNode*>::iterator ii = _ap_map->begin();
//	for(; ii != _ap_map->end(); ii++)
//		to_print->insert(ii->second);

	string res;
	res = "digraph G { rankdir=LR;\n";
	res += "ranksep=1.2;\n";
	res += "nodesep=0.9;\n";
	set<MemNode*>::iterator it = to_print->begin();
	//set<MemNode*> constant_node;
	for (; it != to_print->end(); it++) {

		MemNode* loc = *it;
		AccessPath* ap = loc->get_representative();

//		if(ap->get_ap_type() == AP_CONSTANT){
//			//constant_node.insert(loc);
//			continue;
//		}
//		if(ap->get_ap_type() == AP_DEREF){
//			if(ap->get_inner()->get_ap_type() == AP_CONSTANT){
//				if(loc->get_preds()->size() == 1)
//					continue;
//			}
//
//		}
		res += string("node") + int_to_string((long int) loc)
				+ " [shape = record]";
		res += "[port=e][height=0.1]";
		//cout << "to_dotty ================= 3 " << endl;
		//mark the static_assert_check_variable
		if (loc->is_assert_node()) {

			res += "[color=green]";
		} else if (ap->get_ap_type() == AP_VARIABLE) {
			if (((Variable*) ap)->get_original_var()->is_argument()){
				res += "[color=purple]";
			}else if(((Variable*) ap)->get_original_var()->is_return_variable()){

				res += "[color=brown]";
			}else{

				res += "[color=red]";
			}
		}else if(loc->is_terminal()){
			res += "[color=grey]";
		}
		else if ((loc->is_default_node())
				&& (loc->get_representative()->get_inner()->get_ap_type()
						!= AP_CONSTANT))
			res += "[color=orange]";

		else
			res += "[color=black]";
		res += "[label= \"";
		//cout << "to_dotty ================= 3.1 " << endl;
		res += loc->to_dotty();
		res += "\"]\n";
		//cout << "to_dotty ================= 4 " << endl;
		std::map<int, set<MemEdge*>*>* succs = loc->get_succs();
		std::map<int, set<MemEdge*>*>::iterator it2 = succs->begin();
		for (; it2 != succs->end(); it2++) {
			//cout << "to_dotty ================= 5 " << endl;
			set<MemEdge*>* ss = it2->second;
			set<MemEdge*>::iterator it3 = ss->begin();
			for (; it3 != ss->end(); it3++) {
				MemEdge* e = *it3;
				if (e->is_default_edge())
					res += "edge[color = blue]\n";
				else if(e->is_init_edge())
					res += "edge[color = darkslateblue]\n";
				else
					res += "edge[color = black]\n";
				string s;
				s += " [" + int_to_string(e->get_time_stamp()) + "]";
				res += "\"node" + int_to_string((long int) loc) + "\"";
				if (e->get_source_offset() >= 0)
					res += ":off_s" + int_to_string(e->get_source_offset());
				else
					res += ":off_s_minus"
							+ int_to_string(-e->get_source_offset());
				res += " -> ";
				res += "\"node" + int_to_string((long int) e->get_target_node())
						+ "\"";
				if (e->get_target_offset() >= 0)
					res += ":off_e" + int_to_string(e->get_target_offset());
				else
					res += ":off_e_minus"
							+ int_to_string(-e->get_target_offset());
				res += " [label = \"";
				res += s;
				res += "\"]\n";

			}
			//cout << "to_dotty ================= 6 " << endl;
		}

	}

	res += "}\n";
	delete to_print;
	//cout << "to_dotty ================= end " << endl;
	return res;

}
