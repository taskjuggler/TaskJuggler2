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
// local includes
#include "ruFindDialog.h"

// Qt includes
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <klineedit.h>
#include <qlayout.h>
#include <qregexp.h>

// KDE includes
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcompletion.h>

ruFindDlg::ruFindDlg( QStringList data, QWidget* parent, const char* name )
    : KDialogBase( parent, name, false, i18n( "Resource Usage Search" ), User1|Close, User1,
                   false, KGuiItem( i18n( "&Find" ), "find" ) ),
      m_data( data ), m_firstRun( true )
{
    QVBox * layout = makeVBoxMainWidget();

    grpResource = new QButtonGroup( i18n( "Resource" ), ( QWidget * )layout, "grpResource" );
    grpResource->setColumnLayout( 0, Qt::Vertical );
    grpResource->layout()->setSpacing( KDialog::spacingHint() );
    grpResource->layout()->setMargin( KDialog::marginHint() );

    grpResourceLayout = new QGridLayout( grpResource->layout() );
    grpResourceLayout->setAlignment( Qt::AlignTop );

    lbName = new QLabel( i18n( "&Name:" ), grpResource, "label_name" );
    grpResourceLayout->addWidget( lbName, 0, 0 );

    cbCaseSensitive = new QCheckBox( i18n( "C&ase sensitive" ), grpResource, "cbCaseSensitive" );
    grpResourceLayout->addMultiCellWidget( cbCaseSensitive, 1, 1, 0, 1 );

    leResource = new KLineEdit( grpResource, "leResource" );
    grpResourceLayout->addWidget( leResource, 0, 1 );

    cbRegExp = new QCheckBox( i18n( "&Regular expression" ), grpResource, "cbRegExp" );
    grpResourceLayout->addMultiCellWidget( cbRegExp, 2, 2, 0, 1 );

    spacer3 = new QSpacerItem( 91, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );

    //resize( QSize(305, 185).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // buddies
    lbName->setBuddy( leResource );
    leResource->setFocus();

    // tab order
    setTabOrder( leResource, cbCaseSensitive );
    setTabOrder( cbCaseSensitive, cbRegExp );

    connect( leResource, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( slotTextChanged( const QString & ) ) );

    // completion
    KCompletion * comp = new KCompletion();
    comp->setItems( m_data.gres( QRegExp( "^\\s+" ), "" ) );
    leResource->setCompletionObject( comp );
    leResource->setAutoDeleteCompletionObject( true );

    slotTextChanged( leResource->text() ); // update the Find button
}

ruFindDlg::~ruFindDlg()
{
    // no need to delete child widgets, Qt does it all for us
}


void ruFindDlg::slotUser1()
{
    if ( m_firstRun )
        startSearch();
    else
        findNext();
}

void ruFindDlg::startSearch()
{
    int index = -1;
    QString text = leResource->text();
    bool cs = cbCaseSensitive->isChecked();
    bool regexp = cbRegExp->isChecked();

    for ( QStringList::ConstIterator it = m_data.begin(); it != m_data.end(); ++it )
    {
        index++;

        QString row = ( *it );
        if ( regexp )
        {
            if ( row.find( QRegExp( text ) ) != -1 )
                 m_result.append( index );
        }
        else
        {
            if ( row.find( text, 0, cs ) != -1 )
                m_result.append( index );
        }
    }
    findNext();
}

void ruFindDlg::findNext()
{
    if ( !m_result.isEmpty() )  // return the first result
    {
        emit signalMatch( m_result.first() );
        kdDebug() << "Returning item: " << m_result.first() << endl;
        m_result.pop_front();   // remove the match from the result
    }
    else                        // or signal an empty result set
    {
        if ( m_firstRun )
        {
            kdDebug() << "No results at all" << endl;
            emit signalMatch( -1 );
        }
        else
        {
            kdDebug() << "No more results" << endl;
            emit signalMatch( -2 );
        }
    }
    m_firstRun = false;
}

void ruFindDlg::slotTextChanged( const QString & text )
{
    enableButton( User1, !text.isEmpty() );
}

#include "ruFindDialog.moc"
