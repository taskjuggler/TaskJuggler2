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


#include <qmemarray.h>

#include <QicsHeaderGrid.h>
#include <QicsGridStyle.h>
#include <QicsDimensionManager.h>
#include <QicsSelectionManager.h>
#include <QicsSpanManager.h>
#include <QicsDataModel.h>
#include <QicsRowColumnDrag.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE 
#undef CREATE_OBJS_WITH_QICSTABLE
#endif
#include <QicsCell.h>
#include <QicsHeader.h>

#define MAJOR_AXIS_INDEX(row, col) (myType == RowHeader ? (row) : (col))
#define MAJOR_AXIS_POS(x, y) (myType == RowHeader ? (y) : (x))


///////////////////////////////////////////////////////////////////////

QicsHeaderGrid::QicsHeaderGrid(QWidget *w, QicsGridInfo &info,
			       QicsHeaderType type)
    : QicsScreenGrid(w, info, 0, 0),
      myType(type),
      myGripThreshold(5),
      myMouseInGrip(false),
      myDoingResize(false),
      myDoingSelectionFlag(false),
      myAnchorIndex(-1),
      mySelectionIndex(-1)
{
    myEditable = true;

    QSizePolicy p;

    if (myType == RowHeader)
	p = QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    else
	p = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    setSizePolicy(p);
    setAcceptDrops(true);
}

QicsHeaderGrid::~QicsHeaderGrid()
{
}

QicsHeaderGrid *
QicsHeaderGrid::createGrid(QWidget *w, QicsGridInfo &info,
			   QicsHeaderType type)
{
    return (new QicsHeaderGrid(w, info, type));
}

QicsRegion
QicsHeaderGrid::currentViewport(void) const
{
    QicsRegion vp = QicsGrid::currentViewport();

    if (myType == RowHeader)
    {
	if (vp.endColumn() == QicsLAST_COLUMN)
	    vp.setEndColumn(0);
    }
    else
    {
	if (vp.endRow() == QicsLAST_ROW)
	    vp.setEndRow(0);
    }

    return vp;
}

QSize
QicsHeaderGrid::sizeHint(void) const
{
    QSize min = minimumSizeHint();
    QicsRegion real_vp = currentViewport();
    QicsHeader hdr(&(gridInfo()));

    int height, width;

    if (myType == RowHeader)
    {
	height = min.height();

	width = (dimensionManager().regionWidth(QicsRegion(real_vp.startRow(),
							   real_vp.startColumn(),
							   real_vp.startRow(),
							   real_vp.endColumn())) +
		 (2 * hdr.verticalGridLineWidth()) + 
		 (2 * frameWidth()));
    }
    else
    {
	height = (dimensionManager().regionHeight(QicsRegion(real_vp.startRow(),
							     real_vp.startColumn(),
							     real_vp.endRow(),
							     real_vp.startColumn())) +
		  (2 * hdr.horizontalGridLineWidth()) +
		  (2 * frameWidth()));

	width = min.width();
    }

    return (QSize(width, height));
}

void
QicsHeaderGrid::handleMousePressEvent(const QicsICell &cell, QMouseEvent *m)
{
    if (!cell.isValid())
	return;

    if (m->button() == LeftButton)
    {
	if (myMouseInGrip)
	{
	    myMouseInGrip = false;
	    myDoingResize = true;
	    return;
	}

	if (m->state() & ShiftButton)
	{
	    traverseToCell(cell.row(), cell.column(), false);
	    extendSelection(cell.row(), cell.column());
	}
	else if (m->state() & ControlButton)
	{
	    traverseToCell(cell.row(), cell.column(), false);
	    addSelection(cell.row(), cell.column());
	}
	else
	{
	    traverseToCell(cell.row(), cell.column(), false);
	    beginSelection(cell.row(), cell.column());
	}
    }
    else if (m->button() == MidButton)
    {
	prepareDrag(cell);
    }
}

void
QicsHeaderGrid::handleMouseReleaseEvent(const QicsICell &cell, QMouseEvent *m)
{
    if (myDoingResize)
    {
	QicsHeader hdr(&(gridInfo()));

	if (hdr.allowUserResize())
	    finishResize();
    }
    else if (m->button() == LeftButton)
    {
	endSelection(cell.row(), cell.column());
    }
}

