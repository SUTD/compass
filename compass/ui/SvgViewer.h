/*
 * SvgViewer.h
 *
 *  Created on: Dec 9, 2009
 *      Author: tdillig
 */



#ifndef SVGVIEWER_H_
#define SVGVIEWER_H_
 #include <QSvgWidget>
#include <qgl.h>
#include <qapplication.h>
#include <qimage.h>
#include <QtGui>
#include <string>

using namespace std;

class SvgViewer : public QWidget
{
	Q_OBJECT

 public:
	SvgViewer(QWidget* parent = 0);
	void scaleImage(double factor);
	void draw(const string & dotty);
	void clear();


 // void paintGL();
  //void resizeGL(int w, int h);

 protected:
	void adjustScrollBar(QScrollBar *scrollBar, double factor);
	double ratio(string file);
 // QImage data;//, gldata;
  QSvgWidget *imageLabel;
  QScrollArea *scrollArea;
  QSlider *s;
  QSize init_size;
  double factor;
 public slots:

  void zoom(int i);
};





/*void SvgViewer::paintGL()
{
	cout << "width: " << data.width() << endl;
 glDrawPixels(data.width(), data.height(), GL_RGBA, GL_UNSIGNED_BYTE, gldata.bits());
}
void SvgViewer::resizeGL(int w, int h)
{
 glViewport (0, 0, w, h);
 glMatrixMode (GL_PROJECTION);
 glLoadIdentity();
 glOrtho(0, w,0,h,-1,1);
 glMatrixMode (GL_MODELVIEW);
}
*/


#endif /* SVGVIEWER_H_ */
