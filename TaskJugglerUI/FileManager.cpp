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

#include <assert.h>
#include <time.h>

#include <qwidgetstack.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qregexp.h>

#include <kmainwindow.h>
#include <kstatusbar.h>
#include <klistview.h>
#include <klistviewsearchline.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kstdaction.h>
#include <kaction.h>
#include <klineedit.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/clipboardinterface.h>
#include <ktexteditor/undointerface.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/printinterface.h>
#include <kdatepicker.h>
#include <kcombobox.h>

#include "Utility.h"
#include "CoreAttributes.h"
#include "Report.h"
#include "FileManager.h"
#include "ManagedFileInfo.h"
#include "TjDatePicker.h"

FileManager::FileManager(KMainWindow* m, QWidgetStack* v, KListView* b,
                         KListViewSearchLine* s) :
    mainWindow(m), viewStack(v), browser(b), searchLine(s)
{
    masterFile = 0;
    editorConfigured = false;

    /* Add our own custom editor actions. */
    new KAction(i18n("Insert Date"), "",
                KShortcut(KKey("CTRL+d")),
                this, SLOT(insertDate()),
                mainWindow->actionCollection(), "insert_date");

    // We don't want the URL column to be visible. This is internal data only.
    browser->setColumnWidthMode(1, QListView::Manual);
    browser->hideColumn(1);

    currentGUIClient = 0;
}

FileManager::~FileManager()
{
    clear();
}

void
FileManager::updateFileList(const QStringList& fl, const KURL& url)
{
    if (fl.isEmpty())
        return;

    // First we mark all files as no longer part of the project.
    for (std::list<ManagedFileInfo*>::iterator fli = files.begin();
         fli != files.end(); ++fli)
        (*fli)->setPartOfProject(false);

    /* Then we add the new ones and mark the existing ones as part of the
     * project again. */
    for (QStringList::ConstIterator sli = fl.begin(); sli != fl.end(); ++sli)
    {
        KURL url;
        url.setPath(*sli);

        bool newFile = true;
        // Is this file being managed already?
        for (std::list<ManagedFileInfo*>::iterator mfi = files.begin();
             mfi != files.end(); ++mfi)
        {
            if ((*mfi)->getFileURL() == url)
            {
                (*mfi)->setPartOfProject(true);
                newFile = false;
                break;
            }
        }

        if (newFile)
        {
            // No, so let's add it to our internal list.
            ManagedFileInfo* mfi =
                new ManagedFileInfo(this, KURL::fromPathOrURL(*sli));
            mfi->setPartOfProject(true);
            files.push_back(mfi);
        }
    }
    setMasterFile(url);

    updateFileBrowser();

    if (browser->currentItem())
        showInEditor(getCurrentFileURL());
    else
        showInEditor(files.front()->getFileURL());
}

void
FileManager::addFile(const KURL& url)
{
    if (getMFI(url) == 0)
    {
        // Add new file to list of managed files.
        ManagedFileInfo* mfi = new ManagedFileInfo(this, url);
        files.push_back(mfi);

        // Insert the file into the browser and update the directory hierachy if
        // necessary.
        updateFileBrowser();
        QListViewItem* lvi = mfi->getBrowserEntry();
        browser->clearSelection();
        browser->setCurrentItem(lvi);
        lvi->setSelected(true);
        browser->ensureItemVisible(lvi);
    }

    // Open new file in editor.
    showInEditor(url);
}

void
FileManager::addFile(const KURL& url, const KURL& newURL)
{
    if (getMFI(newURL) == 0)
    {
        // Add new file to list of managed files.
        ManagedFileInfo* mfi = new ManagedFileInfo(this, url);
        files.push_back(mfi);
        // First show the file with the old name so it get's loaded.
        showInEditor(url);
        mfi->saveAs(newURL);

        // Insert the file into the browser and update the directory hierachy if
        // necessary.
        updateFileBrowser();
        QListViewItem* lvi = mfi->getBrowserEntry();
        browser->clearSelection();
        browser->setCurrentItem(lvi);
        lvi->setSelected(true);
        browser->ensureItemVisible(lvi);
    }

    // Open new file in editor.
    showInEditor(newURL);
}

