/*
 * ktvtaskcanvasview.h - TaskJuggler Viewer
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _KTVTASKCANVASVIEW_H
#define _KTVTASKCANVASVIEW_H


#include <klistview.h>
#include <Project.h>
#include <Task.h>
#include <qcanvas.h>

#include <qpixmap.h>

#include "ktvtaskcanvas.h"

class KTVTaskTable;
class KTVTaskCanvasItem;
class QPoint;

class KTVTaskCanvasView: public QCanvasView
{
    Q_OBJECT
public:
    KTVTaskCanvasView( QWidget *parent, KTVTaskTable *tab=0, const char *name = 0 );
    virtual ~KTVTaskCanvasView(){ }
   void showProject( Project * );
   void contentsMousePressEvent( QMouseEvent* e );

   /**
    * find a task item under the clicked point.
    *
    * @returns a task item or zero.
    */
   KTVCanvasItemBase* taskItemAt( const QPoint& );

   void finalise( Project *p );
   
public slots:
   void zoomIn();
   void zoomOut();
   void zoomOriginal();
protected:
   void wheelEvent( QWheelEvent * ) {};
   
private:
   void addTask( Task *t );
   Project *m_pro;

   KTVTaskCanvas *m_canvas;

};


#endif

