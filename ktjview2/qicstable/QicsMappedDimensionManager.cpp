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


#include <QicsGridInfo.h>

#include <QicsDimensionManager.h>
#include <QicsMappedDimensionManager.h>

QicsMappedDimensionManager::QicsMappedDimensionManager(QicsDimensionManager *_dm,
						       QicsGridInfo *_gi) :
	myDM(_dm),
	myInfo(_gi)
{
}

QicsMappedDimensionManager::~QicsMappedDimensionManager()
{
}

void QicsMappedDimensionManager::setDefaultFont(const QFont &fnt)
{
	myDM->setDefaultFont(fnt);
}

void QicsMappedDimensionManager::setRowFont(Qics::QicsGridType grid_type, int row,
					    const QFont &fnt)
{
	myDM->setRowFont(grid_type, myInfo->modelRowIndex(row), fnt);
}

void QicsMappedDimensionManager::unsetRowFont(Qics::QicsGridType grid_type, int row)
{
	myDM->unsetRowFont(grid_type, myInfo->modelRowIndex(row));
}

void QicsMappedDimensionManager::setColumnFont(Qics::QicsGridType grid_type, int col,
					       const QFont &fnt)
{
	myDM->setColumnFont(grid_type, myInfo->modelColumnIndex(col), fnt);
}

void QicsMappedDimensionManager::unsetColumnFont(Qics::QicsGridType grid_type, int col)
{
	myDM->unsetColumnFont(grid_type, myInfo->modelColumnIndex(col));
}

void QicsMappedDimensionManager::setCellFont(Qics::QicsGridType grid_type,
					     int row, int col,
					     const QFont &fnt)
{
	myDM->setCellFont(grid_type, myInfo->modelRowIndex(row),
			  myInfo->modelColumnIndex(col), fnt);
}

void QicsMappedDimensionManager::unsetCellFont(Qics::QicsGridType grid_type,
					       int row, int col)
{
	myDM->unsetCellFont(grid_type, myInfo->modelRowIndex(row),
			    myInfo->modelColumnIndex(col));
}

void QicsMappedDimensionManager::setRowHeightInPixels(int row, int height)
{
	myDM->setRowHeightInPixels(myInfo->modelRowIndex(row), height);
}

void QicsMappedDimensionManager::setRowHeightInChars(int row, int height)
{
	myDM->setRowHeightInChars(myInfo->modelRowIndex(row), height);
}

void QicsMappedDimensionManager::setColumnWidthInPixels(int col, int width)
{
	myDM->setColumnWidthInPixels(myInfo->modelColumnIndex(col), width);
}

void QicsMappedDimensionManager::setColumnWidthInChars(int col, int width)
{
	myDM->setColumnWidthInChars(myInfo->modelColumnIndex(col), width);
}

int QicsMappedDimensionManager::rowHeight(int row) const
{
	return myDM->rowHeight(myInfo->modelRowIndex(row));
}

int QicsMappedDimensionManager::columnWidth(int col) const
{
	return myDM->columnWidth(myInfo->modelColumnIndex(col));
}

void QicsMappedDimensionManager::setRowMinHeightInPixels(int row, int height)
{
	myDM->setRowMinHeightInPixels(myInfo->modelRowIndex(row), height);
}

void QicsMappedDimensionManager::setRowMinHeightInChars(int row, int height)
{
	myDM->setRowMinHeightInChars(myInfo->modelRowIndex(row), height);
}

void QicsMappedDimensionManager::setColumnMinWidthInPixels(int col, int width)
{
	myDM->setColumnMinWidthInPixels(myInfo->modelColumnIndex(col), width);
}

void QicsMappedDimensionManager::setColumnMinWidthInChars(int col, int width)
{
	myDM->setColumnMinWidthInChars(myInfo->modelColumnIndex(col), width);
}

int QicsMappedDimensionManager::rowMinHeight(int row) const
{
	return myDM->rowMinHeight(myInfo->modelRowIndex(row));
}

int QicsMappedDimensionManager::columnMinWidth(int col) const
{
	return myDM->columnMinWidth(myInfo->modelColumnIndex(col));
}

void QicsMappedDimensionManager::setDefaultMargin(int margin)
{
	myDM->setDefaultMargin(margin);
}

void QicsMappedDimensionManager::setRowMargin(Qics::QicsGridType grid_type,
					      int row, int margin)
{
	myDM->setRowMargin(grid_type, myInfo->modelRowIndex(row), margin);
}

void QicsMappedDimensionManager::setColumnMargin(Qics::QicsGridType grid_type,
						 int col, int margin)
{
	myDM->setColumnMargin(grid_type, myInfo->modelColumnIndex(col), margin);
}

void QicsMappedDimensionManager::setCellMargin(Qics::QicsGridType grid_type,
					       int row, int col, int margin)
{
	myDM->setCellMargin(grid_type, myInfo->modelRowIndex(row),
			    myInfo->modelColumnIndex(col), margin);
}

void QicsMappedDimensionManager::setDefaultBorderWidth(int bw)
{
	myDM->setDefaultBorderWidth(bw);
}

void QicsMappedDimensionManager::setRowBorderWidth(Qics::QicsGridType grid_type,
						   int row, int bw)
{
	myDM->setRowBorderWidth(grid_type, myInfo->modelRowIndex(row), bw);
}

void QicsMappedDimensionManager::setColumnBorderWidth(Qics::QicsGridType grid_type,
						      int col, int bw)
{
	myDM->setColumnBorderWidth(grid_type, myInfo->modelColumnIndex(col), bw);
}

void QicsMappedDimensionManager::setCellBorderWidth(Qics::QicsGridType grid_type,
						    int row, int col, int bw)
{
	myDM->setCellBorderWidth(grid_type, myInfo->modelRowIndex(row),
				 myInfo->modelColumnIndex(col), bw);
}

int QicsMappedDimensionManager::regionHeight(const QicsRegion &region) const
{
    int height = 0;
    int i;

    for (i = region.startRow(); i <= region.endRow(); ++i)
    {
	height += myDM->rowHeight(myInfo->modelRowIndex(i));
    }

    int hlw = * static_cast<int *>
	(myInfo->styleManager()->getGridProperty(QicsGridStyle::HorizontalGridLineWidth));

    height += (hlw * (region.endRow() - region.startRow()));

    return height;
}

int QicsMappedDimensionManager::regionWidth(const QicsRegion &region) const
{
    int width = 0;
    int j;

    for (j = region.startColumn(); j <= region.endColumn(); ++j)
    {
	width += myDM->columnWidth(myInfo->modelColumnIndex(j));
    }

    int vlw = * static_cast<int *>
	(myInfo->styleManager()->getGridProperty(QicsGridStyle::VerticalGridLineWidth));

    width += (vlw * (region.endColumn() - region.startColumn()));

    return width;
}
