/*
 * ktgantt.h - TaskJuggler Viewer
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef KTJGANTT_H
#define KTJGANTT_H

#include <time.h>
#include <qsplitter.h>
class QWidget;
class QCanvasView;
class QPainter;
class KURL;
class KTVTaskTable;
class KTVTaskCanvasView;
class KAboutData;
class Project;
class KTVHeader;
class KDateWidget;
class QDate;
class TimeDialog;

/**
 *
 * @short TaskJuggler Gantt Viewer
 * @author Klaas Freitag <freitag@kde.org>
 * @version 0.1
 */

class KTJGantt : public QSplitter
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    KTJGantt( QWidget *parentWidget, const char *widgetName );

    /**
     * Destructor
     */
    virtual ~KTJGantt();


    void showProject( Project * );

    void setInterval( time_t start, time_t end );

    /**
     * Clear the whole gantt widget and forget everything 
     */ 
    void clear();
    
public slots:
    void slZoomIn();
    void slZoomOut();
    void slZoomOriginal();
    void slTimeFrame();
    /**
     *  Start and End of the interval that is displayed
     */
    void slSetWeekStartsMonday( bool t );

protected slots:
    virtual void slCanvasClicked( time_t );
    void slTimeFromDialog();

    void slCanvasMoved( int, int );
    void slTableMoved( int, int );
    
signals:
    void statusBarChange( const QString& );

private:
    KTVTaskCanvasView *m_canvas;
    KTVHeader         *m_header;
    KTVTaskTable      *m_table;

    bool                m_weekStartMon;
    bool                m_doTableMove;
    time_t              m_start;
    time_t              m_end;

    TimeDialog          *m_timeDialog;
};

#endif // KTJVIEWPART_H
