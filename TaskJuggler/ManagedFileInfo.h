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

#ifndef _ManagedFileInfo_h_
#define _ManagedFileInfo_h_

#include <qobject.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <kurl.h>

class KListViewItem;
class FileManager;

class ManagedFileInfo : public QObject
{
    Q_OBJECT
public:
    ManagedFileInfo(FileManager* fm, const KURL& url);
    ~ManagedFileInfo();

    const KURL& getFileURL() const { return fileURL; }

    bool isModified() { return modified; }

    void save();

    void saveAs(const KURL& url);

    void setPartOfProject(bool p) { partOfProject = p; }
    bool isPartOfProject() const { return partOfProject; }

    void setEditor(KTextEditor::View* e) { editor = e; }
    KTextEditor::View* getEditor() const { return editor; }

    void setBrowserEntry(KListViewItem* lvi) { browserEntry = lvi; }
    KListViewItem* getBrowserEntry() const { return browserEntry; }

    QString getWordUnderCursor() const;

public slots:
    void setModified();

private:
    ManagedFileInfo() { }

    FileManager* manager;
    KURL fileURL;
    bool partOfProject;
    bool modified;

    KListViewItem* browserEntry;
    KTextEditor::View* editor;
} ;

#endif

