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

#include <kurl.h>

#include "ktjview2iface.h"

class QPainter;
class KDGanttView;
class KDGanttViewItem;
class KoKoolBar;
class QWidgetStack;
class QTextBrowser;
class QListView;
class QDateTime;
class KPrinter;

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

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString& text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString& text);

protected slots:
    /**
     * Switch between the items in the sidebar
     * @param grp ID of the group
     * @param item ID of the button
     */
    void slotKoolBar( int grp, int item );

private slots:
    /**
     * Centers the gantt chart on the selected item
     */
    void ensureItemVisible( KDGanttViewItem * item );

private:
    /**
     * Parse <project>
     */
    void parseProjectInfo( QDomNode node, QTextBrowser * view );

    /**
     * Parse children of <taskList>
     */
    void parseTasks( QDomNode node, KDGanttViewItem * parent = 0 );
    /**
     * Parse an individual <task> element
     */
    void parseTask( const QDomElement & taskElem, KDGanttViewItem * parent = 0);

    /**
     * Parse children of <resourceList>
     */
    void parseResources( QDomNode node, QListView * view, const QString & group = QString::null );
    /**
     * Parse an individual <resource> element
     */
    void parseResource( const QDomElement & resElem, QListView * view, const QString & group = QString::null );

    /**
     * Parses relations between tasks (dependencies)
     */
    void parseLinks( const QDomElement & taskList, KDGanttView * view );

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

    /**
     * DOM tree of the project
     */
    QDomDocument m_dom;

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
    QListView * m_resListView;
    /// task (flat) view
    QListView * m_taskView;
};

#endif // _KTJVIEW2VIEW_H_
