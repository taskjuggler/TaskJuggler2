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
class KTVHeader;
class KTVTaskCanvasItem;
class QPoint;

class KTVTaskCanvasView: public QCanvasView
{
    Q_OBJECT
public:
    KTVTaskCanvasView( QWidget *parent, KTVTaskTable *tab, KTVHeader *h, const char *name = 0 );
    virtual ~KTVTaskCanvasView(){ }

    /* returns the width the canvas was resized to calculated from start- and end date */
    void showProject( Project * );


    KTVTaskCanvas* getCanvas() { return m_canvas; }
    /**
     * find a task item under the clicked point.
     *
     * @returns a task item or zero.
     */
    KTVCanvasItemBase* taskItemAt( const QPoint& );

    /**
     * @return the time that is currently in center of the viewport.
     */
    time_t getCenterTime();

    /* ensure that the given time is visible at a p percent of the viewport
     * width from the left side.
     */
    void xScrollToTime( int, time_t );

protected:
    void contentsMousePressEvent( QMouseEvent * );

signals:
    void scrolledBy( int, int );
    void canvasClicked( time_t );

public slots:
    void zoomIn();
    void zoomOut();
    void zoomOriginal();
    void slScrollTo(int, int);
    void clear();
    void slTableMoving(int, int);
    
private:
    Project *m_pro;

    KTVTaskCanvas *m_canvas;
    KTVTaskTable  *m_table;
    KTVHeader     *m_header;
    double         m_scaleFactor;

    bool           m_suppressScroll;
};


#endif

