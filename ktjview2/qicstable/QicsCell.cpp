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


#include <QicsCell.h>
#include <QicsDataModel.h>
#include <QicsStyleManager.h>
#include <QicsDimensionManager.h>
#include <QicsSpanManager.h>
#include <QicsDataItem.h>
#include <QicsUtil.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE
#include <QicsTable.h>
#endif

/////////////////////////////////////////////////////////////////////////////////

#ifdef CREATE_OBJS_WITH_QICSTABLE
QicsCell::QicsCell(int row, int col, QicsTable *table, bool follow_model) :
    QicsCellCommon(&(table->gridInfo()), table),
    myFollowModel(follow_model)
{
    setRowIndex(row);
    setColumnIndex(col);

    init();
}
#endif

QicsCell::QicsCell(int row, int col, QicsGridInfo *info, bool follow_model,
		   QObject *parent) :
    QicsCellCommon(info, parent),
    myFollowModel(follow_model)
{
    setRowIndex(row);
    setColumnIndex(col);

    init();
}

void
QicsCell::init(void)
{
    if (!myInfo)
	return;

    if (myFollowModel)
    {
	// connect ourself to the data model's signals

	connect(dataModel(), SIGNAL(rowsInserted(int, int)),
		this, SLOT(handleModelRowInsert(int, int)));
	connect(dataModel(), SIGNAL(columnsInserted(int, int)),
		this, SLOT(handleModelColumnInsert(int, int)));

	connect(dataModel(), SIGNAL(rowsDeleted(int, int)),
		this, SLOT(handleModelRowDelete(int, int)));
	connect(dataModel(), SIGNAL(columnsDeleted(int, int)),
		this, SLOT(handleModelColumnDelete(int, int)));

	// also want to know if the data model has changed
	connect(myInfo, SIGNAL(dataModelChanged(QicsDataModel *, QicsDataModel *)),
		this, SLOT(changeDataModel(QicsDataModel *, QicsDataModel *)));
    }
    else
    {
	// connect to the row and column ordering objects
	connect(myInfo->rowOrdering(),
		SIGNAL(orderChanged(QicsIndexType, QMemArray<int>)),
		this,
		SLOT(handleOrderChanged(QicsIndexType, QMemArray<int>)));
	connect(myInfo->columnOrdering(),
		SIGNAL(orderChanged(QicsIndexType, QMemArray<int>)),
		this,
		SLOT(handleOrderChanged(QicsIndexType, QMemArray<int>)));
    }
}

void
QicsCell::setInfo(QicsGridInfo *info)
{
    if (info == myInfo)
	return;

    if (myInfo)
	disconnect(myInfo, 0, this, 0);

    QicsCellCommon::setInfo(info);
    init();
}

////////////////////////////////////////////////////////////////////////
/////////////////////    Index Methods   ///////////////////////////////
////////////////////////////////////////////////////////////////////////

int
QicsCell::rowIndex(void) const
{
    if (myRow >= 0)
	return myInfo->visualRowIndex(myRow);
    else
	return -1;
}

int
QicsCell::columnIndex(void) const
{ 
    if (myColumn >= 0)
	return myInfo->visualColumnIndex(myColumn);
    else
	return -1;
}

int
QicsCell::modelRowIndex(void) const
{
    return myRow; 
}

int
QicsCell::modelColumnIndex(void) const
{
    return myColumn;
}

void
QicsCell::setRowIndex(int idx)
{
    if (idx < 0)
	myRow = -1;
    else
	myRow = myInfo->modelRowIndex(idx);
}

void
QicsCell::setColumnIndex(int idx)
{
    if (idx < 0)
	myColumn = -1;
    else
	myColumn = myInfo->modelColumnIndex(idx);
}

bool
QicsCell::isValid(void) const
{
    if (dataModel())
	return ((myRow >= 0) && 
		(myRow < dataModel()->numRows()) &&
		(myColumn >= 0) &&
		(myColumn < dataModel()->numColumns()));
    else
	return true;
}

////////////////////////////////////////////////////////////////////////
////////////////////   Attribute methods   /////////////////////////////
////////////////////////////////////////////////////////////////////////

void
QicsCell::setAttr(QicsCellStyle::QicsCellStyleProperty attr, const void *val)
{
    styleManager().setCellProperty(myRow, myColumn, attr, val);
}

void *
QicsCell::getAttr(QicsCellStyle::QicsCellStyleProperty attr) const
{
    return (styleManager().getCellProperty(myRow, myColumn, attr));
}

void
QicsCell::clearAttr(QicsCellStyle::QicsCellStyleProperty attr)
{
    styleManager().clearCellProperty(myRow, myColumn, attr);
}

void
QicsCell::setDMMargin(int margin)
{
    dimensionManager().setCellMargin(myInfo->gridType(), myRow, myColumn,
				     margin);
}

void
QicsCell::setDMFont(const QFont &font)
{
    dimensionManager().setCellFont(myInfo->gridType(), myRow, myColumn, font);
}


