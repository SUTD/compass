#include "compass_ui.h"
#include <iostream>


#include <QtGui>
#include <QApplication>

#include <assert.h>
#include <set>
#include <sys/types.h>
#include <pwd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <string>
#include <iostream>
#include <fstream>
#include "time.h"
#include <sys/stat.h>
#include <QToolButton>
#include <sail/Cfg.h>
#include "DottyViewer.h"

#include <sstream>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>

#include <kxmlguifactory.h>
#include <kmessagebox.h>
#include <ktexteditor/texthintinterface.h>
#include <ktexteditor/markinterface.h>

//#include <kate/document.h>
//#include <kate/view.h>

#include "Manager.h"
#include "DataManager.h"

#include "SummaryStream.h"
#include "SummaryGraph.h"
#include "sail/SuperBlock.h"
#include "sail/SummaryUnit.h"
#include "ConstraintSolver.h"

#include "Error.h"
#include "util.h"
#include <vector>
#include "GlobalAnalysisState.h"
#include "MemoryAnalysis.h"
#include "compass-serialization.h"
#include "fileops.h"
#include "MemoryVisualizer.h"
#include "SvgViewer.h"
#include "Regressions.h"
#include "Compass.h"
#include "FileView.h"
#include "SailGenerator.h"
#include "SummaryViewer.h"
#include "compass-serialization.h"
#include "ProjectSettings.h"
#include "StatsViewer.h"

#include "ErrorTrace.h"
#include "AssertionTrace.h"
#include "ReportClassifier.h"
#include "UserQuery.h"

#include "ClientAnalysis.h"
#include "TaintAnalysis.h"

//#include "sail-serialization.h"


using namespace std;

/*
 * Default file
 */

string default_file = "#include <stdio.h>\n";



/*
 * Configuration options
 */
#define COMPASS_SETTINGS "/.compass/"
#define COMPASS_CURRENT "/.compass/current/"
#define COMPASS_CONFIG "/.compass/config.txt"


#define START_PROJECT "current"
#define NAME_FILE ".name"

string home_dir;



#define EXCLUDED_ICON "/usr/share/icons/oxygen/16x16/actions/format-remove-node"
#define ENTRY_ICON "/usr/share/icons/oxygen/16x16/actions/flag-green"

/*
 * Can you click on errors to diagnose them?
 */
#define ERROR_SELECTABLE false

/*
 * Analysis Options
 */

#define DM_MEMORY 4000



string ee(string e)
{
	string res;
	for(int  i=0; i < e.size(); i++)
	{
		if(e[i] == ' ') res += "\\ ";
		else res += e[i];
	}
	return res;
}


/*
 * File system helpers
 */
void create_folder(string path)
{
	string cur = "";
	for(unsigned int i=0; i < path.size(); i++)
	{
		cur+=path[i];
		if(path[i]=='/' || i == path.size()-1) {
			string dir = cur;
			mkdir(dir.c_str(), S_IRWXU);
		}

	}

}

bool ends_with(const string& s, const string & ending)
{
	if(s.size() < ending.size()) return false;
	size_t pos = s.rfind(ending);
	bool res = (pos == (s.size()-ending.size()));
	return res;
}

bool is_header_file(string s)
{
	return ends_with(s, ".h") || ends_with(s, ".hpp");
}

bool is_c_file(string s)
{
	return ends_with(s, ".c");
}

bool is_cpp_file(string s)
{
	return ends_with(s, ".cpp") || ends_with(s, ".cc");
}

bool is_java_file(string s)
{
	return ends_with(s, ".java");
}

bool is_class_file(string s)
{
	return ends_with(s, ".class");
}


bool is_valid_file(string s)
{
	return is_header_file(s) || is_c_file(s) || is_cpp_file(s)
			|| is_java_file(s);
}



string double_to_string(double s)
{

	stringstream ss;
	ss << s;
	string res;
	ss >> res;
	return res;
}

void collect_all_c_files(set<string> & reg_paths, string prefix)
{
	 DIR *dp =  opendir(prefix.c_str());
	 if(dp  == NULL) return;
	 struct dirent *dirp;
	 while ((dirp = readdir(dp)) != NULL) {
	     	string name = string(dirp->d_name);
	     	if(name == "." || name == "..")
	     		continue;
	     	string new_prefix = prefix + name  + "/";
	     	DIR *cur_dp = opendir(new_prefix.c_str());
	     	if(cur_dp  != NULL)
	     	{
	     		closedir(cur_dp);
	     		collect_all_c_files(reg_paths, new_prefix);
	     		continue;
	     	}
	     	if(is_valid_file(name))
	     	{
	     		reg_paths.insert(prefix + name);
	     	}


	 }
	 closedir(dp);
}

void collect_all_java_files(set<string> & reg_paths, string prefix)
{
	 DIR *dp =  opendir(prefix.c_str());
	 if(dp  == NULL) return;
	 struct dirent *dirp;
	 while ((dirp = readdir(dp)) != NULL) {
	     	string name = string(dirp->d_name);
	     	if(name == "." || name == "..")
	     		continue;
	     	string new_prefix = prefix + name  + "/";
	     	DIR *cur_dp = opendir(new_prefix.c_str());
	     	if(cur_dp  != NULL)
	     	{
	     		closedir(cur_dp);
	     		collect_all_java_files(reg_paths, new_prefix);
	     		continue;
	     	}
	     	if(is_java_file(name))
	     	{
	     		reg_paths.insert(prefix + name);
	     	}


	 }
	 closedir(dp);
}

void collect_all_class_files(set<string> & reg_paths, string prefix)
{
	 DIR *dp =  opendir(prefix.c_str());
	 if(dp  == NULL) return;
	 struct dirent *dirp;
	 while ((dirp = readdir(dp)) != NULL) {
	     	string name = string(dirp->d_name);
	     	if(name == "." || name == "..")
	     		continue;
	     	string new_prefix = prefix + name  + "/";
	     	DIR *cur_dp = opendir(new_prefix.c_str());
	     	if(cur_dp  != NULL)
	     	{
	     		closedir(cur_dp);
	     		collect_all_class_files(reg_paths, new_prefix);
	     		continue;
	     	}
	     	if(is_class_file(name))
	     	{
	     		reg_paths.insert(prefix + name);
	     	}


	 }
	 closedir(dp);
}





string strip_empty_lines(string s)
{
	int end = s.size()-1;
	while(end>=0 && (s[end] == ' ' || s[end] == '\n' || s[end] == '\t'))
		end--;
	return s.substr(0, end+1);
}


bool is_empty_string(string s)
{
	for(unsigned int i = 0; i < s.size(); i++){
		if(s[i] == ' ' || s[i] == '\t' || s[i] == '\n') continue;
		return false;
	}
	return true;
}


string compass_ui::get_id(const string & path)
{
	return path.substr(current_dir.size());
}


void compass_ui::create_new_unit()
{
	string cmd = string("rm -rf ") + ee(current_dir) + string("*");
	system(cmd.c_str());
	string filename = current_dir + "test.c";
	ofstream out;
	out.open(filename.c_str());
	out << default_file;
	out.close();
}

QWidget* compass_ui::new_code_editor(const string& text, language_type lt)
{
	KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();

	if (!editor) {
	  KMessageBox::error(this, "A KDE text-editor component could not be found;\n"
									"please check your KDE installation.");
	  exit(1);

	}

	 KTextEditor::Document *m_document = editor->createDocument(0);
	 if(lt == LANG_C)
		 m_document->setHighlightingMode("c");
	 else if(lt == LANG_CPP)
	 		 m_document->setHighlightingMode("c++");
	 else if(lt == LANG_JAVA)
	 	 		 m_document->setHighlightingMode("java");

	 KTextEditor::View *m_view =
			 qobject_cast<KTextEditor::View*>(m_document->createView(this));
	 m_document->setText(QString::fromStdString(text));
	 m_document->setModified(false);

	 connect(m_document, SIGNAL(modifiedChanged ( KTextEditor::Document*)),
		             this, SLOT(editor_changed(KTextEditor::Document*)));

	 return m_view;
	/*

	QTextEdit* code_edit = new QTextEdit();
	code_edit->setText(QString::fromStdString(text));
	return code_edit;
	*/
}

void compass_ui::editor_changed(KTextEditor::Document *document )
{
	ui.add_regression_button->setEnabled(false);
}

string compass_ui::get_folder(string  s)
{

	if(s == "") s = current_dir;
	if(!is_valid_file(s)){
		return s + "/";
	}
	int i;



	for(i=s.size(); i>=0; i--)
	{
		if(s[i] == '/') break;
	}
	string res = s.substr(0, i) + "/";
	return res;

}

