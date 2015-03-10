/*
 * FileView.cpp
 *
 *  Created on: Dec 2, 2009
 *      Author: tdillig
 */

#include "FileView.h"
#include <iostream>
#include <QtGui>
#include "compass_ui.h"

using namespace std;

FileView::FileView(QWidget *parent, compass_ui* base):QTreeView(parent)
{
	this->base = base;

}

FileView::FileView(compass_ui* base)
{
	this->base = base;


}

void FileView::add_file()
{


	this->base->new_file(base->get_folder(selected));
}

void FileView::delete_file()
{
	if(selected == "") return;

	QMessageBox msgBox;
	string text = "Really delete \"" + base->get_id(selected) + "\"?";
	msgBox.setText(QString::fromStdString(text));
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	int ret = msgBox.exec();
	if(ret != QMessageBox::Ok) return;

	base->delete_file(selected);

}

void FileView::mouseDoubleClickEvent(QMouseEvent * event)
{
	QModelIndex index = this->indexAt(event->pos());
	if(!index.isValid()) return;
	compute_selected(event);
	base->open_file(selected, true);
}


bool FileView::compute_selected(QContextMenuEvent *event)
{
	selected = "";
	QModelIndex index = this->indexAt(event->pos());
	if(!index.isValid()) return false;
	string path;
	do
	{
		QVariant v = index.model()->data(index, 0);
		string segment =  v.toString().toStdString();
		path="/" + segment + path;
		index = index.parent();

	} while(index.parent().isValid());
	selected = path;
	return true;
}

bool FileView::compute_selected(QMouseEvent *event)
{
	selected = "";
	QModelIndex index = this->indexAt(event->pos());
	if(!index.isValid()) return false;
	string path;
	do
	{
		QVariant v = index.model()->data(index, 0);
		string segment =  v.toString().toStdString();
		path="/" + segment + path;
		index = index.parent();

	} while(index.parent().isValid());
	selected = path;
	return true;
}



void FileView::new_folder()
{

	bool ok;
	QString name = QInputDialog::getText(this, "Question",
	                                          tr("New directory name:"), QLineEdit::Normal,
	                                          "", &ok);

	string res = base->get_folder(selected);


	res += name.toStdString();
	base->new_folder(res);

}

void FileView::contextMenuEvent(QContextMenuEvent *event)
{


	QMenu menu(this);
	this->event = event;



	bool selected = compute_selected(event);




	QAction* add_action = new QAction("Add File...", this);
	menu.addAction(add_action);
	connect(add_action, SIGNAL(triggered()), this, SLOT(add_file()));
	QAction* new_folder_action = new QAction("New Folder...", this);
	menu.addAction(new_folder_action);
	connect(new_folder_action, SIGNAL(triggered()), this, SLOT(new_folder()));

	if(selected)
	{
		QAction* delete_action = new QAction("Delete", this);
		menu.addAction(delete_action);
		connect(delete_action, SIGNAL(triggered()), this, SLOT(delete_file()));
	}

	menu.exec(event->globalPos());


}

FileView::~FileView() {
	// TODO Auto-generated destructor stub
}
