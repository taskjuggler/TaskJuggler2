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

#include <qwidgetstack.h>
#include <qstring.h>

#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/editinterface.h>

#include <kdebug.h>

#include "FileManager.h"
#include "ManagedFileInfo.h"

FileManager::FileManager(QWidgetStack* v, KListView* b) :
    viewStack(v), browser(b)
{
    files.setAutoDelete(TRUE);
    masterFile = 0;

    // We don't want the URL column to be visible. This is internal data only.
    browser->setColumnWidthMode(1, QListView::Manual);
    browser->hideColumn(1);
}

void
FileManager::updateFileList(const QStringList& fl, const KURL& url)
{
    if (fl.isEmpty())
        return;

    // First we mark all files as no longer part of the project.
    for (QPtrListIterator<ManagedFileInfo> fli(files); *fli; ++fli)
        (*fli)->setPartOfProject(FALSE);

    /* Then we add the new ones and mark the existing ones as part of the
     * project again. */
    for (QStringList::ConstIterator sli = fl.begin(); sli != fl.end(); ++sli)
    {
        KURL url;
        url.setPath(*sli);

        bool newFile = TRUE;
        // Is this file being managed already?
        for (QPtrListIterator<ManagedFileInfo> mfi(files); *mfi; ++mfi)
        {
            if ((*mfi)->getFileURL() == url)
            {
                (*mfi)->setPartOfProject(TRUE);
                newFile = FALSE;
                break;
            }
        }

        if (newFile)
        {
            // No, so let's add it to our internal list.
            ManagedFileInfo* mfi = new ManagedFileInfo(this, KURL(*sli));
            mfi->setPartOfProject(TRUE);
            files.append(mfi);
        }
    }
    setMasterFile(url);

    updateFileBrowser();

    if (browser->currentItem())
        showInEditor(getCurrentFileURL());
    else
        showInEditor(files.at(0)->getFileURL());
}

QString
FileManager::findCommonPath()
{
    if (files.isEmpty())
        return QString::null;

    int lastMatch = 0;
    int end;
    QString firstURL = files.at(0)->getFileURL().url();
    while ((end = firstURL.find("/", lastMatch)) >= 0)
    {
        for (QPtrListIterator<ManagedFileInfo> mfi(files); *mfi; ++mfi)
        {
            QString url = (*mfi)->getFileURL().url();
            if (url.left(end) != firstURL.left(end))
                goto done;
        }
        lastMatch = end + 1;
    }
done:
    return firstURL.left(lastMatch);
}

void
FileManager::updateFileBrowser()
{
    QString commonPath = findCommonPath();

    // Remove all entries from file browser
    browser->clear();

    for (QPtrListIterator<ManagedFileInfo> mfi(files); *mfi; ++mfi)
    {
        /* Now we are inserting the file into the file browser again.
         * We don't care about the common path and create tree nodes for each
         * remaining directory. So we can browse the files in a directory like
         * tree. */
        QString url = (*mfi)->getFileURL().url();
        // Remove common path from URL.
        QString shortenedURL = url.right(url.length() -
                                         commonPath.length());
        KListViewItem* currentDir = 0;
        int start = 0;
        /* The remaining file name is traversed directory by directory. If
         * there is no node yet for this directory in the browser, we create a
         * directory node. */
        for (int dirNameEnd = -1;
             (dirNameEnd = shortenedURL.find("/", start)) > 0;
             start = dirNameEnd + 1)
        {
            KListViewItem* lvi;
            if ((lvi = static_cast<KListViewItem*>
                 (browser->findItem(shortenedURL.left(dirNameEnd), 1))) == 0)
            {
                if (!currentDir)
                    currentDir =
                        new KListViewItem(browser,
                                          shortenedURL.left(dirNameEnd),
                                          shortenedURL.left(dirNameEnd));
                else
                    currentDir =
                        new KListViewItem(currentDir,
                                          shortenedURL.mid(start, dirNameEnd -
                                                           start),
                                          shortenedURL.left(dirNameEnd));
                currentDir->setPixmap
                    (0, KGlobal::iconLoader()->loadIcon("folder",
                                                        KIcon::Small));
            }
            else
                currentDir = lvi;
        }
        KListViewItem* newFile;
        if (currentDir)
            newFile =
                new KListViewItem(currentDir,
                                  shortenedURL.right(shortenedURL.length() -
                                                 start), url);
        else
            newFile =
                new KListViewItem(browser,
                                  shortenedURL.right(shortenedURL.length() -
                                                     start), url);

        // Save the pointer to the browser entry.
        (*mfi)->setBrowserEntry(newFile);

        /* Decorate files with a file icon. The master file will be decorated
         * differently so it can be easyly identified. */
        if (*mfi == masterFile)
            newFile->setPixmap
                (0, KGlobal::iconLoader()->
                 loadIcon("tj_file_tjp", KIcon::Small));
        else
            newFile->setPixmap
                (0, KGlobal::iconLoader()->
                 loadIcon("tj_file_tji", KIcon::Small));

        /* The 3rd column shows whether the file is part of the current
         * project or not. So we need to set the proper icons. */
        if ((*mfi)->isPartOfProject())
            newFile->setPixmap
                (3, KGlobal::iconLoader()->loadIcon("tj_ok", KIcon::Small));
        else
            newFile->setPixmap
                (3, KGlobal::iconLoader()->loadIcon("tj_not_ok", KIcon::Small));
    }
}

