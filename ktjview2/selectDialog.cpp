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

#include "selectDialog.h"

#include "TaskList.h"
#include "Task.h"
#include "ResourceList.h"
#include "Resource.h"

// Qt includes
#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>

// KDE includes
#include <klocale.h>
#include <klistview.h>

SelectDialog::SelectDialog( TaskListIterator it, bool multi, QWidget* parent, const char* name )
    : KDialogBase( parent, name, true, i18n( "Tasks" ), Ok|Cancel, Ok ), m_multi( multi )
{
    init();
    lbSelect->setText( i18n( "Select tasks:" ) );
}

SelectDialog::SelectDialog( ResourceListIterator it, bool multi, QWidget* parent, const char* name )
    : KDialogBase( parent, name, true, i18n( "Resources" ), Ok|Cancel, Ok ), m_multi( multi )
{
    init();
    lbSelect->setText( i18n( "Select resources:" ) );

    // fill the listview with resources
    for ( ; *it != 0; ++it )
    {
        Resource* res = (*it);

        if ( m_multi )
        {
            QCheckListItem * item = new QCheckListItem( lvContents, res->getId(), QCheckListItem::CheckBox );
            item->setText( 1, res->getName() );
        }
        else
            ( void ) new KListViewItem( lvContents, res->getId(), res->getName() );
    }
}

SelectDialog::~SelectDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void SelectDialog::init()
{
    QVBox * layout = makeVBoxMainWidget();

    lbSelect = new QLabel( i18n( "Select:" ), layout, "lbSelect" );

    lvContents = new KListView( layout, "lvContents" );
    lvContents->addColumn( i18n( "ID" ) );
    lvContents->addColumn( i18n( "Name" ) );
    lvContents->setAllColumnsShowFocus( TRUE );
    lvContents->setShowSortIndicator( TRUE );
    lvContents->setResizeMode( KListView::LastColumn );
    lvContents->setItemsMovable( FALSE );
    lvContents->hideColumn( 0 );
    resize( QSize(492, 394).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

QStringList SelectDialog::resultList() const
{
    QStringList result;

    QListViewItemIterator::IteratorFlag flags;
    if ( m_multi )
        flags = QListViewItemIterator::Checked;
    else
        flags = QListViewItemIterator::Selected;

    QListViewItemIterator it( lvContents, flags );
    while ( it.current() )
    {
        if ( m_multi )
            result.append( static_cast<QCheckListItem *>( *it )->text() );
        else
            result.append( ( *it )->text( 0 ) );

        ++it;
    }

    return result;
}

#include "selectDialog.moc"
