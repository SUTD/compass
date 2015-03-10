/*
 * StatsViewer.h
 *
 *  Created on: Jan 26, 2010
 *      Author: tdillig
 */

#ifndef STATSVIEWER_H_
#define STATSVIEWER_H_

#include <QtGui>
#include <string>
using namespace std;

class StatsViewer:public QWidget
{
	Q_OBJECT
private:
	QLabel* selection;
	QTextEdit* edit;
public:
	StatsViewer();
	void display_stats(string name, string stats);
	void clear_stats();
	virtual ~StatsViewer();
};

#endif /* STATSVIEWER_H_ */
