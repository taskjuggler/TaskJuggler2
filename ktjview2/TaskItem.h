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
#ifndef _TASKITEM_H_
#define _TASKITEM_H_

#include <qdatetime.h>
#include <qstring.h>

#include <klistview.h>

/**
 * This class represents an item in the tasks listview.
 * @short Task item (listview)
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class TaskItem: public KListViewItem
{
public:
    /**
     * CTOR
     * @param parent Parent item (see KListView)
     * @param id ID of the corresponding task
     * @param start Task's start date
     * @param end Task's end date
     */
    TaskItem( KListView * parent, const QString & id, QDateTime start, QDateTime end ):
        KListViewItem( parent, id )
        {
            m_id = id;
            m_start = start;
            m_end = end;
        }
    /**
     * CTOR
     * @param parent Parent item (see KListViewItem)
     * @param id ID of the corresponding task
     * @param start Task's start date
     * @param end Task's end date
     */
    TaskItem( KListViewItem * parent, const QString & id, QDateTime start, QDateTime end ):
        KListViewItem( parent, id )
        {
            m_id = id;
            m_start = start;
            m_end = end;
        }

    /**
     * @return ID of the task
     */
    QString id() const { return m_id ;}

    /**
     * @return The task's start date
     */
    QDateTime startDate() const { return m_start; }

    /**
     * @return The task's end date
     */
    QDateTime endDate() const { return m_end; }

    /**
     * Reimplemented from QListViewItem to get correct sorting (dates, numbers, etc)
     */
    virtual int compare( QListViewItem * i, int col, bool ascending ) const;

private:
    QString m_id;
    QDateTime m_start, m_end;
};

#endif
