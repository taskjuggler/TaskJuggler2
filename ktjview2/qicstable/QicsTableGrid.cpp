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


#include <QicsTableGrid.h>

#include <QicsDataItem.h>
#include <QicsCellDisplay.h>
#include <QicsDimensionManager.h>
#include <QicsSelectionManager.h>
#include <QicsSpanManager.h>
#include <QicsTableRegionDrag.h>

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

QicsTableGrid::QicsTableGrid(QWidget *w, QicsGridInfo &info,
			     int top_row,
			     int left_column)
    :QicsScreenGrid(w, info, top_row, left_column),
     myDoingSelectionFlag(false)
{
    mySelectionAnchorCell = 0;
    mySelectionCurrentCell = 0;
    myEditable = true;

    setAcceptDrops(true);
}

QicsTableGrid::~QicsTableGrid()
{
    delete mySelectionAnchorCell;
    delete mySelectionCurrentCell;
}

QicsTableGrid *
QicsTableGrid::createGrid(QWidget *w, QicsGridInfo &info,
			  int top_row, int left_column)
{
    return (new QicsTableGrid(w, info, top_row, left_column));
}

void
QicsTableGrid::reset(void)
{
    clearSelection();
    uneditCurrentCell();
    setCurrentCell(QicsICell());
}

/////////////////////////////////////////////////////////////////
/////////////        Event Handlers              ////////////////
/////////////////////////////////////////////////////////////////

// We reimplement this so we can get the first shot at key press
// events before the Qt traversal code steals tabs and shift-tabs.

bool QicsTableGrid::event( QEvent *e )
{
    if (e->type() == QEvent::KeyPress)
    {
	QKeyEvent *ke = static_cast<QKeyEvent *> (e);

	keyPressEvent(ke);
	return (ke->isAccepted());
    }

    return QWidget::event(e);
}

void
QicsTableGrid::handleMousePressEvent(const QicsICell &cell, QMouseEvent *m)
{
    if (!cell.isValid())
	return;

    if (m->button() == LeftButton)
    {
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
	else  // no modifiers
	{
	    if (traverseToCell(cell.row(), cell.column(), false))
		beginSelection(cell.row(), cell.column());
	}
    }
    else if (m->button() == MidButton)
    {
	prepareDrag(cell);
    }
}

void
QicsTableGrid::handleMouseReleaseEvent(const QicsICell &cell, QMouseEvent *m)
{
    if (!cell.isValid())
	return;

    if (m->button() == LeftButton)
    {
	if (myDoingSelectionFlag)
	    endSelection(cell.row(), cell.column());
    }
}

void
QicsTableGrid::handleMouseDoubleClickEvent(const QicsICell &cell, QMouseEvent *m)
{
    if (m->state() & LeftButton)
    {
	QicsICell cur_cell = currentCell();

	if (cur_cell.isValid() && (cur_cell == cell))
	{
	    // Double click means edit this cell
	    editCurrentCell();
	}
    }
}

void
QicsTableGrid::handleMouseMoveEvent(const QicsICell &cell, QMouseEvent *m)
{
    if (!cell.isValid())
	return;

    if (m->state() & LeftButton)
    {
	if (myDoingSelectionFlag)
	{
	    if (m->x() < 0)
	    {
		scrollLeft(1);
	    }
	    else if (m->x() > width())
	    {
		scrollRight(1);
	    }
	    else if (m->y() < 0)
	    {
		scrollUp(1);
	    }
	    else if (m->y() > height())
	    {
		scrollDown(1);
	    }

	    traverseToCell(cell.row(), cell.column(), false);
	    dragSelection(cell.row(), cell.column());
	}
    }
    else if (m->state() & MidButton)
    {
	if ((m->state() & ShiftButton) || (m->state() & ControlButton))
	{
	    startDrag(QDragObject::DragCopy);
	}
	else
	{
	    startDrag(QDragObject::DragMove);
	}
    }
}

void
QicsTableGrid::focusInEvent(QFocusEvent *)
{
    QicsICell cur_cell = currentCell();

    if (!myPlacingEntryWidgetFlag && cur_cell.isValid())
    {
	styleManager().setCellProperty(modelRowIndex(cur_cell.row()),
				       modelColumnIndex(cur_cell.column()),
				       QicsCellStyle::Current,
				       static_cast<void *> (new bool(true)));
    }
}

void
QicsTableGrid::focusOutEvent(QFocusEvent *)
{
    QicsICell cur_cell = currentCell();

    if (!myPlacingEntryWidgetFlag && cur_cell.isValid())
    {
	styleManager().setCellProperty(modelRowIndex(cur_cell.row()),
				       modelColumnIndex(cur_cell.column()),
				       QicsCellStyle::Current,
				       static_cast<void *> (new bool(false)));
    }
}

void
QicsTableGrid::dragMoveEvent(QDragMoveEvent *event)
{
    QicsICell cell = cellAt(event->pos().x(), event->pos().y(), false);

    bool canDrop = canDropAt(event, cell);
    QRect area = cellDimensions(cell, true);
    if(canDrop) {
	event->accept(area);
    } else {
	event->ignore(area);
    }
}

