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


#include <QicsDimensionManager.h>
#include <QicsGridInfo.h>
#include <QicsCellStyle.h>
#include <QicsUtil.h>


//////////////////////////////////////////////////////////////////////////////

QicsDimensionManager::QicsCellSetting::QicsCellSetting() :
    row(-1),
    col(-1),
    font_height(-1),
    font_width(-1),
    cell_margin(-1),
    border_width(-1)
{
}

bool
QicsDimensionManager::QicsCellSetting::isEmpty(void) const
{
    return ((font_height < 0) && (font_width < 0) &&
	    (cell_margin < 0) && (border_width < 0));
}

//////////////////////////////////////////////////////////////////////////////

QicsDimensionManager::QicsRowSetting::QicsRowSetting() :
    row(-1),
    font_height(-1),
    font_width(-1),
    cell_margin(-1),
    border_width(-1)
{
}

bool
QicsDimensionManager::QicsRowSetting::isEmpty(void) const
{
    return ((font_height < 0) && (font_width < 0) &&
	    (cell_margin < 0) && (border_width < 0));
}

//////////////////////////////////////////////////////////////////////////////

QicsDimensionManager::QicsColumnSetting::QicsColumnSetting() :
    col(-1),
    font_height(-1),
    font_width(-1),
    cell_margin(-1),
    border_width(-1)
{
}

bool
QicsDimensionManager::QicsColumnSetting::isEmpty(void) const
{
    return ((font_height < 0) && (font_width < 0) &&
	    (cell_margin < 0) && (border_width < 0));
}

//////////////////////////////////////////////////////////////////////////////

QicsDimensionManager::QicsRowHeight::QicsRowHeight() :
    height(0),
    chars(0)

{
    mode = QicsDimensionUnset;
}

QicsDimensionManager::QicsRowHeight::QicsRowHeight(const QicsRowHeight &rh) :
    height(rh.height),
    chars(rh.chars)
{
    mode = rh.mode;
}

//////////////////////////////////////////////////////////////////////////////

QicsDimensionManager::QicsColumnWidth::QicsColumnWidth() :
    width(0),
    chars(0)
{
    mode = QicsDimensionUnset;
}

QicsDimensionManager::QicsColumnWidth::QicsColumnWidth(const QicsColumnWidth &cw) :
    width(cw.width),
    chars(cw.chars)
{
    mode = cw.mode;
}

//////////////////////////////////////////////////////////////////////////////

QicsDimensionManager::QicsDefaultDimensionSetting::QicsDefaultDimensionSetting() :
    height(0),
    width(0),
    height_chars(1),
    width_chars(4),
    cell_margin(-1),
    border_width(-1)
{
    mode = QicsDimensionChar;
}

QicsDimensionManager::QicsDefaultDimensionSetting::QicsDefaultDimensionSetting(const QicsDefaultDimensionSetting &dds) :
    height(dds.height),
    width(dds.width),
    height_chars(dds.height_chars),
    width_chars(dds.width_chars),
    font_height(dds.font_height),
    font_width(dds.font_width),
    cell_margin(dds.cell_margin),
    border_width(dds.border_width)
{
    mode = dds.mode;
}

void QicsDimensionManager::QicsDefaultDimensionSetting::setFont(const QFont &fnt)
{
    font_height = qicsHeightOfFont(fnt);
    font_width = qicsWidthOfFont(fnt);

    compute();
}

