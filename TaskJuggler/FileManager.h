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

#ifndef _FileManager_h_
#define _FileManager_h_

#include <qobject.h>
#include <qptrlist.h>

#include "ManagedFileInfo.h"

class QWidgetStack;
class QStringList;

class KConfig;
class KMainWindow;
class KListView;
class KURL;
class CoreAttributes;
class FindDialog;

class FileManager : public QObject
{
    Q_OBJECT
public:
    FileManager(KMainWindow* m, QWidgetStack* v, KListView* b);
    virtual ~FileManager() { }

    void updateFileList(const QStringList& fl, const KURL& mf);

    void addFile(const KURL& nf, const KURL& nnf);

    KURL getCurrentFileURL() const;
    ManagedFileInfo* getCurrentFile() const;

    void readProperties(KConfig* config);

    void writeProperties(KConfig* config);

    QString getWordUnderCursor() const;

    QWidgetStack* getViewStack() const { return viewStack; }

    void setFocusToEditor() const;

    const KURL& getMasterFile() const;

    bool isProjectLoaded() const;

    void saveAllFiles();
    void clear();

    void find();
    void findNext();
    void findPrevious();

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();

    void configureEditor();

public slots:
    void showInEditor(const KURL& url);
    void showInEditor(const KURL& url, int line, int col);
    void showInEditor(CoreAttributes* ca);
    void saveCurrentFile();
    void saveCurrentFileAs(const KURL& url);
    void closeCurrentFile();
    void setCursorPosition(int line, int col);

    void enableEditorActions(bool enable);
    void enableClipboardActions(bool enable = TRUE);
    void enableUndoActions(bool enable = TRUE);

    void startSearch();

private:
    FileManager() { }

    KMainWindow* mainWindow;

    KConfig* config;
    bool editorConfigured;

    QString findCommonPath();
    void updateFileBrowser();
    void setMasterFile(const KURL& url);

    void search();

    FindDialog* findDialog;
    QString searchPattern;
    uint matchLen;
    bool searchCaseSensitive;
    bool searchWholeWords;
    bool searchFromCursor;
    bool searchBackwards;
    uint lastMatchLine, lastMatchCol;

    QWidgetStack* viewStack;
    KListView* browser;
    QPtrList<ManagedFileInfo> files;
    ManagedFileInfo* masterFile;
} ;

#endif

