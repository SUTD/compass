/*
 * StatsViewer.cpp
 *
 *  Created on: Jan 26, 2010
 *      Author: tdillig
 */

#include "StatsViewer.h"

StatsViewer::StatsViewer()
{
	selection = new QLabel(this);
	selection->setText("No summary unit selected.");
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(selection);

	edit = new QTextEdit(this);
	edit->setReadOnly(true);
	layout->addWidget(edit);
	this->setLayout(layout);

}

void StatsViewer::display_stats(string name, string stats)
{
	selection->setText(QString::fromStdString(name));
	edit->setText(QString::fromStdString(stats));
}
void StatsViewer::clear_stats()
{
	selection->setText("No summary unit selected.");
	edit->setText("");
}

StatsViewer::~StatsViewer()
{

}
