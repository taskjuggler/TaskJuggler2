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


#include <QicsGrid.h>

#include <QicsDataModel.h>
#include <QicsDataItem.h>
#include <QicsCellDisplay.h>
#include <QicsDimensionManager.h>
#include <QicsMappedDimensionManager.h>
#include <QicsSpanManager.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE 
#undef CREATE_OBJS_WITH_QICSTABLE
#endif
#include <QicsCell.h>
#include <QicsMainGrid.h>

#include <qpainter.h>
#include <qpaintdevicemetrics.h> 
#include <qdrawutil.h>

#include "QicsUtil.h"


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

QicsGrid::QicsGrid(QicsGridInfo &info)
    :myInfo(info)
{
    myMainGrid = new QicsMainGrid(&info);
    myCell = new QicsCell(-1, -1, &info, false, 0);
}

QicsGrid::~QicsGrid()
{
    delete myCell;
}

///////////////////////////////////////////////////////////////////////
////////////// Viewport related methods ///////////////////////////////
///////////////////////////////////////////////////////////////////////

QicsRegion
QicsGrid::viewport(void) const
{
    if (myViewport.isValid())
	return myViewport;
    else
	return (myMainGrid->viewport());
}

void
QicsGrid::setViewport(const QicsRegion &reg)
{
    myViewport = reg;
}

// computes the intersection of the data model dimensions and the
// viewport dimensions
QicsRegion
QicsGrid::currentViewport(void) const
{
    if (dataModel())
    {
	int endRow, endCol;

	QicsRegion vp = viewport();

	int lastRow = dataModel()->lastRow();
	int lastCol = dataModel()->lastColumn();

	if (vp.endRow() > lastRow)
	    endRow = lastRow;
	else
	    endRow = vp.endRow();

	if (vp.endColumn() > lastCol)
	    endCol = lastCol;
	else
	    endCol = vp.endColumn();

	return QicsRegion(vp.startRow(), vp.startColumn(),
			  endRow, endCol);
    }
    else
	// return full viewport if no data model
	return viewport();
}

bool
QicsGrid::isCellValid(int row, int col) const
{
    QicsRegion real_vp = currentViewport();

    if (real_vp.isValid())
	return real_vp.containsCell(row, col);
    else
	return false;
}

///////////////////////////////////////////////////////////////////////
/////////////////  Grid layout methods  ///////////////////////////////
///////////////////////////////////////////////////////////////////////

QicsICell
QicsGrid::computeCellPositions(const QRect &bounds,
			       const QicsICell &start)
{
    // Clear out the old position info
    myColumnPositions.clear();
    myRowPositions.clear();

    QicsRegion real_vp = currentViewport();

    //
    // Compute row locations
    //

    int hlw = myMainGrid->horizontalGridLineWidth();

    myRowPositions.setFirstIndex(start.row());

    int this_row = start.row();
    int current_y = bounds.top() + hlw;
    while ((this_row <= real_vp.endRow()) && (current_y <= bounds.bottom()))
    {
	myRowPositions.push_back(current_y);
	current_y += (mappedDM().rowHeight(this_row) + hlw);
	++this_row;
    }
    int bottom_row = this_row - 1;

    //
    // Compute column locations
    //

    int vlw = myMainGrid->verticalGridLineWidth();

    myColumnPositions.setFirstIndex(start.column());

    int this_col = start.column();
    int current_x = bounds.left() + vlw;
    while ((this_col <= real_vp.endColumn()) && (current_x <= bounds.right()))
    {
	myColumnPositions.push_back(current_x);
	current_x += (mappedDM().columnWidth(this_col) + vlw);
	++this_col;
    }
    int right_column = this_col - 1;

    return (QicsICell(bottom_row, right_column));
}

QRect QicsGrid::cellDimensions(int row, int col, bool with_spans) const
{
    QRect retval;
    if ((row < myRowPositions.firstIndex()) ||
	(row > myRowPositions.lastIndex()) ||
	(col < myColumnPositions.firstIndex()) ||
	(col > myColumnPositions.lastIndex()))
    {
	// Cell is not on screen.  Return the invalid rectangle
	return retval;
    }

    retval.setLeft(myColumnPositions[col]);
    retval.setTop(myRowPositions[row]);

    QicsRegion span_region;

    if (with_spans &&
	(styleManager().spanManager()->isSpanner(myInfo, row, col, span_region)))
    {
	retval.setWidth(mappedDM().regionWidth(span_region));
	retval.setHeight(mappedDM().regionHeight(span_region));
    }
    else
    {
	retval.setWidth(mappedDM().columnWidth(col));
	retval.setHeight(mappedDM().rowHeight(row));
    }

    return retval;
}

