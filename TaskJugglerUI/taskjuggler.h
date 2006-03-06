/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */


#ifndef _TASKJUGGLER_H_
#define _TASKJUGGLER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <set>

#include <kapplication.h>
#include <kmainwindow.h>

#include "taskjugglerview.h"
#include "ltstr.h"

class QTimer;
class KToggleAction;
class KRecentFilesAction;
class KURL;

/**
 * This class serves as the main window for TaskJuggler.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Chris Schlaeger <cs@kde.org>
 */
class TaskJuggler : public KMainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    TaskJuggler();

    /**
     * Default Destructor
     */
    virtual ~TaskJuggler();

    void enableActions(bool enable);

    void loadLastProject();

public slots:
    /**
     * Use this method to load whatever file/URL you have
     */
    void load(const KURL& url);
    void changeCaption(const QString& text);

protected:
    /**
     * Overridden virtuals for Qt drag 'n drop (XDND)
     */
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual bool queryClose();
    virtual bool queryExit();

protected slots:
    /**
     * This function is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfig *);

    /**
     * This function is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(KConfig *);


private slots:

    void fileNew();
    void fileNewInclude();
    void fileOpen();
    void fileClose();
    void filePrint();
    void optionsPreferences();
    void showTipOnStart();
    void showTip();

    void changeStatusbar(const QString& text);
    void addRecentURL(const KURL& text);

private:
    void setupAccel();
    void setupActions();

    TaskJugglerView* m_view;

    KRecentFilesAction* m_recentAction;

    QTimer* delayTimer;
    std::set<const char*, ltstr> enabledActionsBuf;
    QString lastURL;
};

#endif // _TASKJUGGLER_H_
