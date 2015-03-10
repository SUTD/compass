#ifndef _IL_H
#define _IL_H


extern void il_test(const_tree tree);

/* Called from c-decl.c and cp/decl2.c
 * */
extern void  process_translation_unit(tree tus);

/* Called from c-gimplify.c */
extern void process_fundecl_body(tree fundecl);

/*
 * Called from cp/decl.c
 */
void process_declaration(tree declaration, tree parent);


extern void set_sail_output_dir(char* dir);

extern void add_loop_mapping(location_t loc, tree init_exp, tree cond,
		tree incr, tree body, tree break_label, tree continue_label,
		bool cond_is_first, bool is_for, tree t, tree top, tree c_label,
		tree b_label, tree entry, tree exit);



extern void enable_regression_output(void);


#endif