string strip_and(const string & s)
{
	string res;
	for(unsigned int i=0; i < s.size(); i++)
	{
		if(s[i] == '&') continue;
		res +=s[i];
	}

	return res;
}


int compass_ui::get_tab_from_file(string path)
{
	string ed_name = path.substr(current_dir.size());
	for(int i=0; i < ui.editor_tabs->count(); i++)
	{
		string text = strip_and(ui.editor_tabs->tabText(i).toStdString());

		if(text == ed_name) {
			return i;
		}
	}
	return -1;
}




string compass_ui::get_file_from_tab(int i)
{
	assert(i >=0 && i < ui.editor_tabs->count());
	string text = strip_and(ui.editor_tabs->tabText(i).toStdString());
	return this->current_dir + text;
}

KTextEditor::MarkInterface::MarkTypes get_mark(error_code_type ec)
{
	switch(ec)
	{
		case ERROR_UNMODELED_STORE:
		case ERROR_ILLEGAL_CAST:
		case ERROR_INVALID_POINTER_ARITHMETIC:
		case ERROR_STATIC_ASSERT:
		case ERROR_UNINITIALIZED_READ:
		case ERROR_NULL_DEREF:
		case ERROR_BUFFER_OVERRUN:
		case ERROR_BUFFER_UNDERRUN:
		case ERROR_STACK_RETURN:
		case ERROR_INCONSISTENT_ELEM_SIZE:
		case ERROR_COMPILATION:
		case ERROR_CRASH:
		case ERROR_SPEC_MISMATCH:
		case ERROR_ILLEGAL_ADT_FUNCTION:
			return KTextEditor::MarkInterface::markType07;
		default:
			return KTextEditor::MarkInterface::markType07;
	}
}



void compass_ui::highlight_errors(KTextEditor::View *v, string file)
{
	/*
	 * First, remove all error marks from the current view
	 *
	 */
	KTextEditor::Document* d = v->document();
	KTextEditor::MarkInterface *iface =
		qobject_cast<KTextEditor::MarkInterface*>(d);
	assert(iface != NULL);

	iface->clearMarks();

	if(errors_per_file.count(file) == 0) return;


	set<Error*> & errors = errors_per_file[file];



	set<Error*>::iterator it = errors.begin();
	for(; it!= errors.end(); it++)
	{
		Error* e = *it;
		iface->addMark(e->line-1, get_mark(e->error));

	}

}

/*
 * Opens the specified file in a new tab
 */
void compass_ui::open_file(string path, bool select, int line)
{
	if(path.find("Makefile")==string::npos  && !is_valid_file(path)) return;

	/*
	 * Do not open files that are already open, only select their
	 * tab, if desired.
	 */
	int tab = get_tab_from_file(path);
	if(tab != -1)
	{
		if(select){
			ui.editor_tabs->setCurrentIndex(tab);
			QWidget* w = ui.editor_tabs->currentWidget();
			if(line !=-1){
				KTextEditor::View *v = (KTextEditor::View *)w;
				KTextEditor::Cursor cursor(line-1, 0);
				v->setCursorPosition(cursor);
			}
		}
		return;
	}


	string content;
	ifstream in;
	in.open(path.c_str());
	if(!in.is_open())
	{
		cout << "Cannot open file " << path << endl;
		assert(false);
	}
	while(!in.eof())
	{
		string cur;
		std::getline(in, cur);
		content+=cur;
		content+="\n";
	}
	in.close();
	content = strip_empty_lines(content);

	assert(path.size() > current_dir.size());
	string tab_name = path.substr(current_dir.size());

	/*
	 * Make a new tab
	 */
	language_type lt = LANG_UNSUPPORTED;
	if(is_c_file(path)) lt = LANG_C;
	else if (is_cpp_file(path)) lt = LANG_CPP;
	else if(is_java_file(path)) lt = LANG_JAVA;
	QWidget* editor = new_code_editor(content, lt);
	editor->setParent(ui.editor_tabs);
	QString tt = QString::fromStdString(tab_name);
	int pos = ui.editor_tabs->addTab(editor, tt);
	QWidget* w = ui.editor_tabs->currentWidget();
	if(line ==-1) line = 0;

	KTextEditor::View *v = (KTextEditor::View *)w;
	KTextEditor::Cursor cursor(line-1, 0);
	v->setCursorPosition(cursor);
	highlight_errors(v, path);

	if(select) {
		ui.editor_tabs->setCurrentIndex(pos);

	}
}

void compass_ui::delete_file(string name)
{
	remove(name.c_str());
	cur_files.erase(name);
	string path = get_dm_id_from_path(name);
	dm->erase_all_data_with_prefix(path);

	int tab_index = get_tab_from_file(name);

	if(tab_index != -1) ui.editor_tabs->removeTab(tab_index);



	file_model->refresh(file_model->index(QString::fromStdString(current_dir)));



}

void compass_ui::new_folder(string name)
{

	create_folder(name);
	file_model->refresh(file_model->index(QString::fromStdString(current_dir)));
}

string convert_to_absolute_path(string  s)
{
	passwd *pw = getpwuid(getuid());
	assert(pw != NULL);
	string dir = pw->pw_dir;
	string res;
	for(unsigned int i = 0; i < s.size(); i++) {
		if(s[i]=='~') res += dir;
		else res+=s[i];
	}
	return res;

}


void compass_ui::read_settings(string path)
{

	/*
	 * Compute absolute paths for settings.
	 */
	errors_per_file.clear();
	clear_per_run_data();
	passwd *pw = getpwuid(getuid());
	assert(pw != NULL);
	home_dir = pw->pw_dir;

	settings_dir = home_dir + COMPASS_SETTINGS;
	current_dir = home_dir + COMPASS_SETTINGS + path + "/";
	compass_config = home_dir + COMPASS_CONFIG;

	//regression_dir = home_dir + REGRESSION_FOLDER;
	regression_dir = convert_to_absolute_path(REGRESSION_FOLDER);

	sail_summary_dir = current_dir + "sail/";
	compass_summary_dir = current_dir + "compass/";

	current_config = current_dir + ".project_config";

	/*
	 * Create directories if they do not exist
	 */
	mkdir(settings_dir.c_str(), S_IRWXU);
	mkdir(current_dir.c_str(), S_IRWXU);
	mkdir(sail_summary_dir.c_str(), S_IRWXU);
	mkdir(compass_summary_dir.c_str(), S_IRWXU);

	/*
	 * Get all files in current dir
	 */
	collect_all_c_files(cur_files, current_dir);
	if(cur_files.size() == 0) {
		create_new_unit();
		collect_all_c_files(cur_files, current_dir);
	}
	set<string>::iterator it = cur_files.begin();

	if(cur_files.size()<=3)
	{
		for(; it != cur_files.end(); it++)
		{
			open_file(*it);
		}
	}

	set_working_directory(current_dir);

	/*
	 * Every read file is modified unless we have summary data.
	 */
	modified_files.insert(cur_files.begin(), cur_files.end());

	ifstream config_if;
	config_if.open(current_config.c_str(), ios::binary);
	if(config_if.is_open())
	{
		ProjectSettings* ps = load_project_settings(config_if);
		this->current_entry_id = ps->entry_point;
		this->excluded_units = ps->excluded_units;
		delete ps;
	}

	get_name();





}

void compass_ui::write_current_project_settings()
{
	ofstream config_if;
	config_if.open(current_config.c_str(), ios::binary | ios::ate);
	assert(config_if.is_open());
	stringstream res;

	ProjectSettings ps;
	ps.entry_point = this->current_entry_id;
	ps.excluded_units = this->excluded_units;
	write_project_settings(res, &ps);

	for(unsigned int i =0; i < res.str().size(); i++)
	{
		config_if << res.str()[i];
	}
}

void compass_ui::new_file()
{
	new_file(this->current_dir);

}

void compass_ui::new_file(const string & path)
{

	bool ok;
	QString text = QInputDialog::getText(this, "Question",
	                                          tr("New file name:"), QLineEdit::Normal,
	                                          "", &ok);
	if (!ok || text.isEmpty())
	   return;

	string s = text.toStdString();
	s = strip(s);
	if(s == "") return;

	string new_file = path + s;

	/*
	 * See if file already exists.
	 */

	if(cur_files.count(new_file) > 0)
	{
		QErrorMessage* msg = new QErrorMessage(this);
		msg->showMessage("File with this name already exists.");
		return;
	}

	if(!is_valid_file(new_file))
	{
		QErrorMessage* msg = new QErrorMessage(this);
		msg->showMessage("You can only add .c or .h files");
		return;
	}




	ofstream out;
	out.open(new_file.c_str());
	if(!out.is_open())
	{
		QErrorMessage* msg = new QErrorMessage(this);
		msg->showMessage("Cannot create file with this name");
		return;
	}
	/*
	 * populate file with default values
	 */

	out << default_file;
	out.close();
	cur_files.insert(new_file);
	open_file(new_file, true);

	file_model->refresh(file_model->index(QString::fromStdString(current_dir)));




}


