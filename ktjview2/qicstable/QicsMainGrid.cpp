/**************************************************************************
**
** Copyright (C) 2002-2003 Integrated Computer Solutions, Inc.
** All rights reserved.
**
** This file is part of the QicsTable Product.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.ics.com/qt/licenses/gpl/ for GPL licensing information.
**
** If you would like to use this software under a commericial license
** for the development of proprietary software, send email to sales@ics.com
** for details.
**
** Contact info@ics.com if any conditions of this licensing are
** not clear to you.
**
**************************************************************************/


#include <QicsMainGrid.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE
#include <QicsTable.h>
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef CREATE_OBJS_WITH_QICSTABLE
QicsMainGrid::QicsMainGrid(QicsTable *table) :
    QicsGridCommon(&(table->gridInfo()), table, true)
{
    initSignals();
}
#endif

QicsMainGrid::QicsMainGrid(QicsGridInfo *info, QObject *parent,
			   bool forward_signals)
    : QicsGridCommon(info, parent, forward_signals)

{
    initSignals();
}

QicsMainGrid::~QicsMainGrid()
{
}


#include "QicsMainGrid.moc"
