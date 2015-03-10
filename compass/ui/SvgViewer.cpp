#include "SvgViewer.h"
#include <fstream>
#include "util.h"
#include <assert.h>
#include <iostream>
#include "util.h"

void SvgViewer::scaleImage(double factor)
{
//    Q_ASSERT(imageLabel->pixmap());

    imageLabel->resize(factor * init_size);

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

}

double SvgViewer::ratio(string file)
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
	i = x+8;
	string hs;
	while(true)
	{
		if(res[i] == 'p') break;
		hs += res[i++];
	}
	int h = string_to_int(hs);
	int w =string_to_int(ws);
	init_size.setHeight(h);
	init_size.setWidth(w);

	return w/h;



}

void SvgViewer::draw(const string & dotty)
{

	s->setSliderPosition(100*factor);
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
	if(ratio(file) == -1) return;
	//cout << "Command: " << cmd << endl;
	QString filename = QString::fromStdString(file);
	imageLabel->load(filename);
	//imageLabel->setPixmap(QPixmap::fromImage(data));

	QSvgWidget* tempt = new QSvgWidget(filename);
	//strange redraw bug workaround
	//scaleImage(10);
	scaleImage(factor);
	remove(temp.c_str());
	remove(file.c_str());
	//imageLabel->resize(imageLabel->pixmap()->size());

}

SvgViewer::SvgViewer(QWidget* parent) : QWidget(parent)

{
	factor = 1.0;


	imageLabel = new QSvgWidget(this);



	//imageLabel = new QLabel;
	imageLabel->setBackgroundRole(QPalette::Base);
	imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	//imageLabel->setScaledContents(true);



	scrollArea = new QScrollArea;
	scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setWidget(imageLabel);
	scrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	//imageLabel->setPixmap(QPixmap::fromImage(data));
	//imageLabel->resize(imageLabel->pixmap()->size());
	QHBoxLayout *clayout = new QHBoxLayout;
	s = new QSlider(this);
	s->setMinimum(5);
	s->setMaximum(200);
	s->setSliderPosition(100);

	clayout->addWidget(scrollArea);
	clayout->addWidget(s);

	this->setLayout(clayout);

	QPalette p(palette());
		// Set background color to white
	p.setColor(QPalette::Dark, Qt::white);
	scrollArea->setPalette(p);

	QObject::connect(s,SIGNAL(valueChanged(int)), this, SLOT(zoom(int)));


 //setCentralWidget(scrollArea);
}
void SvgViewer::clear()
{
	draw("");
}

void SvgViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void SvgViewer::zoom(int i)
{

	factor = ((double)i)/100;
	scaleImage(factor);
}