void compass_ui::write_tab(int i)
{
	KTextEditor::Document* ed =
			((KTextEditor::View*) ui.editor_tabs->widget(i))->document();
	assert(ed != NULL);
	ed->setModified(false);
	string text = ui.editor_tabs->tabText(i).toStdString();
	text = strip_and(text);
	string path = current_dir + text;
	string content = ed->text().toStdString();

	/*
	 * Retrieve the old content just to see if we modified it
	 */

	ifstream in;
	in.open(path.c_str());
	bool modified = true;
	if(in.is_open())
	{
		string read_content;
		while(!in.eof())
		{
			string cur;
			std::getline(in, cur);
			read_content+=cur + "\n";
		}
		if(strip_empty_lines(read_content) ==
				strip_empty_lines(content)){
			modified = false;
		}
	}

	ofstream out;
	out.open(path.c_str());
	if(!out.is_open()) return;
	out << content;
	out.close();
	if(modified)
		modified_files.insert(path);


}

void compass_ui::tab_closed(int i)
{
	write_tab(i);
	ui.editor_tabs->removeTab(i);
}

void compass_ui::lock_disk()
{
	srand(time(NULL));

	int r = rand();
	if(r < 0) r = -r;
	string cmd = "rm " + settings_dir + "*.loc";
	system(cmd.c_str());
	lock_file = settings_dir + "lock_" + int_to_string(r) + ".loc";
	ofstream out(lock_file.c_str());
	assert(out.is_open());
	out << "compass";
	out.close();
}


bool compass_ui::lock_valid()
{

	ifstream in(lock_file.c_str());
	if(!in.is_open()) return false;
	string temp;
	std::getline(in, temp);
	return temp == "compass";
}



compass_ui::compass_ui(QWidget *parent)
    : QWidget(parent)
{


	setWindowIcon(
			QIcon("compass"));

	this->num_funs_to_analyze = -1;


	{
		MemoryAnalysis ma;
		ma.register_summary_callbacks();
	}
	dm = NULL;
	current_project = START_PROJECT;

	ui.setupUi(this);

	ui.num_cpus->setMinimum(1);
	int cpus_installed = sysconf(_SC_NPROCESSORS_CONF);
	ui.num_cpus->setMaximum(cpus_installed);
	//ui.num_cpus->setValue(cpus_installed);
	ui.num_cpus->setValue(1);

	callg_viewer = new SvgViewer(this);
	QWidget * call_widget = ui.callgraph_tab;
	QVBoxLayout *clayout = new QVBoxLayout;
	clayout->addWidget(callg_viewer);
	call_widget->setLayout(clayout);



	cfg_viewer = new SvgViewer(this);
	cfg_l = new QLabel(cfg_viewer);
	cfg_l->setText("No summary unit selected.");
	QWidget * cfg_widget = ui.cfg_tab;
	QVBoxLayout *cfglayout = new QVBoxLayout;
	cfglayout->addWidget(cfg_l);
	cfglayout->addWidget(cfg_viewer);
	cfg_widget->setLayout(cfglayout);

	stats_viewer = new StatsViewer();
	QWidget * stats_widget = ui.stats_tab;
	QVBoxLayout *statslayout = new QVBoxLayout;
	statslayout->addWidget(stats_viewer);
	stats_widget->setLayout(statslayout);



	summary_viewer = new SummaryViewer(this);

	QWidget * sum_widget = ui.summary_tab;
	QVBoxLayout *sumlayout = new QVBoxLayout;
	sumlayout->addWidget(summary_viewer);
	sum_widget->setLayout(sumlayout);

	mv = new MemoryVisualizer(this);
	QVBoxLayout *ml = new QVBoxLayout;
	ml->addWidget(mv);
	ui.memory_tab->setLayout(ml);

	l = new QLabel;
	l->setText("No files");

    ui.editor_tabs->clear();
    QPushButton* button = new QPushButton();
    button->setText("New File");

    ui.editor_tabs->setCornerWidget(button);
    button->show();

    QTabWidget *tab = ui.editor_tabs;

    ui.cancel->setEnabled(false);


    QObject::connect(tab,SIGNAL(tabCloseRequested(int)),this, SLOT(tab_closed(int)));
    QObject::connect(ui.run_button,SIGNAL(clicked()),this, SLOT(run()));
    QObject::connect(ui.cancel,SIGNAL(clicked()),this, SLOT(cancel()));
    QObject::connect(ui.add_regression_button,SIGNAL(clicked()),this,
    		SLOT(add_regression()));

    QObject::connect(button, SIGNAL(clicked()), this, SLOT(new_file()));

    callgraph_model = NULL;
    unit_model = NULL;
    entry_point_label = new QLineEdit;
    entry_point_label->setReadOnly(true);

    read_settings(current_project);
    /*
     * Lock disk
     */
    lock_disk();

    /*
     * Populate the drop-down box with projects from disk
     */

    //find all dirs in COMPASS_SETTINGS
    populate_projects();


	 connect(ui.project_used, SIGNAL(currentIndexChanged(const QString &)),
	             this, SLOT(project_selected(const QString &)));



  //  ui.project_used


    r = new Regressions(regression_dir, this, this);
	QVBoxLayout *rl = new QVBoxLayout;
	rl->addWidget(r);
	ui.regression_tab->setLayout(rl);


    /*
     * Add view for files
     */
   file_tree = new FileView(this);

   QVBoxLayout *layout = new QVBoxLayout;
   layout->addWidget(file_tree);
   ui.files->setLayout(layout);

   file_model = new QDirModel;
   file_tree->setModel(file_model);



   file_tree->setRootIndex(
		file_model->index(QString::fromStdString(current_dir)));


   for(int i=1; i < file_model->columnCount(); i++)
   {
	   file_tree->hideColumn(i);
   }


   file_tree->show();


   /*
    * Add view for callgraph
    */
   callg_tree = new QTreeView(this);
   callg_tree->setIndentation(10);


   QVBoxLayout *layout2 = new QVBoxLayout;
   layout2->addWidget(callg_tree);

   callees_cg= new QRadioButton("Callees");
   callers_cg= new QRadioButton("Callers");

   layout2->addWidget(callees_cg);
   layout2->addWidget(callers_cg);
   callers_cg->setChecked(true);

   connect(callees_cg, SIGNAL(toggled(bool)),
                  this, SLOT(recompute_callg_widget(bool)));
   connect(callers_cg, SIGNAL(toggled(bool)),
                   this, SLOT(recompute_callg_widget(bool)));



   ui.callgraph->setLayout(layout2);
   callgraph_model = new QStandardItemModel;
   callgraph_model->setColumnCount(1);
   {
	   QStringList l;
	   l.push_back("Function");
	   callgraph_model->setHorizontalHeaderLabels(l);
   }
   callg_tree->setModel(callgraph_model);

   connect(callg_tree, SIGNAL(clicked(QModelIndex)),
  			   this, SLOT(callgraph_fn_selected(QModelIndex)));
   connect(callg_tree, SIGNAL(doubleClicked(QModelIndex)),
  			   this, SLOT(callgraph_fn_double_selected(QModelIndex)));

   /*
    * Add Unit view
    */
   unit_tree = new QTreeView(this);
   QVBoxLayout *layout3 = new QVBoxLayout;
   layout3->addWidget(unit_tree);

   QHBoxLayout *entry_layout = new QHBoxLayout;

   QLabel* ep = new QLabel;
   ep->setText("Entry Point:");
   entry_layout->addWidget(ep);
   QPushButton *clear_entry_button = new QPushButton();
   clear_entry_button->setText("Clear");
   entry_layout->addWidget(clear_entry_button);
   layout3->addLayout(entry_layout);
   layout3->addWidget(entry_point_label);

   QObject::connect(clear_entry_button,SIGNAL(clicked()),this,
      		SLOT(entry_point_cleared()));

   ui.Units->setLayout(layout3);
   unit_model = new QStandardItemModel;

   unit_popup_menu = new QMenu(unit_tree);
   QAction* entry_action = new QAction(tr("Make entry point"), this);
   unit_popup_menu->addAction(entry_action);

   connect(entry_action, SIGNAL(triggered()),
                  this, SLOT(entry_selected()));

   QAction* clear_entry_action = new QAction(
		   tr("Clear entry point"), this);
   unit_popup_menu->addAction(clear_entry_action);

   connect(clear_entry_action, SIGNAL(triggered()),
                  this, SLOT(entry_point_cleared()));

   unit_popup_menu->addSeparator();



   QAction* exclude_action = new QAction(tr("Exclude unit"), this);
   unit_popup_menu->addAction(exclude_action);

   connect(exclude_action, SIGNAL(triggered()),
                  this, SLOT(exclude_unit_selected()));




   QAction* include_action = new QAction(tr("Include unit"), this);
   unit_popup_menu->addAction(include_action);

   unit_popup_menu->addSeparator();



   connect(include_action, SIGNAL(triggered()),
                  this, SLOT(include_unit_selected()));

   //---------------------------


   QAction* exclude_transitive_action = new QAction(tr("Exclude transitive"), this);
   unit_popup_menu->addAction(exclude_transitive_action);

   connect(exclude_transitive_action, SIGNAL(triggered()),
                  this, SLOT(exclude_transitive_selected()));




   QAction* include_transitive_action = new QAction(tr("Include transitive"), this);
   unit_popup_menu->addAction(include_transitive_action);



   connect(include_transitive_action, SIGNAL(triggered()),
                  this, SLOT(include_transitive_selected()));


   //------------------





   unit_popup_menu->addSeparator();

   QAction* exclude_all_inits_action = new QAction(
		   tr("Exclude all init functions"), this);
   unit_popup_menu->addAction(exclude_all_inits_action);

   connect(exclude_all_inits_action, SIGNAL(triggered()),
                  this, SLOT(exclude_init_funs_selected()));

   QAction* include_all_inits_action = new QAction(
		   tr("Include all init functions"), this);
   unit_popup_menu->addAction(include_all_inits_action);
   connect(include_all_inits_action, SIGNAL(triggered()),
                  this, SLOT(include_init_funs_selected()));










   unit_tree->setModel(unit_model);
   unit_tree->setContextMenuPolicy(Qt::CustomContextMenu);

   connect(unit_tree, SIGNAL(doubleClicked(QModelIndex)),
               this, SLOT(unit_selected(QModelIndex)));

   connect(unit_tree, SIGNAL(pressed(QModelIndex)),
               this, SLOT(unit_pressed(QModelIndex)));

   connect(unit_tree, SIGNAL(customContextMenuRequested(const QPoint&)),
                  this, SLOT(unit_context_requested(const QPoint&)));


   reset_unit_model();




   /*
    * Add error view
    */
   error_model = new QStandardItemModel();
   /*
   for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            QStandardItem *item = new QStandardItem(QString("row %0, column %1").arg(row).arg(column));
            error_model->setItem(row, column, item);
        }
    }*/
   error_model->setColumnCount(3);
   QStringList l;
   l.push_back("File");
   l.push_back("Line");
   l.push_back("Message");
   error_model->setHorizontalHeaderLabels(l);
   ui.error_view->setModel(error_model);

   connect(ui.error_view, SIGNAL(clicked(QModelIndex)),
            this, SLOT(error_clicked(QModelIndex)));

   connect(ui.error_view, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(error_selected(QModelIndex)));

   ui.add_regression_button->setEnabled(false);


   /*
    * Add the status_list view
    */




   init_data_manager();
   cg = NULL;

   load_sail_and_build_cg();


}


