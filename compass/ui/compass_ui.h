#ifndef COMPASS_UI_H
#define COMPASS_UI_H

#include <QtGui>
#include "ui_compass_ui.h"
#include <string>
#include <map>
#include <set>
#include <vector>
#include "Semaphore.h"
#include "AnalysisResult.h"
#include "language.h"

using namespace std;

class QDirModel;

namespace KTextEditor
{
	class Document;
	class View;

}

class DataManager;
class Callgraph;
class CGNode;
class Compass;
class SvgViewer;
class DottyViewer;
class MemoryVisualizer;
class Regressions;
class StatsViewer;
class Error;
class SummaryViewer;

class ReportClassifier;

#include <boost/thread.hpp>
#include "Callgraph.h"
class SummaryGraph;

/*
 * Location of the framework folder.
 * For example, if your framework is in folder ~/research/,
 * you set this to ~/research. This path can include ~
 *
 */
#define FRAMEWORK_LOCATION "~/Research/GitHub/compass"

/*
 * Macro definitions to be included in every compass run.
 */

static string join_macro = " -D \"_COMPASS_JOIN(x, y)= _COMPASS_JOIN_AGAIN(x, y)\" ";
static string join_again_macro = " -D \" _COMPASS_JOIN_AGAIN(x, y)=x ## y\"";
static string uniquename_macro = " -D \" _COMPASS_UNIQUENAME(prefix)="
		"_COMPASS_JOIN(prefix, __COUNTER__)\"";
static string helper_macros = join_macro + join_again_macro + uniquename_macro;

static string pos_adt = " -D \"POSITION_DEPENDENT_ADT(type)=void " +
		string("_COMPASS_UNIQUENAME(") +
		string(POSITION_DEPENDENT_ADT_PREFIX) + ")(type* x){};\"";

static string single_valued_adt = " -D \"SINGLE_VALUED_ADT(type)=void " +
		string("_COMPASS_UNIQUENAME(") +
		string(SINGLE_VALUED_ADT_PREFIX) + ")(type* x){};\"";

static string multi_valued_adt = " -D \"MULTI_VALUED_ADT(type)=void " +
		string("_COMPASS_UNIQUENAME(") +
		string(MULTI_VALUED_ADT_PREFIX) + ")(type* x){};\"";

static string spec_function = " -D \"SPEC(name, ret_t, ...)=ret_t "+
		string(COMPASS_SPEC_PREFIX) +"##name (__VA_ARGS__) \"";

static string insert_function = " -D \"ADT_INSERT(adt, key, value) = " +
		string(ADT_INSERT) + "(adt, key, value) \"";

static string read_function = " -D \"ADT_READ(adt, key) = " +
		string(ADT_READ) + "(adt, key) \"";

static string read_pos_function = " -D \"ADT_READ_POS(adt, pos) = " +
		string(ADT_READ_POS) + "(adt, pos) \"";

static string read_ref_function = " -D \"ADT_READ_REF(adt, key) = " +
		string(ADT_READ_REF) + "(adt, key) \"";

static string get_size_function = " -D \"ADT_GET_SIZE(adt) = " +
		string(ADT_GET_SIZE) + "(adt) \"";

static string set_size_function = " -D \"ADT_SET_SIZE(adt, size) = " +
		string(ADT_SET_SIZE) + "(adt, size) \"";

static string remove_function = " -D \"ADT_REMOVE(adt, key) = " +
		string(ADT_REMOVE) + "(adt, key) \"";

static string contains_function = " -D \"ADT_CONTAINS(adt, key) = " +
		string(ADT_CONTAINS) + "(adt, key) \"";

static string resize_function = " -D \"ADT_RESIZE(adt, key) = " +
		string(ADT_RESIZE) + "(adt, key) \"";

static string compass_macro_defs = helper_macros + pos_adt + single_valued_adt +
		multi_valued_adt + spec_function + insert_function + read_function
		+ read_pos_function + read_ref_function + get_size_function +
		set_size_function + remove_function + contains_function +
		resize_function;


#define GCC_PATH FRAMEWORK_LOCATION "/gcc/bin/gcc"
#define GPP_PATH FRAMEWORK_LOCATION "/gcc/bin/g++"
//#define DEX_PARSER FRAMEWORK_LOCATION "/framework/build/dex-parser/dex-parser"
#define DEX_PARSER FRAMEWORK_LOCATION "/framework/build/dex-parser-test/dex-parser-test"
#define ANDROID_PATH "~/android-sdk-linux/platform-tools"


#define REGRESSION_FOLDER  FRAMEWORK_LOCATION "/framework/compass/regressions"

/*
 * This file is included on top of every file we compile.
 * It, for instance, defines a prototype for static_assert() to
 * prevent linker errors.
 */
#define COMPASS_INCLUDE_DIR  FRAMEWORK_LOCATION "/framework/compass/runtime/"
#define COMPASS_JAVA_RUNTIME_DIR FRAMEWORK_LOCATION "/framework/compass/java-runtime/"
#define COMPASS_JAVA_RUNTIME FRAMEWORK_LOCATION "/framework/compass/java-runtime/Compass.java"
#define COMPASS_INCLUDE_FILE "compass-runtime.h"

