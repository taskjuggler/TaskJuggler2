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
#ifndef _RESOURCEITEM_H_
#define _RESOURCEITEM_H_

#include <qdatetime.h>
#include <qstring.h>

#include <klistview.h>

/**
 * This class represents an item in the resource listview.
 * @short Resource item (listview)
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class ResourceItem: public KListViewItem
{
public:
    /**
     * CTOR
     * @param parent Parent item (see KListView)
     * @param id ID of the corresponding resource
     */
    ResourceItem( KListView * parent, const QString & id ):
        KListViewItem( parent, id )
        {
            m_id = id;
        }
    /**
     * CTOR
     * @param parent Parent item (see KListViewItem)
     * @param id ID of the corresponding resource
     */
    ResourceItem( KListViewItem * parent, const QString & id ):
        KListViewItem( parent, id )
        {
            m_id = id;
        }

    /**
     * @return ID of the resource
     */
    QString id() const { return m_id ;}

    /**
     * Reimplemented from QListViewItem to get correct sorting (dates, numbers, etc)
     */
    virtual int compare( QListViewItem * i, int col, bool ascending ) const;

private:
    QString m_id;
};

#endif