void compass_ui::recompute_callg_widget(bool ignore)
{
	update_function_control(cur_id);
}



void compass_ui::add_regression()
{
	string name = ui.name->text().toStdString();
	name = strip(name);

	if(name == "")
	{
		QErrorMessage* msg = new QErrorMessage(this);
		msg->showMessage("You have to specify a name for the regression.");
		return;
	}
	if(r->regression_exists(name))
	{
		QMessageBox msgBox;
		string text = "Overwrite existing regression \"" + name + "\"?";
		msgBox.setText(QString::fromStdString(text));
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		int ret = msgBox.exec();
		if(ret != QMessageBox::Ok) return;
	}

	map<string, set<Error*> >::iterator it = errors_per_file.begin();
	set<Error> all_errors;
	for(; it != errors_per_file.end(); it++)
	{
		const set<Error*> errors = it->second;
		set<Error*>::const_iterator it2 = errors.begin();
		for(; it2 != errors.end(); it2++)
		{
			Error e = **it2;
			//strip current dir from file
			assert(e.file.size() > current_dir.size());
			e.file = e.file.substr(current_dir.size());
			all_errors.insert(e);
		}
	}

	r->add_regression(name, this->current_dir, all_errors);
	QMessageBox box;
	box.setText("Regression successfully added.");
	box.exec();
	return;



}

void compass_ui::populate_projects()
{
	 DIR *dp;
	struct dirent *dirp;
	if((dp  = opendir(settings_dir.c_str())) == NULL) {
		cout << "ERROR opening compass dir" <<endl;
		return;
	}
	 set<string> folder_names;
	 while ((dirp = readdir(dp)) != NULL) {
			string name = string(dirp->d_name);
			if(name == "." || name == "..")
				continue;
			string new_dir = settings_dir + "/" + name;
			DIR *cur_dp = opendir(new_dir.c_str());
			if(cur_dp  != NULL)
			{
				closedir(cur_dp);
				folder_names.insert(name);
			}
	 }
	 closedir(dp);

	 /*{
		 cout << "DIRS: " << endl;
		 set<string>::iterator it = folder_names.begin();
		 for(; it!= folder_names.end(); it++)
			 cout << *it << endl;
	 }*/
	 set<string>::iterator it = folder_names.begin();
	 int i = 0;
	 int current_index = -1;
	 for(; it!= folder_names.end(); it++, i++)
	 {
		 string folder = *it;
		 if(folder == START_PROJECT) {
			 current_index = i;
		 }
		 ui.project_used->insertItem(i, QString::fromStdString(folder));
	 }
	 if(current_index != -1)
		 ui.project_used->setCurrentIndex(current_index);
}


void compass_ui::project_selected(const QString & s)
{
	string ss = s.toStdString();
	if(ss == current_project) return;

	current_project = ss;


	change_project(ss);
}

void compass_ui::set_name(string name)
{
	ofstream out;
	out.open(NAME_FILE, ofstream::trunc);
	assert(out.is_open());
	name = strip(name);
	out << name;
	out.close();
	ui.name->setText(QString::fromStdString(name));

}

void compass_ui::get_name()
{
	ui.name->setText("");
	ifstream in;
	in.open(NAME_FILE);
	if(!in.is_open()) return;
	string res;
	std::getline(in, res);
	in.close();
	res = strip(res);
	ui.name->setText(QString::fromStdString(res));
}



void compass_ui::replace_project(string folder, string name)
{

	while(ui.editor_tabs->count()> 0)
	{
		tab_closed(0);
	}
	if(dm!= NULL) dm->flush_syncronized();

	modified_files.clear();
	cur_files.clear();
	unit_model->clear();
	modified_files.clear();
	errors_per_file.clear();
	excluded_units.clear();
	error_model->removeRows(0, error_model->rowCount());
	clear_per_run_data();
	entry_point_cleared();


	read_settings(START_PROJECT);
	while(ui.editor_tabs->count()> 0)
	{
		tab_closed(0);
	}

	remove_directory(current_dir);



	copy_directory(folder, current_dir);


	cur_files.clear();
	modified_files.clear();


	modified_files.clear();
	cur_files.clear();
	modified_files.clear();
	errors_per_file.clear();
	error_model->removeRows(0, error_model->rowCount());
	clear_per_run_data();



	read_settings(START_PROJECT);

	file_tree->setRootIndex(
			file_model->index(QString::fromStdString(current_dir)));
	file_model->refresh(file_model->index(QString::fromStdString(current_dir)));

	set_name(name);
	init_data_manager();
	cg = NULL;

	load_sail_and_build_cg();

}


void compass_ui::change_project(string name)
{
	write_current_project_settings();
	QString nn = ui.name->text();

	string n = nn.toStdString();
	n = strip(n);
	set_name(n);


	while(ui.editor_tabs->count()> 0)
	{
		tab_closed(0);
	}
	if(dm!= NULL) dm->flush_syncronized();

	cur_files.clear();
	unit_model->clear();
	modified_files.clear();
	errors_per_file.clear();
	error_model->removeRows(0, error_model->rowCount());
	clear_per_run_data();
	entry_point_cleared();
	excluded_units.clear();


	read_settings(name);
	file_tree->setRootIndex(
			file_model->index(QString::fromStdString(current_dir)));


	init_data_manager();
	cg = NULL;

	load_sail_and_build_cg();

}

