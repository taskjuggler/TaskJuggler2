// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/***************************************************************************
 *   Copyright (C) 2004 by Lukas Tinkl                                     *
 *   lukas.tinkl@suse.cz                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _SELECTDIALOG_H_
#define _SELECTDIALOG_H_

#include <qwidget.h>

#include <kdialogbase.h>

class QLabel;
class KListView;
class KDialogBase;
class TaskListIterator;
class ResourceListIterator;

/**
 * Dialog for selecting from a list of resources or tasks.
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 * @short Task/resource selection dialog
 */
class SelectDialog : public KDialogBase
{
    Q_OBJECT
public:
    /**
     * Construct a dialog for selecting tasks
     * @param it iterator over list of tasks
     * @param multi whether multiple selection is allowed
     */
    SelectDialog( TaskListIterator it, bool multi = false, QWidget* parent = 0, const char* name = 0 );
    /**
     * Construct a dialog for selecting resources
     * @param it iterator over list of resources
     * @param multi whether multiple selection is allowed
     */
    SelectDialog( ResourceListIterator it, bool multi = false, QWidget* parent = 0, const char* name = 0 );
    ~SelectDialog();

    /**
     * Contains the result list of task/resource IDs selected by the user
     */
    QStringList resultList() const;

private:
    /**
     * Setup the widgets
     */
    void init();

    QLabel* lbSelect;
    KListView* lvContents;

    /// whether multiple selection is allowed
    bool m_multi;
};

#endif
