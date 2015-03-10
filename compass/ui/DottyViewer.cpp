/*
 * DottyViewer.cpp
 *
 *  Created on: Jan 3, 2010
 *      Author: tdillig
 */

#include "DottyViewer.h"

#include <fstream>
#include "util.h"
#include <assert.h>
#include <iostream>
#include "util.h"

DottyViewer::DottyViewer(QWidget* parent):QGLWidget(parent)
{


}


void DottyViewer::draw(const string & dotty)
{
	ofstream out;
	short ss = rand();
	if(ss < 0) ss = -ss;
	string temp = "/tmp/dotty_" + int_to_string(ss) + ".dot";
	string file = "/tmp/dotty_" + int_to_string(ss) + ".svg";

	out.open(temp.c_str());
	assert(out.is_open());
	out << dotty;
	out.close();
	string cmd = "dot -Tsvg " + temp + " > " + file;
	system(cmd.c_str());
	//if(ratio(file) == -1) return;
	//cout << "Command: " << cmd << endl;
	QString filename = QString::fromStdString(file);

	/*
	imageLabel->load(filename);
	//imageLabel->setPixmap(QPixmap::fromImage(data));

	QSvgWidget* tempt = new QSvgWidget(filename);
	scaleImage(1);
	*/




	data.load(filename);
	gldata = QGLWidget::convertToGLFormat(data);
	resize(data.size());
	remove(temp.c_str());
	remove(file.c_str());
}



void DottyViewer::paintGL()
{

	//glRasterPos2s(100,100);

	glPixelZoom(0.5, 0.5);

	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawPixels(data.width(), data.height(), GL_RGBA, GL_UNSIGNED_BYTE,
			gldata.bits());


}

void DottyViewer::scale(double factor)
{
	cout << "scaling..." << factor << endl;
	double w = width()*factor;
	double h = height()*factor;
	glViewport (0, 0, width(), height());
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w,0,h,-2,1);
	glMatrixMode (GL_MODELVIEW);
}


void DottyViewer::resizeGL(int w, int h)
{
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w,0,h,-1,1);
	glMatrixMode (GL_MODELVIEW);

}

DottyViewer::~DottyViewer()
{

}


double DottyViewer::ratio(string file)
{
	ifstream iff;
	iff.open(file.c_str());
	if(!iff.is_open()) return -1;

	string res;
	int i = 0;

	while(!iff.eof() && i < 20)
	{
		i++;
		string temp;
		std::getline(iff, temp);
		res+=temp;
	}
	iff.close();

	bool empty = true;
	for(unsigned int j = 0; j < res.size(); j++)
	{
		if(res[i]!=' ') {
			empty = false;
			break;
		}
	}
	if(empty) return -1;

	int x = res.find("width=\"");
	assert(x != string::npos);
	i = x+7;
	string ws;



	while(true)
	{
		if(res[i] == 'p') break;
		//cout << "i111" << i << endl;
		ws += res[i++];
	}

	x = res.find("height=\"");
	if(x == string::npos) cout << "aaaaaaaaaaaaaaaa" << endl;
	i = x+8;
	string hs;
	while(true)
	{
		if(res[i] == 'p') break;
		hs += res[i++];
	}
	int h = string_to_int(hs);
	int w =string_to_int(ws);
	//init_size.setHeight(h);
	//init_size.setWidth(w);

	return w/h;



}