string get_fun_id_from_dm_id(string s)
{
	size_t x = s.find("@loop");
	if(x == string::npos) return s;
	return s.substr(0, x);


}

SummaryGraph* compass_ui::load_compass_summary(const Identifier & id)
{

	SummaryStream* ss = (SummaryStream*)dm->get_data(id.to_string('#'),
			COMPASS_SUMMARY);

	/*
	 * No such summary available.
	 */
	if(ss == NULL){
		dm->mark_unused(id.to_string('#'), COMPASS_SUMMARY);
		return NULL;
	}

	SummaryGraph* sg = NULL;

	if(loaded_summaries.count(id) > 0)
		sg =  loaded_summaries[id];
	else
	{
		sg = load_summary_graph(ss->get_data());
		loaded_summaries[id] = sg;
	}

	dm->mark_unused(id.to_string('#'), COMPASS_SUMMARY);
	return sg;
}


void compass_ui::load_callee_summaries(const Identifier& id)
{
	if(cg == NULL) return;
	CGNode* n = cg->get_node(id);
	map<CGNode*, cg_edge_id >::iterator it = n->callees.begin();
	for(; it != n->callees.end(); it++)
	{
		CGNode* cur = it->first;
		load_compass_summary(cur->id);
	}
}

void compass_ui::unit_pressed(const QModelIndex & index)
{
	if(QApplication::mouseButtons () & Qt::RightButton)
	{
		left = false;



	}
	else left = true;
}


void compass_ui::unit_context_requested(const QPoint & p)
{

	QModelIndex index = this->unit_tree->indexAt(p);
	if(!index.isValid()) return;

	QStandardItem* item = unit_model->itemFromIndex(index);
	if(unit_item_to_identifier.count(item) == 0) return;

	selected_unit = unit_model->item(item->row(), 2);



	QPoint globalPos = unit_tree->mapToGlobal(p);
	unit_popup_menu->exec(globalPos);



}

void compass_ui::update_unit_icons()
{
	for(int i=0; i < unit_model->rowCount(); i++)
	{
		QStandardItem* cur_it = unit_model->item(i, 2);
		const Identifier & cur_id=unit_item_to_identifier[cur_it];
		if(current_entry_id == cur_id){
			unit_model->item(i, 1)->setIcon(QIcon(ENTRY_ICON));
		}
		else if(this->excluded_units.count(cur_id) > 0) {
			unit_model->item(i, 1)->setIcon(QIcon(EXCLUDED_ICON));
		}
		else {
			unit_model->item(i, 1)->setIcon(QIcon(""));
		}

	}
}

void compass_ui::entry_selected()
{
	const Identifier & id = unit_item_to_identifier[selected_unit];


	current_entry_id = id;
	this->entry_point_label->setText(selected_unit->text());
	this->excluded_units.erase(current_entry_id);

	CGNode* entry_node =  cg->get_node(current_entry_id);
	set<CGNode*> callees;
	cg->get_transitive_callees(entry_node, callees);
	compute_num_units_to_analyze();
	update_unit_icons();

	return;
}

void compass_ui::compute_num_units_to_analyze()
{
	map<Identifier, CGNode*>& all_units = cg->get_nodes();

	// No entry point selected
	if(current_entry_id.is_empty())
	{
		int num_excluded = 0;
		set<Identifier>::iterator it = excluded_units.begin();
		for(; it!= excluded_units.end(); it++) {
			if(all_units.count(*it) > 0) num_excluded++;
		}
		num_funs_to_analyze = all_units.size() - num_excluded;


	}
	else{
		CGNode* entry_node = all_units[current_entry_id];
		set<CGNode*> transitive_callees;
		cg->get_transitive_callees(entry_node, transitive_callees);
		set<Identifier>::iterator it = excluded_units.begin();
		int num_excluded = 0;
		for(; it!= excluded_units.end(); it++) {
			CGNode* node = cg->get_node(*it);
			if(transitive_callees.count(node) > 0) num_excluded++;
		}
		num_funs_to_analyze = transitive_callees.size() - num_excluded;

	}

	ui.total_num->setText("/ " +
			QString::fromStdString(int_to_string(num_funs_to_analyze)));

}

void compass_ui::exclude_unit_selected()
{
	const Identifier & id = unit_item_to_identifier[selected_unit];

	CGNode* cgn = cg->get_node(id);




	if(excluded_units.count(id) > 0) return;
	excluded_units.insert(id);

	if(id == current_entry_id) this->entry_point_cleared();


	compute_num_units_to_analyze();
	update_unit_icons();
}
void compass_ui::include_unit_selected()
{
	const Identifier & id = unit_item_to_identifier[selected_unit];

	excluded_units.erase(id);

	compute_num_units_to_analyze();
	update_unit_icons();
}

void compass_ui::exclude_transitive_selected()
{
	const Identifier & id = unit_item_to_identifier[selected_unit];
	CGNode* cgn = cg->get_node(id);

	set<CGNode*> excluded;
	cg->get_exclusive_callees(cgn, excluded);

	set<CGNode*>::iterator it = excluded.begin();
	for(; it!= excluded.end(); it++)
	{
		CGNode* cur = *it;
		if(cur->id == current_entry_id) entry_point_cleared();
		excluded_units.insert(cur->id);
	}

	compute_num_units_to_analyze();
	update_unit_icons();


}
void compass_ui::include_transitive_selected()
{

	const Identifier & id = unit_item_to_identifier[selected_unit];
	CGNode* cgn = cg->get_node(id);

	set<CGNode*> included;
	cg->get_exclusive_callees(cgn, included);

	set<CGNode*>::iterator it = included.begin();
	for(; it!= included.end(); it++)
	{
		CGNode* cur = *it;
		excluded_units.erase(cur->id);
	}

	compute_num_units_to_analyze();
	update_unit_icons();
}


void compass_ui::exclude_init_funs_selected()
{
	cout << "Exclude init funs." << endl;

	set<CGNode*>& inits = cg->get_init_functions();
	set<CGNode*>::iterator it = inits.begin();
	for(; it!= inits.end(); it++)
	{
		CGNode* cgn = *it;
		const Identifier& id = cgn->id;
		excluded_units.insert(id);
		if(id == current_entry_id) this->entry_point_cleared();

	}

	update_unit_icons();
}
void compass_ui::include_init_funs_selected()
{
	cout << "Include init funs." << endl;

	set<CGNode*>& inits = cg->get_init_functions();
	set<CGNode*>::iterator it = inits.begin();
	for(; it!= inits.end(); it++)
	{
		CGNode* cgn = *it;
		const Identifier& id = cgn->id;
		excluded_units.erase(id);

	}

	update_unit_icons();
}

void compass_ui::unit_selected(const QModelIndex &index)
{
	QStandardItem* it = unit_model->itemFromIndex(index);



	if(unit_item_to_identifier.count(it) == 0){
		cout << "item not found " << endl;
		return;
	}

	Identifier id = unit_item_to_identifier[it];
	if(!left)
	{
		return;

	}



	unit_selected(id);




}

void compass_ui::unit_selected(const Identifier & id)
{
	for(int i=0; i < unit_model->rowCount(); i++)
	{
		QStandardItem* it = unit_model->item(i, 2);
		it->setIcon(QIcon(""));
		if(unit_item_to_identifier.count(it) == 0) continue;
		Identifier cur_id = unit_item_to_identifier[it];
		if(cur_id == id){
			unit_model->item(i, 2)->setIcon(
					QIcon("/usr/share/icons/oxygen/16x16/actions/rating"));
		}
	}
	unit_tree->resizeColumnToContents(2);


	update_function_control(id);


	/*
	 * First, load the current SAIL Function.
	 */
	if(!cur_id.is_empty())
		dm->mark_unused(cur_id.get_function_identifier().to_string('#'),
			SAIL_FUNCTION);

	sail::Function* sail_fn  = (sail::Function*)
					dm->get_data(id.get_function_identifier().to_string('#'),
							SAIL_FUNCTION);

	assert(sail_fn != NULL);
	cur_id = id;
	int line = sail_fn->get_first_line();
	this->open_file(id.get_file_id().to_string(), true, line);

	/*
	 * Draw CFG
	 */
	cfg_l->setText("Summary Unit: " + QString::fromStdString(id.to_string()));
		string cfg = sail_fn->get_cfg()->to_dotty(false);
	cfg_viewer->draw(cfg);

	/*
	 * Show summary
	 */

	SummaryGraph* sg = load_compass_summary(id);
	summary_viewer->display(sg);



	/*
	 * Prepare for stepping through this unit
	 */
	load_callee_summaries(id);
	sail::SummaryUnit* su = sail_fn;
	if(id.is_loop_id())
	{
		su = sail_fn->get_cfg()->get_superblock(id);
		assert(su != NULL);
	}


	mv->load(su, &loaded_summaries, cg->get_node(id),
			&cg->get_signature_to_callid_map(), cg);
	if(loaded_ars.count(id.to_string()) > 0)
	{
		this->stats_viewer->display_stats(id.to_string(),
				loaded_ars[id.to_string()]->constraint_stats);
	}
}

