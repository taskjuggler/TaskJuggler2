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
#include "kdgantt/KDGanttView.h"

#include "Project.h"

class QPainter;
class KDGanttView;
class KDGanttViewItem;
class QWidgetStack;
class QTextBrowser;
class KListView;
class KListViewItem;
class KPrinter;
class TaskList;
class TaskItem;
class QPopupMenu;
class ResUsageView;
class EditorView;

enum { ID_VIEW_INFO = 0, ID_VIEW_GANTT, ID_VIEW_RESOURCES, ID_VIEW_TASKS,
       ID_VIEW_RES_USAGE, ID_VIEW_EDITOR };

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
    bool openURL( const KURL& url );

    /**
     * Invokes the print dialog and prints the view
     */
    void print( KPrinter * printer );

    /**
     * @return the listview that displays the tasks
     */
    KListView * taskListView() const
        { return m_taskView; }

    /**
     * @return the listview that displays the resources
     */
    KListView * resListView() const
        { return m_resListView; }

    /**
     * @return the EditorView component
     */
    EditorView * editor() const
        { return m_editorView; }

    /**
     * Apply preselected filter (@p id) on the task list.
     *
     * @return whether the filter was actually applied
     */
    bool filterForTasks( int id );

    /**
     * Apply preselected filter (@p id) on the resource list.
     *
     * @return whether the filter was actually applied
     */
    bool filterForResources( int id );

    /**
     * Raise the stacked view widget with @p id
     */
    void activateView( int id );

public slots:
    void slotUndo();

    void slotRedo();

    /**
     * Cut the selected text to cliboard (Editor slot)
     */
    void slotCut();

    /**
     * Copy the selected text to cliboard (Editor slot)
     */
    void slotCopy();

    /**
     * Paste the selected text from cliboard (Editor slot)
     */
    void slotPaste();

    /**
     * Set the Gantt to calendar mode depending on @p flag
     */
    void setCalendarMode( bool flag );

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
     * Select a scale for the gantt view
     * @param scale the requested scale
     * @see KDGanttView::Scale
     */
    void slotScale( int scale );

    /**
     * Select a scale for the resource usage view
     * @param scale the requested scale
     * @see ResUsageView::Scale
     */
    void slotResScale( int scale );

    /**
     * Select what data to display in the resource usage view
     * @see ResUsageView::DisplayData
     */
    void slotResDisplay( int display );

    /**
     * Invoke the Find dialog for the RU view
     */
    void slotRUFind();

    /**
     * Zooms so that at least the selected time period is visible after the zoom.
     *
     * @param from the start date
     * @param to the end date
     */
    void slotZoomTimeframe();

    /**
     * Invoked when the config dialog settings change
     */
    void loadSettings();

    /**
     * Display the filter selection dialog
     */
    void filter();

    /**
     * Expand (open) the listview (@p view) items up to @p level
     * (99 expands all)
     */
    void expandToLevel( KListView * view, int level = 99 );

    /**
     * Collapse (close) all the listview items
     */
    void collapseAll( KListView * view );

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar( const QString& text );

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption( const QString& text );

    /**
     * Signal the main window that we want to switch views programatically
     */
    void signalSwitchView( int type );

    /**
     * Emitted when the Gantt component changes the scale
     */
    void signalScaleChanged( int );

private slots:
    /**
     * Centers the gantt chart on the selected @p item
     */
    void ensureItemVisible( KDGanttViewItem * item );

    /**
     * Popup a context menu over @p item, at global position @p pos, column @p col
     */
    void popupGanttItemMenu( KDGanttViewItem * item, const QPoint & pos, int col );

    /**
     * Jump from the selected Gantt item into the detailed tasklist item
     */
    void slotJumpToTask();

    /**
     * Emits a signal for the mainwindow that the Gantt scale changed
     *
     * @param scale The new gantt scale
     */
    void slotScaleChanged( KDGanttView::Scale scale );

private:
    void recreateProject();

    /**
     * Apply settings dialog changes for the Gantt
     */
    void setupGantt();

    /**
     * Clear all view components, e.g. before loading a new project
     */
    void clearAllViews();

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
     * @param it iterator over the list of tasks
     * @param sc index of the scenario
     */
    void parseTasks( TaskListIterator it, int sc = 0 );

    /**
     * Parse the task list and build the gantt chart
     * @param it iterator over the list of tasks
     * @param sc index of the scenario
     */
    void parseGantt( TaskListIterator it, int sc = 0 );

    void parseResUsage();

    /**
     * @return a comma separated list of resources responsible for @p task
     */
    QString formatAllocations( Task* task ) const;

    /**
     * @return a tooltip text for a tasklink (separated by commas)
     * @param from list of link start points
     * @param to list of link end points
     */
    QString formatLinks( const QPtrList<KDGanttViewItem> & from, const QPtrList<KDGanttViewItem> & to ) const;

    /**
     * Parse the resources list and fill the list view
     * @param it iterator over the list of resources
     * @param parentItem the parent item in the list view to append this item to
     */
    void parseResources( ResourceListIterator it, KListViewItem * parentItem = 0 );

    /**
     * Parses relations between tasks (dependencies)
     * @param it iterator over the list of tasks
     */
    void parseLinks( TaskListIterator it );

    /**
     * Converts a time_t representation into QDateTime
     *
     * @param secs number of seconds since 1970-01-01, UTC
     */
    QDateTime time_t2Q( time_t secs ) const;

    /**
     * Converts a time_t representation to human readable, locale aware string
     *
     * @param secs number of seconds since 1970-01-01, UTC
     */
    QString time_t2QS( time_t secs ) const;

    /**
     * Internal URL of the project, invalid if no project loaded
     */
    KURL m_projectURL;

    /// our project, @see Project
    Project * m_project;

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
    /// resource usage view
    ResUsageView * m_resUsageView;
    /// editor view
    EditorView * m_editorView;

    /// gantt listview item popup menu
    QPopupMenu * m_ganttPopupMenu;
};

#endif // _KTJVIEW2VIEW_H_
