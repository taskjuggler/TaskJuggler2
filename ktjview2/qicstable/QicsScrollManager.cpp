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


#include <QicsScrollManager.h>
#include <QicsScroller.h>
#include <QicsScreenGrid.h>


QicsScrollManager::QicsScrollManager()
{
    myPrimaryGrid = 0;
    myRowIndex = 0;
    myColumnIndex = 0;
}

QicsScrollManager::~QicsScrollManager()
{
}

void
QicsScrollManager::connectScroller(QicsScroller *scroller)
{
    if (!scroller)
	return;

    connect(scroller, SIGNAL(indexChanged(QicsIndexType, int)),
	    this, SLOT(setIndex(QicsIndexType, int)));

    connect(this, SIGNAL(minIndexChanged(QicsIndexType, int)),
	    scroller, SLOT(setMinIndex(QicsIndexType, int)));
    connect(this, SIGNAL(maxIndexChanged(QicsIndexType, int)),
	    scroller, SLOT(setMaxIndex(QicsIndexType, int)));
    connect(this, SIGNAL(indexChanged(QicsIndexType, int)),
	    scroller, SLOT(setIndex(QicsIndexType, int)));
}

void
QicsScrollManager::disconnectScroller(QicsScroller *scroller)
{
    disconnect(scroller, 0, this, 0);
    disconnect(this, 0, scroller, 0);
}

void
QicsScrollManager::setPrimaryGrid(QicsScreenGrid *grid)
{
    myPrimaryGrid = grid;

    gridBoundsChanged();

    connect(grid, SIGNAL(newBoundsInfo()),
	    this, SLOT(gridBoundsChanged()));

    connect(grid, SIGNAL(destroyed(QObject *)),
	    this, SLOT(gridDestroyed(QObject *)));
}

void
QicsScrollManager::connectGrid(QicsScreenGrid *grid,
			       bool control_rows, bool control_columns)
{
    if (!grid)
	return;

    if (control_rows)
    {
	connect(this, SIGNAL(rowIndexChanged(int)),
		grid, SLOT(setTopRow(int)));
    }

    if (control_columns)
    {
	connect(this, SIGNAL(columnIndexChanged(int)),
		grid, SLOT(setLeftColumn(int)));
    }
}

void
QicsScrollManager::disconnectGrid(QicsScreenGrid *grid)
{
    disconnect(this, 0, grid, 0);
}

//////////////////////////////////////////////////////////////////////

void
QicsScrollManager::setRowIndex(int idx)
{
    myRowIndex = idx;
    emit rowIndexChanged(idx);
    emit indexChanged(RowIndex, idx);
}

void
QicsScrollManager::setColumnIndex(int idx)
{
    myColumnIndex = idx;
    emit columnIndexChanged(idx);
    emit indexChanged(ColumnIndex, idx);
}

void
QicsScrollManager::setIndex(QicsIndexType type, int idx)
{
    if (type == RowIndex)
    {
	setRowIndex(idx);
    }
    else
    {
	setColumnIndex(idx);
    }
}

void
QicsScrollManager::setRowMinValue(int val)
{
    emit minIndexChanged(RowIndex, val);
}

void
QicsScrollManager::setRowMaxValue(int val)
{
    emit maxIndexChanged(RowIndex, val);
}

void
QicsScrollManager::setColumnMinValue(int val)
{
    emit minIndexChanged(ColumnIndex, val);
}

void
QicsScrollManager::setColumnMaxValue(int val)
{
    emit maxIndexChanged(ColumnIndex, val);
}

void
QicsScrollManager::gridBoundsChanged(void)
{
    QicsRegion vp = myPrimaryGrid->currentViewport();

    setRowMinValue(vp.startRow());
    setRowMaxValue(QICS_MAX((vp.endRow() - myPrimaryGrid->lastPageRows() + 1),
			    vp.startRow()));

    setColumnMinValue(vp.startColumn());
    setColumnMaxValue(QICS_MAX((vp.endColumn() - myPrimaryGrid->lastPageColumns() + 1),
			       vp.startColumn()));
}

void
QicsScrollManager::gridDestroyed(QObject *)
{
    myPrimaryGrid = 0;
}

#include "QicsScrollManager.moc"