void
QicsHeaderGrid::handleMouseMoveEvent(const QicsICell &, QMouseEvent *m)
{
    if ((myDoingResize) && (m->state() == LeftButton))
    {
	handleCellResize(m);
	return;
    }

    if (m->state() & LeftButton)
    {
	if (myDoingSelectionFlag)
	{
	    dragSelection(m);
	}
    }
    else if (m->state() & MidButton) {
	if ((m->state() & ShiftButton) || (m->state() & ControlButton))
	{
	    startDrag(QDragObject::DragCopy);
       	} else
	{
	    startDrag(QDragObject::DragMove);
       	}
    }
    else
    {
	// check if we are interested in resizing anything..

	QicsHeader hdr(&(gridInfo()));

	if (hdr.allowUserResize())
	{
	    setResizeCursor(m);
	}
    }
}

void
QicsHeaderGrid::handleMouseDoubleClickEvent(const QicsICell &, QMouseEvent *m)
{
    int in_cell, close_cell;

    if (isWithinResizeThreshold(m->x(), m->y(), &in_cell, &close_cell))
    {
	emit gripDoubleClicked(QICS_MIN(in_cell, close_cell),
			       m->button(),
			       myType);
    }
}

void
QicsHeaderGrid::finishResize(void)
{
    // unset...
    unsetCursor();

    myDoingResize = false;

    if (myExpandingCellStartPosition == myExpandingCellCurrentPosition)
	return;

    int new_size = myExpandingCellSize +
        (myExpandingCellCurrentPosition -
         myExpandingCellStartPosition);
    int old_size;

    // set the new value, do some bounding checks to make sure they
    //aren't doing something which is insane...
    if (myType == RowHeader)
    {
        int min = dimensionManager().rowMinHeight(myExpandingCell);

        if (new_size < min)
            new_size = min;

	old_size = dimensionManager().rowHeight(myExpandingCell);

	dimensionManager().setRowHeightInPixels(myExpandingCell, new_size);
    }
    else
    {
        int min = dimensionManager().columnMinWidth(myExpandingCell);

        if (new_size < min)
            new_size = min;

	old_size = dimensionManager().columnWidth(myExpandingCell);

	dimensionManager().setColumnWidthInPixels(myExpandingCell, new_size);
    }

    // let the world know...
    emit sizeChange(myExpandingCell, old_size, new_size, myType);

    recomputeAndDraw();
}

void
QicsHeaderGrid::handleCellResize( QMouseEvent *m )
{
    if (myType == RowHeader)
	myExpandingCellCurrentPosition = m->y();
    else
	myExpandingCellCurrentPosition = m->x();

    emit resizeInProgress(myExpandingCell, myExpandingCellCurrentPosition, myType);
}

bool QicsHeaderGrid::isWithinResizeThreshold(int x, int y,
					     int *in_cell, int *close_cell) const
{
    QicsICell cell = cellAt(x, y, true);

    if (myType == RowHeader)
    {
	*in_cell = cell.row();

	if ((cell.row() < myTopRow) || (cell.row() > myBottomRow))
	    return false;

	int prev_row, next_row;
	QicsRegion span_region;

	if ((styleManager().spanManager())->isSpanner(myInfo, cell.row(), cell.column(),
						      span_region))
	{
	    prev_row = cell.row() - 1; 
	    next_row = span_region.endRow() + 1;
	}
	else
	{
	    prev_row = cell.row() - 1;
	    next_row = cell.row() + 1;
	}

	if ((next_row <= myBottomRow) &&
	    ((myRowPositions[next_row] - myGripThreshold) < y))
	{
	    *close_cell = next_row;
	    return true;
	}

	if (next_row > myBottomRow)
	{
	    int bottom_edge = (myRowPositions[myBottomRow] +
			       dimensionManager().rowHeight(myBottomRow));

	    if ((y > (bottom_edge - myGripThreshold)) && (y < bottom_edge))
	    {
		*close_cell = next_row;
		return true;
	    }
	}

	if ((prev_row >= myTopRow) &&
	    ((myRowPositions[prev_row] + myGripThreshold) > y))
	{
	    *close_cell = prev_row;
	    return true;
	}

	return false;
    }
    else
    {
	*in_cell = cell.column();

	if ((cell.column() < myLeftColumn) || (cell.column() > myRightColumn))
	    return false;

	int prev_col, next_col;
	QicsRegion span_region;

	if ((styleManager().spanManager())->isSpanner(myInfo, cell.row(), cell.column(),
						      span_region))
	{
	    prev_col = cell.column() - 1; 
	    next_col = span_region.endColumn() + 1;
	}
	else
	{
	    prev_col = cell.column() - 1;
	    next_col = cell.column() + 1;
	}

	if ((next_col <= myRightColumn) &&
	    ((myColumnPositions[next_col] - myGripThreshold) < x))
	{
	    *close_cell = next_col;
	    return true;
	}

	if (next_col > myRightColumn)
	{
	    int right_edge = (myColumnPositions[myRightColumn] +
			      dimensionManager().columnWidth(myRightColumn));

	    if ((x > (right_edge - myGripThreshold)) && (x < right_edge))
	    {
		*close_cell = next_col;
		return true;
	    }
	}

	if ((prev_col >= myLeftColumn) &&
	    ((myColumnPositions[cell.column()] + myGripThreshold) > x))
	{
	    *close_cell = prev_col;
	    return true;
	}

	return false;
    }
}

