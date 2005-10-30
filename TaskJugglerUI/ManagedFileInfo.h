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

#include <kate/view.h>
#include <kate/document.h>
#include <kurl.h>

class KConfig;
class KListViewItem;
class FileManager;

class ManagedFileInfo : public QObject
{
    Q_OBJECT
public:
    ManagedFileInfo(FileManager* fm, const KURL& url);
    ~ManagedFileInfo();

    void readProperties(KConfig* config);

    void writeProperties(KConfig* config);

    const KURL& getFileURL() const { return fileURL; }

    const QString getUniqueName() const;

    bool isModified() { return modified; }

    void save(bool ask);

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
    void setModifiedOnDisc(Kate::Document*, bool modified,
                           unsigned char reason);

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

