// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/***************************************************************************
 *   Copyright (C) 2004 by Lukas Tinkl                                     *
 *   lukas.tinkl@suse.cz                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _KTJVIEW2VIEW_H_
#define _KTJVIEW2VIEW_H_

#include <qwidget.h>
#include <qdom.h>
#include <qlistview.h>
#include <qdatetime.h>

#include <kurl.h>
#include <klistview.h>

#include "ktjview2iface.h"

#include "Project.h"

class QPainter;
class KDGanttView;
class KDGanttViewItem;
class KoKoolBar;
class QWidgetStack;
class QTextBrowser;
class KListView;
class KListViewItem;
class KPrinter;
class TaskList;
class TaskItem;

/**
 * This is the main view class for ktjview2.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 *
 * @short Main view
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class ktjview2View : public QWidget, public ktjview2Iface
{
    Q_OBJECT
public:
	/**
	 * Default constructor
	 */
    ktjview2View( QWidget *parent );

	/**
	 * Destructor
	 */
    virtual ~ktjview2View();

    /**
     * @return URL of the currently open project
     */
    KURL currentURL() const;

    /**
     * DCOP: Open a project from a given @p url
     */
    virtual void openURL( QString url );

    /**
     * Open a project from a given @p url
     */
    virtual void openURL( const KURL& url );

    /**
     * Invokes the print dialog and prints the view
     */
    void print( KPrinter * printer );

public slots:
    /**
     * Zoom in by 10%
     */
    void zoomIn();

    /**
     * Zoom out by 10%
     */
    void zoomOut();

    /**
     * Zooms the Gantt so that the whole project is visible
     */
    void zoomFit();

    /**
     * Select a scale for the gantt view.
     * @param scale the requested scale
     * @see KDGanttView::Scale
     */
    void slotScale( int scale );

    /**
     * Zooms so that at least the selected time period is visible after the zoom.
     *
     * @param from the start date
     * @param to the end date
     */
    void slotZoomTimeframe();

    void queryResource();

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar( const QString& text );

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption( const QString& text );

protected slots:
    /**
     * Switch between the items in the sidebar
     * @param grp ID of the group
     * @param item ID of the button
     */
    void slotKoolBar( int grp, int item );

private slots:
    /**
     * Centers the gantt chart on the selected @p item
     */
    void ensureItemVisible( KDGanttViewItem * item );

private:

    /**
     * Convert TaskStatus enum to human readable string
     */
    QString status2Str( int ts ) const;

    /**
     * Parse and display general project info
     */
    void parseProjectInfo();

    /**
     * Parse the task list of the project and fill the list view
     * @param sc Index of the scenario
     */
    void parseTasks( int sc = 0 );

    /**
     * Parse the task list and build the gantt chart
     * @param sc Index of the scenario
     */
    void parseGantt( TaskListIterator it, int sc = 0 );

    /**
     * Parse the resources list and fill the list view
     * @param it iterator over the list of resources
     * @param parentItem the parent item in the list view to append this item to
     */
    void parseResources( ResourceListIterator it, KListViewItem * parentItem = 0 );

    /**
     * Parses relations between tasks (dependencies)
     * @param it iterator over the list of resources
     */
    void parseLinks( TaskListIterator it );

    /**
     * Converts a time_t representation into QDateTime
     *
     * @param secs number of seconds since 1970-01-01, UTC
     */
    QDateTime time_t2Q( time_t secs );

    /**
     * Converts a time_t representation to human readable, locale aware string
     *
     * @param secs number of seconds since 1970-01-01, UTC
     */
    QString time_t2QS( time_t secs );

    /**
     * Internal URL of the project, invalid if no project loaded
     */
    KURL m_projectURL;

    /// our project, @see Project
    Project m_project;

    int mainGroup;
    /// IDs for the sidebar buttons
    int infoPage, ganttPage, resPage, tasksPage;
    /// sidebar
    KoKoolBar * m_koolBar;

    /// stacked widget on the right side
    QWidgetStack * m_widgetStack;

    /// gantt component
    KDGanttView * m_ganttView;
    /// project info view
    QTextBrowser * m_textBrowser;
    /// resources list view
    KListView * m_resListView;
    /// task (flat) view
    KListView * m_taskView;
};

#endif // _KTJVIEW2VIEW_H_