/////////////////////////////////////////////////////////////////
/////////////////    Actions      ///////////////////////////////
/////////////////////////////////////////////////////////////////

bool
QicsTableGrid::canDropAt(QDragMoveEvent *event, const QicsICell &cell) const
{
    if (!cell.isValid())
	return false;

    myCell->setRowIndex(cell.row());
    myCell->setColumnIndex(cell.column());
    
    if (!myCell->enabled() || myCell->readOnly())
	return false;

    return (QTextDrag::canDecode(event) || QImageDrag::canDecode(event));
}

void
QicsTableGrid::dropAt(QDropEvent *event, const QicsICell &cell)
{
    gridInfo().paste(event, cell);
}

//////////////////////////////////////////////////////////////////////
/////////////        Selection methods                 ///////////////
//////////////////////////////////////////////////////////////////////

void
QicsTableGrid::selectCell(int row, int col)
{
    clearSelection();
    addSelection(row, col);
    endSelection(row, col);
}

void
QicsTableGrid::beginSelection(int row, int col)
{
    if (isCellValid(row, col))
    {
	myDoingSelectionFlag = true;
	reportSelection(row, col, SelectionBegin);
    }
}

void
QicsTableGrid::endSelection(int row, int col)
{
    if (isCellValid(row, col))
    {
	myDoingSelectionFlag = false;

	reportSelection(row, col, SelectionEnd);
    }
}

void
QicsTableGrid::dragSelection(int row, int col)
{
    if (isCellValid(row, col))
    {
	reportSelection(row, col, SelectionDrag);
    }
}

void
QicsTableGrid::extendSelection(int row, int col)
{
    if (isCellValid(row, col))
    {
	reportSelection(row, col, SelectionExtend);
    }
}

void
QicsTableGrid::addSelection(int row, int col)
{
    if (isCellValid(row, col))
    {
	myDoingSelectionFlag = true;
	reportSelection(row, col, SelectionAdd);
    }
}

void QicsTableGrid::clearSelection(void)
{
    reportSelection(-1, -1, SelectionNone);
}

void
QicsTableGrid::reportSelection(int row, int col, QicsSelectionType stype)
{
    int last_row, last_col;
    int anchor_row, anchor_col;

    if (mySelectionCurrentCell)
    {
	last_row = mySelectionCurrentCell->row();
	last_col = mySelectionCurrentCell->column();
    }
    else
    {
	last_row = row;
	last_col = col;
    }


    // If we are dragging and we haven't moved into a new cell, don't
    // signal anything

    if ((stype == SelectionDrag) &&
	(row == last_row) && (col == last_col))
    {
	return;
    }

    // Set anchor cell if necessary
    if ((stype == SelectionBegin) ||
	(stype == SelectionAdd))
    {
	setSelectionAnchorCell(new QicsICell(row, col));
    }

    if (!mySelectionAnchorCell || (stype == SelectionAdd))
    {
	anchor_row = row;
	anchor_col = col;
    }
    else
    {
	anchor_row = mySelectionAnchorCell->row();
	anchor_col = mySelectionAnchorCell->column();
    }

    // No more selection...
    if (stype == SelectionNone)
	setSelectionAnchorCell(0);

    // Set selection cell
    QicsICell *cell;

    if (stype == SelectionNone)
	cell = 0;
    else
	cell = new QicsICell(row, col);

    setSelectionCurrentCell(cell);

    selectionManager().processSelectionEvent(stype,
					     anchor_row, anchor_col,
					     row, col);

#ifdef notdef
    qDebug("Grid SELECTION: (%d) (%d, %d) to (%d, %d)\n", stype,
	   anchor_row, anchor_col, row, col);
#endif
}

void
QicsTableGrid::setSelectionAnchorCell(QicsICell *cell)
{
    if (mySelectionAnchorCell)
	delete mySelectionAnchorCell;

    mySelectionAnchorCell = cell;
}

void
QicsTableGrid::setSelectionCurrentCell(QicsICell *cell)
{
    if (mySelectionCurrentCell)
	delete mySelectionCurrentCell;

    mySelectionCurrentCell = cell;
}

//////////////////////////////////////////////////////////////////////
/////////////     Entry Widget methods                 ///////////////
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
////////////         Slots Methods                     ///////////////
//////////////////////////////////////////////////////////////////////

void
QicsTableGrid::handleGridPropertyChange(QicsGridStyle::QicsGridStyleProperty prop)
{
    bool recalc = false;
    bool repaint = false;

    switch (prop)
    {
    case QicsGridStyle::ClickToEdit:
    case QicsGridStyle::AutoSelectCellContents:
    case QicsGridStyle::EnterTraversalDirection:
    case QicsGridStyle::TabTraversalDirection:
	// Don't need to do anything for these
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

#include "QicsTableGrid.moc"
