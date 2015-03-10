/********************************************************************************
** Form generated from reading ui file 'compass_ui.ui'
**
** Created: Mon Dec 14 19:48:53 2009
**      by: Qt User Interface Compiler version 4.5.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_COMPASS_UI_H
#define UI_COMPASS_UI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>



QT_BEGIN_NAMESPACE

class Ui_compass_uiClass
{
public:
    QHBoxLayout *horizontalLayout_2;
    QTabWidget *Tob1;
    QWidget *tab;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *project_used;
    QLabel *label_2;
    QLineEdit *name;
    QFrame *line;
    QHBoxLayout *horizontalLayout_3;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *selector_layout;
    QTabWidget *tabWidget_2;
    QWidget *files;
    QWidget *Units;
    QWidget *callgraph;
    QFrame *line_3;
    QFrame *line_4;

    QWidget *tab_4;
    QFrame *line_2;
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *right_layout;
    QVBoxLayout *verticalLayout_7;
    QLabel *label_5;
    QFrame *line_5;
    QHBoxLayout *horizontalLayout_6;
    QVBoxLayout *verticalLayout_13;
    QLabel *label_7;
    QLabel *label_4;
    QLabel *label_6;
    QVBoxLayout *verticalLayout_14;
    QSpinBox *num_cpus;
    QLabel *walltime;
    QLabel *cputime;
    QListWidget *status_list;
    QProgressBar *progress_bar;
    QGridLayout *gridLayout_2;
    QCheckBox *buffer_check;
    QCheckBox *null_check;
    QCheckBox *uninit_check;
    QCheckBox *taint_check;
    QLabel *loc_label;
    QHBoxLayout *horizontalLayout_7;
    QLabel *total_num;
    QLabel *num_analyzed;
    QHBoxLayout *horizontalLayout_13;
    QPushButton *run_button;
    QPushButton *cancel;
    QPushButton *add_regression_button;
    QHBoxLayout *horizontalLayout_5;
    QFrame *line_6;
    QGridLayout *gridLayout;

    QVBoxLayout *verticalLayout_3;
    QWidget *callgraph_tab;
    QWidget *cfg_tab;
    QWidget *summary_tab;
    QWidget *stats_tab;
    QWidget *memory_tab;
    QWidget *regression_tab;

    /*
     * Four main window components
     */
    QWidget* control;
    QWidget* selector;
    QTreeView *error_view;
    QTabWidget *editor_tabs;



    QSplitter* splitter;

    void setupUi(QWidget *compass_uiClass)
    {
        if (compass_uiClass->objectName().isEmpty())
            compass_uiClass->setObjectName(QString::fromUtf8("compass_uiClass"));
        compass_uiClass->resize(1175, 791);
        horizontalLayout_2 = new QHBoxLayout(compass_uiClass);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setMargin(11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        Tob1 = new QTabWidget(compass_uiClass);
        Tob1->setObjectName(QString::fromUtf8("Tob1"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout_2 = new QVBoxLayout(tab);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setMargin(11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(tab);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        project_used = new QComboBox(tab);
        project_used->setObjectName(QString::fromUtf8("project_used"));
        project_used->setMinimumSize(QSize(150, 0));

        horizontalLayout->addWidget(project_used);

        label_2 = new QLabel(tab);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        name = new QLineEdit(tab);
        name->setObjectName(QString::fromUtf8("name"));

        horizontalLayout->addWidget(name);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addLayout(verticalLayout);

        line = new QFrame(tab);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        selector_layout = new QVBoxLayout();
        selector_layout->setSpacing(6);
        selector_layout->setObjectName(QString::fromUtf8("verticalLayout_4"));
        tabWidget_2 = new QTabWidget(tab);
        tabWidget_2->setObjectName(QString::fromUtf8("tabWidget_2"));
        tabWidget_2->setMinimumSize(QSize(100, 0));


        files = new QWidget();
        files->setObjectName(QString::fromUtf8("files"));
        tabWidget_2->addTab(files, QString());
        Units = new QWidget();
        Units->setObjectName(QString::fromUtf8("Units"));
        tabWidget_2->addTab(Units, QString());
        callgraph = new QWidget();
        callgraph->setObjectName(QString::fromUtf8("callgraph"));
        tabWidget_2->addTab(callgraph, QString());

        selector_layout->addWidget(tabWidget_2);



        selector =new QWidget();
        selector->setLayout(selector_layout);
        selector->setMaximumSize(600, 99999);
        selector->setBaseSize(100, 99999);
        selector->setSizeIncrement(0, 0);




        QSplitter* h_splitter = new QSplitter();
        h_splitter->addWidget(selector);

      //  h_splitter->addWidget(selector);
        horizontalLayout_4->addWidget(h_splitter);
        //horizontalLayout_4->addWidget(selector);





        line_3 = new QFrame(tab);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        horizontalLayout_4->addWidget(line_3);

        line_4 = new QFrame(tab);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setFrameShape(QFrame::VLine);
        line_4->setFrameShadow(QFrame::Sunken);

        horizontalLayout_4->addWidget(line_4);

        editor_tabs = new QTabWidget(tab);
        editor_tabs->setObjectName(QString::fromUtf8("editor_tabs"));
        editor_tabs->setTabsClosable(true);
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        editor_tabs->addTab(tab_4, QString());

        //duck
        //horizontalLayout_4->addWidget(editor_tabs);
        h_splitter->addWidget(editor_tabs);
        editor_tabs->setSizeIncrement(1, 1);

        QList<int> sizes;
        sizes.push_back(150);
        sizes.push_back(500);
        h_splitter->setSizes(sizes);

        line_2 = new QFrame(tab);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);

        horizontalLayout_4->addWidget(line_2);

        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));

        horizontalLayout_4->addLayout(verticalLayout_5);


        horizontalLayout_3->addLayout(horizontalLayout_4);

        right_layout = new QVBoxLayout();
        right_layout->setSpacing(6);
        right_layout->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setSpacing(6);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        label_5 = new QLabel(tab);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        verticalLayout_7->addWidget(label_5);


        right_layout->addLayout(verticalLayout_7);

        line_5 = new QFrame(tab);
        line_5->setObjectName(QString::fromUtf8("line_5"));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);

        right_layout->addWidget(line_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalLayout_6->setSizeConstraint(QLayout::SetMinimumSize);
        verticalLayout_13 = new QVBoxLayout();
        verticalLayout_13->setSpacing(6);
        verticalLayout_13->setObjectName(QString::fromUtf8("verticalLayout_13"));
        label_7 = new QLabel(tab);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        verticalLayout_13->addWidget(label_7);

        label_4 = new QLabel(tab);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        verticalLayout_13->addWidget(label_4);

        label_6 = new QLabel(tab);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        verticalLayout_13->addWidget(label_6);


        horizontalLayout_6->addLayout(verticalLayout_13);

        verticalLayout_14 = new QVBoxLayout();
        verticalLayout_14->setSpacing(6);
        verticalLayout_14->setObjectName(QString::fromUtf8("verticalLayout_14"));
        num_cpus = new QSpinBox(tab);
        num_cpus->setObjectName(QString::fromUtf8("num_cpus"));
        num_cpus->setMaximumSize(QSize(50, 16777215));


        verticalLayout_14->addWidget(num_cpus);

        walltime = new QLabel(tab);
        walltime->setObjectName(QString::fromUtf8("walltime"));

        verticalLayout_14->addWidget(walltime);

        cputime = new QLabel(tab);
        cputime->setObjectName(QString::fromUtf8("cputime"));

        verticalLayout_14->addWidget(cputime);


        horizontalLayout_6->addLayout(verticalLayout_14);


        right_layout->addLayout(horizontalLayout_6);

        status_list = new QListWidget(tab);
        status_list->setObjectName(QString::fromUtf8("status_list"));
        status_list->setMaximumSize(QSize(130, 16777215));

        right_layout->addWidget(status_list);

        progress_bar = new QProgressBar(tab);
        progress_bar->setObjectName(QString::fromUtf8("progress_bar"));
        progress_bar->setMinimumSize(QSize(100, 0));
        progress_bar->setMaximumSize(QSize(130, 16777215));
        progress_bar->setValue(0);

        right_layout->addWidget(progress_bar);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setSpacing(6);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        buffer_check = new QCheckBox(tab);
        buffer_check->setObjectName(QString::fromUtf8("buffer_check"));

        gridLayout_2->addWidget(buffer_check, 2, 0, 1, 1);

        null_check = new QCheckBox(tab);
        null_check->setObjectName(QString::fromUtf8("null_check"));

        gridLayout_2->addWidget(null_check, 3, 0, 1, 1);

        uninit_check = new QCheckBox(tab);
        uninit_check->setObjectName(QString::fromUtf8("uninit_check"));

        gridLayout_2->addWidget(uninit_check, 4, 0, 1, 1);



        taint_check = new QCheckBox(tab);
        taint_check->setObjectName(QString::fromUtf8("taint_check"));

        gridLayout_2->addWidget(taint_check, 5, 0, 1, 1);




        loc_label = new QLabel(tab);
        loc_label->setObjectName(QString::fromUtf8("loc_label"));

        gridLayout_2->addWidget(loc_label, 1, 0, 1, 1);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        total_num = new QLabel(tab);
        total_num->setObjectName(QString::fromUtf8("total_num"));


        num_analyzed = new QLabel(tab);
        num_analyzed->setObjectName(QString::fromUtf8("num_analyzed"));


        horizontalLayout_7->addWidget(num_analyzed);
         horizontalLayout_7->addWidget(total_num);

        gridLayout_2->addLayout(horizontalLayout_7, 0, 0, 1, 1);


        right_layout->addLayout(gridLayout_2);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setSpacing(6);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        run_button = new QPushButton(tab);
        run_button->setObjectName(QString::fromUtf8("run_button"));

        horizontalLayout_13->addWidget(run_button);

        cancel = new QPushButton(tab);
        cancel->setObjectName(QString::fromUtf8("cancel"));

        horizontalLayout_13->addWidget(cancel);


        right_layout->addLayout(horizontalLayout_13);

        add_regression_button = new QPushButton(tab);
        add_regression_button->setObjectName(QString::fromUtf8("add_regression_button"));

        right_layout->addWidget(add_regression_button);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));

        right_layout->addLayout(horizontalLayout_5);

        line_6 = new QFrame(tab);
        line_6->setObjectName(QString::fromUtf8("line_6"));
        line_6->setFrameShape(QFrame::HLine);
        line_6->setFrameShadow(QFrame::Sunken);

        right_layout->addWidget(line_6);



        control = new QWidget();
        control->setMaximumWidth(140);
        control->setLayout(right_layout);
        horizontalLayout_3->addWidget(control);



        verticalLayout_2->addLayout(horizontalLayout_3);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));

        verticalLayout_2->addLayout(gridLayout);

        error_view = new QTreeView(tab);
        error_view->setObjectName(QString::fromUtf8("error_view"));
        error_view->setMaximumSize(QSize(16777215, 140));

        verticalLayout_2->addWidget(error_view);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));

        verticalLayout_2->addLayout(verticalLayout_3);

        Tob1->addTab(tab, QString());
        callgraph_tab = new QWidget();
        callgraph_tab->setObjectName(QString::fromUtf8("callgraph_tab"));
        Tob1->addTab(callgraph_tab, QString());
        cfg_tab = new QWidget();
        cfg_tab->setObjectName(QString::fromUtf8("cfg_tab"));
        Tob1->addTab(cfg_tab, QString());
        summary_tab = new QWidget();
        summary_tab->setObjectName(QString::fromUtf8("summary_tab"));
        Tob1->addTab(summary_tab, QString());

        stats_tab = new QWidget();
        stats_tab->setObjectName(QString::fromUtf8("stats_tab"));
        Tob1->addTab(stats_tab, QString());


        memory_tab = new QWidget();
        memory_tab->setObjectName(QString::fromUtf8("memory_tab"));
        Tob1->addTab(memory_tab, QString());
        regression_tab = new QWidget();
        regression_tab->setObjectName(QString::fromUtf8("regression_tab"));
        Tob1->addTab(regression_tab, QString());

        horizontalLayout_2->addWidget(Tob1);


        retranslateUi(compass_uiClass);

        Tob1->setCurrentIndex(0);
        tabWidget_2->setCurrentIndex(0);
        editor_tabs->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(compass_uiClass);
    } // setupUi

    void retranslateUi(QWidget *compass_uiClass)
    {
        compass_uiClass->setWindowTitle(QApplication::translate("compass_uiClass", "Compass", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("compass_uiClass", "Project used:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("compass_uiClass", "Name:", 0, QApplication::UnicodeUTF8));
        tabWidget_2->setTabText(tabWidget_2->indexOf(files), QApplication::translate("compass_uiClass", "Files", 0, QApplication::UnicodeUTF8));
        tabWidget_2->setTabText(tabWidget_2->indexOf(Units), QApplication::translate("compass_uiClass", "Units", 0, QApplication::UnicodeUTF8));
        tabWidget_2->setTabText(tabWidget_2->indexOf(callgraph), QApplication::translate("compass_uiClass", "Callgraph", 0, QApplication::UnicodeUTF8));
        editor_tabs->setTabText(editor_tabs->indexOf(tab_4), QApplication::translate("compass_uiClass", "Tab 2", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("compass_uiClass", "Status", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("compass_uiClass", "Num CPUs", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("compass_uiClass", "Walltime", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("compass_uiClass", "CPU time", 0, QApplication::UnicodeUTF8));
        walltime->setText(QApplication::translate("compass_uiClass", "0", 0, QApplication::UnicodeUTF8));
        cputime->setText(QApplication::translate("compass_uiClass", "0", 0, QApplication::UnicodeUTF8));
        buffer_check->setText(QApplication::translate("compass_uiClass", "Check Buffers", 0, QApplication::UnicodeUTF8));
        null_check->setText(QApplication::translate("compass_uiClass", "Check Null", 0, QApplication::UnicodeUTF8));
        uninit_check->setText(QApplication::translate("compass_uiClass", "Check Uninit", 0, QApplication::UnicodeUTF8));
        taint_check->setText(QApplication::translate("compass_uiClass", "Check Taint", 0, QApplication::UnicodeUTF8));
        loc_label->setText(QString());
        total_num->setText(QString());
        num_analyzed->setText(QString());
        run_button->setText(QApplication::translate("compass_uiClass", "Run", 0, QApplication::UnicodeUTF8));
        cancel->setText(QApplication::translate("compass_uiClass", "Cancel", 0, QApplication::UnicodeUTF8));
        add_regression_button->setText(QApplication::translate("compass_uiClass", "Add as regression", 0, QApplication::UnicodeUTF8));
        Tob1->setTabText(Tob1->indexOf(tab), QApplication::translate("compass_uiClass", "Enter Code", 0, QApplication::UnicodeUTF8));
        Tob1->setTabText(Tob1->indexOf(callgraph_tab), QApplication::translate("compass_uiClass", "Callgraph", 0, QApplication::UnicodeUTF8));
        Tob1->setTabText(Tob1->indexOf(cfg_tab), QApplication::translate("compass_uiClass", "Cfg", 0, QApplication::UnicodeUTF8));
        Tob1->setTabText(Tob1->indexOf(summary_tab), QApplication::translate("compass_uiClass", "Summary", 0, QApplication::UnicodeUTF8));
        Tob1->setTabText(Tob1->indexOf(stats_tab), QApplication::translate("compass_uiClass", "Statistics", 0, QApplication::UnicodeUTF8));
        Tob1->setTabText(Tob1->indexOf(memory_tab), QApplication::translate("compass_uiClass", "Step through Unit", 0, QApplication::UnicodeUTF8));
        Tob1->setTabText(Tob1->indexOf(regression_tab), QApplication::translate("compass_uiClass", "Regressions", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(compass_uiClass);
    } // retranslateUi

};

namespace Ui {
    class compass_uiClass: public Ui_compass_uiClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMPASS_UI_H
