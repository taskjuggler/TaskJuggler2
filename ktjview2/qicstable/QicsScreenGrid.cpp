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


#include <QicsScreenGrid.h>

#include <QicsDataModel.h>
#include <QicsDataItem.h>
#include <QicsCellDisplay.h>
#include <QicsDimensionManager.h>
#include <QicsMappedDimensionManager.h>
#include <QicsSelectionManager.h>
#include <QicsSpanManager.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE 
#undef CREATE_OBJS_WITH_QICSTABLE
#endif
#include <QicsCell.h>
#include <QicsMainGrid.h>

#include <qpainter.h>
#include <qstyle.h>
#include <qlineedit.h>
#include <qdrawutil.h>
#include <qpaintdevicemetrics.h> 

#include "QicsUtil.h"


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#include <qtooltip.h>

class QicsGridToolTip: public QToolTip
{
public:
    QicsGridToolTip(QicsScreenGrid *grid);

protected:
    virtual void maybeTip(const QPoint & pt);

    QicsScreenGrid *myGrid;
};


QicsGridToolTip::QicsGridToolTip(QicsScreenGrid *grid):
    QToolTip(grid)
{
    myGrid = grid;
}

void
QicsGridToolTip::maybeTip(const QPoint & pt)
{
    QicsICell cell = myGrid->cellAt(pt.x(), pt.y(), false);

    if (!cell.isValid())
	return;

    QString text = myGrid->tooltipText(cell);

    if (!text.isEmpty())
    {
	tip(myGrid->cellDimensions(cell, true), text);
    }
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

QicsScreenGrid::QicsScreenGrid(QWidget *w, QicsGridInfo &info,
			       int top_row,
			       int left_column)
    :QFrame (w),
     QicsGrid(info),
     myTopRow(top_row),
     myLeftColumn(left_column),
     myBottomRow(-1),
     myRightColumn(-1),
     myReqVisibleRows(-1),
     myReqVisibleColumns(-1),
     myLastResizeLinePosition(-1),
     myNeedsRecomputeCellsFlag(true),
     myNeedsRecomputeLastPageFlag(true),
     myNeedsRepaintFlag(false),
     myLastPageRows(0),
     myLastPageColumns(0),
     myTraversalRow(-1),
     myTraversalColumn(-1),
     myEditingCurrentCell(false),
     myEditable(false),
     myPlacingEntryWidgetFlag(false),
     myDragCell(0)
{
    // We will need this for cursor changes
    setMouseTracking(true);

    setFocusPolicy(QWidget::ClickFocus);

    myGridToolTip = new QicsGridToolTip(this);

    updateLineWidth();
    updateFrameStyle();
}

QicsScreenGrid::~QicsScreenGrid()
{
    delete myGridToolTip;

    delete myDragCell;
}

void
QicsScreenGrid::reset(void)
{
    // invalidates the current cell
    setCurrentCell(QicsICell());
}

///////////////////////////////////////////////////////////////////////
////////////// Viewport related methods ///////////////////////////////
///////////////////////////////////////////////////////////////////////

void
QicsScreenGrid::setTopRow(int value)
{
    QicsRegion vp = viewport();
    int endRow = vp.endRow();

    if ((vp.endRow() == Qics::QicsLAST_ROW) && dataModel())
	endRow = dataModel()->lastRow();

    if ((value >= vp.startRow()) && (value <= endRow))
    {
	myTopRow = value;
	recomputeAndDraw();
    }
}

void
QicsScreenGrid::setLeftColumn(int value)
{
    QicsRegion vp = viewport();
    int endCol = vp.endColumn();

    if ((vp.endColumn() == Qics::QicsLAST_COLUMN) && dataModel())
	endCol = dataModel()->lastColumn();

    if ((value >= vp.startColumn()) && (value <= endCol))
    {
	myLeftColumn = value;
	recomputeAndDraw();
    }
}

void
QicsScreenGrid::setViewport(const QicsRegion &reg)
{
    QicsGrid::setViewport(reg);
    updateViewport();

    myNeedsRecomputeLastPageFlag = true;
    recomputeAndDraw();
}

void
QicsScreenGrid::fixHeightToViewport(bool set)
{
    myHeightFixed = set;

    if (set)
    {
	QicsRegion reg = currentViewport();
	setFixedHeight(mappedDM().regionHeight(reg) + 
		       (2 * myMainGrid->horizontalGridLineWidth()) +
		       (2 * frameWidth()));
    }
    else
    {
	QSizePolicy sp = sizePolicy();

	sp.setVerData(QSizePolicy::Expanding);
	setSizePolicy(sp);
    }
}

void
QicsScreenGrid::fixWidthToViewport(bool set)
{
    myWidthFixed = set;

    if (set)
    {
	QicsRegion reg = currentViewport();
	setFixedWidth(mappedDM().regionWidth(reg) +
		      (2 * myMainGrid->verticalGridLineWidth()) +
	              (2 * frameWidth()));
    }
    else
    {
	QSizePolicy sp = sizePolicy();

	sp.setHorData(QSizePolicy::Expanding);
	setSizePolicy(sp);
    }
}

/// make this minimum size the size of ONE cell.
QSize
QicsScreenGrid::minimumSizeHint() const
{
    return(QSize((mappedDM().columnWidth(myLeftColumn) +
		  (2 * myMainGrid->verticalGridLineWidth()) +
		  (2 * frameWidth())),
		 (mappedDM().rowHeight(myTopRow) +
		  (2 * myMainGrid->horizontalGridLineWidth()) +
		  (2 * frameWidth()))));
}

QSize
QicsScreenGrid::sizeHint() const
{
    int width;
    int height;

    if (myReqVisibleColumns > 0)
    {
	QicsRegion viz(myTopRow, myLeftColumn,
		       myTopRow, (myLeftColumn + myReqVisibleColumns - 1));

	width = (mappedDM().regionWidth(viz) +
		 (2 * myMainGrid->verticalGridLineWidth()) +
		 (2 * frameWidth()));
    }
    else
	width = -1;

    if (myReqVisibleRows > 0)
    {
	QicsRegion viz(myTopRow, myLeftColumn,
		       (myTopRow + myReqVisibleRows - 1), myLeftColumn);

	height = (mappedDM().regionHeight(viz) +
		  (2 * myMainGrid->horizontalGridLineWidth()) +
		  (2 * frameWidth()));
    }
    else
	height = -1;

    return QSize(width, height);
}

int
QicsScreenGrid::visibleRows(void) const
{
    return (myBottomRow - myTopRow + 1);
}

int
QicsScreenGrid::visibleColumns(void) const
{
    return (myRightColumn - myLeftColumn + 1);
}

void
QicsScreenGrid::setVisibleRows(int num)
{
    myReqVisibleRows = num;
    layout();
    updateGeometry();
}

void
QicsScreenGrid::setVisibleColumns(int num)
{
    myReqVisibleColumns = num;
    layout();
    updateGeometry();
}

///////////////////////////////////////////////////////////////////////
/////////////////  Grid layout methods  ///////////////////////////////
///////////////////////////////////////////////////////////////////////

void
QicsScreenGrid::layout(void)
{
    if (gridInfo().gridRepaintBehavior() == RepaintOff)
	return;

    if (myNeedsRecomputeCellsFlag)
	computeCellPositions();

    if (myNeedsRecomputeLastPageFlag)
	computeLastPage();
}

void
QicsScreenGrid::computeCellPositions(void)
{
    QicsICell end_cell = QicsGrid::computeCellPositions(contentsRect(),
							QicsICell(myTopRow,
								  myLeftColumn));

    myBottomRow = end_cell.row();
    myRightColumn = end_cell.column();
    myNeedsRecomputeCellsFlag = false;

    // Now that we have computed the new positions, we have to notify
    // the cell displayers of any cells that need to know when they
    // have moved off screen.  (This is mostly for QicsWidgetCellDisplay
    // objects, so they can hide their widgets.)

    QicsICellV::iterator iter;

    for (iter = myCellsToNotify.begin(); iter != myCellsToNotify.end(); ++iter)
    {
	QicsICell cell = *iter;
	int visRow = myInfo.visualRowIndex(cell.row());
	int visCol = myInfo.visualColumnIndex(cell.column());

	if (!isCellVisible(visRow, visCol))
	{
	    QicsCellDisplay *cd = cellDisplay(visRow, visCol);
	    if (cd)
		cd->hideEdit(this);
	}
    }

    // Ok, we've notified everybody.  Now let's clear the notify list
    // and rebuild it with the cells from the new positions.

    myCellsToNotify.clear();

    int i, j;
    for (i = myTopRow; i <= myBottomRow; ++i)
    {
	for (j = myLeftColumn; j <= myRightColumn; ++j)
	{
	    QicsCellDisplay *cd = cellDisplay(i, j);

	    if (cd && cd->needsVisibilityNotification())
	    {
		myCellsToNotify.push_back(QicsICell(modelRowIndex(i),
						    modelColumnIndex(j)));
	    }
	}
    }

    myOverflows.clear();
}

void
QicsScreenGrid::computeLastPage(void)
{
    myLastPageRows = -1;
    myLastPageColumns = -1;

    int start_row, start_col;

    // We are going backwards, from the bottom right of the table.
    // The goal is to figure out how many rows and columns would
    // be displayed if the bottom-right cell was in the bottom-right
    // spot on the grid.  This information will be used by the table
    // to set the correct values on the scrollbars

    QicsRegion real_vp = currentViewport();
    QRect contents = contentsRect();

    //
    // Compute row locations/sizes here
    //

    int this_row = real_vp.endRow();
    int current_y = 0;
    while (this_row >= real_vp.startRow())
    {
	int proposed_y = (current_y + mappedDM().rowHeight(this_row) +
			  myMainGrid->horizontalGridLineWidth());
	if (proposed_y > contents.height())
	    break;

	current_y = proposed_y;
	--this_row;
    }
    start_row = this_row + 1;

    //
    // Compute column locations/sizes here
    //

    int this_col = real_vp.endColumn();
    int current_x = 0;
    while (this_col >= real_vp.startColumn())
    {
	int proposed_x = (current_x + mappedDM().columnWidth(this_col) +
			  myMainGrid->verticalGridLineWidth());
	if (proposed_x > contents.width())
	    break;

	current_x = proposed_x;
	--this_col;
    }
    start_col = this_col + 1;


    myLastPageRows = real_vp.endRow() - start_row + 1;
    myLastPageColumns = real_vp.endColumn() - start_col + 1;

#ifdef notdef
    qDebug("lastpage - bottom = %d, right = %d\n", myBottomRow, myRightColumn);
    qDebug("lastpage - vp.bot = %d, vp.right = %d\n",
	   real_vp.endRow(), real_vp.endColumn() );
    qDebug("lastpage - nrows = %d, ncols = %d\n", myLastPageRows, myLastPageColumns);
#endif

    myNeedsRecomputeLastPageFlag = false;

    emit newBoundsInfo();
}

int
QicsScreenGrid::lastPageRows(void)
{
    layout();

    return myLastPageRows;
}

int
QicsScreenGrid::lastPageColumns(void)
{
    layout();

    return myLastPageColumns;
}
 
QicsICell
QicsScreenGrid::cellAt(int x, int y, bool nearest) const
{
    int row = rowAt(y, nearest);
    int col = columnAt(x, nearest);

    QicsRegion reg;
    if (styleManager().spanManager()->isSpanned(myInfo, row, col, reg))
    {
	row = reg.startRow();
	col = reg.startColumn();
    }

    return QicsICell(row, col);
}

int
QicsScreenGrid::rowAt(int y, bool nearest) const
{
    int i;

    int nrows =  myRowPositions.size();

    if (nrows < ((myBottomRow - myTopRow)))
	return (-2);

    if ((nrows > 0) && (y < myRowPositions[myTopRow]))
	return (nearest ? myTopRow : -1);

    for (i = myTopRow; i <= myBottomRow; ++i)
    {
	if (y >= myRowPositions[i])
	{
	    if (i == myBottomRow)
	    {
		if (nearest)
		    return (i);
		else
		{
		    int row_height = 
			mappedDM().rowHeight(i);

		    if (y > (myRowPositions[i] + row_height))
			return -1;
		    else
			return (i);
		}
	    }
	    else if (y < myRowPositions[i+1])
	    {
		return (i);
	    }
	}
    }

    if (nearest)
	return myBottomRow;
    else
	return -1;
}

int
QicsScreenGrid::columnAt(int x, bool nearest) const
{
    int ncols =  myColumnPositions.size();
    int i;

    if (ncols < ((myRightColumn - myLeftColumn)))
	return (-2);

    if ((ncols > 0) && (x < myColumnPositions[myLeftColumn]))
	return (nearest ?  myLeftColumn : -1);

    for (i = myLeftColumn; i <= myRightColumn; ++i)
    {
	if (x >= myColumnPositions[i])
	{
	    if (i == myRightColumn)
	    {
		if (nearest)
		    return (i);
		else
		{
		    int col_width = 
			mappedDM().columnWidth(i);

		    if (x > (myColumnPositions[i] + col_width))
			return -1;
		    else
			return (i);
		}
	    }
	    else if (x < myColumnPositions[i+1])
	    {
		return (i);
	    }
	}
    }

    if (nearest)
	return myRightColumn;
    else
	return -1;
}

///////////////////////////////////////////////////////////////////////
/////////////////  Grid drawing methods  //////////////////////////////
///////////////////////////////////////////////////////////////////////

void
QicsScreenGrid::resetAndDraw(void)
{
    reset();

    myNeedsRecomputeLastPageFlag = true;
    recomputeAndDraw();
}

void
QicsScreenGrid::recomputeAndDraw(void)
{
    myNeedsRecomputeCellsFlag = true;

    if (isVisible())
    {
	// paintRegion will do the recompute
	paintRegion(contentsRect(), 0);
    }

    // invalidate this
    myLastResizeLinePosition = -1;
}

void
QicsScreenGrid::redraw(void)
{
    paintRegion(contentsRect(), 0);
}

void
QicsScreenGrid::redraw(QicsRegion region)
{
    QicsRegion screen = QicsRegion(myTopRow, myLeftColumn,
				   myBottomRow, myRightColumn);

    if (screen.intersects(region))
	paintRegion(region, 0);
}

void
QicsScreenGrid::redraw(QicsSpan span)
{
    QicsRegion reg(span.row(), span.column(),
		   span.row()+span.height()-1, span.column()+span.width()-1);
    redraw(reg);
}

void
QicsScreenGrid::drawContents(QPainter *painter)
{
    QRect rect = (painter->clipRegion()).boundingRect();

    paintRegion(rect, painter);
}

void
QicsScreenGrid::paintRegion(const QRect &rect, QPainter *painter)
{
#ifdef notdef
    qDebug("paintRegion XY: (%d, %d) (%d x %d)\n",
	   rect.left(), rect.top(), rect.width(), rect.height());
#endif

    // this only computes cell positions if necessary
    layout();

    if (gridInfo().gridRepaintBehavior() != RepaintOn)
    {
	myNeedsRepaintFlag = true;
	return;
    }

    // Don't use cellAt() here -- it will account for spanned cells which
    // is not what we want here...
    QicsICell start_cell(rowAt(rect.top(), true), columnAt(rect.left(), true));
    QicsICell end_cell(rowAt(rect.bottom(), true), columnAt(rect.right(), true));

    QicsRegion region(start_cell, end_cell);

    paintRegion(region, painter);

    // clear any unused space in the widget
    //
    QRect end_cell_dims = cellDimensions(end_cell, true);

    int vlw = myMainGrid->verticalGridLineWidth();
    int hlw = myMainGrid->horizontalGridLineWidth();

    QRect r1(QPoint(rect.left(), end_cell_dims.bottom() + hlw + 1),
	     QPoint(end_cell_dims.right() + vlw + 1, rect.bottom()));

    QRect r2(QPoint(end_cell_dims.right() + vlw + 1, rect.top()),
	     rect.bottomRight());

    if (r1.isValid())
	erase(r1);

    if (r2.isValid())
	erase(r2);

    placeEntryWidget();
}

void
QicsScreenGrid::paintRegion(const QicsRegion &region, QPainter *painter)
{
#ifdef notdef
    if (this->inherits("QicsTableGrid"))
    {
	qDebug("paintGridRegion: (%d, %d) (%d, %d)\n",
	       region.startRow(), region.startColumn(),
	       region.endRow(), region.endColumn());
    }
#endif

    // this only computes cell positions if necessary
    layout();

    if (gridInfo().gridRepaintBehavior() != RepaintOn)
    {
	myNeedsRepaintFlag = true;
	return;
    }

    bool created_painter = false;
    if (painter == 0)
    {
	painter = new QPainter(this);
	painter->setClipRect(contentsRect());
	created_painter = true;
    }

    myAlreadyDrawnCells.clear();

    QicsRegion dr(QICS_MAX(region.startRow(), myTopRow),
		  QICS_MAX(region.startColumn(), myLeftColumn),
		  QICS_MIN(region.endRow(), myBottomRow),
		  QICS_MIN(region.endColumn(), myRightColumn));

    drawGridLines(dr, painter);
    drawRegion(dr, painter);

    myNeedsRepaintFlag = false;

    if (created_painter)
	delete painter;

    placeEntryWidget();
}

void
QicsScreenGrid::drawCell(int row, int col, int x, int y,
		   bool look_for_overflower, QPainter *painter)
{
    // don't overflow this cell if it is the current cell

    QicsICell cur_cell = currentCell();

    bool overflow = (look_for_overflower && ((!cur_cell.isValid()) ||
					     (row != cur_cell.row()) ||
					     (col !=  cur_cell.column())));

    QicsGrid::drawCell(row, col, x, y, overflow, painter);
}

bool
QicsScreenGrid::prepareToDraw(int, int, const QRect &rect, QPainter *)
{
    bool out_of_grid_bounds = ((rect.x() + rect.width()) > width() ||
           (rect.y() + rect.height()) > height());

    return (myMainGrid->drawPartialCells() || !out_of_grid_bounds);
}

bool
QicsScreenGrid::requestCellOverflow(const QicsRegion &cur_area,
			      const QRect &cur_rect,
			      QicsRegion &new_area,
			      QRect &new_rect)
{
    QicsICell endCell = cur_area.endCell();
    QicsICell cur_cell = currentCell();

    if (cur_cell.isValid() && (cur_cell.row() == endCell.row()) &&
	(cur_cell.column() == endCell.column()))
    {
	// Can't have the current cell in an overflow
	return false;
    }

    return (QicsGrid::requestCellOverflow(cur_area, cur_rect,
					  new_area, new_rect));
}

/////////////////////////////////////////////////////////////////
/////////////        Event Handlers              ////////////////
/////////////////////////////////////////////////////////////////

void
QicsScreenGrid::resizeEvent( QResizeEvent *re )
{
    // This will be followed by a paint event, so we just flag that
    // we need to recompute before we draw.

    myNeedsRecomputeCellsFlag = true;
    myNeedsRecomputeLastPageFlag = true;

    QFrame::resizeEvent(re);
}

void
QicsScreenGrid::mousePressEvent( QMouseEvent *m )
{
    QicsICell cell = cellAt(m->x(), m->y(), false);

    if (cell.isValid())
    {
	myPressedCell = cell;

	emit pressed(cell.row(), cell.column(), m->button(), m->pos());

	QicsCellDisplay *cd = cellDisplay(cell.row(), cell.column());

	if (cd && cd->handleMouseEvent(this, cell.row(), cell.column(), m))
	    return;

	handleMousePressEvent(cell, m);
    }
}

void
QicsScreenGrid::mouseReleaseEvent( QMouseEvent *m )
{
    QicsICell cell = cellAt(m->x(), m->y(), false);

    if (cell.isValid())
    {
	if (cell == myPressedCell)
	    emit clicked(cell.row(), cell.column(), m->button(), m->pos());

	myPressedCell = QicsICell();

	QicsCellDisplay *cd = cellDisplay(cell.row(), cell.column());
	if (cd && cd->handleMouseEvent(this, cell.row(), cell.column(), m))
	    return;
    }

    handleMouseReleaseEvent(cell, m);
}

void
QicsScreenGrid::mouseDoubleClickEvent( QMouseEvent *m )
{
    QicsICell cell = cellAt(m->x(), m->y(), false);

    if (cell.isValid())
    {
	emit doubleClicked(cell.row(), cell.column(), m->button(), m->pos());
    }

    handleMouseDoubleClickEvent(cell, m);
}

void
QicsScreenGrid::mouseMoveEvent( QMouseEvent *m )
{
    QicsICell cell = cellAt(m->x(), m->y(), true);

    handleMouseMoveEvent(cell, m);
}

void
QicsScreenGrid::keyPressEvent (QKeyEvent *ke)
{
    QicsICell cur_cell = currentCell();

    if (!cur_cell.isValid())
	return;

    // These keys by themselves aren't worth looking at.  If we send
    // them to the cell display and ClickToEdit is false, these keys
    // will cause the cell to be edited, which we don't want.
    if ((ke->key() == Qt::Key_Alt) ||
	(ke->key() == Qt::Key_Control) ||
	(ke->key() == Qt::Key_Shift))
    {
	return;
    }

    if (!handleTraversalKeys(ke))
    {
	if (!myEditingCurrentCell)
	{
	    if (!myMainGrid->clickToEdit())
		editCell(cur_cell.row(), cur_cell.column());
	}

	QicsCellDisplay *cd = cellDisplay(cur_cell.row(), cur_cell.column());
	if (!cd->handleKeyEvent(this, cur_cell.row(), cur_cell.column(), ke))
	{
	    handleKeyPressEvent(cur_cell, ke);
	}
    }
}

void
QicsScreenGrid::keyReleaseEvent (QKeyEvent *ke)
{
    QicsICell cur_cell = currentCell();

    if (!cur_cell.isValid())
	return;

    handleKeyReleaseEvent(cur_cell, ke);
}

void
QicsScreenGrid::dropEvent(QDropEvent *event)
{
    QicsICell cell = cellAt(event->pos().x(), event->pos().y(), false);
    dropAt(event, cell);
}

void
QicsScreenGrid::handleMousePressEvent(const QicsICell &, QMouseEvent *)
{
}

void
QicsScreenGrid::handleMouseReleaseEvent(const QicsICell &, QMouseEvent *)
{
}

void
QicsScreenGrid::handleMouseDoubleClickEvent(const QicsICell &, QMouseEvent *)
{
}

void
QicsScreenGrid::handleMouseMoveEvent(const QicsICell &cell, QMouseEvent *)
{
    myCell->setRowIndex(cell.row());
    myCell->setColumnIndex(cell.column());

    setCursor(myCell->cursor());
}

void
QicsScreenGrid::handleKeyPressEvent(const QicsICell &, QKeyEvent *)
{
}

void
QicsScreenGrid::handleKeyReleaseEvent(const QicsICell &, QKeyEvent *)
{
}

/////////////////////////////////////////////////////////////////
/////////////////    Actions      ///////////////////////////////
/////////////////////////////////////////////////////////////////

void
QicsScreenGrid::scrollLeft(int num)
{
    emit scrollRequest(ScrollLeft, num);
}

void
QicsScreenGrid::scrollRight(int num)
{
    emit scrollRequest(ScrollRight, num);
}

void
QicsScreenGrid::scrollUp(int num)
{
    emit scrollRequest(ScrollUp, num);
}

void
QicsScreenGrid::scrollDown(int num)
{
    emit scrollRequest(ScrollDown, num);
}

void
QicsScreenGrid::traverseToBeginningOfTable(void)
{
    QicsRegion vp = viewport();

    int row = vp.startRow();
    int col = vp.startColumn();

    if (traverseToCell(row, col) && !isCellVisible(row, col))
    {
	scrollLeft(myLeftColumn - vp.startColumn());
	scrollUp(myTopRow - vp.startRow());
    }
}

void
QicsScreenGrid::traverseToEndOfTable(void)
{
    QicsRegion real_vp = currentViewport();
    int row = real_vp.endRow();
    int col = real_vp.endColumn();

    QicsRegion reg;
    if ((styleManager().spanManager())->isSpanned(myInfo, row, col, reg))
    {
	row = reg.startRow();
	col = reg.startColumn();
    }

    if (traverseToCell(row, col) && !isCellVisible(row, col))
    {
	scrollRight(col - myLastPageColumns - myLeftColumn + 1);
	scrollDown(row - myLastPageRows - myTopRow + 1);
    }
}

void
QicsScreenGrid::traverseLeft(void)
{
    QicsRegion real_vp = currentViewport();
    QicsRegion reg;
    int row, col;
    int start_row;

    QicsICell cur_cell = currentCell();

    if (!cur_cell.isValid())
	return;

    if (myTraversalRow >= 0)
	row = start_row = myTraversalRow;
    else
	row = start_row = cur_cell.row();
    col = cur_cell.column() - 1;

    bool spanned;
    bool enabled = true;

    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    while ((col >= real_vp.startColumn()) &&
	   ((spanned = (styleManager().spanManager())->isSpanned(myInfo, row, col, reg)) ||
	    (! (enabled = myCell->enabled()))))
    {
	if (spanned)
	{
	    row = reg.startRow();
	    col = reg.startColumn();
	}
	else if (!enabled)
	{
	    row = start_row;
	    --col;
	}

	myCell->setRowIndex(row);
	myCell->setColumnIndex(col);
    }

    if (col < real_vp.startColumn())
	return;

    if (traverseToCell(row, col))
    {
	myTraversalRow = start_row;
	myTraversalColumn = -1;
	makeCellFullyVisible(row, col);
    }
}

void
QicsScreenGrid::traverseRight(void)
{
    QicsRegion real_vp = currentViewport();
    QicsRegion reg;
    int row, col;
    int start_row;

    QicsICell cur_cell = currentCell();

    if (!cur_cell.isValid())
	return;

    if (myTraversalRow >= 0)
	row = start_row = myTraversalRow;
    else
	row = start_row = cur_cell.row();

    if ((styleManager().spanManager())->isSpanner(myInfo, cur_cell.row(),
						  cur_cell.column(), reg))
    {
	col = reg.endColumn() + 1;
    }
    else
	col = cur_cell.column() + 1;

    bool spanned;
    bool enabled = true;

    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    while ((col <= real_vp.endColumn()) &&
	   ((spanned = (styleManager().spanManager())->isSpanned(myInfo, row, col, reg)) ||
	    (! (enabled = myCell->enabled()))))
    {
	if (spanned)
	{
	    row = reg.startRow();
	    col = reg.startColumn();
	}
	else if (!enabled)
	{
	    row = start_row;
	    ++col;
	}

	myCell->setRowIndex(row);
	myCell->setColumnIndex(col);
    }

    if (col > real_vp.endColumn())
	return;

    if (traverseToCell(row, col))
    {
	myTraversalRow = start_row;
	myTraversalColumn = -1;
	makeCellFullyVisible(row, col);
    }
}

void
QicsScreenGrid::traverseUp(void)
{
    QicsRegion real_vp = currentViewport();
    QicsRegion reg;
    int row, col;
    int start_col;

    QicsICell cur_cell = currentCell();

    if (!cur_cell.isValid())
	return;

    row = cur_cell.row() - 1;
    if (myTraversalColumn >= 0)
	col = start_col = myTraversalColumn;
    else
	col = start_col = cur_cell.column();

    bool spanned;
    bool enabled = true;

    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    while ((row >= real_vp.startRow()) &&
	   ((spanned = (styleManager().spanManager())->isSpanned(myInfo, row, col, reg)) ||
	    (! (enabled = myCell->enabled()))))
    {
	if (spanned)
	{
	    row = reg.startRow();
	    col = reg.startColumn();
	}
	else if (!enabled)
	{
	    --row;
	    col = start_col;
	}

	myCell->setRowIndex(row);
	myCell->setColumnIndex(col);
    }

    if (row < real_vp.startRow())
	return;

    if (traverseToCell(row, col))
    {
	myTraversalRow = -1;
	myTraversalColumn = start_col;
	makeCellFullyVisible(row, col);
    }
}

void
QicsScreenGrid::traverseDown(void)
{
    QicsRegion real_vp = currentViewport();
    QicsRegion reg;
    int row, col;
    int start_col;

    QicsICell cur_cell = currentCell();

    if (!cur_cell.isValid())
	return;

    if ((styleManager().spanManager())->isSpanner(myInfo, cur_cell.row(),
						  cur_cell.column(), reg))
    {
	row = reg.endRow() + 1;
    }
    else
	row = cur_cell.row() + 1;

    if (myTraversalColumn >= 0)
	col = start_col = myTraversalColumn;
    else
	col = start_col = cur_cell.column();

    bool spanned;
    bool enabled = true;

    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    while ((row <= real_vp.endRow()) &&
	   ((spanned = (styleManager().spanManager())->isSpanned(myInfo, row, col, reg)) ||
	    (! (enabled = myCell->enabled()))))
    {
	if (spanned)
	{
	    row = reg.startRow();
	    col = reg.startColumn();
	}
	else if (!enabled)
	{
	    ++row;
	    col = start_col;
	}

	myCell->setRowIndex(row);
	myCell->setColumnIndex(col);
    }

    if (row > real_vp.endRow())
	return;

    if (traverseToCell(row, col))
    {
	myTraversalRow = -1;

	myTraversalColumn = start_col;
	makeCellFullyVisible(row, col);
    }
}

void
QicsScreenGrid::traverseToBeginningOfRow(void)
{
    QicsRegion real_vp = currentViewport();
    QicsRegion reg;
    int row, col;

    QicsICell cur_cell = currentCell();

    if (!cur_cell.isValid())
	return;

    row = cur_cell.row();
    col = real_vp.startColumn();

    bool spanned;
    bool enabled = true;

    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    while ((col <= real_vp.endColumn()) &&
	   ((spanned = (styleManager().spanManager())->isSpanned(myInfo, row, col, reg)) ||
	    (! (enabled = myCell->enabled()))))
    {
	if (spanned)
	{
	    col = reg.endColumn() + 1;
	}
	else if (!enabled)
	{
	    ++col;
	}

	myCell->setRowIndex(row);
	myCell->setColumnIndex(col);
    }

    if (col > real_vp.endColumn())
	return;

    if (traverseToCell(row, col) && !isCellVisible(row, col))
	scrollLeft(myLeftColumn - viewport().startColumn());
}

void
QicsScreenGrid::traverseToEndOfRow(void)
{
    QicsRegion real_vp = currentViewport();
    QicsRegion reg;
    int row, col;

    QicsICell cur_cell = currentCell();

    if (!cur_cell.isValid())
	return;

    row = cur_cell.row();
    col = real_vp.endColumn();

    bool spanned;
    bool enabled = true;

    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    while ((col >= real_vp.startColumn()) &&
	   ((spanned = (styleManager().spanManager())->isSpanned(myInfo, row, col, reg)) ||
	    (! (enabled = myCell->enabled()))))
    {
	if (spanned)
	{
	    col = reg.startColumn();
	}
	else if (!enabled)
	{
	    --col;
	}

	myCell->setRowIndex(row);
	myCell->setColumnIndex(col);
    }

    if (col < real_vp.startColumn())
	return;

    if (traverseToCell(row, col) && !isCellVisible(row, col))
	scrollRight(col - myLastPageColumns - myLeftColumn + 1);
}


void
QicsScreenGrid::dropAt(QDropEvent *event, const QicsICell & /*cell*/)
{
	event->ignore();
}

////////////////////////////////////////////////////////////////////////////
/////////////////////    Traversal/Editing Methods    //////////////////////
////////////////////////////////////////////////////////////////////////////

bool
QicsScreenGrid::handleTraversalKeys(QKeyEvent *ke)
{
    if (!currentCell().isValid())
	return false;

    bool handled = true;

    if (ke->state() & Qt::ControlButton)
    {
	switch (ke->key())
	{
	case Qt::Key_Home:
	    traverseToBeginningOfTable();
	    break;
	case Qt::Key_End:
	    traverseToEndOfTable();
	    break;
	default:
	    handled = false;
	    break;
	}
    }
    else if (ke->state() & Qt::ShiftButton)
    {
	switch (ke->key())
	{
	case Qt::Key_Enter:
	case Qt::Key_Return:
	{
	    traverse(myMainGrid->enterTraversalDirection(), false);
	    break;
	}
	case Qt::Key_Tab:
	case Qt::Key_BackTab:
	{
	    traverse(myMainGrid->tabTraversalDirection(), false);
	    break;
	}
	default:
	    handled = false;
	    break;
	}
    }
    else  // no modifiers
    {
	switch (ke->key())
	{
	case Qt::Key_Left:
	    traverseLeft();
	    break;
	case Qt::Key_Right:
	    traverseRight();
	    break;
	case Qt::Key_Up:
	    traverseUp();
	    break;
	case Qt::Key_Down:
	    traverseDown();
	    break;
	case Qt::Key_Home:
	    traverseToBeginningOfRow();
	    break;
	case Qt::Key_End:
	    traverseToEndOfRow();
	    break;
	case Qt::Key_Enter:
	case Qt::Key_Return:
	{
	    traverse(myMainGrid->enterTraversalDirection(), true);
	    break;
	}
	case Qt::Key_Tab:
	{
	    traverse(myMainGrid->tabTraversalDirection(), true);
	    break;
	}
	default:
	    handled = false;
	    break;
	}
    }

    return handled;
}

bool
QicsScreenGrid::traverseToCell(int row, int col, bool select_cell)
{
    // Sanity Check the row and column
    if (!isCellValid(row, col))
    {
	return (false);
    }

    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    // Can't traverse to a cell that's not enabled
    if (!myCell->enabled())
	return false;


    QicsICell cur_cell = currentCell();

    myTraversalRow = -1;
    myTraversalColumn = -1;

    if (cur_cell.isValid())
    {
	if ((cur_cell.row() == row) &&
	    (cur_cell.column() == col))
	{
	    // We're already there
	    return (true);
	}
    }

    uneditCurrentCell();

    setCurrentCell(QicsICell(row, col));

    if (select_cell)
	selectCell(row, col);

    if (cellDisplay(row, col)->editWhenCurrent())
	editCurrentCell();

    return (true);
}

void
QicsScreenGrid::traverse(Qt::Orientation orient, bool forward)
{
    QicsScrollDirection dir;

    if (orient == Qt::Vertical)
    {
	if (forward)
	    dir = Qics::ScrollDown;
	else
	    dir = Qics::ScrollUp;
    }
    else
    {
	if (forward)
	    dir = Qics::ScrollRight;
	else
	    dir = Qics::ScrollLeft;
    }

    traverse(dir);
}


bool
QicsScreenGrid::editCell(int row, int col)
{
    if (!myEditable)
	return false;

    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    if (myCell->readOnly())
	return (false);

    if (!traverseToCell(row, col))
	return (false);

    myEditingCurrentCell = true;

    cellDisplay(row, col)->startEdit(this, row, col, cellValue(row, col));

    placeEntryWidget();

    return (true);
}

bool
QicsScreenGrid::editCurrentCell(void)
{
    QicsICell cur_cell = currentCell();

    // Sanity Check
    if (!cur_cell.isValid())
	return (false);

    return editCell(cur_cell.row(), cur_cell.column());
}

void
QicsScreenGrid::uneditCurrentCell(void)
{
    if (myEditingCurrentCell)
    {
	myEditingCurrentCell = false;

	QicsICell cur_cell = currentCell();
	cellDisplay(cur_cell.row(), cur_cell.column())->endEdit(this,
								cur_cell.row(),
								cur_cell.column());
	cellDisplay(cur_cell.row(), cur_cell.column())->hideEdit(this);

	this->setFocus();
    }
}

void
QicsScreenGrid::setEditable(bool b)
{
    myEditable = b;

    if (!myEditable)
	uneditCurrentCell();
}

void QicsScreenGrid::placeEntryWidget(void)
{
    QicsICell cur_cell = currentCell();

    if (!cur_cell.isValid())
	return;

    if (!myEditingCurrentCell)
	return;

    QicsCellDisplay *cd = cellDisplay(cur_cell.row(), cur_cell.column());

    if (!isCellVisible(cur_cell.row(), cur_cell.column()))
    {
	cd->hideEdit(this);
	return;
    }

    QRect rect = cellDimensions(cur_cell.row(), cur_cell.column(), true);
    myPlacingEntryWidgetFlag = true;
    cd->moveEdit(this, cur_cell.row(), cur_cell.column(), rect);
    myPlacingEntryWidgetFlag = false;
}

////////////////////////////////////////////////////////////////////////////
//////////////////////    Drag and Drop Methods    /////////////////////////
////////////////////////////////////////////////////////////////////////////

void
QicsScreenGrid::prepareDrag(const QicsICell &cell)
{
    myDragCell = new QicsICell(cell);
}

void
QicsScreenGrid::startDrag(QDragObject::DragMode mode)
{
    if (!myDragCell || !myDragCell->isValid())
	return;

    if (!myMainGrid->dragEnabled())
	return;

    // Get the drag object
    QDragObject *obj = dragObject(mode);
    if (!obj)
	return;

    // Nuke this so we don't do a drag each time we get a mouse move event
    delete myDragCell;
    myDragCell = 0;

    bool remove = false;

    // Do the drag
    if (mode == QDragObject::DragCopy)
	obj->dragCopy();
    else if (mode == QDragObject::DragMove)
    {
	remove = true;
	obj->dragMove();
    }
    else
	remove = obj->drag();

    QWidget *target = QDragObject::target();

    // NOTE: remove is only true after a successful dragMove operation
    // NOTE: target is NULL if the drop is on another application

#ifdef notdef
    qDebug("Done with drag, targ = 0x%x, remove=%d\n", target, remove);
#endif

    finishDrag(mode, remove, target);
}

void
QicsScreenGrid::finishDrag(QDragObject::DragMode, bool remove,
			  QWidget *)
{

    // TODO: we may want to consider setting remove to
    // false if target is NULL.  That would effectively
    // prevent dragging data out into another application

    gridInfo().finishCut(remove);
}

QDragObject *
QicsScreenGrid::dragObject(QDragObject::DragMode)
{
    return (gridInfo().cutCopyData(this, myDragCell));
}

bool
QicsScreenGrid::canDropAt(QDragMoveEvent *, const QicsICell &) const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool
QicsScreenGrid::isCellVisible(int row, int col) const
{
    return ((myTopRow <= row ) && (row <= myBottomRow) &&
	    (myLeftColumn <= col) && (col <= myRightColumn));
}

void
QicsScreenGrid::makeCellFullyVisible(int row, int col)
{
    QicsRegion real_vp = currentViewport();

    // first check rows

    if (row < myTopRow)
    {
	emit scrollRequest(ScrollUp, myTopRow - row);
    }
    else
    {
	int cur_row = myTopRow;
	int h = 0;

	while (cur_row <= real_vp.endRow())
	{
	    int rh = mappedDM().rowHeight(cur_row);

	    if ((h + rh) > height())
		break;

	    h += rh;
	    ++cur_row;
	}

	int first_row = myTopRow;
	int last_full_row = cur_row - 1;

	while ((row > last_full_row) && (last_full_row < real_vp.endRow()))
	{
	    // take away the topmost row
	    h -= mappedDM().rowHeight(first_row);
	    ++first_row;

	    // add as many rows below as will fit
	    cur_row = last_full_row + 1;
	    while (cur_row <= real_vp.endRow())
	    {
		int rh = mappedDM().rowHeight(cur_row);

		if ((h + rh) > height())
		    break;

		h += rh;
		++cur_row;
	    }

	    last_full_row = cur_row - 1;
	}

	if (first_row > myTopRow)
	    emit scrollRequest(ScrollDown, (first_row - myTopRow));
    }


    // now check columns

    if (col < myLeftColumn)
    {
	emit scrollRequest(ScrollLeft, myLeftColumn - col);
    }
    else
    {
	int cur_col = myLeftColumn;
	int w = 0;

	while (cur_col <= real_vp.endColumn())
	{
	    int cw = mappedDM().columnWidth(cur_col);

	    if ((w + cw) > width())
		break;

	    w += cw;
	    ++cur_col;
	}

	int first_col = myLeftColumn;
	int last_full_col = cur_col - 1;

	while ((col > last_full_col) && (last_full_col < real_vp.endColumn()))
	{
	    // take away the leftmost column
	    w -= mappedDM().columnWidth(first_col);
	    ++first_col;

	    // add as many columns to the right as will fit
	    cur_col = last_full_col + 1;
	    while (cur_col <= real_vp.endColumn())
	    {
		int cw = mappedDM().columnWidth(cur_col);

		if ((w + cw) > width())
		    break;

		w += cw;
		++cur_col;
	    }

	    last_full_col = cur_col - 1;
	}

	if (first_col > myLeftColumn)
	    emit scrollRequest(ScrollRight, (first_col - myLeftColumn));
    }
}

QString
QicsScreenGrid::tooltipText(const QicsICell &cell) const
{
    QString text;

    if (myMainGrid->cellOverflowBehavior() == ToolTip)
    {
	QicsCellDisplay *cd = cellDisplay(cell.row(), cell.column());
	QRect rect = cellDimensions(cell, true);
	const QicsDataItem *itm = cellValue(cell.row(), cell.column());

	text = cd->tooltipText(&myInfo, cell.row(), cell.column(), itm, rect);
    }

    return (text);
}

void
QicsScreenGrid::updateViewport(void)
{
    QicsRegion vp = viewport();

    // Make sure we only show from the beginning of the viewport
    if (myTopRow < vp.startRow())
	myTopRow = vp.startRow();

    if (myLeftColumn < vp.startColumn())
	myLeftColumn = vp.startColumn();
}

void
QicsScreenGrid::updateLineWidth(void)
{
    setLineWidth(myMainGrid->frameLineWidth());
}

void
QicsScreenGrid::updateFrameStyle(void)
{
    int frame_shape = myMainGrid->frameStyle() & QFrame::MShape;
    int frame_shadow = myMainGrid->frameStyle() & QFrame::MShadow;

    if (frame_shape == QFrame::NoFrame)
    {
	// The problem with NoFrame is that we really want a "blank" frame.
	// So, to get a blank frame, we make a box frame that's drawn in
	// the widget's background color, rendering it invisible.

	frame_shape = QFrame::Box;
	setPaletteForegroundColor(paletteBackgroundColor());
	setFrameStyle(frame_shape | frame_shadow);
    }
    else
	setFrameStyle(myMainGrid->frameStyle());
}


void
QicsScreenGrid::orderRowsBy(int column, QicsSortOrder order, DataItemComparator func)
{    
	QicsGrid::orderRowsBy(column, order, func);
	recomputeAndDraw();
}

void
QicsScreenGrid::orderColumnsBy(int row, QicsSortOrder order, DataItemComparator func)
{    
	QicsGrid::orderColumnsBy(row, order, func);
	recomputeAndDraw();
}

//////////////////////////////////////////////////////////////////////
////////////         Slots Methods                     ///////////////
//////////////////////////////////////////////////////////////////////

void
QicsScreenGrid::handleCellPropertyChange(QicsRegion region,
					 QicsCellStyle::QicsCellStyleProperty prop)
{
    bool recalc;

    switch (prop)
    {
    case QicsCellStyle::Font:
	recalc = true;
	break;
   default:
	recalc = false;
    }

    if (recalc)
    {
	myNeedsRecomputeLastPageFlag = true;
	recomputeAndDraw();
    }
    else if (isVisible())
    {
	QicsRegion vr = gridInfo().visualRegion(region);
	paintRegion(vr, 0);
    }
}

void
QicsScreenGrid::handleGridPropertyChange(QicsGridStyle::QicsGridStyleProperty prop)
{
    bool recalc = false;
    bool repaint = false;

    switch (prop)
    {
    case QicsGridStyle::HorizontalGridLinesVisible:
    case QicsGridStyle::VerticalGridLinesVisible:
    case QicsGridStyle::HorizontalGridLineStyle:
    case QicsGridStyle::VerticalGridLineStyle:
    case QicsGridStyle::HorizontalGridLinePen:
    case QicsGridStyle::VerticalGridLinePen:
    case QicsGridStyle::GridCellClipping:
    case QicsGridStyle::DrawPartialCells:
    case QicsGridStyle::CellOverflowBehavior:
    case QicsGridStyle::MaxOverflowCells:
    case QicsGridStyle::CurrentCellBorderWidth:
	// These only require a redraw, not a recalc
	repaint = true;
	break;

    case QicsGridStyle::HorizontalGridLineWidth:
    case QicsGridStyle::VerticalGridLineWidth:
	// Have to recalc when these change
	recalc = true;
	repaint = true;
	break;

    case QicsGridStyle::GridRepaintBehavior:
	if (myNeedsRecomputeCellsFlag)
	    recalc = true;
	else if (myNeedsRepaintFlag)
	    repaint = true;
	break;

    case QicsGridStyle::FrameLineWidth:
	// Have to set this in the widget
	updateLineWidth();
	recalc = true;
	repaint = true;
	break;

    case QicsGridStyle::FrameStyle:
	// Have to set this in the widget
	updateFrameStyle();
	recalc = true;
	repaint = true;
	break;

    case QicsGridStyle::Viewport:
	// Only handle this if we don't have an explicit viewport set
	if (!myViewport.isValid())
	{
	    updateViewport();
	    recalc = true;
	    repaint = true;
	}
	break;

    default:
	// If we don't know about it, take the safe road and recalc/repaint...
	recalc = true;
	repaint = true;
	break;
    }

    if (recalc)
    {
	myNeedsRecomputeLastPageFlag = true;
	recomputeAndDraw();
    }
    else if (repaint && isVisible())
    {
	redraw();
    }
}

void
QicsScreenGrid::drawHeaderResizeBar(int, int pos, QicsHeaderType type)
{
    QPainter paint(this);
    QPen pen(gray);

    paint.setRasterOp(XorROP);
    paint.setClipRect(contentsRect());

    if (type == RowHeader)
    {
	pen.setWidth(myMainGrid->horizontalGridLineWidth());
	paint.setPen(pen);

	int x1 = contentsRect().left();
	int x2 = contentsRect().right();

	// Clear the old line
	if (myLastResizeLinePosition >= 0)
	{
	    paint.drawLine(x1, myLastResizeLinePosition,
			   x2, myLastResizeLinePosition);
	}

	// Draw the new line
	paint.drawLine(x1, pos, x2, pos);

	myLastResizeLinePosition = pos;
    }
    else
    {
	pen.setWidth(myMainGrid->verticalGridLineWidth());
	paint.setPen(pen);

	int y1 = contentsRect().top();
	int y2 = contentsRect().bottom();

	// Clear the old line
	if (myLastResizeLinePosition >= 0)
	{
	    paint.drawLine(myLastResizeLinePosition, y1,
			   myLastResizeLinePosition, y2);
	}

	// Draw the new line
	paint.drawLine(pos, y1, pos, y2);

	myLastResizeLinePosition = pos;
    }
}

void
QicsScreenGrid::traverse(QicsScrollDirection dir)
{
    switch(dir)
    {
    case ScrollLeft:
	traverseLeft();
	break;
    case ScrollRight:
	traverseRight();
	break;
    case ScrollUp:
	traverseUp();
	break;
    case ScrollDown:
	traverseDown();
	break;
    default:
	return;
    }
}