void
QicsGrid::dumpPositions(void) const
{
    QicsPositionList::iterator iter;
	int i;

    qDebug("ROWS: ");
    for (iter = myRowPositions.begin();
	 iter != myRowPositions.end();
	 ++iter)
    {
        qDebug("%d ", *iter);
    }
    qDebug("\n");
	
    qDebug("HEIGHTS: ");
    for (i = myRowPositions.firstIndex();
	 i <= myRowPositions.lastIndex();
	 ++i)
    {
	qDebug("%d ", mappedDM().rowHeight(i));
    }
    qDebug("\n");

    qDebug("COLS: ");
    for (iter = myColumnPositions.begin();
	 iter != myColumnPositions.end();
	 ++iter)
    {
        qDebug("%d ", *iter);
    }
    qDebug("\n");

    qDebug("WIDTHS: ");
    for (i = myColumnPositions.firstIndex();
	 i <= myColumnPositions.lastIndex();
	 ++i)
    {
	qDebug("%d ", mappedDM().columnWidth(i));
    }
    qDebug("\n");
}

///////////////////////////////////////////////////////////////////////
/////////////////  Grid drawing methods  //////////////////////////////
///////////////////////////////////////////////////////////////////////

void
QicsGrid::drawRegion(const QicsRegion &region, QPainter *painter)
{
    int i, j;
    int x, y;

    if (region.isEmpty())
	return;

    for (i = region.startRow(); i <= region.endRow(); ++i)
    {
	y = myRowPositions[i];

	for (j = region.startColumn(); j <= region.endColumn(); ++j)
	{
	    // don't draw it if it's already been done
	    if (myAlreadyDrawnCells.findIndex(QicsICell(i, j)) != -1)
		continue;

	    // Look for this cell in our current overflow regions
	    QicsRegionV::iterator iter;
	    bool found = false;

	    for (iter = myOverflows.begin(); iter != myOverflows.end(); ++iter)
	    {
		QicsRegion overflow = *iter;
		if (overflow.contains(QicsICell(i, j)))
		{
		    // get rid of the overflow
		    myOverflows.erase(iter);

		    // now we must redraw every cell in the overflow region
		    drawRegion(overflow, painter);
		    found = true;
		    break;
		}
	    }
	    
	    if (found)
		continue;

	    // Ok, this is a "normal" cell (it might still be in a span
	    // region, but that's handled in drawCell()), so let's draw it

	    x = myColumnPositions[j];

	    drawCell(i, j, x, y, (j == region.startColumn()), painter);
	}
    }
}