QString
FileManager::findCommonPath()
{
    if (files.empty())
        return QString::null;

    int lastMatch = 0;
    int end;
    QString firstURL = files.front()->getFileURL().url();
    while ((end = firstURL.find("/", lastMatch)) >= 0)
    {
        for (std::list<ManagedFileInfo*>::iterator mfi = files.begin();
             mfi != files.end(); ++mfi)
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

    QStringList openDirectories, closedDirectories;
    for (QListViewItemIterator lvi(browser); *lvi; ++lvi)
        if ((*lvi)->firstChild())
            if ((*lvi)->isOpen())
                openDirectories.append((*lvi)->text(1));
            else
                closedDirectories.append((*lvi)->text(1));
    QString currentFile;
    if (browser->currentItem())
        currentFile = browser->currentItem()->text(1);

    // Remove all entries from file browser
    browser->clear();

    for (std::list<ManagedFileInfo*>::iterator mfi = files.begin();
         mfi != files.end(); ++mfi)
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
            // Ignore multiple slahes
            if (dirNameEnd == start + 1)
                continue;

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

            if (openDirectories.find(currentDir->text(1)) !=
                openDirectories.end())
                currentDir->setOpen(true);
            else if (closedDirectories.find(currentDir->text(1)) !=
                     closedDirectories.end())
                currentDir->setOpen(false);
            else
                currentDir->setOpen(true);
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

        if (newFile->text(1) == currentFile)
            browser->setCurrentItem(newFile);

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

    searchLine->updateSearch();
}

void
FileManager::setMasterFile(const KURL& url)
{
    for (std::list<ManagedFileInfo*>::iterator mfi = files.begin();
         mfi != files.end(); ++mfi)
        if ((*mfi)->getFileURL() == url)
        {
            masterFile = *mfi;
            return;
        }
    // Master file must be in list of managed files.
    assert(0);
}

const KURL&
FileManager::getMasterFileURL() const
{
    static KURL dummy;
    return masterFile ? masterFile->getFileURL() : dummy;
}

bool
FileManager::isProjectLoaded() const
{
    return masterFile != 0;
}

void
FileManager::showInEditor(const KURL& url)
{
    for (std::list<ManagedFileInfo*>::iterator mfi = files.begin();
         mfi != files.end(); ++mfi)
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
                              "be found; please check your KDE "
                              "installation."));
                    return;
                }
                if (!editorConfigured)
                {
                    KTextEditor::configInterface(document)->readConfig(config);
                    editorConfigured = true;
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
                document->setReadWrite(true);
                document->setModified(false);

                // Signal to update the file-modified status
                connect(document, SIGNAL(textChanged()),
                        *mfi, SLOT(setModified()));

                connect(document,
                        SIGNAL(modifiedOnDisc(Kate::Document*, bool,
                                              unsigned char)),
                        *mfi,
                        SLOT(setModifiedOnDisc(Kate::Document*, bool,
                                               unsigned char)));

                /* Remove some actions of the editor that we don't want to
                 * show in the menu/toolbars */
                KActionCollection* ac = editor->actionCollection();
                ac->remove(ac->action("file_print"));
                ac->action("view_folding_markers")->setShortcut(KShortcut());
                ac->action("view_border")->setShortcut(KShortcut());
                ac->action("view_line_numbers")->setShortcut(KShortcut());
                ac->action("view_dynamic_word_wrap")->setShortcut(KShortcut());

/*                KActionPtrList actionList =
                    editor->actionCollection()->actions();
                for (KActionPtrList::iterator it = actionList.begin();
                     it != actionList.end(); ++it)
                {
                    printf("** Action found: %s\n", (*it)->name());
                }*/
            }
            viewStack->raiseWidget((*mfi)->getEditor());

            browser->clearSelection();
            QListViewItem* lvi = (*mfi)->getBrowserEntry();
            if (lvi)
            {
                browser->setCurrentItem(lvi);
                lvi->setSelected(true);
            }

            break;
        }
}

void
FileManager::showInEditor(const KURL& url, int line, int col)
{
    showInEditor(url);
    setCursorPosition(line, col);
    showEditor();
}

void
FileManager::showInEditor(CoreAttributes* ca)
{
    KURL url = KURL(ca->getDefinitionFile());
    int line, col;
    line = ca->getDefinitionLine();
    col = 0;
    showInEditor(url, line, col);
}

void
FileManager::showInEditor(const Report* report)
{
    KURL url = KURL(report->getDefinitionFile());
    int line, col;
    line = report->getDefinitionLine();
    col = 0;
    showInEditor(url, line, col);
}

KURL
FileManager::getCurrentFileURL() const
{
    return files.empty() || browser->currentItem() == 0 ? KURL() :
        KURL::fromPathOrURL(browser->currentItem()->text(1));
}

