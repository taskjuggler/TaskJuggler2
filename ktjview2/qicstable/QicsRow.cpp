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


#include <QicsRow.h>
#include <QicsStyleManager.h>
#include <QicsDimensionManager.h>
#include <QicsDataModel.h>
#include <QicsUtil.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE
#include <QicsTable.h>
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef CREATE_OBJS_WITH_QICSTABLE
QicsRow::QicsRow(int row, QicsTable *table, bool follow_model) :
    QicsCellCommon(&(table->gridInfo()), table),
    myFollowModel(follow_model)
{
    setRowIndex(row);
    init();
}
#endif

QicsRow::QicsRow(int row, QicsGridInfo *info, bool follow_model, QObject *parent) :
    QicsCellCommon(info, parent),
    myFollowModel(follow_model)
{
    setRowIndex(row);
    init();
}

void
QicsRow::init(void)
{
    if (!myInfo)
	return;

    if (myFollowModel)
    {
	// connect ourself to the data model's signals

	connect(dataModel(), SIGNAL(rowsInserted(int, int)),
		this, SLOT(handleModelRowInsert(int, int)));
	connect(dataModel(), SIGNAL(rowsDeleted(int, int)),
		this, SLOT(handleModelRowDelete(int, int)));

	// also want to know if the data model has changed
	connect(myInfo, SIGNAL(dataModelChanged(QicsDataModel *, QicsDataModel *)),
		this, SLOT(changeDataModel(QicsDataModel *, QicsDataModel *)));
    }
    else
    {
	// connect to the row ordering object
	connect(myInfo->rowOrdering(),
		SIGNAL(orderChanged(QicsIndexType, QMemArray<int>)),
		this,
		SLOT(handleOrderChanged(QicsIndexType, QMemArray<int>)));
    }
}

void
QicsRow::setInfo(QicsGridInfo *info)
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
QicsRow::rowIndex(void) const
{
    if (myRow >= 0)
	return myInfo->visualRowIndex(myRow);
    else
	return -1;
}

int
QicsRow::modelRowIndex(void) const
{
    return myRow; 
}

void
QicsRow::setRowIndex(int idx)
{
    if (idx < 0)
	myRow = -1;
    else
	myRow = myInfo->modelRowIndex(idx);
}

bool
QicsRow::isValid(void) const
{
    if (dataModel())
	return ((myRow >= 0) && (myRow < dataModel()->numRows()));
    else
	return true;
}

////////////////////////////////////////////////////////////////////////
////////////////////   Attribute methods   /////////////////////////////
////////////////////////////////////////////////////////////////////////

void
QicsRow::setAttr(QicsCellStyle::QicsCellStyleProperty attr, const void *val)
{
    styleManager().setRowProperty(myRow, attr, val);
}

void *
QicsRow::getAttr(QicsCellStyle::QicsCellStyleProperty attr) const
{
    return (styleManager().getRowProperty(myRow, attr));
}

void
QicsRow::clearAttr(QicsCellStyle::QicsCellStyleProperty attr)
{
    styleManager().clearRowProperty(myRow, attr);
}

void
QicsRow::setDMMargin(int margin)
{
    dimensionManager().setRowMargin(myInfo->gridType(), myRow, margin);
}

void
QicsRow::setDMFont(const QFont &font)
{
    dimensionManager().setRowFont(myInfo->gridType(), myRow, font);
}


QicsDataModelRow QicsRow::dataValues(int first_col, int last_col) const
{
    if (dataModel() && isValid())
	return dataModel()->rowItems(myRow, first_col, last_col);
    else
	return QicsDataModelRow();
}

void
QicsRow::setDataValues(QicsDataModelRow &vals)
{
    if (dataModel())
	dataModel()->setRowItems(myRow, vals);
}

int
QicsRow::heightInPixels(void) const
{
    return (dimensionManager().rowHeight(myRow));
}

void
QicsRow::setHeightInPixels(int height)
{
    dimensionManager().setRowHeightInPixels(myRow, height);
}

int
QicsRow::heightInChars(void) const
{
    // divide the height in pixels by the height of the font -- round down.

    return (dimensionManager().rowHeight(myRow)  /
	    qicsHeightOfFont(font()));
}

void
QicsRow::setHeightInChars(int height)
{
    dimensionManager().setRowHeightInChars(myRow, height);
}

int
QicsRow::minHeightInPixels(void) const
{
    return (dimensionManager().rowMinHeight(myRow));
}

void
QicsRow::setMinHeightInPixels(int height)
{
    dimensionManager().setRowMinHeightInPixels(myRow, height);
}

int
QicsRow::minHeightInChars(void) const
{
    // divide the height in pixels by the height of the font -- round down.

    return (dimensionManager().rowMinHeight(myRow)  /
	    qicsHeightOfFont(font()));
}

void
QicsRow::setMinHeightInChars(int height)
{
    dimensionManager().setRowMinHeightInChars(myRow, height);
}

////////////////////////////////////////////////////////////////////////
//////////////////  Data Model Slots  //////////////////////////////////
////////////////////////////////////////////////////////////////////////

void
QicsRow::handleModelRowInsert(int nrows, int pos)
{
    if (myRow >= pos)
	myRow += nrows;
}

void
QicsRow::handleModelRowDelete(int nrows, int pos)
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
QicsRow::handleOrderChanged(Qics::QicsIndexType type, QMemArray<int> visChange)
{
    if (type == Qics::RowIndex)
    {
	int new_row = visChange[myRow];
	if (new_row >= 0)
	    myRow = new_row;
    }
}

void
QicsRow::changeDataModel(QicsDataModel *old_dt, QicsDataModel *)
{
    // get rid of connections to old data model
    disconnect(old_dt, 0, this, 0);

    // the new data model will already be set in our grid info object
    init();
}