void compass_ui::error_clicked(const QModelIndex &index)
{
	//QStandardItem* item = error_model->itemFromIndex(index);
	int row = index.row();
	QStandardItem* file_item = error_model->item(row, 0);
	QStandardItem* line_item = error_model->item(row, 1);

	string file = current_dir + file_item->text().toStdString();
	int line = string_to_int(line_item->text().toStdString());

	this->open_file(file, true, line);

}

void compass_ui::error_selected(const QModelIndex &index)
{
	if(!ERROR_SELECTABLE) return;

	//QStandardItem* item = error_model->itemFromIndex(index);
	int row = index.row();
	QStandardItem* file_item = error_model->item(row, 0);
	QStandardItem* line_item = error_model->item(row, 1);

	string file = current_dir + file_item->text().toStdString();
	int line = string_to_int(line_item->text().toStdString());

	if(errors_per_file.count(file) == 0) return;

	Error* e = NULL;
	set<Error*>::iterator it = errors_per_file[file].begin();
	for(; it != errors_per_file[file].end(); it++) {
		Error* cur = *it;
		if(cur->line == line){
			e = cur;
			break;
		}
	}
	if(e == NULL) return;
	cout << "found Error!!!" << e->to_string() << endl;
	cout << e->id.to_string() << endl;

	if(e->id.is_empty()) return;
	SummaryGraph* sg = load_compass_summary(e->id);
	//find error trace for this report
	const set<ErrorTrace*> & traces = sg->get_error_traces();
	set<ErrorTrace*>::const_iterator it2 = traces.begin();
	for(; it2 != traces.end(); it2++) {
		Constraint stmt_guard;
		ErrorTrace *t = *it2;
		if(t->get_trace_type() == ASSERTION_TRACE) {
			AssertionTrace* at = static_cast<AssertionTrace*>(t);
			stmt_guard = at->get_stmt_guard();
		}

		if(t->get_line() == e->line){
			cout << "Found trace!!! " << t->to_string() << endl;
			cout << "Stmt Guard in trace: " << stmt_guard << endl;

			Constraint c = sg->get_return_cond();
			cout << "Ret cond: " << c << endl;
			cout << "stmt guard: " << stmt_guard << endl;
			c.assume(stmt_guard);
			ReportClassifier rc(t, c, sg);
			display_query(rc);


		}
	}

}
/*
 * QMessageBox msgBox;
		string text = "Another instance of Compass is already running. "
				"Running both will corrupt state. You should only continue "
				"if you are trying to compare two runs. Continue?";
		msgBox.setText(QString::fromStdString(text));
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		int ret = msgBox.exec();
		if(ret != QMessageBox::Ok) return;
 */
void compass_ui::display_query(ReportClassifier & rc)
{

	while(true)
	{
		if(rc.error_discharged()){
			cout << "************ ERROR DISCHARGED " << endl;
			break;
		}
		if(rc.error_validated()) {
			cout << "************ ERROR VALIDATED " << endl;
			break;
		}
		if(rc.queries_exhausted()) {
			cout << "**********QUERIES EXHAUSTED" << endl;
			break;
		}
		UserQuery uq = rc.get_query();

		/*if(uq.to_string().find("true")!= string::npos) {
			//proven
			cout << "Proven" << endl;
			break;
		}*/
	//	if(uq.to_string().find("false")!= string::npos) {
		//	cout << "Queries exhausted" << endl;
		//	break;
		//}


		cout << "Proof obligation: " << uq.to_string() << endl;

		QMessageBox msgBox;
		string text;
		text += uq.to_string();
		msgBox.setText(QString::fromStdString(text));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No |
				QMessageBox::Ignore);
		msgBox.setDefaultButton(QMessageBox::Ignore);
		int ret = msgBox.exec();
		if(ret == QMessageBox::Yes)
			rc.set_query_result(YES_RESULT);
		else if(ret == QMessageBox::No)
			rc.set_query_result(NO_RESULT);
		else if(ret == QMessageBox::Ignore)
			rc.set_query_result(UNRESOLVED);
		else break;

	}

	//DUCK
}


string qt_escape_string(const string & s)
{
	string res;
	for(unsigned int i = 0; i < s.size(); i++)
	{
		char c = s[i];
		if((int)c < 0) res +="";
		else res+=c;
	}

	return res;
}

void compass_ui::set_working_directory(string dir)
{
	chdir(dir.c_str());
}

void compass_ui::init_data_manager()
{
	delete dm;
	map<sum_data_type, string> paths;
	paths[SAIL_FUNCTION] = sail_summary_dir;
	paths[COMPASS_SUMMARY] = compass_summary_dir;
	dm = new DataManager(DM_MEMORY, paths);

}

bool is_number(string s)
{
	for(unsigned int i=0; i < s.size(); i++)
	{
		char c = s[i];
		if(c>='0' && c<='9') continue;
		return false;
	}
	return true;

}

Error* compass_ui::make_compilation_error(const string & message)
{

	/*
	 * Format is file:line:message
	 */

	int first_col_pos = message.find(":");
	Identifier id;
	if(first_col_pos == string::npos) return new Error(-1, "", ERROR_COMPILATION, "", id);
	string file = message.substr(0, first_col_pos);
	string rest = message.substr(first_col_pos+1);
	int second_col_pos = rest.find(":");
	if(second_col_pos == string::npos) return new Error(-1, "", ERROR_COMPILATION, "", id);
	string line = rest.substr(0, second_col_pos);


	string msg = rest.substr(second_col_pos+1);
	if(!is_number(line)) return make_compilation_error(msg);
	int line_int = string_to_int(line);

	return new Error(line_int, file, ERROR_COMPILATION, msg, id);
}




void compass_ui::cancel()
{
	canceled = true;
	compass->cancel();
	ui.status_list->clear();
}

string compass_ui::get_dm_id_from_path(string path)
{
	for(int i=0; i<path.size(); i++)
	{
		if(path[i] == '/') path[i]= '#';
	}
	return path;
}