void
QicsGrid::drawGridLines(const QicsRegion &region, QPainter *painter)
{
    if (region.isEmpty())
	return;

    int vlw = myMainGrid->verticalGridLineWidth();
    int vlro = (vlw / 2) + (vlw % 2);
    int vllo = vlw - vlro;
    int hlw = myMainGrid->horizontalGridLineWidth();
    int hlbo = (hlw / 2) + (hlw % 2);
    int hlto = hlw - hlbo;

    int start_row = region.startRow();
    int end_row = region.endRow();

    int start_col = region.startColumn();
    int end_col = region.endColumn();
    int i, j;

    bool for_printer = painter->device()->isExtDev();

    bool do_horiz = myMainGrid->horizontalGridLinesVisible();
    bool do_vert = myMainGrid->verticalGridLinesVisible();

    QicsLineStyle hls = myMainGrid->horizontalGridLineStyle();
    QicsLineStyle vls = myMainGrid->verticalGridLineStyle();

    const QColorGroup &cg = myMainGrid->gridPalette().active();
    QColor bg = cg.color(QColorGroup::Background);

    if (do_horiz && ((hls == Qics::Sunken) || (hls == Qics::Raised)))
    {
	for (i = start_row; i <= end_row; ++i)
	{
	    QPoint p1(myColumnPositions[start_col],
		      myRowPositions[i] - hlbo);
	    QPoint p2((myColumnPositions[end_col] +
		       mappedDM().columnWidth(end_col)),
		      myRowPositions[i] - hlbo);

	    qDrawShadeLine(painter, p1.x(), p1.y(), p2.x(), p2.y(),
			   cg, (hls == Qics::Sunken), hlw/2);
	}

	QPoint p1((myColumnPositions[start_col] - vlw),
		  (myRowPositions[end_row] +
		   mappedDM().rowHeight(end_row) + hlto));
	QPoint p2((myColumnPositions[end_col] +
		   mappedDM().columnWidth(end_col) + vlw),
		  (myRowPositions[end_row] +
		   mappedDM().rowHeight(end_row) + hlto));

	qDrawShadeLine(painter, p1.x(), p1.y(), p2.x(), p2.y(),
		       cg, (hls == Qics::Sunken), hlw/2);
    }
    else
    {
	QPen pen;
	bool skip_lines = false;

	if (!do_horiz || (hls == Qics::None))
	{
	    if (for_printer)
		skip_lines = true;
	    else
	    {
		pen.setColor(bg);
		pen.setWidth(hlw);
		painter->setPen(pen);
	    }
	}
	else
	{
	    QPen hlp = myMainGrid->horizontalGridLinePen();
	    hlp.setWidth(hlw);		
	    painter->setPen(hlp);
	}
	
	if (!skip_lines)
	{
	    for (i = start_row; i <= end_row; ++i)
	    {
		QPoint p1(myColumnPositions[start_col],
			  myRowPositions[i] - hlbo);
		QPoint p2((myColumnPositions[end_col] +
			   mappedDM().columnWidth(end_col)),
			  myRowPositions[i] - hlbo);

		painter->drawLine(p1, p2);
	    }

  	    QPoint p1((myColumnPositions[start_col] - vlw),
  		      (myRowPositions[end_row] +
  		       mappedDM().rowHeight(end_row) + hlto));
  	    QPoint p2((myColumnPositions[end_col] +
  		       mappedDM().columnWidth(end_col) + vlw),
  		      (myRowPositions[end_row] +
  		       mappedDM().rowHeight(end_row) + hlto));

  	    painter->drawLine(p1, p2);
  	}
      }

      if (do_vert && ((hls == Qics::Sunken) || (hls == Qics::Raised)))
      {
  	for (j = start_col; j <= end_col; ++j)
  	{
  	    QPoint p1(myColumnPositions[j] - vlro,
  		      myRowPositions[start_row] - hlw);
  	    QPoint p2(myColumnPositions[j] - vlro,
  		      (myRowPositions[end_row] +
  		       mappedDM().rowHeight(end_row)));

  	    qDrawShadeLine(painter, p1.x(), p1.y(), p2.x(), p2.y(),
  			   cg, (vls == Qics::Sunken), vlw/2);
  	}

  	QPoint p1((myColumnPositions[end_col] +
  		   mappedDM().columnWidth(end_col) + vllo),
  		  myRowPositions[start_row] - hlw);
  	QPoint p2((myColumnPositions[end_col] +
  		   mappedDM().columnWidth(end_col) + vllo),
  		  (myRowPositions[end_row] +
  		   mappedDM().rowHeight(end_row) + hlw));

  	qDrawShadeLine(painter, p1.x(), p1.y(), p2.x(), p2.y(),
  		       cg, (vls == Qics::Sunken), vlw/2);
      }
      else
      {
  	QPen pen;
  	bool skip_lines = false;

  	if (!do_vert || (vls == Qics::None))
  	{
	    if (for_printer)
		skip_lines = true;
	    else
	    {
		pen.setColor(bg);
		pen.setWidth(vlw);
		painter->setPen(pen);
	    }
	}
	else
	{
	    QPen vlp = myMainGrid->verticalGridLinePen();
	    vlp.setWidth(vlw);
	    painter->setPen(vlp);
	}

	if (!skip_lines)
	{
	    for (j = start_col; j <= end_col; ++j)
	    {
		QPoint p1(myColumnPositions[j] - vlro,
			  myRowPositions[start_row] - hlw);
		QPoint p2(myColumnPositions[j] - vlro,
			  (myRowPositions[end_row] +
			   mappedDM().rowHeight(end_row)));

		painter->drawLine(p1, p2);
	    }

	    QPoint p1((myColumnPositions[end_col] +
		       mappedDM().columnWidth(end_col) + vllo),
		      myRowPositions[start_row] - hlw);
	    QPoint p2((myColumnPositions[end_col] +
		       mappedDM().columnWidth(end_col) + vllo),
		      (myRowPositions[end_row] +
		       mappedDM().rowHeight(end_row) + hlw));

	    painter->drawLine(p1, p2);
	}
    }
}

