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


#include <QicsRowHeader.h>
#include <QicsDataModel.h>
#include <QicsStyleManager.h>
#include <QicsDimensionManager.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE
#include <QicsTable.h>
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef CREATE_OBJS_WITH_QICSTABLE
QicsRowHeader::QicsRowHeader(QicsTable *table) :
    QicsHeader(&(table->rhGridInfo()), table, true)
{
}
#endif

QicsRowHeader::QicsRowHeader(QicsGridInfo *info, QObject *parent,
			     bool forward_signals)
    : QicsHeader(info, parent, forward_signals)

{
}

QicsRowHeader::~QicsRowHeader()
{
}

int
QicsRowHeader::numColumns(void) const
{
    return dataModel()->numColumns();
}

void
QicsRowHeader::setNumColumns(int num)
{
    if (!dataModel())
	return;

    int last = dataModel()->lastColumn();

    if (num > (last + 1))
	dataModel()->addColumns(num-last-1);
    else if (num <= last)
	dataModel()->deleteColumns(last-num+1, num);
}