void compass_ui::run()
{

	if(!lock_valid())
	{
		QMessageBox msgBox;
		string text = "Another instance of Compass is already running. "
				"Running both will corrupt state. You should only continue "
				"if you are trying to compare two runs. Continue?";
		msgBox.setText(QString::fromStdString(text));
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		int ret = msgBox.exec();
		if(ret != QMessageBox::Ok) return;
	}

	write_current_project_settings();

	dm->erase_all_data(COMPASS_SUMMARY);
	dm->erase_all_data(SAIL_FUNCTION);


	canceled = false;
	GlobalAnalysisState::clear();
	AccessPath::clear();
	Constraint::clear();
	if(ui.buffer_check->isChecked())
		GlobalAnalysisState::enable_buffer_check();
	if(ui.null_check->isChecked())
		GlobalAnalysisState::enable_null_check();
	if(ui.uninit_check->isChecked())
		GlobalAnalysisState::enable_uninit_check();

	ClientAnalysis::clear_clients();

	if(ui.taint_check->isChecked())
	{
		cout << "REGISTERING TAINT!!!!" << endl;
		TaintAnalysis* ta = new TaintAnalysis();
		ClientAnalysis::register_client(ta);
	}

	finished = false;
	ui.run_button->setEnabled(false);
	ui.cancel->setEnabled(true);
	ui.progress_bar->setValue(0);
	num_analyzed = 0;
	error_model->removeRows(0, error_model->rowCount());
	mv->clear();
	analysis_time = 0.0;


	/*
	 * First, write all tabs to disk.
	 */
	for(int i=0; i < ui.editor_tabs->count(); i++)
	{
		write_tab(i);
	}



	//for now
	modified_files.insert(this->cur_files.begin(), cur_files.end());

	//delete precompiled headers
	system("rm -rf *.gch");


	set<string>::iterator delete_file_it = modified_files.begin();
	for(; delete_file_it != modified_files.end(); delete_file_it++)
	{
		const string & file = *delete_file_it;
		string path = get_dm_id_from_path(file);
		dm->erase_all_data_with_prefix(path);
	}


	set<Error*> errors;
	SailGenerator sg(current_dir, sail_summary_dir, errors);
	system("rm -rf *.o");


	int loc = sg.get_total_lines();
	string loc_s = int_to_string(loc) + " lines";
	ui.loc_label->setText(QString::fromStdString(loc_s));


	/*
	 * Delete all compilation errors
	 */
	map<string, set<Error*> >::iterator e_it = errors_per_file.begin();
	for(; e_it != errors_per_file.end(); e_it++)
	{
		 set<Error*>::iterator it = e_it->second.begin();
		 set<Error*> to_delete;
		 for(; it != e_it->second.end(); it++)
		 {
			 Error* e = *it;
			 if(e->error == ERROR_COMPILATION)
				 to_delete.insert(e);
		 }
		 it = to_delete.begin();
		 for(; it != to_delete.end(); it++)
			 e_it->second.erase(*it);
	}

	set<string>::iterator file_it = sg.get_compiled_files().begin();
	for(; file_it != sg.get_compiled_files().end(); file_it++)
	{
		const string & file = *file_it;
		if(errors_per_file.count(file) == 0) continue;
		const set<Error*> & errors = errors_per_file[file];
		set<Error*>::iterator e_it = errors.begin();
		for(; e_it != errors.end(); e_it++)
			delete *e_it;
		errors_per_file.erase(file);
	}

	set<Error*>::iterator error_it = errors.begin();
	for(; error_it != errors.end(); error_it++)
	{
		Error* e = *error_it;
		errors_per_file[e->file].insert(e);
	}

	update_results();

	if(errors.size() > 0){
		ui.run_button->setEnabled(true);
		ui.cancel->setEnabled(false);
		finished = true;
		return;
	}
	errors_per_file.clear();

	dm->flush_syncronized();


	//delete precompiled headers
	system("rm -rf *.gch");

	load_sail_and_build_cg();


	compass = new Compass(dm, cg, ui.num_cpus->value(), 60, current_entry_id,
			&excluded_units);
    QObject::connect(compass,SIGNAL(finished()),this, SLOT(compass_finished()));
    QObject::connect(compass,SIGNAL(sum_started(QString)),this,
    		SLOT(compass_sum_started(QString)));

    QObject::connect(compass,SIGNAL(sum_finished(QString,
    		AnalysisResult *)),this,
       		SLOT(compass_sum_finished(QString,
       		AnalysisResult *)));

	compass->start();





}

void compass_ui::compass_sum_started(QString fid)
{
	if(canceled) return;
	string temp = fid.toStdString();
	if(temp.size() > current_dir.size())
		temp = temp.substr(current_dir.size());
	ui.status_list->addItem(QString::fromStdString(temp));
}



void compass_ui::compass_sum_finished(QString fid, AnalysisResult * ar)
{

	string temp = fid.toStdString();
	if(temp.size() > current_dir.size())
		temp = temp.substr(current_dir.size());


	QList<QListWidgetItem *> items = ui.status_list->findItems(
			QString::fromStdString(temp), Qt::MatchExactly);
	if(items.size() == 0) return;
	assert(items.size() == 1);
	QListWidgetItem *ii = *items.begin();
	int row = ui.status_list->row(ii);
	ui.status_list->takeItem(row);
	delete ii;

	num_analyzed++;
	int num_funs = unit_model->rowCount();
	if(this->num_funs_to_analyze != -1) {
		num_funs = this->num_funs_to_analyze;
	}
	if(num_funs > 0)
		ui.progress_bar->setValue(100.0*num_analyzed/ num_funs);


	ui.num_analyzed->setText(QString::fromStdString(int_to_string(num_analyzed)));


	/*
	 * See if there are errors to display
	 */
	if(ar->errors.size() > 0)
	{
		vector<Error*>::iterator it = ar->errors.begin();
		for(; it != ar->errors.end(); it++)
		{
			Error* e = *it;
			cout << "Error: " << e->file << endl;
			errors_per_file[expand_dot(e->file)].insert(e);
		}
		update_results();
	}

	/*
	 * Update the unit view
	 */
	update_unit_view(ar, fid.toStdString());
	if(ar->termination_value == 0)
		analysis_time += ar->time;
	string s = double_to_string(analysis_time);
	ui.cputime->setText(QString::fromStdString(s));
	loaded_ars[fid.toStdString()] = ar;




}

/*
void compass_ui::compass_error_reported(Error* e)
{


	errors_per_file[expand_dot(e->file)].insert(e);
	cout << "Error: " << e->to_string() << endl;
	update_results();
}
*/
void compass_ui::compass_finished()
{
	dm->flush_syncronized();
	ui.run_button->setEnabled(true);
	ui.cancel->setEnabled(false);
	ui.add_regression_button->setEnabled(true);
}




void compass_ui::clear_per_run_data()
{
	map<Identifier, SummaryGraph*>::iterator it = loaded_summaries.begin();
	for(; it != loaded_summaries.end(); it++)
	{
		delete it->second;
	}

	loaded_summaries.clear();
	loaded_ars.clear();

	cur_id = Identifier();
	mv->clear();
	stats_viewer->clear_stats();
	summary_viewer->clear();
	if(callgraph_model != NULL)
		callgraph_model->removeRows(0, callgraph_model->rowCount());
	call_item_to_id.clear();

	this->reset_unit_model();

	il::type::clear();
}

void compass_ui::entry_point_cleared()
{
	this->entry_point_label->setText("None selected");
	current_entry_id = Identifier();
	this->num_funs_to_analyze =  -1;

	for(int i=0; i < unit_model->rowCount(); i++)
	{
		QStandardItem* cur_it = unit_model->item(i, 2);
		cur_it->setBackground(Qt::NoBrush);
	}

	compute_num_units_to_analyze();
	update_unit_icons();

}

void compass_ui::load_sail_and_build_cg()
{
	clear_per_run_data();
	dm->mark_everything_unused();
	//dm->mark_unused(get_dm_id_from_path(dm_id_selected), COMPASS_SUMMARY);
//	dm->mark_unused(get_dm_id_from_path(
		//	get_fun_id_from_dm_id(dm_id_selected)), SAIL_FUNCTION);

	dm_id_selected = "";
	set<string> fn_ids;
	//build_all_function_identifiers(fn_ids);
	dm->get_serializable_ids(fn_ids, SAIL_FUNCTION);

	/*set<string>::iterator it = fn_ids.begin();
	for(; it != fn_ids.end(); it++)
	{
		sail::Function* f = (sail::Function*) dm->get_data(*it, SAIL_FUNCTION);
		cout << "Fn id: " << *it << endl;
		cout << "Body of f: " << f->get_cfg()->to_dotty(false) << endl;
	}*/

	/*
	 * First, build callgraph
	 */

	delete cg;
	cg = new Callgraph(fn_ids, dm, errors_per_file);
	update_results();

	callg_viewer->draw(cg->to_dotty(this->current_dir));




	reset_unit_model();

   update_unit_control();

   compute_num_units_to_analyze();

   /*
   ui.num_analyzed->setText("0");

   if(num_funs_to_analyze != -1)
	   ui.total_num->setText("/ " +
   				QString::fromStdString(int_to_string(num_funs_to_analyze)));
   else ui.total_num->setText("/ " +
			QString::fromStdString(int_to_string(cg->get_nodes().size())));*/

}

void compass_ui::reset_unit_model()
{
	if(unit_model == NULL) return;


	unit_model->clear();

	unit_model->setColumnCount(4);
   {
	   QStringList l;
	   l.push_back("");
	   l.push_back("");
	   l.push_back("Unit");
	   l.push_back("Time");
	   unit_model->setHorizontalHeaderLabels(l);
	   unit_tree->setRootIsDecorated(false);
	   unit_tree->resizeColumnToContents(1);
	   unit_tree->hideColumn(0);

   }
   unit_item_to_identifier.clear();


}

string compass_ui::remove_current_directory(const string & file)
{
	assert(file.size() >= this->current_dir.size());
	return file.substr(current_dir.size());
}


