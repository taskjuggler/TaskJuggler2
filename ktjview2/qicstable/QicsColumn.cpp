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


#include <QicsColumn.h>
#include <QicsStyleManager.h>
#include <QicsDimensionManager.h>
#include <QicsDataModel.h>
#include <QicsUtil.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE
#include <QicsTable.h>
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef CREATE_OBJS_WITH_QICSTABLE
QicsColumn::QicsColumn(int column, QicsTable *table, bool follow_model) :
    QicsCellCommon(&(table->gridInfo()), table),
    myFollowModel(follow_model)
{
    setColumnIndex(column);
    init();
}
#endif

QicsColumn::QicsColumn(int column, QicsGridInfo *info, bool follow_model,
		       QObject *parent) :
    QicsCellCommon(info, parent),
    myFollowModel(follow_model)
{
    setColumnIndex(column);
    init();
}

void
QicsColumn::init(void)
{
    if (!myInfo)
	return;

    if (myFollowModel)
    {
	// connect ourself to the data model's signals

	connect(dataModel(), SIGNAL(columnsInserted(int, int)),
		this, SLOT(handleModelColumnInsert(int, int)));
	connect(dataModel(), SIGNAL(columnsDeleted(int, int)),
		this, SLOT(handleModelColumnDelete(int, int)));

	// also want to know if the data model has changed
	connect(myInfo, SIGNAL(dataModelChanged(QicsDataModel *, QicsDataModel *)),
		this, SLOT(changeDataModel(QicsDataModel *, QicsDataModel *)));
    }
    else
    {
	// connect to the column ordering object
	connect(myInfo->columnOrdering(),
		SIGNAL(orderChanged(QicsIndexType, QMemArray<int>)),
		this,
		SLOT(handleOrderChanged(QicsIndexType, QMemArray<int>)));
    }
}

void
QicsColumn::setInfo(QicsGridInfo *info)
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
QicsColumn::columnIndex(void) const
{
    if (myColumn >= 0)
	return myInfo->visualColumnIndex(myColumn);
    else
	return -1;
}

int
QicsColumn::modelColumnIndex(void) const
{
    return myColumn; 
}

void
QicsColumn::setColumnIndex(int idx)
{
    if (idx < 0)
	myColumn = -1;
    else
	myColumn = myInfo->modelColumnIndex(idx);
}

bool
QicsColumn::isValid(void) const
{
    if (dataModel())
	return ((myColumn >= 0) && (myColumn < dataModel()->numColumns()));
    else
	return true;
}

////////////////////////////////////////////////////////////////////////
////////////////////   Attribute methods   /////////////////////////////
////////////////////////////////////////////////////////////////////////

void
QicsColumn::setAttr(QicsCellStyle::QicsCellStyleProperty attr, const void *val)
{
    styleManager().setColumnProperty(myColumn, attr, val);
}

void *
QicsColumn::getAttr(QicsCellStyle::QicsCellStyleProperty attr) const
{
    return (styleManager().getColumnProperty(myColumn, attr));
}

void
QicsColumn::clearAttr(QicsCellStyle::QicsCellStyleProperty attr)
{
    styleManager().clearColumnProperty(myColumn, attr);
}

void
QicsColumn::setDMMargin(int margin)
{
    dimensionManager().setColumnMargin(myInfo->gridType(), myColumn, margin);
}

void
QicsColumn::setDMFont(const QFont &font)
{
    dimensionManager().setColumnFont(myInfo->gridType(), myColumn, font);
}


QicsDataModelColumn QicsColumn::dataValues(int first_row, int last_row) const
{
    if (dataModel() && isValid())
	return dataModel()->columnItems(myColumn, first_row, last_row);
    else
	return QicsDataModelColumn();
}

void
QicsColumn::setDataValues(QicsDataModelColumn &vals)
{
    dataModel()->setColumnItems(myColumn, vals);
}

int
QicsColumn::widthInPixels(void) const
{
    return (dimensionManager().columnWidth(myColumn));
}

void
QicsColumn::setWidthInPixels(int width)
{
    dimensionManager().setColumnWidthInPixels(myColumn, width);
}

int
QicsColumn::widthInChars(void) const
{
    // divide the width in pixels by the width of the font -- round down.

    return (dimensionManager().columnWidth(myColumn)  /
	    qicsWidthOfFont(font()));
}

void
QicsColumn::setWidthInChars(int width)
{
    dimensionManager().setColumnWidthInChars(myColumn, width);
}

int
QicsColumn::minWidthInPixels(void) const
{
    return (dimensionManager().columnMinWidth(myColumn));
}

void
QicsColumn::setMinWidthInPixels(int width)
{
    dimensionManager().setColumnMinWidthInPixels(myColumn, width);
}

int
QicsColumn::minWidthInChars(void) const
{
    // divide the width in pixels by the width of the font -- round down.

    return (dimensionManager().columnMinWidth(myColumn)  /
	    qicsWidthOfFont(font()));
}

void
QicsColumn::setMinWidthInChars(int width)
{
    dimensionManager().setColumnMinWidthInChars(myColumn, width);
}

////////////////////////////////////////////////////////////////////////
//////////////////  Data Model Slots  //////////////////////////////////
////////////////////////////////////////////////////////////////////////

void
QicsColumn::handleModelColumnInsert(int ncols, int pos)
{
    if (myColumn >= pos)
	myColumn += ncols;
}

void
QicsColumn::handleModelColumnDelete(int ncols, int pos)
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
QicsColumn::handleOrderChanged(Qics::QicsIndexType type, QMemArray<int> visChange)
{
    if (type == Qics::ColumnIndex)
    {
	int new_col = visChange[myColumn];
	if (new_col >= 0)
	    myColumn = new_col;
    }
}

void
QicsColumn::changeDataModel(QicsDataModel *old_dt, QicsDataModel *)
{
    // get rid of connections to old data model
    disconnect(old_dt, 0, this, 0);

    // the new data model will already be set in our grid info object
    init();
}