void
FileManager::setMasterFile(const KURL& url)
{
    for (QPtrListIterator<ManagedFileInfo> mfi(files); *mfi; ++mfi)
        if ((*mfi)->getFileURL() == url)
        {
            masterFile = *mfi;
            return;
        }
    kdFatal() << "Master file not in list of managed files" << endl;
}

const KURL&
FileManager::getMasterFile() const
{
    static KURL dummy;
    if (!masterFile)
        return dummy;
    else
        return masterFile->getFileURL();
}

bool
FileManager::isProjectLoaded() const
{
    return masterFile != 0;
}

void
FileManager::showInEditor(const KURL& url)
{
    for (QPtrListIterator<ManagedFileInfo> mfi(files); *mfi; ++mfi)
        if ((*mfi)->getFileURL() == url)
        {
            if (!(*mfi)->getEditor())
            {
                // The file has not yet been loaded, so we create an editor for
                // it.
                KTextEditor::Document* document;
                if (!(document = KTextEditor::EditorChooser::createDocument
                      (viewStack, "KTextEditor::Document", "Editor")))
                {
                    KMessageBox::error
                        (viewStack,
                         i18n("A KDE text-editor component could not "
                              "be found;\nplease check your KDE "
                              "installation."));
                }

                KTextEditor::View* editor =
                    document->createView(viewStack);
                viewStack->addWidget(editor);
                (*mfi)->setEditor(editor);
                editor->setMinimumSize(400, 200);
                editor->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,
                                                  QSizePolicy::Maximum, 0, 85,
                                                  editor->sizePolicy()
                                                  .hasHeightForWidth()));
                document->openURL(url);
                document->setReadWrite(TRUE);
                document->setModified(FALSE);

                connect(document, SIGNAL(textChanged()),
                        *mfi, SLOT(setModified()));
            }
            viewStack->raiseWidget((*mfi)->getEditor());

            browser->setSelected((*mfi)->getBrowserEntry(), TRUE);

            break;
        }
}

void
FileManager::showInEditor(const KURL& url, int line, int col)
{
    showInEditor(url);
    setCursorPosition(line, col);
    setFocusToEditor();
}

KURL
FileManager::getCurrentFileURL() const
{
    return KURL(browser->currentItem()->text(1));
}

ManagedFileInfo*
FileManager::getCurrentFile() const
{
    if (files.isEmpty())
        return 0;

    KURL url = getCurrentFileURL();
    for (QPtrListIterator<ManagedFileInfo> mfi(files); *mfi; ++mfi)
        if ((*mfi)->getFileURL() == url)
            return *mfi;

    return 0;
}

QString
FileManager::getWordUnderCursor() const
{
    static QString dummy;

    ManagedFileInfo* mfi = getCurrentFile();
    if (!mfi)
        return dummy;

    return mfi->getWordUnderCursor();
}

void
FileManager::saveCurrentFile()
{
    if (getCurrentFile())
        getCurrentFile()->save();
}

void
FileManager::saveCurrentFileAs(const KURL& url)
{
    if (getCurrentFile())
    {
        getCurrentFile()->saveAs(url);
        updateFileBrowser();
    }
}

void
FileManager::closeCurrentFile()
{
    ManagedFileInfo* mfi;
    if ((mfi = getCurrentFile()) != 0)
    {
        viewStack->removeWidget(mfi->getEditor());
        viewStack->raiseWidget(0);
        files.removeRef(mfi);
        updateFileBrowser();
        if (masterFile == mfi)
            masterFile = 0;
    }
}

void
FileManager::saveAllFiles()
{
    for (QPtrListIterator<ManagedFileInfo> mfi(files); *mfi; ++mfi)
        (*mfi)->save();
}

void
FileManager::clear()
{
    masterFile = 0;
    files.clear();
}

void
FileManager::setCursorPosition(int line, int col)
{
    if (getCurrentFile())
        KTextEditor::viewCursorInterface(getCurrentFile()->getEditor())->
            setCursorPosition(line, col);
}

void
FileManager::setFocusToEditor() const
{
    if (getCurrentFile())
        getCurrentFile()->getEditor()->setFocus();
}

