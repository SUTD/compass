/*
 * DottyViewer.h
 *
 * This visualizes a dotty file and exports zoom functionality.
 * For speed, it draws using OpenGL. This means if your graphics card does
 * not have 3D acceleration or you do not have 3D drivers installed, you
 * will not see *any* compass visualization.
 *
 *  Created on: Jan 3, 2010
 *      Author: tdillig
 */

#ifndef DOTTYVIEWER_H_
#define DOTTYVIEWER_H_

#include <QSvgWidget>
#include <qgl.h>
#include <qapplication.h>
#include <qimage.h>
#include <QtGui>
#include <qgl.h>
#include <string>

using namespace std;


class DottyViewer : public QGLWidget
{
	Q_OBJECT
protected:
	QImage data, gldata;
public:
	DottyViewer(QWidget* parent = 0);

	/*
	 * Draw the provided dotty string
	 */
	void draw(const string & dotty);

	void scale(double factor);

	/*
	 * Clear the current drawing
	 */
	void clear();
	virtual ~DottyViewer();


protected:
	void paintGL();
	void resizeGL(int w, int h);
	double ratio(string file);
};

#endif /* DOTTYVIEWER_H_ */
