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
class KListView;
class KURL;

class FileManager : public QObject
{
    Q_OBJECT
public:
    FileManager(QWidgetStack* v, KListView* b);
    virtual ~FileManager() { }

    void updateFileList(const QStringList& fl, const KURL& mf);

    KURL getCurrentFileURL() const;
    ManagedFileInfo* getCurrentFile() const;

    QString getWordUnderCursor() const;

    QWidgetStack* getViewStack() const { return viewStack; }

    void setFocusToEditor() const;

    const KURL& getMasterFile() const;

    bool isProjectLoaded() const;

    void saveAllFiles();
    void clear();

public slots:
    void showInEditor(const KURL& url);
    void showInEditor(const KURL& url, int line, int col);
    void saveCurrentFile();
    void saveCurrentFileAs(const KURL& url);
    void closeCurrentFile();
    void setCursorPosition(int line, int col);

private:
    FileManager() { }

    QString findCommonPath();
    void updateFileBrowser();
    void setMasterFile(const KURL& url);

    QWidgetStack* viewStack;
    KListView* browser;
    QPtrList<ManagedFileInfo> files;
    ManagedFileInfo* masterFile;
} ;

#endif

