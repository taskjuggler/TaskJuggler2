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

#include "ktjReportDialog.h"
#include "reportSelectionWidget.h"

// Qt includes
#include <qcombobox.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>

// KDE includes
#include <klocale.h>
#include <kguiitem.h>

KtjReportDialog::KtjReportDialog( const QDateTime & start, const QDateTime & end, QWidget * parent, const char * name )
    : KDialogBase( parent, name, true, i18n( "Create Report" ), Ok|Cancel, Ok )
{
    m_base = new ReportSelectionWidget( this, "report_selection_widget" );
    setMainWidget( m_base );
    setStartDate( start );
    setEndDate( end );
    setButtonOK( KGuiItem( i18n( "Create" ), "ok",
                           i18n( "Create the report with the specified options" ) ) ); // FIXME doesn't set the tooltip
}

KtjReportDialog::~KtjReportDialog()
{
    delete m_base;
}

QDateTime KtjReportDialog::startDate() const
{
    return m_base->deStart->dateTime();
}

void KtjReportDialog::setStartDate( const QDateTime & date )
{
    m_base->deStart->setDateTime( date );
}

QDateTime KtjReportDialog::endDate() const
{
    return m_base->deEnd->dateTime();
}

void KtjReportDialog::setEndDate( const QDateTime & date )
{
    m_base->deEnd->setDateTime( date );
}

KtjReport::Scale KtjReportDialog::scale() const
{
    return static_cast<KtjReport::Scale>( m_base->cbScale->currentItem() );
}

QString KtjReportDialog::scaleString() const
{
    return m_base->cbScale->currentText();
}

#include "ktjReportDialog.moc"