void QicsDimensionManager::QicsDefaultDimensionSetting::compute(void)
{
    if (mode == QicsDimensionChar)
    {
	height = ((font_height * height_chars) + (2 * cell_margin) +
		  (2 * border_width));
	width = ((font_width * width_chars) + (2 * cell_margin) +
		 (2 * border_width));
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define SPACE_BETWEEN_LINES 3

QicsDimensionManager::QicsDimensionManager(QicsStyleManager &sm) :
    QObject(),
    myStyleManager(sm),
    myRowDM(0),
    myColumnDM(0)
{
    myOrigDefaultDimensions.cell_margin =  * static_cast<int *>
	(myStyleManager.getDefaultProperty(QicsCellStyle::CellMargin));

    myOrigDefaultDimensions.border_width =  * static_cast<int *>
	(myStyleManager.getDefaultProperty(QicsCellStyle::BorderWidth));

    const QFont &fnt = * static_cast<QFont *>
	(myStyleManager.getDefaultProperty(QicsCellStyle::Font));
    myOrigDefaultDimensions.setFont(fnt);

    myCurrentDefaultDimensions = myOrigDefaultDimensions;

    myOrigDefaultMinDimensions.setFont(fnt);
    myOrigDefaultMinDimensions.mode = QicsDimensionPixel;
    myOrigDefaultMinDimensions.height = 5;
    myOrigDefaultMinDimensions.width = 5;
    myCurrentDefaultMinDimensions = myOrigDefaultMinDimensions;
}

QicsDimensionManager::QicsDimensionManager(const QicsDimensionManager &dm,
					   QicsStyleManager &sm) :
    QObject(),
    myStyleManager(sm),
    myRowDM(0),
    myColumnDM(0)
{
    myOrigDefaultDimensions = dm.myOrigDefaultDimensions;
    myCurrentDefaultDimensions = dm.myCurrentDefaultDimensions;

    myOrigDefaultMinDimensions = dm.myOrigDefaultMinDimensions;
    myCurrentDefaultMinDimensions = myOrigDefaultMinDimensions;

	QicsCellSettingV::const_iterator iter_cell;
    for (iter_cell = dm.mySetCells.begin();
	 iter_cell != dm.mySetCells.end();
	 ++iter_cell)
    {
	mySetCells.push_back(*iter_cell);
    }

	QicsRowSettingV::const_iterator iter_rs;
    for (iter_rs = dm.mySetRows.begin();
	 iter_rs != dm.mySetRows.end();
	 ++iter_rs)
    {
	mySetRows.push_back(*iter_rs);
    }

	QicsColumnSettingV::const_iterator iter_cs;
    for (iter_cs = dm.mySetColumns.begin();
	 iter_cs != dm.mySetColumns.end();
	 ++iter_cs)
    {
	mySetColumns.push_back(*iter_cs);
    }

	QicsRowHeightPV::const_iterator iter_rh;
    for (iter_rh = dm.myRowHeights.begin();
	 iter_rh != dm.myRowHeights.end();
	 ++iter_rh)
    {
	QicsRowHeight *rh;

	if (*iter_rh)
	    rh  = new QicsRowHeight(**iter_rh);
	else
	    rh = 0;

	myRowHeights.push_back(rh);
    }


    for (iter_rh = dm.myRowMinHeights.begin();
	 iter_rh != dm.myRowMinHeights.end();
	 ++iter_rh)
    {
	QicsRowHeight *rh;

	if (*iter_rh)
	    rh  = new QicsRowHeight(**iter_rh);
	else
	    rh = 0;

	myRowMinHeights.push_back(rh);
    }

	 QicsColumnWidthPV::const_iterator iter_cw;
    for (iter_cw = dm.myColumnWidths.begin();
	 iter_cw != dm.myColumnWidths.end();
	 ++iter_cw)
    {
	QicsColumnWidth *cw;

	if (*iter_cw)
	    cw  = new QicsColumnWidth(**iter_cw);
	else
	    cw = 0;

	myColumnWidths.push_back(cw);
    }

    for (iter_cw = dm.myColumnMinWidths.begin();
	 iter_cw != dm.myColumnMinWidths.end();
	 ++iter_cw)
    {
	QicsColumnWidth *cw;

	if (*iter_cw)
	    cw  = new QicsColumnWidth(**iter_cw);
	else
	    cw = 0;

	myColumnMinWidths.push_back(cw);
    }
}

QicsDimensionManager::~QicsDimensionManager()
{
    for (QicsRowHeightPV::iterator iter_rh = myRowHeights.begin();
	 iter_rh != myRowHeights.end(); ++iter_rh)
    {
	delete(*iter_rh);
    }

    for (QicsColumnWidthPV::iterator iter_cw = myColumnWidths.begin();
	 iter_cw != myColumnWidths.end(); ++iter_cw)
    {
	delete(*iter_cw);
    }
}

void
QicsDimensionManager::setControllingRowDimensionManager(QicsDimensionManager *dm)
{
    if (myRowDM)
    {
	disconnect(myRowDM, 0, this, 0);
    }

    myRowDM = dm;

    if (myRowDM)
    {
	connect(myRowDM, SIGNAL(dimensionChanged()),
		this, SIGNAL(dimensionChanged()));
    }
}

void 
QicsDimensionManager::setControllingColumnDimensionManager(QicsDimensionManager *dm)
{
    if (myColumnDM)
    {
	disconnect(myColumnDM, 0, this, 0);
    }

    myColumnDM = dm;

    if (myColumnDM)
    {
	connect(myColumnDM, SIGNAL(dimensionChanged()),
		this, SIGNAL(dimensionChanged()));
    }
}

QicsDimensionManager::QicsGridRole
QicsDimensionManager::gridRole(QicsGridType grid_type) const
{
    if (grid_type == QicsGridInfo::RowHeaderGrid)
	return QicsRowHeaderGridRole;
    else if (grid_type == QicsGridInfo::ColumnHeaderGrid)
	return QicsColumnHeaderGridRole;
    else
	return QicsTableGridRole;
}

//////////////////////////////////////////////////////////////////////
/////////////////////  Font set methods  /////////////////////////////
//////////////////////////////////////////////////////////////////////

void
QicsDimensionManager::setDefaultFont(const QFont &fnt)
{
    myCurrentDefaultDimensions.setFont(fnt);

    computeAllRowHeights();
    computeAllColumnWidths();
}

void
QicsDimensionManager::setRowFont(QicsGridType grid_type,
				 int row, const QFont &fnt)
{
    bool changed = false;

    if (myRowDM)
    {
	myRowDM->setRowFont(grid_type, row, fnt);
    }
    else
    {
	QicsGridRole role = gridRole(grid_type);

	QicsRowSettingV::iterator iter;
	
	bool found = false;
	for (iter = mySetRows.begin(); iter != mySetRows.end(); ++iter)
	{
	    QicsRowSetting &rfs = *iter;
	    
	    if ((rfs.row == row) && (rfs.role == role))
	    {
		found = true;
		rfs.font_height = qicsHeightOfFont(fnt);
		rfs.font_width = qicsWidthOfFont(fnt);
		break;
	    }
	}

	if (!found)
	{
	    QicsRowSetting rfs;

	    rfs.role = role;
	    rfs.row = row;
	    rfs.font_height = qicsHeightOfFont(fnt);
	    rfs.font_width = qicsWidthOfFont(fnt);

	    mySetRows.push_back(rfs);
	}

	changed |= computeRowHeight(row, myRowHeights,
				    myCurrentDefaultDimensions);
    }

    changed |= computeDefaultColumnFontWidth();

    if (changed)
	emit dimensionChanged();
}

void
QicsDimensionManager::unsetRowFont(QicsGridType grid_type, int row)
{
    bool changed = false;

    if (myRowDM)
    {
	myRowDM->unsetRowFont(grid_type, row);
    }
    else
    {
	QicsGridRole role = gridRole(grid_type);

	QicsRowSettingV::iterator iter;

	bool found = false;
	for (iter = mySetRows.begin(); iter != mySetRows.end(); ++iter)
	{
	    QicsRowSetting &rfs = *iter;
	    
	    if ((rfs.row == row) && (rfs.role == role))
	    {
		rfs.font_height = -1;
		rfs.font_width = -1;

		if (rfs.isEmpty())
		    mySetRows.erase(iter);

		found = true;
		break;
	    }
	}
	
	if (found)
	{
	    changed |= computeRowHeight(row, myRowHeights,
					myCurrentDefaultDimensions);
	}
    }

    changed |= computeDefaultColumnFontWidth();

    if (changed)
	emit dimensionChanged();
}

void
QicsDimensionManager::setColumnFont(QicsGridType grid_type,
				    int col, const QFont &fnt)
{
    bool changed = false;

    if (myColumnDM)
    {
	myColumnDM->setColumnFont(grid_type, col, fnt);
    }
    else
    {
	QicsGridRole role = gridRole(grid_type);
	
	QicsColumnSettingV::iterator iter;

	bool found = false;
	for (iter = mySetColumns.begin(); iter != mySetColumns.end(); ++iter)
	{
	    QicsColumnSetting &cfs = *iter;

	    if ((cfs.col == col) && (cfs.role == role))
	    {
		found = true;
		cfs.font_width = qicsWidthOfFont(fnt);
		cfs.font_height = qicsHeightOfFont(fnt);
		break;
	    }
	}

	if (!found)
	{
	    QicsColumnSetting cfs;
	    
	    cfs.role = role;
	    cfs.col = col;
	    cfs.font_width = qicsWidthOfFont(fnt);
	    cfs.font_height = qicsHeightOfFont(fnt);

	    mySetColumns.push_back(cfs);
	}

	changed |= computeColumnWidth(col, myColumnWidths,
				      myCurrentDefaultDimensions);
    }

    changed |= computeDefaultRowFontHeight();

    if (changed)
	emit dimensionChanged();
}

void
QicsDimensionManager::unsetColumnFont(QicsGridType grid_type, int col)
{
    bool changed = false;

    if (myColumnDM)
    {
	myColumnDM->unsetColumnFont(grid_type, col);
    }
    else
    {
	QicsGridRole role = gridRole(grid_type);

	QicsColumnSettingV::iterator iter;

	bool found = false;
	for (iter = mySetColumns.begin(); iter != mySetColumns.end(); ++iter)
	{
	    QicsColumnSetting &cfs = *iter;

	    if ((cfs.col == col) && (cfs.role == role))
	    {
		cfs.font_height = -1;
		cfs.font_width = -1;

		if (cfs.isEmpty())
		    mySetColumns.erase(iter);

		found = true;
		break;
	    }
	}

	if (found)
	{
	    changed |= computeColumnWidth(col, myColumnWidths,
					  myCurrentDefaultDimensions);
	}
    }

    changed |= computeDefaultRowFontHeight();

    if (changed)
	emit dimensionChanged();
}

void
QicsDimensionManager::setCellFont(QicsGridType grid_type,
				  int row, int col, const QFont &fnt)
{
    QicsGridRole role = gridRole(grid_type);

    QicsCellSettingV::iterator iter;
    int font_height = qicsHeightOfFont(fnt);
    int font_width = qicsWidthOfFont(fnt);

    bool changed = false;

    bool found = false;

    // Search for this cell in the list
    for (iter = mySetCells.begin(); iter != mySetCells.end(); ++iter)
    {
	QicsCellSetting &setting = *iter;

	if ((setting.row == row) && (setting.col = col) &&
	    (setting.role == role))
	{
	    setting.font_height = font_height;
	    setting.font_width = font_width;

	    found = true;
	    break;
	}
    }

    // Didn't find it, must be a new cell

    QicsCellSetting setting;

    setting.role = role;
    setting.row = row;
    setting.col = col;
    setting.font_height = font_height;
    setting.font_width = font_width;

    mySetCells.push_back(setting);

    if (myRowDM)
	myRowDM->setCellFont(grid_type, row, col, fnt);
    else
	changed |= computeRowHeight(row, myRowHeights,
				    myCurrentDefaultDimensions);

    if (myColumnDM)
	myColumnDM->setCellFont(grid_type, row, col, fnt);
    else
	changed |= computeColumnWidth(col, myColumnWidths,
				      myCurrentDefaultDimensions);

    if (changed)
	emit dimensionChanged();
}

void
QicsDimensionManager::unsetCellFont(QicsGridType grid_type,
				    int row, int col)
{
    QicsGridRole role = gridRole(grid_type);

    QicsCellSettingV::iterator iter;

    bool changed = false;

    bool found = false;
    // Search for this cell in the list
    for (iter = mySetCells.begin(); iter != mySetCells.end(); ++iter)
    {
	QicsCellSetting &setting = *iter;

	if ((setting.row == row) && (setting.col = col) &&
	    (setting.role == role))
	{
	    setting.font_height = -1;
	    setting.font_width = -1;

	    if (setting.isEmpty())
		mySetCells.erase(iter);

	    found = true;
	    break;
	}
    }

    if (found)
    {
	if (myRowDM)
	    myRowDM->unsetCellFont(grid_type, row, col);
	else
	    changed |= computeRowHeight(row, myRowHeights,
					myCurrentDefaultDimensions);

	if (myColumnDM)
	    myColumnDM->unsetCellFont(grid_type, row, col);
	else
	    changed |= computeColumnWidth(col, myColumnWidths,
					  myCurrentDefaultDimensions);
    }

    if (changed)
	emit dimensionChanged();
}

//////////////////////////////////////////////////////////////////////
/////////////////// Row Height set methods ///////////////////////////
//////////////////////////////////////////////////////////////////////

void
QicsDimensionManager::setRowHeightInPixels(int row, int height)
{
    if (myRowDM)
    {
	myRowDM->setRowHeightInPixels(row, height);
    }
    else
    {
	if (row >= static_cast<int> (myRowHeights.size()))
	    myRowHeights.resize(row + 20);
	
	if (myRowHeights[row] == 0)
	    myRowHeights[row] = new QicsRowHeight();
	
	(myRowHeights[row])->mode = QicsDimensionPixel;
	(myRowHeights[row])->height = height;
	
	emit dimensionChanged();
    }
}

void
QicsDimensionManager::setRowHeightInChars(int row, int height)
{
    if (myRowDM)
    {
	myRowDM->setRowHeightInChars(row, height);
    }
    else
    {
	if (row >= static_cast<int> (myRowHeights.size()))
	    myRowHeights.resize(row + 20);

	if (myRowHeights[row] == 0)
	    myRowHeights[row] = new QicsRowHeight();

	(myRowHeights[row])->mode = QicsDimensionChar;
	(myRowHeights[row])->chars = height;

	if (computeRowHeight(row, myRowHeights, myCurrentDefaultDimensions))
	    emit dimensionChanged();
    }
}

void
QicsDimensionManager::setRowMinHeightInPixels(int row, int height)
{
    if (myRowDM)
    {
	myRowDM->setRowMinHeightInPixels(row, height);
    }
    else
    {
	if (row >= static_cast<int> (myRowMinHeights.size()))
	    myRowMinHeights.resize(row + 20);
	
	if (myRowMinHeights[row] == 0)
	    myRowMinHeights[row] = new QicsRowHeight();
	
	(myRowMinHeights[row])->mode = QicsDimensionPixel;
	(myRowMinHeights[row])->height = height;

	if (height > rowHeight(row))
	    setRowHeightInPixels(row, height);
    }
}

void
QicsDimensionManager::setRowMinHeightInChars(int row, int height)
{
    if (myRowDM)
    {
	myRowDM->setRowMinHeightInChars(row, height);
    }
    else
    {
	if (row >= static_cast<int> (myRowMinHeights.size()))
	    myRowMinHeights.resize(row + 20);

	if (myRowMinHeights[row] == 0)
	    myRowMinHeights[row] = new QicsRowHeight();

	(myRowMinHeights[row])->mode = QicsDimensionChar;
	(myRowMinHeights[row])->chars = height;

	computeRowHeight(row, myRowMinHeights, myCurrentDefaultMinDimensions);

	if (height > rowHeight(row))
	    setRowHeightInChars(row, height);
    }
}

//////////////////////////////////////////////////////////////////////
/////////////////// Column Width set methods  ////////////////////////
//////////////////////////////////////////////////////////////////////

void 
QicsDimensionManager::setColumnWidthInPixels(int col, int width)
{
    if (myColumnDM)
    {
	myColumnDM->setColumnWidthInPixels(col, width);
    }
    else
    {
	if (col >= static_cast<int> (myColumnWidths.size()))
	    myColumnWidths.resize(col + 20);

	if (myColumnWidths[col] == 0)
	    myColumnWidths[col] = new QicsColumnWidth();

	(myColumnWidths[col])->mode = QicsDimensionPixel;
	(myColumnWidths[col])->width = width;

	emit dimensionChanged();
    }
}

void
QicsDimensionManager::setColumnWidthInChars(int col, int width)
{
    if (myColumnDM)
    {
	myColumnDM->setColumnWidthInChars(col, width);
    }
    else
    {
	if (col >= static_cast<int> (myColumnWidths.size()))
	    myColumnWidths.resize(col + 20);

	if (myColumnWidths[col] == 0)
	    myColumnWidths[col] = new QicsColumnWidth();

	(myColumnWidths[col])->mode = QicsDimensionChar;
	(myColumnWidths[col])->chars = width;

	if (computeColumnWidth(col, myColumnWidths, myCurrentDefaultDimensions))
	    emit dimensionChanged();
    }
}

void 
QicsDimensionManager::setColumnMinWidthInPixels(int col, int width)
{
    if (myColumnDM)
    {
	myColumnDM->setColumnMinWidthInPixels(col, width);
    }
    else
    {
	if (col >= static_cast<int> (myColumnMinWidths.size()))
	    myColumnMinWidths.resize(col + 20);

	if (myColumnMinWidths[col] == 0)
	    myColumnMinWidths[col] = new QicsColumnWidth();

	(myColumnMinWidths[col])->mode = QicsDimensionPixel;
	(myColumnMinWidths[col])->width = width;

	if (width > columnWidth(col))
	    setColumnWidthInPixels(col, width);
    }
}

void
QicsDimensionManager::setColumnMinWidthInChars(int col, int width)
{
    if (myColumnDM)
    {
	myColumnDM->setColumnMinWidthInChars(col, width);
    }
    else
    {
	if (col >= static_cast<int> (myColumnMinWidths.size()))
	    myColumnMinWidths.resize(col + 20);

	if (myColumnMinWidths[col] == 0)
	    myColumnMinWidths[col] = new QicsColumnWidth();

	(myColumnMinWidths[col])->mode = QicsDimensionChar;
	(myColumnMinWidths[col])->chars = width;

	computeColumnWidth(col, myColumnMinWidths, myCurrentDefaultMinDimensions);

	if (width > columnWidth(col))
	    setColumnWidthInChars(col, width);
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////// Margin set methods //////////////////////////////
//////////////////////////////////////////////////////////////////////

void
QicsDimensionManager::setDefaultMargin(int margin)
{
    myCurrentDefaultDimensions.cell_margin = margin;

    computeAllRowHeights();
    computeAllColumnWidths();
}

void
QicsDimensionManager::setRowMargin(QicsGridType grid_type,
				   int row, int margin)
{
    if (myRowDM)
    {
	myRowDM->setRowMargin(grid_type, row, margin);
    }
    else
    {
	QicsGridRole role = gridRole(grid_type);

	QicsRowSettingV::iterator iter;
	
	bool found = false;
	for (iter = mySetRows.begin(); iter != mySetRows.end(); ++iter)
	{
	    QicsRowSetting &rfs = *iter;
	    
	    if ((rfs.row == row) && (rfs.role == role))
	    {
		found = true;
		rfs.cell_margin = margin;
		break;
	    }
	}

	if (!found)
	{
	    QicsRowSetting rfs;

	    rfs.role = role;
	    rfs.row = row;
	    rfs.cell_margin = margin;

	    mySetRows.push_back(rfs);
	}

	if (computeRowHeight(row, myRowHeights, myCurrentDefaultDimensions))
	    emit dimensionChanged();
    }
}

void
QicsDimensionManager::setColumnMargin(QicsGridType grid_type,
				      int col, int margin)
{
    if (myColumnDM)
    {
	myColumnDM->setColumnMargin(grid_type, col, margin);
    }
    else
    {
	QicsGridRole role = gridRole(grid_type);

	QicsColumnSettingV::iterator iter;
	
	bool found = false;
	for (iter = mySetColumns.begin(); iter != mySetColumns.end(); ++iter)
	{
	    QicsColumnSetting &rfs = *iter;
	    
	    if ((rfs.col == col) && (rfs.role == role))
	    {
		found = true;
		rfs.cell_margin = margin;
		break;
	    }
	}

	if (!found)
	{
	    QicsColumnSetting rfs;

	    rfs.role = role;
	    rfs.col = col;
	    rfs.cell_margin = margin;

	    mySetColumns.push_back(rfs);
	}

	if (computeColumnWidth(col, myColumnWidths, myCurrentDefaultDimensions))
	    emit dimensionChanged();
    }
}

void
QicsDimensionManager::setCellMargin(QicsGridType grid_type,
				    int row, int col, int margin)
{
    QicsGridRole role = gridRole(grid_type);

    QicsCellSettingV::iterator iter;

    bool changed = false;

    bool found = false;

    // Search for this cell in the list
    for (iter = mySetCells.begin(); iter != mySetCells.end(); ++iter)
    {
	QicsCellSetting setting = *iter;

	if ((setting.row == row) && (setting.col = col) &&
	    (setting.role == role))
	{
	    setting.cell_margin = margin;

	    found = true;
	    break;
	}
    }

    // Didn't find it, must be a new cell

    QicsCellSetting setting;

    setting.role = role;
    setting.row = row;
    setting.col = col;
    setting.cell_margin = margin;

    mySetCells.push_back(setting);

    if (myRowDM)
	myRowDM->setCellMargin(grid_type, row, col, margin);
    else
	changed |= computeRowHeight(row, myRowHeights,
				    myCurrentDefaultDimensions);

    if (myColumnDM)
	myColumnDM->setCellMargin(grid_type, row, col, margin);
    else
	changed |= computeColumnWidth(col, myColumnWidths,
				      myCurrentDefaultDimensions);

    if (changed)
	emit dimensionChanged();
}

/////////////////////////////////////////////////////////////////////////
//////////////////// Border width  methods //////////////////////////////
/////////////////////////////////////////////////////////////////////////

void
QicsDimensionManager::setDefaultBorderWidth(int bw)
{
    myCurrentDefaultDimensions.border_width = bw;

    computeAllRowHeights();
    computeAllColumnWidths();
}

void
QicsDimensionManager::setRowBorderWidth(QicsGridType grid_type,
					int row, int bw)
{
    if (myRowDM)
    {
	myRowDM->setRowBorderWidth(grid_type, row, bw);
    }
    else
    {
	QicsGridRole role = gridRole(grid_type);

	QicsRowSettingV::iterator iter;
	
	bool found = false;
	for (iter = mySetRows.begin(); iter != mySetRows.end(); ++iter)
	{
	    QicsRowSetting &rfs = *iter;
	    
	    if ((rfs.row == row) && (rfs.role == role))
	    {
		found = true;
		rfs.border_width = bw;
		break;
	    }
	}

	if (!found)
	{
	    QicsRowSetting rfs;

	    rfs.role = role;
	    rfs.row = row;
	    rfs.border_width = bw;

	    mySetRows.push_back(rfs);
	}

	if (computeRowHeight(row, myRowHeights, myCurrentDefaultDimensions))
	    emit dimensionChanged();
    }
}

void
QicsDimensionManager::setColumnBorderWidth(QicsGridType grid_type,
					   int col, int bw)
{
    if (myColumnDM)
    {
	myColumnDM->setColumnBorderWidth(grid_type, col, bw);
    }
    else
    {
	QicsGridRole role = gridRole(grid_type);

	QicsColumnSettingV::iterator iter;
	
	bool found = false;
	for (iter = mySetColumns.begin(); iter != mySetColumns.end(); ++iter)
	{
	    QicsColumnSetting &rfs = *iter;
	    
	    if ((rfs.col == col) && (rfs.role == role))
	    {
		found = true;
		rfs.border_width = bw;
		break;
	    }
	}

	if (!found)
	{
	    QicsColumnSetting rfs;

	    rfs.role = role;
	    rfs.col = col;
	    rfs.border_width = bw;

	    mySetColumns.push_back(rfs);
	}

	if (computeColumnWidth(col, myColumnWidths, myCurrentDefaultDimensions))
	    emit dimensionChanged();
    }
}

void
QicsDimensionManager::setCellBorderWidth(QicsGridType grid_type,
					 int row, int col, int bw)
{
    QicsGridRole role = gridRole(grid_type);

    QicsCellSettingV::iterator iter;

    bool changed = false;

    bool found = false;

    // Search for this cell in the list
    for (iter = mySetCells.begin(); iter != mySetCells.end(); ++iter)
    {
	QicsCellSetting setting = *iter;

	if ((setting.row == row) && (setting.col = col) &&
	    (setting.role == role))
	{
	    setting.border_width = bw;

	    found = true;
	    break;
	}
    }

    // Didn't find it, must be a new cell

    QicsCellSetting setting;

    setting.role = role;
    setting.row = row;
    setting.col = col;
    setting.border_width = bw;

    mySetCells.push_back(setting);

    if (myRowDM)
	myRowDM->setCellBorderWidth(grid_type, row, col, bw);
    else
	changed |= computeRowHeight(row, myRowHeights,
				    myCurrentDefaultDimensions);

    if (myColumnDM)
	myColumnDM->setCellBorderWidth(grid_type, row, col, bw);
    else
	changed |= computeColumnWidth(col, myColumnWidths,
				      myCurrentDefaultDimensions);

    if (changed)
	emit dimensionChanged();
}

////////////////////////////////////////////////////////////////////
/////////////////  Height/Width retrieval methods  /////////////////
////////////////////////////////////////////////////////////////////

int
QicsDimensionManager::rowHeight(int row) const
{
    if (myRowDM)
	return (myRowDM->rowHeight(row));

    // Check for overridden row height
    int orh = overriddenRowHeight(row);
    if (orh >= 0)
	return orh;

    QicsRowHeight *rh;
    int height;

    if (row < static_cast<int> (myRowHeights.size()))
	rh = myRowHeights[row];
    else
	rh = 0;

    if (rh)
    {
	height = rh->height;
    }
    else
    {
	height = myCurrentDefaultDimensions.height;
    }

    return height;
}

int
QicsDimensionManager::columnWidth(int col) const
{
    if (myColumnDM)
	return (myColumnDM->columnWidth(col));

    // Check for overridden column width
    int ocw = overriddenColumnWidth(col);
    if (ocw >= 0)
	return ocw;

    QicsColumnWidth *cw;
    int width;

    if (col < static_cast<int> (myColumnWidths.size()))
	cw = myColumnWidths[col];
    else
	cw = 0;

    if (cw)
	width = cw->width;
    else
	width = myCurrentDefaultDimensions.width;

    return width;
}

int
QicsDimensionManager::rowMinHeight(int row) const
{
    if (myRowDM)
	return (myRowDM->rowMinHeight(row));

    if (row < static_cast<int> (myRowMinHeights.size()))
	return ((myRowMinHeights[row])->height);

    return (myCurrentDefaultMinDimensions.height);
}

int
QicsDimensionManager::columnMinWidth(int col) const
{
    if (myColumnDM)
	return (myColumnDM->columnMinWidth(col));

    if (col < static_cast<int> (myColumnMinWidths.size()))
	return ((myColumnMinWidths[col])->width);

    return (myCurrentDefaultMinDimensions.width);
}

int
QicsDimensionManager::regionHeight(const QicsRegion &region) const
{
    int height = 0;
    int i;

    for (i = region.startRow(); i <= region.endRow(); ++i)
    {
	height += rowHeight(i);
    }

    int hlw = * static_cast<int *>
	(myStyleManager.getGridProperty(QicsGridStyle::HorizontalGridLineWidth));

    height += (hlw * (region.endRow() - region.startRow()));

    return height;
}

int
QicsDimensionManager::regionWidth(const QicsRegion &region) const
{
    int width = 0;
    int j;

    for (j = region.startColumn(); j <= region.endColumn(); ++j)
    {
	width += columnWidth(j);
    }

    int vlw = * static_cast<int *>
	(myStyleManager.getGridProperty(QicsGridStyle::VerticalGridLineWidth));

    width += (vlw * (region.endColumn() - region.startColumn()));

    return width;
}

void
QicsDimensionManager::overrideRowHeight(int row, int height)
{
    // Search for existing override

    QicsOverrideSettingL::iterator iter;

    for (iter = myRowOverrides.begin(); iter != myRowOverrides.end(); ++iter)
    {
	QicsOverrideSetting &os = *iter;

	if (os.index() == row)
	{
	    os.setValue(height);
	    emit dimensionChanged();
	    return;
	}
    }

    // Add the new override

    myRowOverrides.push_front(QicsOverrideSetting(row, height));
    emit dimensionChanged();
}

void
QicsDimensionManager::overrideColumnWidth(int col, int width)
{
    // Search for existing override

    QicsOverrideSettingL::iterator iter;

    for (iter = myColumnOverrides.begin(); iter != myColumnOverrides.end(); ++iter)
    {
	QicsOverrideSetting &os = *iter;

	if (os.index() == col)
	{
	    os.setValue(width);
	    emit dimensionChanged();
	    return;
	}
    }

    // Add the new override

    myColumnOverrides.push_front(QicsOverrideSetting(col, width));
    emit dimensionChanged();
}

void
QicsDimensionManager::resetRowHeight(int row)
{
    // Search for existing override

    QicsOverrideSettingL::iterator iter;

    for (iter = myRowOverrides.begin(); iter != myRowOverrides.end(); ++iter)
    {
	QicsOverrideSetting &os = *iter;

	if (os.index() == row)
	{
	    myRowOverrides.erase(iter);
	    emit dimensionChanged();
	    return;
	}
    }
}

void
QicsDimensionManager::resetColumnWidth(int col)
{
    // Search for existing override

    QicsOverrideSettingL::iterator iter;

    for (iter = myColumnOverrides.begin(); iter != myColumnOverrides.end(); ++iter)
    {
	QicsOverrideSetting &os = *iter;

	if (os.index() == col)
	{
	    myColumnOverrides.erase(iter);
	    emit dimensionChanged();
	    return;
	}
    }
}

int
QicsDimensionManager::overriddenRowHeight(int row) const
{
    // Search for existing override

    QicsOverrideSettingL::const_iterator iter;

    for (iter = myRowOverrides.begin(); iter != myRowOverrides.end(); ++iter)
    {
	const QicsOverrideSetting &os = *iter;

	if (os.index() == row)
	    return os.value();
    }

    return -1;
}

int
QicsDimensionManager::overriddenColumnWidth(int col) const
{
    // Search for existing override

    QicsOverrideSettingL::const_iterator iter;

    for (iter = myColumnOverrides.begin(); iter != myColumnOverrides.end(); ++iter)
    {
	const QicsOverrideSetting &os = *iter;

	if (os.index() == col)
	    return os.value();
    }

    return -1;
}

////////////////////////////////////////////////////////////////////
///////////////   Protected computation methods   //////////////////
////////////////////////////////////////////////////////////////////

bool
QicsDimensionManager::computeRowHeight(int row, QicsRowHeightPV &row_heights,
				       QicsDefaultDimensionSetting &default_dims)
{
    QicsRowHeight *rh;

    if (row < static_cast<int> (row_heights.size()))
    {
	rh = row_heights[row];
    }
    else
	rh = 0;

    QicsDimensionMode mode;
    int chars;

    if (rh && (rh->mode != QicsDimensionUnset))
	mode = rh->mode;
    else
	mode = default_dims.mode;

    // We don't need to do anything if we are currently set by pixel
    if (mode == QicsDimensionPixel)
	return false;

    if (rh && (rh->chars > 0))
	chars = rh->chars;
    else
	chars = default_dims.height_chars;

    int row_height = ((chars * default_dims.font_height) +
		      ((chars - 1) * SPACE_BETWEEN_LINES) +
		      (2 * default_dims.cell_margin) +
		      (2 * default_dims.border_width));
   
    // Search for an explicit setting for this row
    for (QicsRowSettingV::iterator iter_rs = mySetRows.begin();
	 iter_rs != mySetRows.end(); ++iter_rs)
    {
	QicsRowSetting &setting = *iter_rs;

	if (setting.row == row)
	{
	    int font_height;
	    int cell_margin;
	    int border_width;

	    if (setting.font_height > 0)
		font_height = setting.font_height;
	    else
		font_height = default_dims.font_height;

	    if (setting.cell_margin >= 0)
		cell_margin = setting.cell_margin;
	    else
		cell_margin = default_dims.cell_margin;

	    if (setting.border_width >= 0)
		border_width = setting.border_width;
	    else
		border_width = default_dims.border_width;

	    int height = ((chars * font_height) + 
			  ((chars - 1) * SPACE_BETWEEN_LINES) +
			  (2 * cell_margin) +
			  (2 * border_width));

	    row_height = QICS_MAX(row_height, height);
	}
    }

    // Search for an explicit setting for any cells in this row
    for (QicsCellSettingV::iterator iter_cell = mySetCells.begin();
	 iter_cell != mySetCells.end(); ++iter_cell)
    {
	QicsCellSetting &setting = *iter_cell;

	if (setting.row == row)
	{
	    int font_height;
	    int cell_margin;
	    int border_width;

	    if (setting.font_height > 0)
		font_height = setting.font_height;
	    else
		font_height = default_dims.font_height;

	    if (setting.cell_margin >= 0)
		cell_margin = setting.cell_margin;
	    else
		cell_margin = default_dims.cell_margin;

	    if (setting.border_width >= 0)
		border_width = setting.border_width;
	    else
		border_width = default_dims.border_width;

	    int height = ((chars * font_height) +
			  ((chars - 1) * SPACE_BETWEEN_LINES) +
			  (2 * cell_margin) +
			  (2 * border_width));

	    row_height = QICS_MAX(row_height, height);
	}
    }

    // If there was no explicit setting for this row, and the row
    // height is the same as the default row height, we don't need
    // to do anything

    if (!rh && (row_height == default_dims.height))
	return false;

    // Otherwise, we store the new row height.

    if (row >= static_cast<int> (row_heights.size()))
	row_heights.resize(row + 20);

    if (row_heights[row] == 0)
	row_heights[row] = new QicsRowHeight();

    if ((row_heights[row])->height == row_height)
	return false;
    else
    {
	(row_heights[row])->height = row_height;
	return true;
    }
}

bool
QicsDimensionManager::computeColumnWidth(int col, QicsColumnWidthPV &col_widths,
				       QicsDefaultDimensionSetting &default_dims)
{
    QicsColumnWidth *cw;

    if (col < static_cast<int> (col_widths.size()))
    {
	cw = col_widths[col];
    }
    else
	cw = 0;

    QicsDimensionMode mode;
    int chars;

    if (cw && (cw->mode != QicsDimensionUnset))
	mode = cw->mode;
    else
	mode = default_dims.mode;

    // We don't need to do anything if we are currently set by pixel
    if (mode == QicsDimensionPixel)
	return false;

    if (cw && (cw->chars > 0))
	chars = cw->chars;
    else
	chars = default_dims.width_chars;

    int col_width = ((chars * default_dims.font_width) +
		      (2 * default_dims.cell_margin) +
			  (2 * default_dims.border_width));
   
    // Search for an explicit setting for this column
    for (QicsColumnSettingV::iterator iter_cs = mySetColumns.begin();
	 iter_cs != mySetColumns.end(); ++iter_cs)
    {
	QicsColumnSetting &setting = *iter_cs;

	if (setting.col == col)
	{
	    int font_width;
	    int cell_margin;
	    int border_width;

	    if (setting.font_width > 0)
		font_width = setting.font_width;
	    else
		font_width = default_dims.font_width;

	    if (setting.cell_margin >= 0)
		cell_margin = setting.cell_margin;
	    else
		cell_margin = default_dims.cell_margin;

	    if (setting.border_width >= 0)
		border_width = setting.border_width;
	    else
		border_width = default_dims.border_width;

	    int width = ((chars * font_width) + (2 * cell_margin) +
			 (2 * border_width));

	    col_width = QICS_MAX(col_width, width);
	}
    }

    // Search for an explicit setting for any cells in this column
    for (QicsCellSettingV::iterator iter = mySetCells.begin();
	 iter != mySetCells.end(); ++iter)
    {
	QicsCellSetting &setting = *iter;

	if (setting.col == col)
	{
	    int font_width;
	    int cell_margin;
	    int border_width;

	    if (setting.font_width > 0)
		font_width = setting.font_width;
	    else
		font_width = default_dims.font_width;

	    if (setting.cell_margin >= 0)
		cell_margin = setting.cell_margin;
	    else
		cell_margin = default_dims.cell_margin;

	    if (setting.border_width >= 0)
		border_width = setting.border_width;
	    else
		border_width = default_dims.border_width;

	    int width = ((chars * font_width) + (2 * cell_margin) +
			 (2 * border_width));

	    col_width = QICS_MAX(col_width, width);
	}
    }

    // If the max width is the default column width, we don't need
    // to do anything
    if (col_width == default_dims.width)
	return false;

    if (col >= static_cast<int> (col_widths.size()))
	col_widths.resize(col + 20);

    if (col_widths[col] == 0)
	col_widths[col] = new QicsColumnWidth();

    if ((col_widths[col])->width == col_width)
	return false;
    else
    {
	(col_widths[col])->width = col_width;
	return true;
    }
}

bool
QicsDimensionManager::computeDefaultRowFontHeight(void)
{
    int font_height = 0;

    for (QicsColumnSettingV::iterator iter = mySetColumns.begin();
	 iter != mySetColumns.end(); ++iter)
    {
	QicsColumnSetting &setting = *iter;

	font_height = QICS_MAX(font_height, setting.font_height);
    }

    int old_height = myCurrentDefaultDimensions.height;

    myCurrentDefaultDimensions.font_height = font_height;
    myCurrentDefaultDimensions.compute();

    myCurrentDefaultMinDimensions.font_height = font_height;
    myCurrentDefaultMinDimensions.compute();

    return (old_height != myCurrentDefaultDimensions.height);
}

bool
QicsDimensionManager::computeDefaultColumnFontWidth(void)
{
    int font_width = 0;

    for (QicsRowSettingV::iterator iter = mySetRows.begin();
	 iter != mySetRows.end(); ++iter)
    {
	QicsRowSetting &setting = *iter;


	font_width = QICS_MAX(font_width, setting.font_width);
    }

    int old_width = myCurrentDefaultDimensions.width;

    myCurrentDefaultDimensions.font_width = font_width;
    myCurrentDefaultDimensions.compute();

    myCurrentDefaultMinDimensions.font_width = font_width;
    myCurrentDefaultMinDimensions.compute();

    return (old_width != myCurrentDefaultDimensions.width);
}

bool
QicsDimensionManager::computeAllRowHeights(void)
{
    bool changed = false;
	int i;

    for (i = 0; i < static_cast<int> (myRowHeights.size()); ++i)
    {
	changed |= computeRowHeight(i, myRowHeights,
				    myCurrentDefaultDimensions);
    }

    for (i = 0; i < static_cast<int> (myRowMinHeights.size()); ++i)
    {
	computeRowHeight(i, myRowMinHeights, myCurrentDefaultMinDimensions);
    }

    return changed;
}

bool
QicsDimensionManager::computeAllColumnWidths(void)
{
    bool changed = false;
	int i;

    for (i = 0; i < static_cast<int> (myColumnWidths.size()); ++i)
    {
	changed |= computeColumnWidth(i, myColumnWidths,
				      myCurrentDefaultDimensions);
    }

    for (i = 0; i < static_cast<int> (myColumnMinWidths.size()); ++i)
    {
	computeColumnWidth(i, myColumnMinWidths,
			   myCurrentDefaultMinDimensions);
    }

    return changed;
}

void 
QicsDimensionManager::dumpWidths(void) const
{
    qDebug("Default Column Width:  %d\n", myCurrentDefaultDimensions.width);

    for (int i = 0; i < static_cast<int> (myColumnWidths.size()); ++i)
    {
	if (myColumnWidths[i])
	{
	    qDebug("col %d:  %d\n", i, (myColumnWidths[i])->width);
	}
    }
}
 
void 
QicsDimensionManager::dumpHeights(void) const
{
    qDebug("Default Row Height:  %d\n", myCurrentDefaultDimensions.height);

    for (int i = 0; i < static_cast<int> (myRowHeights.size()); ++i)
    {
	if (myRowHeights[i])
	{
	    qDebug("col %d:  %d\n", i, (myRowHeights[i])->height);
	}
    }
}

//////////////////////////////////////////////////////////////////////
/////////////////////  Resizing Slots  ///////////////////////////////
//////////////////////////////////////////////////////////////////////

void
QicsDimensionManager::insertRows(int num, int start_position)
{
    // If another dimension manager is handling rows,
    // we don't need to do anything.
    if (myRowDM)
	return;

    // make sure we are in bounds..
    // and no one is trying to clobber us with a bad number of rows.
    if ((start_position < 0) || (num <= 0))
        return;

    // First, we do the row heights

    if (start_position < static_cast<int> (myRowHeights.size()))
    {
        QicsRowHeightPV::iterator pos = myRowHeights.begin();
        pos += start_position;

        myRowHeights.insert(pos, static_cast<size_t> (num), 0);
    }

    // Next, any row settings

    for (QicsRowSettingV::iterator iter_rs = mySetRows.begin();
	 iter_rs != mySetRows.end(); ++iter_rs)
    {
	QicsRowSetting &setting = *iter_rs;

	if (setting.row >= start_position)
	    setting.row += num;
    }

    // Finally, any cell settings

    for (QicsCellSettingV::iterator iter_cell = mySetCells.begin();
	 iter_cell != mySetCells.end(); ++iter_cell)
    {
	QicsCellSetting &setting = *iter_cell;

	if (setting.row >= start_position)
	    setting.row += num;
    }
}

void
QicsDimensionManager::insertColumns(int num, int start_position)
{
    // If another dimension manager is handling columns,
    // we don't need to do anything.
    if (myColumnDM)
	return;

    // make sure we are in bounds..
    // and no one is trying to clobber us with a bad number of columns.
    if ((start_position < 0) || (num <= 0))
        return;

    // First, we do the column widths

    if (start_position < static_cast<int> (myColumnWidths.size()))
    {
        QicsColumnWidthPV::iterator pos = myColumnWidths.begin();
        pos += start_position;

        myColumnWidths.insert(pos, static_cast<size_t> (num), 0);
    }

    // Next, any column settings

    for (QicsColumnSettingV::iterator iter_cs = mySetColumns.begin();
	 iter_cs != mySetColumns.end(); ++iter_cs)
    {
	QicsColumnSetting &setting = *iter_cs;

	if (setting.col >= start_position)
	    setting.col += num;
    }

    // Finally, any cell settings

    for (QicsCellSettingV::iterator iter_cell = mySetCells.begin();
	 iter_cell != mySetCells.end(); ++iter_cell)
    {
	QicsCellSetting &setting = *iter_cell;

	if (setting.col >= start_position)
	    setting.col += num;
    }
}

void
QicsDimensionManager::deleteRows(int num, int start_position)
{
    // If another dimension manager is handling rows,
    // we don't need to do anything.
    if (myRowDM)
	return;

    // make sure we are in bounds..
    // and no one is trying to clobber us with a bad number of rows.
    if ((start_position < 0) || (num <= 0))
        return;

    // First, we do the row heights

    if (start_position < static_cast<int> (myRowHeights.size()))
    {
        QicsRowHeightPV::iterator start_pos = myRowHeights.begin();
        start_pos += start_position;

        QicsRowHeightPV::iterator end_pos;
        if ((start_position + num) >=
            static_cast<int> (myRowHeights.size()))
        {
            end_pos = myRowHeights.end();
        }
        else
        {
            end_pos= myRowHeights.begin();
            end_pos += (start_position + num);
        }

        myRowHeights.erase(start_pos, end_pos);
    }

    // Next, any row settings

    {
	int last = start_position + num;
	QicsRowSettingV::iterator iter = mySetRows.begin();

	while (iter != mySetRows.end())
	{
	    QicsRowSetting &setting = *iter;

	    if (setting.row >= start_position)
	    {
		if (setting.row < last)
		{
		    mySetRows.erase(iter);
		}
		else
		{
		    setting.row -= num;
		    ++iter;
		}
	    }
	    else
	    {
		++iter;
	    }
	}
    }

    // Finally, any cell settings

    {
	int last = start_position + num;
	QicsCellSettingV::iterator iter = mySetCells.begin();

	while (iter != mySetCells.end())
	{
	    QicsCellSetting &setting = *iter;

	    if (setting.row >= start_position)
	    {
		if (setting.row < last)
		{
		    mySetCells.erase(iter);
		}
		else
		{
		    setting.row -= num;
		    ++iter;
		}
	    }
	    else
	    {
		++iter;
	    }
	}
    }
}

void
QicsDimensionManager::deleteColumns(int num, int start_position)
{
    // If another dimension manager is handling columns,
    // we don't need to do anything.
    if (myColumnDM)
	return;

    // make sure we are in bounds..
    // and no one is trying to clobber us with a bad number of columns.
    if ((start_position < 0) || (num <= 0))
        return;

    // First, we do the column widths

    if (start_position < static_cast<int> (myColumnWidths.size()))
    {
        QicsColumnWidthPV::iterator start_pos = myColumnWidths.begin();
        start_pos += start_position;

        QicsColumnWidthPV::iterator end_pos;
        if ((start_position + num) >=
            static_cast<int> (myColumnWidths.size()))
        {
            end_pos = myColumnWidths.end();
        }
        else
        {
            end_pos= myColumnWidths.begin();
            end_pos += (start_position + num);
        }

        myColumnWidths.erase(start_pos, end_pos);
    }

    // Next, any column settings

    {
	int last = start_position + num;
	QicsColumnSettingV::iterator iter = mySetColumns.begin();

	while (iter != mySetColumns.end())
	{
	    QicsColumnSetting &setting = *iter;

	    if (setting.col >= start_position)
	    {
		if (setting.col < last)
		{
		    mySetColumns.erase(iter);
		}
		else
		{
		    setting.col -= num;
		    ++iter;
		}
	    }
	    else
	    {
		++iter;
	    }
	}
    }

    // Finally, any cell settings

    {
	int last = start_position + num;
	QicsCellSettingV::iterator iter = mySetCells.begin();

	while (iter != mySetCells.end())
	{
	    QicsCellSetting &setting = *iter;

	    if (setting.col >= start_position)
	    {
		if (setting.col < last)
		{
		    mySetCells.erase(iter);
		}
		else
		{
		    setting.col -= num;
		    ++iter;
		}
	    }
	    else
	    {
		++iter;
	    }
	}
    }
}

#include "QicsDimensionManager.moc"
