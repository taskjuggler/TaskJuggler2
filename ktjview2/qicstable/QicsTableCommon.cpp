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


#include <QicsTableCommon.h>
#include <QicsHeader.h>
#include <QicsStyleManager.h>
#include <QicsDimensionManager.h>
#include <QicsSpanManager.h>
#include <QicsUtil.h>

////////////////////////////////////////////////////////////////////////////////

QicsTableCommon::QicsTableCommon(QObject *parent, bool forward_signals) :
    QicsGridCommon(parent, forward_signals),
    myMainGridInfo(TableGrid),
    myRowHeaderGridInfo(RowHeaderGrid),
    myColumnHeaderGridInfo(ColumnHeaderGrid),
    myRHDataModel(0),
    myCHDataModel(0),
    myCell(0),
    myRow(0),
    myColumn(0),
    myMainGrid(0),
    myRowHeader(0),
    myColumnHeader(0),
    myTopHeaderVisible(DisplayAlways),
    myBottomHeaderVisible(DisplayNever),
    myLeftHeaderVisible(DisplayAlways),
    myRightHeaderVisible(DisplayNever),
    myTableMargin(10),
    myTableSpacing(10),
    myGridSpacing(0),
    myRowHeaderUsesModel(false),
    myColumnHeaderUsesModel(false)
{
    setInfo(&gridInfo());
}

QicsTableCommon::~QicsTableCommon()
{
    delete myCell;
    delete myRow;
    delete myColumn;
    delete myMainGrid;
    delete myRowHeader;
    delete myColumnHeader;
}

QicsDataModel *
QicsTableCommon::dataModel(void) const
{
    return gridInfo().dataModel();
}

void
QicsTableCommon::setDataModel(QicsDataModel *dm)
{
    gridInfo().setDataModel(dm);
}

void
QicsTableCommon::setAttr(QicsCellStyle::QicsCellStyleProperty attr, const void *val)
{
    gridInfo().styleManager()->setDefaultProperty(attr, val);
    rhGridInfo().styleManager()->setDefaultProperty(attr, val);
    chGridInfo().styleManager()->setDefaultProperty(attr, val);
}

void
QicsTableCommon::setGridAttr(QicsGridStyle::QicsGridStyleProperty attr,
			     const void *val)
{
    gridInfo().styleManager()->setGridProperty(attr, val);

    // Viewport is a special case -- we don't want to set it on the
    // headers.
    if (attr != QicsGridStyle::Viewport)
    {
	rhGridInfo().styleManager()->setGridProperty(attr, val);
	chGridInfo().styleManager()->setGridProperty(attr, val);
    }
}

void
QicsTableCommon::clearGridAttr(QicsGridStyle::QicsGridStyleProperty attr)
{
    gridInfo().styleManager()->setGridProperty(attr, static_cast<void *> (0));
    rhGridInfo().styleManager()->setGridProperty(attr, static_cast<void *> (0));
    chGridInfo().styleManager()->setGridProperty(attr, static_cast<void *> (0));
}

////////////////////////////////////////////////////////////////////
///////////         Row, Column, Cell methods         //////////////
////////////////////////////////////////////////////////////////////

QicsRow &
QicsTableCommon::rowRef(int rownum)
{
    if (!myRow)
	myRow = new QicsRow(rownum, &gridInfo(), this);
    else
	myRow->setRowIndex(rownum);

    return (*myRow);
}

const QicsRow &
QicsTableCommon::rowRef(int rownum) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->rowRef(rownum));
}

QicsRow *
QicsTableCommon::row(int rownum, bool follow_model)
{
    return (new QicsRow(rownum, &gridInfo(), follow_model, this));
}

const QicsRow *
QicsTableCommon::row(int rownum, bool follow_model) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->row(rownum, follow_model));
}

QicsColumn &
QicsTableCommon::columnRef(int colnum)
{
    if (!myColumn)
	myColumn = new QicsColumn(colnum, &gridInfo(), this);
    else
	myColumn->setColumnIndex(colnum);

    return (*myColumn);
}

const QicsColumn &
QicsTableCommon::columnRef(int colnum) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->columnRef(colnum));
}

QicsColumn *
QicsTableCommon::column(int colnum, bool follow_model)
{
    return (new QicsColumn(colnum, &gridInfo(), follow_model, this));
}

const QicsColumn *
QicsTableCommon::column(int colnum, bool follow_model) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->column(colnum, follow_model));
}

QicsCell &
QicsTableCommon::cellRef(int rownum, int colnum)
{
    if (!myCell)
	myCell = new QicsCell(rownum, colnum, &gridInfo(), this);
    else
    {
	myCell->setRowIndex(rownum);
	myCell->setColumnIndex(colnum);
    }

    return (*myCell);
}

const QicsCell &
QicsTableCommon::cellRef(int rownum, int colnum) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->cellRef(rownum, colnum));
}

QicsCell *
QicsTableCommon::cell(int rownum, int colnum, bool follow_model)
{
    return (new QicsCell(rownum, colnum, &gridInfo(), follow_model, this));
}

const QicsCell *
QicsTableCommon::cell(int rownum, int colnum, bool follow_model) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->cell(rownum, colnum, follow_model));
}

QicsMainGrid &
QicsTableCommon::mainGridRef(void)
{
    if (!myMainGrid)
	myMainGrid = new QicsMainGrid(&gridInfo(), this);

    return (*myMainGrid);
}

