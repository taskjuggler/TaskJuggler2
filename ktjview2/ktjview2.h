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

#ifndef _KTJVIEW2_H_
#define _KTJVIEW2_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kmainwindow.h>
#include <kactionclasses.h>

#include "ktjview2view.h"

class QLabel;
class KURL;
class KListViewSearchLine;

/**
 * This class serves as the main window for ktjview2.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class ktjview2 : public KMainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    ktjview2();

    /**
     * Default Destructor
     */
    virtual ~ktjview2();

public slots:
    /**
     * Load a project from @p url
     */
    void load( const KURL& url );

protected:
    /**
     * Overridden virtuals for Qt drag 'n drop (XDND)
     */
    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void dropEvent( QDropEvent *event );
    virtual bool queryExit();

    /**
     * This function is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties( KConfig * );

    /**
     * This function is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties( KConfig * );

private slots:
    /**
     * Open a new blank window
     */
    void fileNew();

    /**
     * Open a file
     */
    void fileOpen();

    /**
     * Tell the view to print
     */
    void filePrint();

    /**
     * Toggle the gantt view calendar mode status
     */
    void setCalendarMode();

    /**
     * Show the config keys dialog
     */
    void optionsConfigureKeys();

    /**
     * Show the config toolbars dialog
     */
    void optionsConfigureToolbars();

    /**
     * Show the config dialog
     */
    void optionsPreferences();

    /**
     * Invoked when the toolbar config changes
     */
    void newToolbarConfig();

    /**
     * Change status bar text to @p text
     */
    void changeStatusbar( const QString& text );

    /**
     * Apply preselected filters on the task list
     */
    void slotFilterFor();

    void slotSidebarInfo();
    void slotSidebarGantt();
    void slotSidebarResources();
    void slotSidebarTasks();

    /**
     * Activate the sidebar actions, switch the view
     */
    void slotSwitchView( int type );

private:
    /**
     * Create actions
     */
    void setupActions();
    /**
     * Setup the sidebar attributes
     */
    void setupSidebar();



    ktjview2View *m_view;

    // actions
    KRecentFilesAction *m_recentAction;
    KSelectAction * m_scaleAction;
    KToggleAction * m_calendarAction;

    // filter stuff
    KSelectAction * m_filterForAction;
    QStringList m_filterItems;
    int m_activeFilter;

    // search line
    QLabel * m_searchLabel;
    KListViewSearchLine * m_searchLine;

    // sidebar actions
    KRadioAction * m_sidebarInfo, * m_sidebarGantt, * m_sidebarResources, * m_sidebarTasks;
};

#endif // _KTJVIEW2_H_
