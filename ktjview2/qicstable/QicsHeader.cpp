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


#include <qcolor.h>

#include <QicsHeader.h>
#include <QicsStyleManager.h>
#include <QicsSpanManager.h>
#include <QicsDimensionManager.h>
#include <QicsScreenGrid.h>
#include <QicsUtil.h>


QicsHeader::QicsHeader(QicsGridInfo *info, QObject *parent, bool forward_signals) :
    QicsGridCommon(info, parent, forward_signals),
    myCell(0),
    myRow(0),
    myColumn(0)
{
    initSignals();
}

QicsHeader::~QicsHeader()
{
}

QicsRow &
QicsHeader::rowRef(int rownum) const
{
    QicsHeader *self = const_cast<QicsHeader *> (this);

    if (!self->myRow)
	self->myRow = new QicsRow(rownum, myInfo, this);
    else
	self->myRow->setRowIndex(rownum);

    return (*myRow);
}

QicsRow *
QicsHeader::row(int idx, bool follow_model) const
{
    return (new QicsRow(idx, myInfo, follow_model, parent()));
}

QicsColumn &
QicsHeader::columnRef(int colnum) const
{
    QicsHeader *self = const_cast<QicsHeader *> (this);

    if (!self->myColumn)
	self->myColumn = new QicsColumn(colnum, myInfo, this);
    else
	self->myColumn->setColumnIndex(colnum);

    return (*myColumn);
}

QicsColumn *
QicsHeader::column(int idx, bool follow_model) const
{
    return (new QicsColumn(idx, myInfo, follow_model, parent()));
}

QicsCell &
QicsHeader::cellRef(int rownum, int colnum) const
{
    QicsHeader *self = const_cast<QicsHeader *> (this);

    if (!self->myCell)
	self->myCell = new QicsCell(rownum, colnum, myInfo, this);
    else
    {
	self->myCell->setRowIndex(rownum);
	self->myCell->setColumnIndex(colnum);
    }

    return (*myCell);
}

QicsCell *
QicsHeader::cell(int row, int col, bool follow_model) const
{
    return (new QicsCell(row, col, myInfo, follow_model, parent()));
}

bool
QicsHeader::allowUserResize(void) const
{
    return (* static_cast<bool *>
	    (styleManager().getGridProperty(QicsGridStyle::AllowUserResize)));
}

void
QicsHeader::setAllowUserResize(bool b)
{
    bool *val = new bool(b);

    styleManager().setGridProperty(QicsGridStyle::AllowUserResize,
				   static_cast<void *> (val));
}

bool
QicsHeader::allowUserMove(void) const
{
    return (* static_cast<bool *>
	    (styleManager().getGridProperty(QicsGridStyle::AllowUserMove)));
}

void
QicsHeader::setAllowUserMove(bool b)
{
    bool *val = new bool(b);

    styleManager().setGridProperty(QicsGridStyle::AllowUserMove,
				   static_cast<void *> (val));
}

//////////////////////////////////////////////////////////////////

void
QicsHeader::connectGrid(QicsScreenGrid *grid)
{
    QicsGridCommon::connectGrid(grid);

    if (grid->inherits("QicsHeaderGrid"))
    {
	connect(grid, SIGNAL(sizeChange(int, int, int, QicsHeaderType)),
		this, SIGNAL(sizeChange(int, int, int, QicsHeaderType)));
	connect(grid, SIGNAL(resizeInProgress(int, int, QicsHeaderType)),
		this, SIGNAL(resizeInProgress(int, int, QicsHeaderType)));
	connect(grid, SIGNAL(gripDoubleClicked(int, int, QicsHeaderType)),
		this, SIGNAL(gripDoubleClicked(int, int, QicsHeaderType)));
    }
}

#include "QicsHeader.moc"
