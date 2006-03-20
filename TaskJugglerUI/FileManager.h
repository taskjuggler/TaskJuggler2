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

#ifndef _FileManager_h_
#define _FileManager_h_

#include <list>

#include <qobject.h>

#include "ManagedFileInfo.h"

class QWidgetStack;
class QStringList;

class KConfig;
class KMainWindow;
class KListView;
class KListViewSearchLine;
class KURL;
class CoreAttributes;
class Report;

class FileManager : public QObject
{
    Q_OBJECT
public:
    FileManager(KMainWindow* m, QWidgetStack* v, KListView* b,
                KListViewSearchLine* s);
    virtual ~FileManager();

    KMainWindow* getMainWindow() const { return mainWindow; }

    void updateFileList(const QStringList& fl, const KURL& mf);

    void addFile(const KURL& url);

    void addFile(const KURL& nf, const KURL& nnf);

    void setMasterFile(const KURL& url);

    KURL getCurrentFileURL() const;
    ManagedFileInfo* getCurrentFile() const;

    void readProperties(KConfig* config);

    void writeProperties(KConfig* config);

    QString getWordUnderCursor() const;

    QWidgetStack* getViewStack() const { return viewStack; }

    void showEditor();
    void hideEditor();

    const KURL& getMasterFileURL() const;
    ManagedFileInfo* getMasterFile() { return masterFile; }

    bool isProjectLoaded() const;

    void saveAllFiles(bool ask = false);
    void saveCurrentFile(bool ask = false);
    void expandMacros();
    void clear();

    void print();

    void configureEditor();

public slots:
    void showInEditor(const KURL& url);
    void showInEditor(const KURL& url, int line, int col);
    void showInEditor(CoreAttributes* ca);
    void showInEditor(const Report* report);
    void saveCurrentFileAs(const KURL& url);
    void closeCurrentFile();
    void setCursorPosition(int line, int col);

    void enableEditorActions(bool enable);
    void enableClipboardActions(bool enable = TRUE);
    void enableUndoActions(bool enable = TRUE);

private slots:
    void insertDate();

private:
    FileManager() { }

    KMainWindow* mainWindow;

    KConfig* config;
    bool editorConfigured;

    QString findCommonPath();
    void updateFileBrowser();
    ManagedFileInfo* getMFI(const KURL& url);

    QWidgetStack* viewStack;
    KListView* browser;
    KListViewSearchLine* searchLine;
    std::list<ManagedFileInfo*> files;
    ManagedFileInfo* masterFile;
    KTextEditor::View* currentGUIClient;
} ;

#endif

