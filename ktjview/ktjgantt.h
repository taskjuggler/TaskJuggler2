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

#include <qsplitter.h>
class QWidget;
class QCanvasView;
class QPainter;
class KURL;
class KTVTaskTable;
class KTVTaskCanvasView;
class KAboutData;
class Project;


/**
 *
 * @short TaskJuggler Gantt Viewer
 * @author Klaas Freitag <freitag@kde.org>
 * @version 0.1
 */
class KActionCollection;

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
   
public slots:
   void slZoomIn();
   void slZoomOut();
   void slZoomOriginal();
signals:
   void statusBarChange( const QString& );

private:
   KTVTaskCanvasView *m_canvas;
   KTVTaskTable *m_table;
};

#endif // KTJVIEWPART_H