bool
QicsHeaderGrid::setResizeCursor( QMouseEvent *m )
{
    int idx;
    int close_idx;

    bool isWithin = isWithinResizeThreshold(m->x(), m->y(),
					    &idx, &close_idx);

    if (!myMouseInGrip && isWithin)
    {
	if (myType == RowHeader)
	    setCursor(splitVCursor);
	else
	    setCursor(splitHCursor);
	
	myMouseInGrip = true;
	myExpandingCell = QICS_MIN(idx, close_idx);
	if (myType == RowHeader)
	{
	    myExpandingCellSize =
		dimensionManager().rowHeight(myExpandingCell);
	    
	    myExpandingCellStartPosition = m->y();
	    myExpandingCellCurrentPosition = m->y();
	}
	else
	{
	    myExpandingCellSize =
		dimensionManager().columnWidth(myExpandingCell);
	    
	    myExpandingCellStartPosition = m->x();
	    myExpandingCellCurrentPosition = m->x();
	}

	return true;
    }

    if (myMouseInGrip && !isWithin)
    {
	unsetCursor();
	myMouseInGrip = false;
    }

    if (!myMouseInGrip)
    {
	QicsICell cell = cellAt(m->x(), m->y(), false);

	myCell->setRowIndex(cell.row());
	myCell->setColumnIndex(cell.column());

	setCursor(myCell->cursor());
    }

    return (false);
}

void
QicsHeaderGrid::keyPressEvent (QKeyEvent *)
{
}

//////////////////////////////////////////////////////////////////////
/////////////        Selection methods                 ///////////////
//////////////////////////////////////////////////////////////////////

void
QicsHeaderGrid::selectCell(int, int)
{
}


void
QicsHeaderGrid::beginSelection(int row, int col)
{
    myAnchorIndex = MAJOR_AXIS_INDEX(row, col);
    myDoingSelectionFlag = true;

    reportSelection(row, col, SelectionBegin);
}

void
QicsHeaderGrid::endSelection(int row, int col)
{
    myDoingSelectionFlag = false;

    reportSelection(row, col, SelectionEnd);
}

void
QicsHeaderGrid::extendSelection(int row, int col)
{
    reportSelection(row, col, SelectionExtend);
}

void
QicsHeaderGrid::addSelection(int row, int col)
{
    myDoingSelectionFlag = true;
    reportSelection(row, col, SelectionAdd);
}

void QicsHeaderGrid::clearSelection(void)
{
    reportSelection(-1, -1, SelectionNone);
}

void
QicsHeaderGrid::dragSelection( QMouseEvent *m )
{
    QicsICell cell = cellAt(m->x(), m->y(), true);

    QicsScrollDirection direction = ScrollNone;

    if (myType == RowHeader)
    {
	if (m->y() < 0)
	{
	    direction = ScrollUp;
	}
	else if (m->y() > height())
	{
	    direction = ScrollDown;
	}
    }
    else
    {
	if (m->x() < 0)
	{
	    direction = ScrollLeft;
	}
	else if (m->x() > width())
	{
	    direction = ScrollRight;
	}
    }
    
    if (direction != ScrollNone)
    {
	emit scrollRequest(direction, 1);
    }
    
    reportSelection(cell.row(), cell.column(), SelectionDrag);
}

void
QicsHeaderGrid::reportSelection(int row, int col, QicsSelectionType stype)
{
    int idx = MAJOR_AXIS_INDEX(row, col);
    int prev_index;
    int anchor_index;

    int start_index, end_index;

    QicsRegion span;
    if ((styleManager().spanManager())->isSpanner(myInfo, row, col, span))
    {
	start_index = MAJOR_AXIS_INDEX(span.startRow(), span.startColumn());
	end_index = MAJOR_AXIS_INDEX(span.endRow(), span.endColumn());
    }
    else
    {
	start_index = end_index = idx;
    }

    if (mySelectionIndex != -1)
    {
	prev_index = mySelectionIndex;
    }
    else
    {
	prev_index = idx;
    }

    // If we are dragging and we haven't moved into a new cell, don't
    // signal anything

    if ((stype == SelectionDrag) && (idx == prev_index))
	return;

    if (stype == SelectionNone)
	mySelectionIndex = -1;
    else
	mySelectionIndex = idx;

    if (stype == SelectionAdd)
    {
	anchor_index = start_index;
    }
    else
    {
	anchor_index = myAnchorIndex;
    }

    if (myType == RowHeader)
    {
	selectionManager().processSelectionEvent(stype,
						 anchor_index, 0,
						 end_index, Qics::QicsLAST_COLUMN);
    }
    else
    {
	selectionManager().processSelectionEvent(stype,
						 0, anchor_index,
						 Qics::QicsLAST_ROW, end_index);
    }
}