void
FileManager::readProperties(KConfig* cfg)
{
    // We can't do much here for now. As soon as we have the first editor
    // part, we use the config pointer to read the editor properties. So we
    // only store the pointer right now.
    config = cfg;
}

void
FileManager::writeProperties(KConfig* config)
{
    if (getCurrentFile())
        KTextEditor::configInterface(getCurrentFile()->getEditor()->
                                     document())->writeConfig(config);

    for (std::list<ManagedFileInfo*>::iterator mfi = files.begin();
         mfi != files.end(); ++mfi)
        (*mfi)->writeProperties(config);
}

ManagedFileInfo*
FileManager::getCurrentFile() const
{
    if (files.empty())
        return 0;

    KURL url = getCurrentFileURL();
    for (std::list<ManagedFileInfo*>::const_iterator mfi = files.begin();
         mfi != files.end(); ++mfi)
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
FileManager::saveCurrentFile(bool ask)
{
    if (getCurrentFile())
        getCurrentFile()->save(ask);
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

        for (std::list<ManagedFileInfo*>::iterator mfit = files.begin();
             mfit != files.end(); ++mfit)
            if (*mfit == mfi)
            {
                if (masterFile == mfi)
                    masterFile = 0;

                delete mfi;
                mfi = 0;
                files.erase(mfit);
                break;
            }
        assert(mfi == 0);

        updateFileBrowser();
    }
}

void
FileManager::saveAllFiles(bool ask)
{
    for (std::list<ManagedFileInfo*>::iterator mfi = files.begin();
         mfi != files.end(); ++mfi)
        (*mfi)->save(ask);
}

void
FileManager::expandMacros()
{
    QMap<QString, QString> map;
    map["projectstart"] = time2user(time(0), "%Y-%m-%d");
    map["projectend"] = time2user(time(0) + 60 * 60 * 24 * 180, "%Y-%m-%d");

    if (getCurrentFile())
    {
        KTextEditor::EditInterface* ei =
            KTextEditor::editInterface(getCurrentFile()->getEditor()->
                                       document());
        for (unsigned int i = 0; i < ei->numLines(); ++i)
        {
            QString line = ei->textLine(i);
            if (line.find("@@"))
            {
                QMap<QString, QString>::Iterator it;
                for (it = map.begin(); it != map.end(); ++it)
                    line.replace(QString("@@") + it.key() + "@@", it.data());
                ei->removeLine(i);
                ei->insertLine(i, line);
            }
        }
    }
}

void
FileManager::clear()
{
    for (std::list<ManagedFileInfo*>::iterator it = files.begin();
         it != files.end(); ++it)
        delete *it;
    files.clear();
    masterFile = 0;
    currentGUIClient = 0;
}

void
FileManager::insertDate()
{
    if (!getCurrentFile())
        return;

    // Create some shortcuts for the edit and cursor interface.
    KTextEditor::EditInterface* ei =
        KTextEditor::editInterface(getCurrentFile()->getEditor()->
                                   document());
    KTextEditor::ViewCursorInterface* ci =
        KTextEditor::viewCursorInterface(getCurrentFile()->getEditor());
    // Save current cursor position
    unsigned int l, c;
    ci->cursorPosition(&l, &c);

    // Get current line.
    QString line = ei->textLine(l);
    // Find the word under the cursor and save it.
    int cStart;
    unsigned int cEnd;
    for (cStart = c; cStart >= 0 && line[cStart] != ' '; --cStart)
        ;
    /* Make sure cStart points to the first character of the word or keep the
     * current cursor position in case there is no word. */
    if (cStart < 0 || line[cStart] == ' ')
        cStart++;
    for (cEnd = cStart + 1; cEnd < line.length() && line[cEnd] != ' '; ++cEnd)
        ;
    /* Make sure cEnd points to the last character of the word or keep the
     * current cursor position in case there is no word. */
    if (cEnd > line.length() || line[cEnd] == ' ')
        cEnd--;

    TjDatePicker* picker = new TjDatePicker(mainWindow);
    QString tZone;
    unsigned int wLength = 0;
    if (line[cStart] != ' ')
    {
        // We have a word under the cursor.
        wLength = cEnd - cStart + 1;
        QString currentWord = line.mid(cStart, wLength);

        // Now test if it is a valid date.
        if (QRegExp("\\d{4}-\\d{1,2}-\\d{1,2}(-\\d{1,2}:\\d{1,2}"
                    "(:\\d{1,2}(-\\w*|)|)|)")
            .search(currentWord) == 0)
        {
            // If it is, use it to initialize the date picker widget.
            QStringList tokens = QStringList::split(QRegExp("[-:]"),
                                                    currentWord);
            QDate date = QDate(tokens[0].toInt(), tokens[1].toInt(),
                               tokens[2].toInt());
            picker->date->setDate(date);
            if (tokens.count() > 3)
            {
                picker->hours->setCurrentText(tokens[3]);
                picker->minutes->setCurrentText(tokens[4]);
            }
            if (tokens.count() > 5)
                tZone = tokens[6];
        }
        else
        {
            wLength = 0;
            cStart = c;
        }
    }
    else
        cStart = c;

    // Display the date picker widget.
    if (picker->exec() == QDialog::Rejected)
        return;

    // Extract the picked date and time.
    QString pickedDate = picker->date->date().toString(Qt::ISODate);
    if (picker->hours->currentText().toInt() != 0 ||
        picker->minutes->currentText().toInt() != 0)
    {
        pickedDate += "-" + picker->hours->currentText() + ":" +
            picker->minutes->currentText();
        if (!tZone.isEmpty())
            pickedDate += "-" + tZone;
    }

    // Replace the old date with the newly picked date.
    if (wLength > 0)
        line.replace(cStart, wLength, pickedDate);
    else
        line.insert(cStart, pickedDate);
    // Replace the old line with the new line.
    ei->removeLine(l);
    ei->insertLine(l, line);
    // Put cursor right after the inserted date.
    ci->setCursorPosition(l, cStart + pickedDate.length());
}