string compass_ui::get_prety_print_name(const Identifier& id,
		map<string, Identifier> & short_name_to_id)
{

	string pretty_print = id.get_function_name();




	if(pretty_print.find("$init")!=string::npos) {
		if(pretty_print.size() > current_dir.size())
			pretty_print = pretty_print.substr(current_dir.size());
	}
	pretty_print= id.get_namespace().to_string() + pretty_print;
	int pos = pretty_print.find('::');
	if(pos != string::npos) {
		//We are looking at a java unit whose method name ends with ;
		int slash_pos = pretty_print.substr(0, pos).rfind('/');
		if(slash_pos != string::npos) {
			pretty_print = pretty_print.substr(slash_pos+1);
		}
	}

	if(id.is_loop_id()) pretty_print += "@" + int_to_string(id.get_loop_id());

	/*
	 * Check if this name was already used for pretty printing.
	 * After this mess, pretty_print is safe to use
	 */
	{
		if(short_name_to_id.count(pretty_print) > 0)
		{
			pretty_print = id.get_function_name() + "<" +
					id.get_function_signature()->to_string() + ">";
			if(id.is_loop_id())
				pretty_print += "@" + int_to_string(id.get_loop_id());

			{
				if(short_name_to_id.count(pretty_print) > 0)
				{
					pretty_print = remove_current_directory(
							id.get_file_id().to_string());
					pretty_print += id.get_function_name() + "<" +
							id.get_function_signature()->to_string() + ">";

					if(id.is_loop_id())
						pretty_print += "@" + int_to_string(id.get_loop_id());
				}
			}
		}
	}

	return pretty_print;
}


void compass_ui::update_unit_control()
{

	if(cg == NULL) return;



	map<Identifier, CGNode*> & ids = cg->get_nodes();

	map<string, Identifier> short_name_to_id;
	map<Identifier, CGNode*>::iterator it = ids.begin();
	for(; it!= ids.end(); it++)
	{
		const Identifier& id = it->first;
		string pp_name = get_prety_print_name(id, short_name_to_id);
		short_name_to_id[pp_name] = id;
	}

	// Clear existing rows
	while(unit_model->rowCount() > 0)
	{
		unit_model->removeRow(0);
	}

	map<string, Identifier>::iterator it2 = short_name_to_id.begin();
	for(; it2!= short_name_to_id.end(); it2++)
	{
		const Identifier& id = it2->second;
		const string& pretty_print = it2->first;

		QList<QStandardItem*> to_insert;

		QStandardItem* q = new QStandardItem(QString::fromStdString(id.to_string()));
		unit_item_to_identifier[q] = id;

		q->setEditable(false);
		to_insert.push_back(q);

		QStandardItem* q0 = new QStandardItem(QString::fromStdString(""));
		unit_item_to_identifier[q0] = id;

		if(excluded_units.count(id) > 0)
			q0->setIcon(QIcon(EXCLUDED_ICON));

		q0->setEditable(false);
		to_insert.push_back(q0);





		QStandardItem* q2 = new QStandardItem(QString::fromStdString(pretty_print));
		unit_item_to_identifier[q2] = id;
		if(id == current_entry_id){
			q0->setIcon(QIcon(ENTRY_ICON));
		}



		q2->setEditable(false);
		to_insert.push_back(q2);

		QStandardItem* q3 = new QStandardItem(QString::fromStdString(""));
		unit_item_to_identifier[q3] = id;
		q3->setEditable(false);
		to_insert.push_back(q3);
		unit_model->appendRow(to_insert);

	}
	unit_tree->resizeColumnToContents(2);





}




void compass_ui::update_unit_view(AnalysisResult * ar, string f_id)
{

	QList<QStandardItem *>	result =
			unit_model->findItems(QString::fromStdString(f_id));

	if(result.size() == 0) return;
	QStandardItem *item = *result.begin();
	if(item == NULL){

		return;
	}
	int row = item->row();

	if(ar->termination_value != 0) {
		QStandardItem* q = new QStandardItem(
				QIcon("/usr/share/icons/oxygen/16x16/actions/tools-report-bug"), "");
		q->setEditable(false);
		unit_model->setItem(row, 3, q);
	}
	else
	{
		string time = double_to_string(ar->time);
		QStandardItem* q = new QStandardItem(
				QString::fromStdString(time));
		q->setEditable(false);
		unit_model->setItem(row, 3, q);
	}

}

void compass_ui::update_function_control(const Identifier & id)
{
	callgraph_model->removeRows(0, callgraph_model->rowCount());
	call_item_to_id.clear();
	if(id.is_empty()) return;

	CGNode* node = cg->get_node(id);
	if(node == NULL) return;

	update_function_control_rec(node, callgraph_model->invisibleRootItem(),
			callers_cg->isChecked());

	callg_tree->expandToDepth(0);





}

void compass_ui::update_function_control_rec(CGNode* n, QStandardItem *parent,
		bool callers)
{



	QStandardItem *item = new QStandardItem(QString::fromStdString(
			n->id.get_function_name()));
	call_item_to_id[item] = n->id;
	item->setEditable(false);
	parent->appendRow(item);

	if(callers)
	{
		map<CGNode*, cg_edge_id>::iterator it = n->callers.begin();
		for(; it != n->callers.end(); it++)
		{
			//Disregard backedges here
			if(it->second.is_backedge()) continue;
			update_function_control_rec(it->first, item, callers);
		}
	}
	else
	{

		map<CGNode*, cg_edge_id>::iterator it = n->callees.begin();
		for(; it != n->callees.end(); it++)
		{
			//Disregard backedges here
			if(it->second.is_backedge()) continue;
			update_function_control_rec(it->first, item, callers);
		}
	}


}

/*
 * Open selected fn in editor
 */
void compass_ui::callgraph_fn_selected(QModelIndex index)
{
	QStandardItem*  it = callgraph_model->itemFromIndex(index);
	if(call_item_to_id.count(it) == 0) return;
	Identifier id = call_item_to_id[it];

	sail::Function* sail_fn  = (sail::Function*)
					dm->get_data(id.get_function_identifier().to_string('#'),
							SAIL_FUNCTION);
	assert(sail_fn != NULL);
	int line = sail_fn->get_first_line();
	this->open_file(id.get_file_id().to_string(), true, line);


}

void compass_ui::callgraph_fn_double_selected(QModelIndex index)
{
	QStandardItem*  it = callgraph_model->itemFromIndex(index);
	if(call_item_to_id.count(it) == 0) return;
	Identifier id = call_item_to_id[it];
	unit_selected(id);
}



string compass_ui::expand_dot(string file)
{
	if(file.size()<1)
	return "";
	if(file.size() == 1) return file;
	if(file[0] == '.' && file[1] == '/') return current_dir+file.substr(2);
	return file;
}


void compass_ui::update_results()
{
	error_model->removeRows(0, error_model->rowCount());


	map<string, set<Error*> >::iterator it = errors_per_file.begin();
	for(; it!= errors_per_file.end(); it++)
	{
		//cout << "*********** File: " << it->first << endl;
		set<Error*> & errors = it->second;
		set<Error*>::iterator it2 = errors.begin();
		for(; it2 != errors.end(); it2++)
		{
			Error* e = *it2;
			QList<QStandardItem*> entry;
			string filename = expand_dot(e->file);
			//cout << "filename: " << filename << endl;
			if(filename.size() >current_dir.size())
				filename = filename.substr(this->current_dir.size());

			QStandardItem *file = new QStandardItem(
					QString::fromStdString(filename));
			file->setEditable(false);
			entry.push_back(file);
			QStandardItem *line = new QStandardItem(
					QString::fromStdString(int_to_string(e->line)));
			line->setEditable(false);
			entry.push_back(line);
			string res = qt_escape_string(e->message);
			//cout << "res: " << res << endl;

			QStandardItem *message = new QStandardItem(
					QString::fromStdString(res));
			message->setEditable(false);
			entry.push_back(message);

			error_model->appendRow(entry);
			//cout << "Error: " << e->to_string() << endl;
		}

	}
	error_model->sort(0);
	for(int i=0; i < ui.editor_tabs->count(); i++)
	{
		KTextEditor::View* v = (KTextEditor::View*) ui.editor_tabs->widget(i);
		highlight_errors(v, get_file_from_tab(i));
	}



}



/*
 * Called when the UI quits.
 * Here, we have to 1) save all modified files
 * 2) serialize the content of the datamanager out
 */
void compass_ui::app_quit()
{

	if(!lock_valid()) return;
	write_current_project_settings();
	for(int i=0; i < ui.editor_tabs->count(); i++)
	{
		write_tab(i);
	}
	if(dm!= NULL) dm->flush_syncronized();
	QString nn = ui.name->text();

	string n = nn.toStdString();
	n = strip(n);
	set_name(n);

}

compass_ui::~compass_ui()
{

}