const QicsMainGrid &
QicsTableCommon::mainGridRef(void) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->mainGridRef());
}

QicsMainGrid *
QicsTableCommon::mainGrid(void)
{
    return (new QicsMainGrid(&gridInfo(), this));
}

const QicsMainGrid *
QicsTableCommon::mainGrid(void) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->mainGrid());
}

QicsRowHeader &
QicsTableCommon::rowHeaderRef(void)
{
    if (!myRowHeader)
	myRowHeader = rowHeader();

    return (*myRowHeader);
}

const QicsRowHeader &
QicsTableCommon::rowHeaderRef(void) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->rowHeaderRef());
}

QicsRowHeader *
QicsTableCommon::rowHeader(void)
{
    return (new QicsRowHeader(&rhGridInfo(), this, true));
}

const QicsRowHeader *
QicsTableCommon::rowHeader(void) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->rowHeader());
}

QicsColumnHeader &
QicsTableCommon::columnHeaderRef(void)
{
    if (!myColumnHeader)
	myColumnHeader = columnHeader();

    return (*myColumnHeader);
}

const QicsColumnHeader &
QicsTableCommon::columnHeaderRef(void) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->columnHeaderRef());
}

QicsColumnHeader *
QicsTableCommon::columnHeader(void)
{
    return (new QicsColumnHeader(&chGridInfo(), this, true));
}

const QicsColumnHeader *
QicsTableCommon::columnHeader(void) const
{
    QicsTableCommon *self = const_cast<QicsTableCommon *> (this);

    return (self->columnHeader());
}

////////////////////////////////////////////////////////////////////////

void 
QicsTableCommon::setTopHeaderVisible(QicsTableDisplayOption tdo)
{
    myTopHeaderVisible = tdo;
}

void 
QicsTableCommon::setBottomHeaderVisible(QicsTableDisplayOption tdo)
{
    myBottomHeaderVisible = tdo;
}

void 
QicsTableCommon::setLeftHeaderVisible(QicsTableDisplayOption tdo)
{
    myLeftHeaderVisible = tdo;
}

void 
QicsTableCommon::setRightHeaderVisible(QicsTableDisplayOption tdo)
{
    myRightHeaderVisible = tdo;
}

void
QicsTableCommon::setTableMargin(int margin)
{
    myTableMargin = margin;
}

void
QicsTableCommon::setTableSpacing(int spacing)
{
    myTableSpacing = spacing;
}

void
QicsTableCommon::setGridSpacing(int spacing)
{
    myGridSpacing = spacing;
}

void
QicsTableCommon::sortRows(int column, QicsSortOrder order,
			  int from, int to,
			  DataItemComparator func)
{
    gridInfo().orderRowsBy(column, order, from, to, func);
}

void
QicsTableCommon::sortColumns(int row, QicsSortOrder order,
			     int from, int to,
			     DataItemComparator func)
{
    gridInfo().orderColumnsBy(row, order, from, to, func);
}

void
QicsTableCommon::moveRows(int target_row, const QMemArray<int> &rows)
{
    gridInfo().moveRows(target_row, rows);
}

void
QicsTableCommon::moveColumns(int target_col, const QMemArray<int> &cols)
{
    gridInfo().moveColumns(target_col, cols);
}

void
QicsTableCommon::addRows(int how_many)
{
    while(how_many-- > 0) gridInfo().insertRow(-1);
}
void
QicsTableCommon::insertRow(int row)
{
    gridInfo().insertRow(row);
}
void
QicsTableCommon::deleteRow(int row)
{
    gridInfo().deleteRows(1, row);
}

void
QicsTableCommon::addColumns(int how_many)
{
    while(how_many-- > 0) gridInfo().insertColumn(-1);
}
void
QicsTableCommon::insertColumn(int col)
{
    gridInfo().insertColumn(col);
}
void
QicsTableCommon::deleteColumn(int col)
{
    gridInfo().deleteColumns(1, col);
}

void
QicsTableCommon::setRowHeaderUsesModel(bool b)
{
    myRowHeaderUsesModel = b; 
    QicsRegion vp = viewport();

    if (b)
    {
	// If we are currently showing column 0, don't show it anymore
	if (vp.startColumn() == 0)
	{
	    vp.setStartColumn(1);
	    setViewport(vp);
	}

	rhGridInfo().setDataModel(dataModel());
    }
    else
    {
	// If we are currently not showing column 0, show it now
	if (vp.startColumn() == 1)
	{
	    vp.setStartColumn(0);
	    setViewport(vp);
	}

	rhGridInfo().setDataModel(myRHDataModel);
    }
}

void
QicsTableCommon::setColumnHeaderUsesModel(bool b)
{
    myColumnHeaderUsesModel = b; 
    QicsRegion vp = viewport();

    if (b) 
    {
	 // If we are currently showing row 0, don't show it anymore
	 if (vp.startRow() == 0)
	 {
	     vp.setStartRow(1);
	     setViewport(vp);
	 }

	chGridInfo().setDataModel(dataModel());
    }
    else
    {
	 // If we are currently not showing row 0, show it now
	 if (vp.startRow() == 1)
	 {
	     vp.setStartRow(0);
	     setViewport(vp);
	 }

	chGridInfo().setDataModel(myCHDataModel);
    }
}



#include "QicsTableCommon.moc"