void
QicsGrid::drawCell(int row, int col, int x, int y,
		   bool look_for_overflower, QPainter *painter)
{
    QicsSpanManager *spm = styleManager().spanManager();
    QicsCellDisplay *cd;

    if (look_for_overflower)
    {
	// If this cell is empty, we look leftward for the first non-empty
	// cell that might want to overflow into this cell

	QicsRegion real_vp = currentViewport();

	QicsRegion reg;
	int new_col = col;

	while (((col - new_col) < myMainGrid->maxOverflowCells()) &&
	       (new_col >= real_vp.startColumn()) &&
	       !spm->isSpanner(myInfo, row, new_col, reg) &&
	       !spm->isSpanned(myInfo, row, new_col, reg))
	{
	    cd = cellDisplay(row, col);
	    if (cd && !(cd->isEmpty(&myInfo, row, new_col,
				    cellValue(row, new_col))))
	    {
		// Don't bother if this is the original cell
		if (new_col == col)
		    break;

		QicsRegion newreg(row, new_col, row, col - 1);
		int vlw = myMainGrid->verticalGridLineWidth();
		int new_x = x - mappedDM().regionWidth(newreg) - vlw;
		
		drawCell(row, new_col, new_x, y, false, painter);
		break;
	    }
		
	    --new_col;
	}
    }

    // We may have "drawn" this cell already via an overflow,
    // so before we do anything else we had better check...

    if (myAlreadyDrawnCells.findIndex(QicsICell(row, col)) != -1)
	return;

    QicsRegion span_region;
    int width, height;

    // If this cell is spanned we don't draw it...
    if (spm->isSpanned(myInfo, row, col, span_region))
    {
	// ...unless, of course, the spanner cell is off the screen
	// to the left or to the top, in which case we have to make sure
	// the spanner cell is partially drawn.

	int spanner_row = span_region.startRow();
	int spanner_col = span_region.startColumn();

	// Look through the list of drawn cells in case the spanner has
	// already been drawn

	if (myAlreadyDrawnCells.findIndex(QicsICell(spanner_row, spanner_col)) != -1)
	    return;

	// We need to (partially) draw the spanned cell.  We begin
	// by constructing a region that we can use to determine how
	// much of the span region is off the screen.  We then use
	// these dimensions to negatively offset the starting position
	// of the spanned cell so we only draw part of it.

	QicsRegion r(spanner_row, spanner_col,
		   QICS_MAX(spanner_row, row-1), QICS_MAX(spanner_col, col-1));

	int vlw = myMainGrid->verticalGridLineWidth();
	int hlw = myMainGrid->horizontalGridLineWidth();

	if (spanner_col < col)
	    x -= (mappedDM().regionWidth(r) + vlw);
	if (spanner_row < row)
	    y -= (mappedDM().regionHeight(r) + hlw);

	width = mappedDM().regionWidth(span_region);
	height = mappedDM().regionHeight(span_region);

	// Change the row and column that we are going to draw to the
	// spanner cell.
	row = spanner_row;
	col = spanner_col;

	// Save off the spanner cell so when we get to other spanned cells, we
	// know that this cell has already been drawn
	myAlreadyDrawnCells.push_back(QicsICell(row, col));
    }
    else if (spm->isSpanner(myInfo, row, col, span_region))
    {
	// the dimensions of the entire spanned region
	width = mappedDM().regionWidth(span_region);
	height = mappedDM().regionHeight(span_region);

	// Save off this cell so when we get to the spanned cells, we
	// know that this cell has already been drawn
	myAlreadyDrawnCells.push_back(QicsICell(row,col));
    }
    else
    {
	width = mappedDM().columnWidth(col);
	height = mappedDM().rowHeight(row);
    }

    QRect rect(x, y, width, height);

    if (prepareToDraw(row, col, rect, painter))
    {
	cd = cellDisplay(row, col);
	if (cd)
	{
	    cd->displayCell(this, row, col, cellValue(row, col), rect, painter);
	}
    }
}