void
FileManager::print()
{
    if (getCurrentFile())
        KTextEditor::printInterface(getCurrentFile()->getEditor()->
                                    document())->print();
}

void
FileManager::configureEditor()
{
    if (getCurrentFile())
        KTextEditor::configInterface(getCurrentFile()->getEditor()->
                                     document())->configDialog();
}

void
FileManager::enableEditorActions(bool enable)
{
    mainWindow->action("insert_date")->setEnabled(enable);
}

void
FileManager::enableClipboardActions(bool enable)
{
    bool hasSelection = false;
    if (getCurrentFile())
        hasSelection = KTextEditor::selectionInterface
            (getCurrentFile()->getEditor()->document())->hasSelection();
    bool isClipEmpty = QApplication::clipboard()->
        text(QClipboard::Clipboard).isEmpty();

    mainWindow->action(KStdAction::name(KStdAction::Cut))->
        setEnabled(enable && hasSelection );
    mainWindow->action(KStdAction::name(KStdAction::Copy))->
        setEnabled(enable && hasSelection);
    mainWindow->action( KStdAction::name(KStdAction::Paste))->
        setEnabled( enable && !isClipEmpty);
}

void
FileManager::enableUndoActions(bool enable)
{
    bool undoEnable = false;
    if (getCurrentFile())
        undoEnable = (KTextEditor::undoInterface
                      (getCurrentFile()->getEditor()->document())->
                      undoCount() > 0 );
    bool redoEnable = false;
    if (getCurrentFile())
        redoEnable = (KTextEditor::undoInterface
                      (getCurrentFile()->getEditor()->document())->
                      redoCount() > 0 );

    mainWindow->action(KStdAction::name(KStdAction::Undo))->
        setEnabled(enable && undoEnable);
    mainWindow->action(KStdAction::name(KStdAction::Redo))->
        setEnabled(enable && redoEnable);
}

void
FileManager::setCursorPosition(int line, int col)
{
    if (getCurrentFile())
        KTextEditor::viewCursorInterface(getCurrentFile()->getEditor())->
            setCursorPosition(line, col);
}

void
FileManager::showEditor()
{
    if (getCurrentFile())
    {
        KTextEditor::View* editor = getCurrentFile()->getEditor();

        if (currentGUIClient)
            mainWindow->guiFactory()->removeClient(currentGUIClient);
        mainWindow->guiFactory()->addClient(editor);
        currentGUIClient = editor;
        editor->setFocus();
    }
}

void
FileManager::hideEditor()
{
    // Disable all editor actions.
    enableEditorActions(false);

    if (currentGUIClient)
    {
        mainWindow->guiFactory()->removeClient(currentGUIClient);
        currentGUIClient = 0;
    }
}

ManagedFileInfo*
FileManager::getMFI(const KURL& url)
{
    for (std::list<ManagedFileInfo*>::iterator mfi = files.begin();
         mfi != files.end(); ++mfi)
        if ((*mfi)->getFileURL() == url)
            return *mfi;

    return 0;
}

#include "FileManager.moc"
