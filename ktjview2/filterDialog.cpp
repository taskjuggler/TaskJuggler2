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

// Qt includes
#include <qcombobox.h>
#include <qlistbox.h>
#include <qtable.h>

// KDE includes
#include <kdialogbase.h>
#include <kguiitem.h>
#include <klocale.h>

// TJ includes
#include "Resource.h"
#include "ResourceList.h"
#include "Project.h"

// local includes
#include "filterDialog.h"
#include "filterWidget.h"


FilterDialog::FilterDialog( QWidget * parent, const char * name )
    : KDialogBase( parent, name, true, i18n( "Filter" ), Ok|Cancel, Ok )
{
    m_conditions << i18n( "contains" ) << i18n( "doesn't contain" ) << i18n( "equals" ) << i18n( "doesn't equal" )
                 << i18n( "matches reqexp" ) << i18n( "doesn't match regexp" )
                 << i18n( "greater than" ) << i18n( "less than or equal" ) << i18n( "less than" ) << i18n( "greater than or equal" );

    m_base = new FilterWidget( this, name );
    setMainWidget( m_base );

    connect( ( QWidget * ) m_base->btnMore, SIGNAL( clicked() ), this, SLOT( slotMore() ) );
    connect( ( QWidget * ) m_base->btnFewer, SIGNAL( clicked() ), this, SLOT( slotFewer() ) );
    connect( ( QWidget * ) m_base->btnClear, SIGNAL( clicked() ), this, SLOT( slotClear() ) );
    connect( ( QWidget * ) m_base->btnNew, SIGNAL( clicked() ), this, SLOT( slotNewFilter() ) );
    connect( ( QWidget * ) m_base->btnDelete, SIGNAL( clicked() ), this, SLOT( slotDeleteFilter() ) );
    connect( ( QWidget * ) m_base->btnRename, SIGNAL( clicked() ), this, SLOT( slotRenameFilter() ) );
}

FilterDialog::~FilterDialog()
{
    delete m_base;
}

void FilterDialog::slotMore()
{
    // append a row at the end
    m_base->tbConditions->insertRows( m_base->tbConditions->numRows(), 1 );

    // fill the row with list of conditions
    QComboTableItem * item = new QComboTableItem( m_base->tbConditions, m_conditions );
    m_base->tbConditions->setItem( m_base->tbConditions->numRows() - 1, 1, item );
    m_base->tbConditions->adjustColumn( 1 );
}

void FilterDialog::slotFewer()
{
    m_base->tbConditions->removeRow( m_base->tbConditions->numRows() - 1 ); // remove the last row
}

void FilterDialog::slotClear()
{
    int count = m_base->tbConditions->numRows() - 1;
    for ( int i = count; i >= 0; i-- )
        m_base->tbConditions->removeRow( i );
}

void FilterDialog::slotNewFilter()
{
    QString name = i18n( "new" );
    if ( m_base->lbFilters->findItem( name, Qt::ExactMatch ) != 0 ) // name already exists
    {

    }
    m_base->lbFilters->insertItem( name, 0 );
}

void FilterDialog::slotDeleteFilter()
{

}

void FilterDialog::slotRenameFilter()
{

}

#include "filterDialog.moc"