string double_to_string(double s);
void collect_all_c_files(set<string> & reg_paths, string prefix);
void collect_all_java_files(set<string> & reg_paths, string prefix);
void collect_all_class_files(set<string> & reg_paths, string prefix);

bool is_header_file(string s);
bool is_c_file(string s);
bool is_cpp_file(string s);
bool is_java_file(string s);


string ee(string e);

class FileView;


class compass_ui : public QWidget
{
    Q_OBJECT

public:
    QDirModel *file_model;
    set<string> cur_files;

    QStandardItemModel* callgraph_model;
    QTreeView *callg_tree;
    map<QStandardItem*, Identifier> call_item_to_id;

    QStandardItemModel* unit_model;
    QTreeView *unit_tree;

    QMenu* unit_popup_menu;

    /*
     * Maps the QStandardItem of identifiers to the actual identifier.
     */
    map<QStandardItem*, Identifier> unit_item_to_identifier;

    QLineEdit* entry_point_label;

    Identifier current_entry_id;
    int num_funs_to_analyze;

    QStandardItem* selected_unit;

    set<Identifier> excluded_units;




    /*
     * set of files we modified
     */
    set<string> modified_files;



    map<string, set<Error*> > errors_per_file;

    string settings_dir;
    string current_dir;
    string compass_config;
    string regression_dir;
    string current_config;

    string sail_summary_dir;
    string compass_summary_dir;

    QLabel* l;

    DataManager* dm;
    Callgraph* cg;
    QStandardItemModel* error_model;
    FileView *file_tree;

    Compass* compass;

    SvgViewer* callg_viewer;

    SvgViewer* cfg_viewer;
    QLabel* cfg_l;

    StatsViewer* stats_viewer;

    string dm_id_selected;

    SummaryViewer* summary_viewer;



    MemoryVisualizer* mv;

    Regressions* r;


    int num_analyzed;
    double analysis_time;


    Identifier cur_id;
    map<Identifier, SummaryGraph*> loaded_summaries;
    map<string, AnalysisResult*> loaded_ars;


    bool finished;

    bool canceled;

    string current_project;

    string lock_file;

    QRadioButton *callers_cg;
    QRadioButton *callees_cg;

    bool left;



public:
    compass_ui(QWidget *parent = 0);
    ~compass_ui();
    Ui::compass_uiClass* get_ui() {return &ui;};
    void delete_file(string name);
    void open_file(string s, bool select = false, int line=-1);
    string get_id(const string & path);
    void new_folder(string name);
    void new_file(const string & path);
    string get_folder(string  s);
    string expand_dot(string file);

    void replace_project(string folder, string name);


private:
    void read_settings(string path);
    int get_tab_from_file(string path);
    void create_new_unit();
    void write_tab(int i);
    QWidget* new_code_editor(const string& text, language_type lt);

    void init_data_manager();
    void set_working_directory(string dir);
    void update_results();
    Error* make_compilation_error(const string & message);
    void highlight_errors(KTextEditor::View *v, string file);
    string get_file_from_tab(int i);
    void update_unit_control();
    void update_function_control(const Identifier & id);
    void update_function_control_rec(CGNode* n, QStandardItem *parent,
    		bool callers);
    void load_sail_and_build_cg();
    void update_unit_view(AnalysisResult * ar, string f_id);
    void compute_num_units_to_analyze();

    void update_unit_icons();

    void write_current_project_settings();



    void change_project(string name);

    void populate_projects();
    void set_name(string name);
    void get_name();

    string get_dm_id_from_path(string path);


    string remove_current_directory(const string & file);

    string get_prety_print_name(const Identifier& unit_id,
    		map<string, Identifier> & short_name_to_id);



    void reset_unit_model();


    void clear_per_run_data();
    SummaryGraph* load_compass_summary(const Identifier & id);
    void load_callee_summaries(const Identifier& id);

    void lock_disk();
    bool lock_valid();

    void unit_selected(const Identifier & id);

    void display_query(ReportClassifier& rc);



    Ui::compass_uiClass ui;
public slots:
	void app_quit();
	void new_file();
	void run();
	void cancel();
	void error_clicked(const QModelIndex &index);
	void error_selected(const QModelIndex &index);
	void unit_selected(const QModelIndex &index);
	void unit_pressed(const QModelIndex & index);
	void unit_context_requested(const QPoint & p);

	void entry_selected();
	void exclude_unit_selected();
	void include_unit_selected();
	void exclude_transitive_selected();
	void include_transitive_selected();
	void exclude_init_funs_selected();
	void include_init_funs_selected();

	void compass_finished();
	void compass_sum_started(QString id);
	void compass_sum_finished(QString id, AnalysisResult * ar);


	void project_selected(const QString & s);

	void editor_changed(KTextEditor::Document *document );

	void add_regression();

	void entry_point_cleared();



	void tab_closed(int i);

	void callgraph_fn_selected(QModelIndex index);
	void callgraph_fn_double_selected(QModelIndex index);

	void recompute_callg_widget(bool ignore);


};

#endif // COMPASS_UI_H