bool QicsGrid::prepareToDraw(int, int, const QRect &, QPainter *)
{
    return true;
}

bool
QicsGrid::requestCellOverflow(const QicsRegion &cur_area,
			      const QRect &cur_rect,
			      QicsRegion &new_area,
			      QRect &new_rect)
{
    if (myMainGrid->cellOverflowBehavior() != Overflow)
	return false;

    QicsICell startCell = cur_area.startCell();
    QicsICell endCell = cur_area.endCell();

    QicsSpanManager *spm = styleManager().spanManager();
    QicsRegion reg;

    if (spm->isSpanner(myInfo, startCell.row(), startCell.column(), reg) ||
	spm->isSpanned(myInfo, startCell.row(), startCell.column(), reg))
    {
	// Can't have spans in an overflow
	return false;
    }

    QicsRegion real_vp = currentViewport();
    if (endCell.column() == real_vp.endColumn())
	// at the end of the row
	return false;

    // Determine next cell
    endCell.setColumn(endCell.column() + 1);

    if (gridInfo().currentCell() == endCell)
    {
	// Can't overflow into the current cell
	return false;
    }

    if ((endCell.column() - startCell.column()) > myMainGrid->maxOverflowCells())
    {
	// Can't have more overflow cells than MaxOverflowCells
	return false;
    }

    if (spm->isSpanner(myInfo, endCell.row(), endCell.column(), reg) ||
	spm->isSpanned(myInfo, endCell.row(), endCell.column(), reg))
    {
	// Can't have spans in an overflow
	return false;
    }

    // Determine if next cell is empty
    QicsCellDisplay *cd = cellDisplay(endCell.row(), endCell.column());
    if (cd->isEmpty(&myInfo, endCell.row(), endCell.column(),
		    cellValue(endCell.row(), endCell.column())))
    {
	new_area = QicsRegion(cur_area.startCell(), endCell);

	new_rect = cur_rect;
	new_rect.setWidth(mappedDM().regionWidth(new_area));
	
	return true;
    }
    else
    {
	return false;
    }
}

void
QicsGrid::acceptCellOverflow(QicsRegion &area)
{
    if (myMainGrid->cellOverflowBehavior() != Overflow)
	return;

    // mark off the overflowed cells as already drawn so we don't
    // overwrite the overflowed text

    for (int i = area.startRow(); i <= area.endRow(); ++i)
    {
	for (int j = area.startColumn(); j <= area.endColumn(); ++j)
	{
	    myAlreadyDrawnCells.push_back(QicsICell(i, j));
	}
    }

    myOverflows.push_back(area);
}


const QicsDataItem *
QicsGrid::cellValue(int row, int col) const
{
    return gridInfo().cellValue(row, col);
}

QicsCellDisplay *
QicsGrid::cellDisplay(int row, int col) const
{
    QicsRegion reg;
    if (styleManager().spanManager()->isSpanned(myInfo, row, col, reg))
    {
	row = reg.startRow();
	col = reg.startColumn();
    }

    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    return (myCell->displayer());
}

int QicsGrid::modelColumnIndex(int c) const
{
	return myInfo.modelColumnIndex(c);
}

int QicsGrid::modelRowIndex(int r) const
{
	return myInfo.modelRowIndex(r);
}

//////////////////////////////////////////////////////////////////////
/////////////////    Mapping/Ordering methods    /////////////////////
//////////////////////////////////////////////////////////////////////

void
QicsGrid::orderRowsBy(int column, QicsSortOrder order, DataItemComparator func)
{    
	myInfo.orderRowsBy(column, order, 0, -1, func);
}

void
QicsGrid::orderColumnsBy(int row, QicsSortOrder order, DataItemComparator func)
{    
	myInfo.orderColumnsBy(row, order, 0, -1, func);
}
