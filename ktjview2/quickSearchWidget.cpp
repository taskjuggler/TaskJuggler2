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

#include "quickSearchWidget.h"
#include "klistviewsearchline.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qvaluelist.h>
#include <qcombobox.h>

#include <klistview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdialog.h>
#include <ktoolbarbutton.h>

QuickSearchWidget::QuickSearchWidget( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    QHBoxLayout * lay = new QHBoxLayout( this );
    lay->setSpacing( KDialog::spacingHint() );

    m_clearButton = new KToolBarButton( QApplication::reverseLayout() ? "clear_left" : "locationbar_erase",
                                        0, this );

    connect( m_clearButton, SIGNAL( clicked() ), this, SLOT( reset() ) );
    lay->add( m_clearButton );

    m_searchLabel = new QLabel( i18n( "Search &for:" ), this, "kde toolbar widget" );
    QToolTip::add( m_searchLabel, i18n( "Clear the search" ) );
    lay->add( m_searchLabel );

    m_searchLine = new KListViewSearchLine( this, 0, "search_line" );
    lay->addWidget( m_searchLine, 1 ); // adds the widget stretched

    m_inLabel = new QLabel( i18n( "in:" ), this, "kde toolbar widget" );
    lay->add( m_inLabel );

    m_colCombo = new QComboBox( this, "column_combo" );
    connect( m_colCombo, SIGNAL( activated( int ) ), this, SLOT( setSearchColumn( int ) ) );
    lay->add( m_colCombo );

    m_searchLabel->setBuddy( m_searchLine );
}

void QuickSearchWidget::setListView( KListView * view )
{
    m_searchLine->setListView( view );
    fillColumnCombo();
    setSearchColumn( m_colCombo->currentItem(), false );
}

void QuickSearchWidget::reset()
{
    m_searchLine->clear();
}

void QuickSearchWidget::fillColumnCombo()
{
    if ( !m_searchLine->listView() )
        return;

    m_colCombo->clear();

    for ( int i = 0; i < m_searchLine->listView()->columns(); i++ )
    {
        m_colCombo->insertItem( m_searchLine->listView()->columnText( i ) );
    }
}

void QuickSearchWidget::setSearchColumn( int index, bool update )
{
    QValueList<int> list;
    list.append( index );
    m_searchLine->setSearchColumns( list );
    if ( update )
        m_searchLine->updateSearch();
}

#include "quickSearchWidget.moc"
