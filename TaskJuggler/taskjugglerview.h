/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */


#ifndef _TASKJUGGLERVIEW_H_
#define _TASKJUGGLERVIEW_H_

#include <qwidget.h>

#include <kparts/part.h>
#include <ktexteditor/view.h>
#include <kurl.h>

#include <taskjuggleriface.h>

class QTimer;
class QPainter;
class QListViewItem;
class QSplitter;
class QProgressBar;
class KURL;
class KListView;
class KConfig;
class MainWidget;
class Project;
class FileManager;
class ReportManager;

/**
 * This is the main view class for TaskJuggler.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * This taskjuggler uses an HTML component as an example.
 *
 * @short Main view
 * @author Chris Schlaeger <cs@suse.de>
 * @version 0.1
 */
class TaskJugglerView : public QWidget, public TaskJugglerIface
{
    Q_OBJECT
public:
	/**
	 * Default constructor
	 */
    TaskJugglerView(QWidget *parent);

	/**
	 * Destructor
	 */
    virtual ~TaskJugglerView();

    /**
     * Random 'get' function
     */
    QString currentURL();

    virtual void newProject(const KURL& url);

    virtual void newInclude(const KURL& url);

    /**
     * Random 'set' function accessed by DCOP
     */
    virtual void openURL(QString url);

    /**
     * Random 'set' function
     */
    virtual void openURL(const KURL& url);

    virtual void save();

    virtual void saveAs(const KURL& url);

    virtual void close();

    /**
     * Print this view to any medium -- paper or not
     */
    void print(QPainter *, int height, int width);

    void readProperties(KConfig* config);
    void saveProperties(KConfig* config);

public slots:
    void schedule();
    void nextProblem();
    void previousProblem();
    void setFocusToTaskList();
    void setFocusToResourceList();
    void setFocusToAccountList();
    void setFocusToReportList();
    void setFocusToFileList();
    void setFocusToEditor();
    void setFocusToReport();
    void zoomIn();
    void zoomOut();
    void changeStatusBar(const QString& text);

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString& text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString& text);
    void announceRecentURL(const KURL& url);

private slots:
    void slotOnURL(const QString& url);
    void loadAfterTimerTimeout();
    void slotSetTitle(const QString& title);
    void addWarningMessage(const QString& msg, const QString& file, int line);
    void addErrorMessage(const QString& msg, const QString& file, int line);
    void focusListViews(int idx);
    void focusBigTab(QWidget* page);
    void taskListClicked(QListViewItem* lvi);
    void resourceListClicked(QListViewItem* lvi);
    void accountListClicked(QListViewItem* lvi);
    void reportListClicked(QListViewItem* lvi);
    void fileListClicked(QListViewItem* lvi);
    void messageListClicked(QListViewItem* lvi);
    void showProgressInfo(const QString& fn);
    void keywordHelp();

private:
    bool loadProject(const KURL& url);

    void setLoadingProject(bool lp);

    void addMessage(const QString& msg, const QString& file, int line,
                    bool error);

    Project* project;
    MainWidget* mw;
    QProgressBar* progressBar;
    KListView* messageListView;
    QSplitter* editorSplitter;

    FileManager* fileManager;
    ReportManager* reportManager;

    int messageCounter;
    QTimer* loadDelayTimer;
    KURL urlToLoad;
    bool loadingProject;
};

#endif // _TASKJUGGLERVIEW_H_