const QicsDataItem *
QicsCell::dataValue(void) const
{
    if (dataModel() && isValid())
	return (dataModel()->item(myRow, myColumn));
    else
	return 0;
}

void
QicsCell::setDataValue(const QicsDataItem &val)
{
    if (dataModel() && isValid())
	dataModel()->setItem(myRow, myColumn, val);
}

void
QicsCell::clearDataValue(void)
{
    if (dataModel() && isValid())
	dataModel()->clearItem(myRow, myColumn);
}

bool
QicsCell::addSpan(int height, int width)
{
    if (!isValid())
	return false;

    if(height < 1) height = 1;
    if(width < 1) width = 1;

    QicsSpan span(myRow, myColumn, height, width);
    return styleManager().spanManager()->addCellSpan(span);
}

void
QicsCell::removeSpan(void)
{
    if (isValid())
	styleManager().spanManager()->removeCellSpan(myRow, myColumn);
}

bool
QicsCell::isCurrent(void) const
{
    return (* static_cast<bool *> (getAttr(QicsCellStyle::Current)));
}

int
QicsCell::widthInPixels(void) const
{
    return (dimensionManager().columnWidth(myColumn));
}

void
QicsCell::setWidthInPixels(int width)
{
    dimensionManager().setColumnWidthInPixels(myColumn, width);
}

int
QicsCell::widthInChars(void) const
{
    // divide the width in pixels by the width of the font -- round down.

    return (dimensionManager().columnWidth(myColumn)  /
	    qicsWidthOfFont(font()));
}

void
QicsCell::setWidthInChars(int width)
{
    dimensionManager().setColumnWidthInChars(myColumn, width);
}

int
QicsCell::minWidthInPixels(void) const
{
    return (dimensionManager().columnMinWidth(myColumn));
}

void
QicsCell::setMinWidthInPixels(int width)
{
    dimensionManager().setColumnMinWidthInPixels(myColumn, width);
}

int
QicsCell::minWidthInChars(void) const
{
    // divide the width in pixels by the width of the font -- round down.

    return (dimensionManager().columnMinWidth(myColumn)  /
	    qicsWidthOfFont(font()));
}

void
QicsCell::setMinWidthInChars(int width)
{
    dimensionManager().setColumnMinWidthInChars(myColumn, width);
}

int
QicsCell::heightInPixels(void) const
{
    return (dimensionManager().rowHeight(myRow));
}

void
QicsCell::setHeightInPixels(int height)
{
    dimensionManager().setRowHeightInPixels(myRow, height);
}

int
QicsCell::heightInChars(void) const
{
    // divide the height in pixels by the height of the font -- round down.

    return (dimensionManager().rowHeight(myRow)  /
	    qicsHeightOfFont(font()));
}

void
QicsCell::setHeightInChars(int height)
{
    dimensionManager().setRowHeightInChars(myRow, height);
}

int
QicsCell::minHeightInPixels(void) const
{
    return (dimensionManager().rowMinHeight(myRow));
}

void
QicsCell::setMinHeightInPixels(int height)
{
    dimensionManager().setRowMinHeightInPixels(myRow, height);
}

int
QicsCell::minHeightInChars(void) const
{
    // divide the height in pixels by the height of the font -- round down.

    return (dimensionManager().rowMinHeight(myRow)  /
	    qicsHeightOfFont(font()));
}

void
QicsCell::setMinHeightInChars(int height)
{
    dimensionManager().setRowMinHeightInChars(myRow, height);
}

////////////////////////////////////////////////////////////////////////
//////////////////  Data Model Slots  //////////////////////////////////
////////////////////////////////////////////////////////////////////////

void
QicsCell::handleModelRowInsert(int nrows, int pos)
{
    if (myRow >= pos)
	myRow += nrows;
}

void
QicsCell::handleModelColumnInsert(int ncols, int pos)
{
    if (myColumn >= pos)
	myColumn += ncols;
}

void
QicsCell::handleModelRowDelete(int nrows, int pos)
{
    if (myRow >= pos)
    {
	if  (myRow < (pos + nrows))
	    myRow = -1;
	else
	    myRow -= nrows;
    }
}

void
QicsCell::handleModelColumnDelete(int ncols, int pos)
{
    if (myColumn >= pos)
    {
	if  (myColumn < (pos + ncols))
	    myColumn = -1;
	else
	    myColumn -= ncols;
    }
}

void
QicsCell::handleOrderChanged(Qics::QicsIndexType type, QMemArray<int> visChange)
{
    if (type == Qics::RowIndex)
    {
	int new_row = visChange[myRow];
	if (new_row >= 0)
	    myRow = new_row;
	    
    }
    else
    {
	int new_col = visChange[myColumn];
	if (new_col >= 0)
	    myColumn = new_col;
	    
    }
}

void
QicsCell::changeDataModel(QicsDataModel *old_dt, QicsDataModel *)
{
    // get rid of connections to old data model
    disconnect(old_dt, 0, this, 0);

    // the new data model will already be set in our grid info object
    init();
}

