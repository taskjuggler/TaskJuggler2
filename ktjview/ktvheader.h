/*
 * ktvheader.h - TaskJuggler Viewer header widget
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef KTVHEADER_H
#define KTVHEADER_H

#include <qscrollview.h>
#include <time.h>

class QFont;

/**
 *
 * @short TaskJuggler Gantt Viewer Header Widget
 * @author Klaas Freitag <freitag@suse.de>
 * @version 0.1
 */

class KTVHeader : public QScrollView
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    KTVHeader( QWidget *parentWidget, const char *widgetName );

    /**
     * Destructor
     */
    virtual ~KTVHeader();
    void drawContents ( QPainter * p, int clipx, int clipy, int clipw, int cliph );

    time_t timeFromX( int );
    int    midnightToX( time_t );
    int    timeToX( time_t );

    int    dayWidth() { return m_dayWidth; }
    int    overallWidth();
    void   setInterval( time_t start, time_t end );
    int    daysInInterval();
    time_t startTime() { return m_start; }
    time_t endTime()   { return m_end;   }
    void   setWeekStartMon( bool t ) { m_weekStartMon = t; }
protected:
    enum topOffsetPart { Month, Week, Day, All };
    virtual int        topOffset( topOffsetPart part = Day );
    virtual int        getHeaderLineHeight( topOffsetPart = Day );

public slots:
    void slSetDayWidth(int);
    void slSetHeight(int);
private:
    int       m_height;
    time_t    m_start, m_end;
    int       m_dayWidth;

    bool      m_weekStartMon;
    QFont     m_headerFont;

};

#endif // KTJVIEWPART_H
