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

#include "ManagedFileInfo.h"

#include <klistview.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <ktexteditor/document.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/editinterface.h>

ManagedFileInfo::ManagedFileInfo(FileManager* fm, const KURL& url) :
    manager(fm), fileURL(url)
{
    editor = 0;
    modified = FALSE;
    browserEntry = 0;
}

ManagedFileInfo::~ManagedFileInfo()
{
    if (editor)
    {
        KTextEditor::Document* doc = editor->document();
        delete editor;
        delete doc;
    }
}

void
ManagedFileInfo::readProperties(KConfig*)
{
}

void
ManagedFileInfo::writeProperties(KConfig*)
{
}

const QString
ManagedFileInfo::getUniqueName() const
{
    /* Return the file name that we use in the file browser. It may contain
     * path fragments to make it unique. */
    return browserEntry->text(0);
}

void
ManagedFileInfo::setModified()
{
    modified = TRUE;
    browserEntry->setPixmap(2, KGlobal::iconLoader()->loadIcon("filesaveas",
                                                               KIcon::Small));
}

QString
ManagedFileInfo::getWordUnderCursor() const
{
    unsigned int line, row;
    KTextEditor::viewCursorInterface(editor)->cursorPositionReal(&line, &row);
    QString textLine = KTextEditor::editInterface
        (editor->document())->textLine(line);
    uint start, length;
    for (start= row; textLine[start - 1].isLetter(); --start)
        ;
    for (length = 0; textLine[start + length].isLetter(); ++length)
        ;

    return textLine.mid(start, length);
}

void
ManagedFileInfo::save()
{
    if (modified && editor)
    {
        editor->document()->save();
        modified = FALSE;
        browserEntry->setPixmap(2, 0);
    }
}

void
ManagedFileInfo::saveAs(const KURL& url)
{
    if (editor)
    {
        editor->document()->saveAs(url);
        modified = FALSE;
        browserEntry->setPixmap(2, 0);
    }
    fileURL = url;
}

#include "ManagedFileInfo.moc"