////////////////////////////////////////////////////////////////////////

void
QicsHeaderGrid::reset()
{
    updateGeometry();
}

void
QicsHeaderGrid::handleGridPropertyChange(QicsGridStyle::QicsGridStyleProperty prop)
{
    bool recalc = false;
    bool repaint = false;

    switch (prop)
    {
    case QicsGridStyle::AllowUserResize:
	break;
    default:
	QicsScreenGrid::handleGridPropertyChange(prop);
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


////////////////////////////////////////////////////////////////////////////
//////////////////////    Drag and Drop Methods    /////////////////////////
////////////////////////////////////////////////////////////////////////////


void
QicsHeaderGrid::dragMoveEvent(QDragMoveEvent *event)
{
    QicsICell cell = cellAt(event->pos().x(), event->pos().y(), false);
    
    bool canDrop = canDropAt(event, cell); 
    QRect area = cellDimensions(cell, true);

    if (canDrop)
	event->accept(area);
    else
	event->ignore(area);
}


bool
QicsHeaderGrid::canDropAt(QDragMoveEvent *event, const QicsICell &cell) const
{
    if (!cell.isValid())
	return false;

    QicsHeader hdr(&(gridInfo()));

    if (!hdr.enabled() || hdr.readOnly())
	return false;

    QicsIndexType select = (myType == RowHeader ? RowIndex : ColumnIndex);
    if(event->source() == 0)
    {
	select = (QicsIndexType) -1;
    }
    bool tmp = QicsRowColumnDrag::canDecode(event, select);
    // qDebug("canDecode() -> %d\n", tmp);
    return tmp;
}

void
QicsHeaderGrid::dropAt(QDropEvent *event, const QicsICell &cell)
{
    /*!
     * dropAt tries the following methods of getting the data
     * If it's an intra-application drag
     *     look for row/column list first, then we can do an easy
     *     move or copy
     * if that fails, or if it is an inter-app drag, go right
     * to trying to decode celldata
     */

    if(event->source() != 0) {
	// intra-application drag
	QMemArray<int> items;

	// qDebug("intra-app drag to %d,%d\n", cell.row(), cell.column());
	/* If we can decode a row or column list (as needed, depending
	 * on what kind of header we are), then we can do a move.
	 * - We have to map back from model coordinates to view, because
	 * that's what the user sees.
	 * - We'll want to sort the list, because multiple selections
	 * can come in any order, but the user would not want them
	 * any way but in visually increasing order.
	 */
	QicsIndexType desiredType = (myType == RowHeader ?
					RowIndex : ColumnIndex);
	if(QicsRowColumnDrag::decode(event,desiredType,items))
	{
	    for(unsigned int i = 0; i < items.size(); i++)
	    {
		// qDebug("model index %d\n", items[i]);
		if(myType == RowHeader) {
		    items.at(i) = gridInfo().visualRowIndex(items.at(i));
		} else {
		    items.at(i) = gridInfo().visualColumnIndex(items.at(i));
		}
	    }
	    // we sort the indices to null out the effects of out
	    // of order selection
	    items.sort();

	    if(myType == RowHeader)
	    {
		gridInfo().moveRows(cell.row(), items);
	    }
	    else
	    {
		gridInfo().moveColumns(cell.column(), items);
	    }
	}
	return;
    }
    else 
    {
	qDebug("Drag from another application not supported yet.\n");
    }

    if(event->provides(QICS_MIME_CELLDATA)) {
	qDebug("provides celldata\n");
	// TODO: the whole XML transfer thing
	// return;
    }
    event->ignore();
    return;
}



void
QicsHeaderGrid::startDrag(QDragObject::DragMode mode)
{
    QicsHeader hdr(&(gridInfo()));
    
    if (!hdr.allowUserMove() || !myDragCell || !myDragCell->isValid())
	return;

    // Get the drag object
    QicsRowColumnDrag *obj =
	QicsRowColumnDrag::getDragObject(&gridInfo(),
					 (myType == RowHeader ? RowIndex : ColumnIndex),
					 myDragCell,
					 this);
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
	remove = obj->dragMove();
    else
	remove = obj->drag();
}

