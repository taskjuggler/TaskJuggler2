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

#include "ManagedFileInfo.h"

#include <kmainwindow.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kurl.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <ktexteditor/document.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/editinterface.h>
#include <kate/document.h>

#include "FileManager.h"

ManagedFileInfo::ManagedFileInfo(FileManager* fm, const KURL& url) :
    manager(fm), fileURL(url)
{
    editor = 0;
    modified = false;
    browserEntry = 0;
}

ManagedFileInfo::~ManagedFileInfo()
{/*
    if (editor)
    {
        KTextEditor::Document* doc = editor->document();
        delete editor;
        delete doc;
    }*/
}

void
ManagedFileInfo::setEditor(KTextEditor::View* e)
{
    editor = e;
    connect(editor->action("file_save"), SIGNAL(activated()),
            this, SLOT(fileSaved()));
}

void
ManagedFileInfo::readProperties(KConfig*)
{
}

void
ManagedFileInfo::writeProperties(KConfig*)
{
}

const KURL&
ManagedFileInfo::getFileURL() const
{
    return fileURL;
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
    if (browserEntry)
        browserEntry->setPixmap(2, KGlobal::iconLoader()->loadIcon
                                ("filesaveas", KIcon::Small));
}

void
ManagedFileInfo::setModifiedOnDisc(Kate::Document* doc, bool isModified,
                                   unsigned char)
{
    if (!editor || !isModified)
        return;

    doc->reloadFile();
    modified = false;
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
ManagedFileInfo::save(bool ask)
{
    /* For some reason editor->document()->save() triggers a
     * KMainWindow::saveAll() which then ends up calling this function
     * recursively. To avoid this, we use a static flag to detect this
     * condition and avoid the recursive execution. */
    static bool rec = false;

    if (rec)
        return;

    rec = true;
    if (modified && editor)
    {
        if (ask && KMessageBox::warningYesNo
            (manager->getMainWindow(),
             i18n("The file %1 has unsaved modifications.\n"
                  "Do you want to save your modifications?")
             .arg(fileURL.url())) != KMessageBox::Yes)
            return;

        editor->document()->save();
        fileSaved();
    }
    rec = false;
}

void
ManagedFileInfo::saveAs(const KURL& url)
{
    if (editor)
    {
        editor->document()->saveAs(url);
        fileURL = url;
        fileSaved();
    }
}

void
ManagedFileInfo::fileSaved()
{
    modified = false;
    if (browserEntry)
        browserEntry->setPixmap(2, 0);
}

#include "ManagedFileInfo.moc"
