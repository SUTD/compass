/*
 * FileView.h
 *
 *  Created on: Dec 2, 2009
 *      Author: tdillig
 */

#ifndef FILEVIEW_H_
#define FILEVIEW_H_

#include <string>
using namespace std;

#include <QtGui/QWidget>
#include <QTreeView>

class compass_ui;

class FileView : public QTreeView{
	Q_OBJECT
private:
	string selected;
	compass_ui* base;
	QContextMenuEvent *event;
public:
	FileView(QWidget *parent, compass_ui* base);
	FileView(compass_ui* base);
	virtual ~FileView();
protected:
  void contextMenuEvent(QContextMenuEvent *event);
  void mouseDoubleClickEvent ( QMouseEvent * event );
private:
  bool compute_selected(QContextMenuEvent *event);
  bool compute_selected(QMouseEvent *event);
public slots:
  void delete_file();
  void add_file();
  void new_folder();
};

#endif /* FILEVIEW_H_ */
