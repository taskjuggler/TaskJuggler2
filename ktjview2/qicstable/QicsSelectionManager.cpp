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


#include <QicsSelectionManager.h>
#include <QicsRegion.h>
#include <QicsGrid.h>
#include <QicsHeaderGrid.h>


//#define DEBUG_SELECTION

#ifdef notdef
static void dumpList(QicsSelectionList *list)
{
    if(list == 0) return;
    QicsSelectionList::iterator iter;
    for (iter = list->begin(); iter != list->end(); ++iter) {
	QicsSelection sel = *iter;

	qDebug("[%d,%d] -- [%d,%d]\n", sel.topRow(), sel.leftColumn(),
				sel.bottomRow(), sel.rightColumn());
    }
}
#endif

//////////////////////////////////////////////////////////////////
/////////////       QicsSelection                    /////////////
//////////////////////////////////////////////////////////////////

QicsSelection::QicsSelection(void):
    mySelectState(true),
    myAnchorRow(-1),
    myAnchorCol(-1),
    myEndRow(-1),
    myEndCol(-1)
{
}

QicsSelection::QicsSelection(int anchor_row, int anchor_col,
			     int end_row, int end_col,
			     bool select_state) :
    mySelectState(select_state),
    myAnchorRow(anchor_row),
    myAnchorCol(anchor_col),
    myEndRow(end_row),
    myEndCol(end_col)
{
}

QicsSelection::QicsSelection(const QicsSelection &sel) :
    mySelectState(sel.mySelectState),
    myAnchorRow(sel.myAnchorRow),
    myAnchorCol(sel.myAnchorCol),
    myEndRow(sel.myEndRow),
    myEndCol(sel.myEndCol)
{
}

bool
QicsSelection::isValid(void) const
{
    return ((anchorRow() >= 0) && (anchorColumn() >= 0) &&
	    (endRow() >= 0) && (endColumn() >= 0));
}

void
QicsSelection::setAnchorRow(int anchor_row)
{
    myAnchorRow = anchor_row;
}

void
QicsSelection::setAnchorColumn(int anchor_col)
{
    myAnchorCol = anchor_col;
}

void 
QicsSelection::setAnchorCell(int anchor_row, int anchor_col)
{
    myAnchorRow = anchor_row;
    myAnchorCol = anchor_col;
}

void
QicsSelection::setEndRow(int end_row)
{
    myEndRow = end_row;
}

void
QicsSelection::setEndColumn(int end_col)
{
    myEndCol = end_col;
}

void
QicsSelection::setEndCell(int end_row, int end_col)
{
    myEndRow = end_row;
    myEndCol = end_col;
}

void
QicsSelection::setEntireRow(int row)
{
    myAnchorRow = row;
    myAnchorCol = 0;

    myEndRow = row;
    myEndCol = Qics::QicsLAST_COLUMN;
}

void
QicsSelection::setEntireColumn(int col)
{
    myAnchorRow = 0;
    myAnchorCol = col;

    myEndRow = Qics::QicsLAST_ROW;
    myEndCol = col;
}

bool
QicsSelection::intersects(const QicsSelection &sel)
{
    return ((QICS_MAX(topRow(), sel.topRow()) <=
	     QICS_MIN(bottomRow(), sel.bottomRow())) &&
	    (QICS_MAX(leftColumn(), sel.leftColumn()) <=
	     QICS_MIN(rightColumn(), sel.rightColumn())));
}

QicsSelection
QicsSelection::operator&(const QicsSelection &sel)
{
    QicsSelection ns;

    ns.setAnchorRow(QICS_MAX(topRow(), sel.topRow()));
    ns.setEndRow(QICS_MIN(bottomRow(), sel.bottomRow()));
    ns.setAnchorColumn(QICS_MAX(leftColumn(), sel.leftColumn()));
    ns.setEndColumn(QICS_MIN(rightColumn(), sel.rightColumn()));

    return ns;
}

QicsSelection
QicsSelection::intersect(const QicsSelection &sel)
{
    return (*this & sel);
}

bool operator==(const QicsSelection &r1, const QicsSelection &r2)
{
    return ((r1.myAnchorRow == r2.myAnchorRow) &&
	    (r1.myAnchorCol == r2.myAnchorCol) &&
	    (r1.myEndRow == r2.myEndRow) &&
	    (r1.myEndCol == r2.myEndCol) &&
	    (r1.mySelectState == r2.mySelectState));
}

bool operator!=(const QicsSelection &r1, const QicsSelection &r2)
{
    return ((r1.myAnchorRow != r2.myAnchorRow) ||
	    (r1.myAnchorCol != r2.myAnchorCol) ||
	    (r1.myEndRow != r2.myEndRow) ||
	    (r1.myEndCol != r2.myEndCol) ||
	    (r1.mySelectState != r2.mySelectState));
}

////////////////////////////////////////////////////////////////////
/////////////       QicsSelectionList                  /////////////
////////////////////////////////////////////////////////////////////

QicsSelectionList::QicsSelectionList() :
    QValueVector<QicsSelection>()
{
}

bool
QicsSelectionList::isCellSelected(int row, int col) const
{
    QicsSelectionList::const_iterator iter;

    for (iter = begin(); iter != end(); ++iter)
    {
	const QicsSelection &sel = *iter;

	if ((row >= sel.topRow()) && (row <= sel.bottomRow()) &&
	    (col >= sel.leftColumn()) && (col <= sel.rightColumn()))
	{
	    return true;
	}
    }

    return false;
}

bool
QicsSelectionList::isRowSelected(int row) const
{
    QicsSelectionList::const_iterator iter;

    for (iter = begin(); iter != end(); ++iter)
    {
	const QicsSelection &sel = *iter;

	if ((row >= sel.topRow()) && (row <= sel.bottomRow()) &&
	    (sel.leftColumn() == 0) && (sel.rightColumn() == Qics::QicsLAST_COLUMN))
	{
	    return true;
	}
    }

    return false;
}

bool
QicsSelectionList::isColumnSelected(int col) const
{
    QicsSelectionList::const_iterator iter;

    for (iter = begin(); iter != end(); ++iter)
    {
	const QicsSelection &sel = *iter;

	if ((col >= sel.leftColumn()) && (col <= sel.rightColumn()) &&
	    (sel.topRow() == 0) && (sel.bottomRow() == Qics::QicsLAST_ROW))
	{
	    return true;
	}
    }

    return false;
}


////////////////////////////////////////////////////////////////////
/////////////       QicsSelectionManager               /////////////
////////////////////////////////////////////////////////////////////


QicsSelectionManager::QicsSelectionManager():
    QObject(),
    myStyleManager(0),
    myDataModel(0),
    mySelectionList(0),
    mySelectionActionList(0),
    mySelectionPolicy(SelectMultiple)
{
}

QicsSelectionManager::~QicsSelectionManager(void)
{
    if (mySelectionList)
	delete mySelectionList;

    if (mySelectionActionList)
	delete mySelectionActionList;
}

QicsDataModel *
QicsSelectionManager::dataModel(void) const
{
    return myDataModel;
}

void
QicsSelectionManager::setDataModel(QicsDataModel *sm)
{
    QicsDataModel *oldDM = myDataModel;
    myDataModel = sm;

    if (oldDM)
    {
	disconnect(oldDM, 0, this, 0);
    }

    if (myDataModel)
    {
	connect(myDataModel, SIGNAL(rowsInserted(int, int)),
		this, SLOT(insertRows(int, int)));
	connect(myDataModel, SIGNAL(columnsInserted(int, int)),
		this, SLOT(insertColumns(int, int)));
	connect(myDataModel, SIGNAL(rowsDeleted(int, int)),
		this, SLOT(deleteRows(int, int)));
	connect(myDataModel, SIGNAL(columnsDeleted(int, int)),
		this, SLOT(deleteColumns(int, int)));
    }
}

QicsStyleManager *
QicsSelectionManager::styleManager(void) const
{
    return myStyleManager;
}

void
QicsSelectionManager::setStyleManager(QicsStyleManager *sm)
{
    myStyleManager = sm;
}

void
QicsSelectionManager::setGridInfo(QicsGridInfo *gi)
{
    myGridInfo = gi;
}

void
QicsSelectionManager::addHeader(QicsHeaderGrid *header)
{
    myHeaderList.push_back(header);
}

void
QicsSelectionManager::removeHeader(QObject *header)
{
    QicsHeaderGridPV::iterator iter;

    for (iter = myHeaderList.begin(); iter != myHeaderList.end(); ++iter)
    {
	if (*iter == header)
	{
	    myHeaderList.erase(iter);
	    break;
	}
    }
}

/////////////////////////////////////////////////////////////////////////////


QicsSelectionList *QicsSelectionManager::selectionList(void) const
{
    if (mySelectionList)
	return new QicsSelectionList(*mySelectionList);
    else
	return 0;
}

QicsSelectionList *QicsSelectionManager::selectionActionList(void) const
{
    if (mySelectionActionList)
	return new QicsSelectionList(*mySelectionActionList);
    else
	return 0;
}

void QicsSelectionManager::setSelectionList(QicsSelectionList &sel_list)
{
    // Blow away the old one
    deleteSelection();

    if (SelectNone == mySelectionPolicy)
	return;

    // Copy the new list

    mySelectionList = new QicsSelectionList();
    mySelectionActionList = new QicsSelectionList();

    QicsSelectionList::iterator iter;

    // Go through each selection in list
    for (iter = sel_list.begin(); iter != sel_list.end();
	 ++iter)
    {
	QicsSelection sel = *iter;

	if ((mySelectionPolicy == SelectSingleRow) ||
	    (mySelectionPolicy == SelectMultipleRow))
	{
	    // Skip this selection if not selecting entire row
	    if ((sel.leftColumn() != 0) ||
		(sel.rightColumn() != Qics::QicsLAST_COLUMN))
	    {
		continue;
	    }
	}

	if ((mySelectionList->size() > 0) &&
	    ((mySelectionPolicy == SelectSingle) ||
	     (mySelectionPolicy == SelectSingleRow)))
	{
	    // We already got one selection
	    break;
	}

	// Save it off
	addToSelectionList(sel);
    }

    // Set the selection properties
    for (iter = mySelectionActionList->begin();
	 iter != mySelectionActionList->end();
	 ++iter)
    {
	QicsSelectState state;

	if ((*iter).selected())
	    state = QicsSelectTrue;
	else
	    state = QicsSelectFalse;

	setSelectionProperty(*iter, state);
    }

    announceChanges(false);
}

void QicsSelectionManager::clearSelectionList(void)
{
    deleteSelection();
    announceChanges(false);
}

void QicsSelectionManager::addSelection(QicsSelection &selection)
{
    if ((mySelectionPolicy != SelectMultiple) &&
	(mySelectionPolicy != SelectMultipleRow))
    {
	// Can't have more than one selection in other modes
	if (mySelectionList && mySelectionList->size() > 0)
	    return;
    }

    QicsSelectState state;

    if (selection.selected())
	state = QicsSelectTrue;
    else
	state = QicsSelectFalse;

    addToSelectionList(selection);
    setSelectionProperty(selection, state);
    announceChanges(false);
}

void QicsSelectionManager::setSelectionPolicy(QicsSelectionPolicy policy)
{
    mySelectionPolicy = policy;

    // Clear the current selection

    deleteSelection();
    announceChanges(false);
}


void QicsSelectionManager::processSelectionEvent(QicsSelectionType stype,
						 int begin_row, int begin_col,
						 int end_row, int end_col)
{
#ifdef DEBUG_SELECTION
    qDebug("***** QicsSelectionManager(%d, %d) to (%d, %d) %d",
	   begin_row, begin_col, end_row, end_col, stype);
#endif

    // Invalidate the region
    myAffectedRegion.setSize(QSize(-1,-1));

    if ((mySelectionPolicy == SelectSingleRow) ||
	(mySelectionPolicy == SelectMultipleRow))
    {
	// If the user clicked on the column header, we don't allow that.

	if ((begin_row == 0) && (end_row == QicsLAST_ROW))
	    return;

	begin_col = 0;
	end_col = Qics::QicsLAST_COLUMN;
    }

    bool in_progress = false;

    switch (stype)
    {
    case SelectionBegin:
	deleteSelection();
	beginSelection(begin_row, begin_col, end_row, end_col);
	in_progress = true;
	break;
    case SelectionDrag:
	dragSelection(begin_row, begin_col, end_row, end_col);
	in_progress = true;
	break;
    case SelectionEnd:
	endSelection(begin_row, begin_col, end_row, end_col);
	in_progress = false;
	break;
    case SelectionExtend:
	extendSelection(begin_row, begin_col, end_row, end_col);
	in_progress = false;
	break;
    case SelectionAdd:
	addSelection(begin_row, begin_col, end_row, end_col);
	in_progress = true;
	break;
    case SelectionNone:
	deleteSelection();
	in_progress = false;
	break;
    default:
	return;
    }

    announceChanges(in_progress);
}


void QicsSelectionManager::beginSelection(int begin_row, int begin_col,
					int end_row, int end_col)
{
    if (mySelectionPolicy == SelectNone)
	return;

    myCurrentSelection.setSelected(true);
    myCurrentSelection.setAnchorCell(begin_row, begin_col);
    myCurrentSelection.setEndCell(end_row, end_col);

    myCurrentDragAction = QicsSelectTrue;

    setSelectionProperty(QICS_MIN(begin_row, end_row),
			 QICS_MIN(begin_col, end_col),
			 QICS_MAX(begin_row, end_row),
			 QICS_MAX(begin_col, end_col),
			 QicsSelectTrue);
}

void QicsSelectionManager::dragSelection(int begin_row, int begin_col,
				       int end_row, int end_col)
{
    if ((mySelectionPolicy != SelectMultiple) &&
	(mySelectionPolicy != SelectMultipleRow))
    {
	// Can't drag select in other modes
	return;
    }

    QicsSelection &sel = myCurrentSelection;

    if (!sel.isValid())
	return;

    if ((end_row != sel.endRow()) && (end_col != sel.endColumn()))
    {
	extendSelection(begin_row, begin_col, end_row, end_col);
	return;
    }

    QPoint new_tl(QICS_MIN(sel.anchorColumn(), end_col),
		  QICS_MIN(sel.anchorRow(), end_row));
    QPoint new_br(QICS_MAX(sel.anchorColumn(), end_col),
		  QICS_MAX(sel.anchorRow(), end_row));

    QRect current_selection(new_tl, new_br);

    // If the old end cell is inside of the current selection rectangle,
    // the selection is growing
    bool growing = current_selection.contains(sel.endColumn(), sel.endRow());

    // Inside means closer to the anchor cell
    int inside_row = (growing ? sel.endRow() : end_row);
    int inside_col = (growing ? sel.endColumn() : end_col);
    int outside_row = (!growing ? sel.endRow() : end_row);
    int outside_col = (!growing ? sel.endColumn() : end_col);

    int inside_row_adj, inside_col_adj;

    if (inside_row == outside_row)
	inside_row_adj = 0;
    else if (inside_row < outside_row)
	inside_row_adj = 1;
    else
	inside_row_adj = -1;

    if (inside_col == outside_col)
	inside_col_adj = 0;
    else if (inside_col < outside_col)
	inside_col_adj = 1;
    else
	inside_col_adj = -1;

    QicsSelectState action;

    if (growing)
    {
	if (myCurrentDragAction == QicsSelectTrue)
	    action = QicsSelectTrue;
	else if (myCurrentDragAction == QicsSelectFalse)
	    action = QicsSelectFalse;
	else if (myCurrentDragAction == QicsSelectTrueRevert)
	    action = QicsSelectTrue;
	else
	    action = QicsSelectFalse;
    }
    else
    {
	if (myCurrentDragAction == QicsSelectTrue)
	    action = QicsSelectFalse;
	else if (myCurrentDragAction == QicsSelectFalse)
	    action = QicsSelectTrue;
	else if (myCurrentDragAction == QicsSelectTrueRevert)
	    action = QicsSelectFalseRevert;
	else
	    action = QicsSelectTrueRevert;
    }
    
    if (inside_col_adj != 0)
    {
	setSelectionProperty(QICS_MIN(sel.anchorRow(), outside_row),
			     QICS_MIN(inside_col+inside_col_adj, outside_col) ,
			     QICS_MAX(sel.anchorRow(), outside_row),
			     QICS_MAX(inside_col+inside_col_adj, outside_col),
			     action);
    }

    if (inside_row_adj != 0)
    {
	setSelectionProperty(QICS_MIN(inside_row+inside_row_adj, outside_row),
			     QICS_MIN(sel.anchorColumn(), inside_col),
			     QICS_MAX(inside_row+inside_row_adj, outside_row),
			     QICS_MAX(sel.anchorColumn(), inside_col),
			     action);
    }

    // Update the selection with the new end cell
    sel.setEndCell(end_row, end_col);
}

void QicsSelectionManager::extendSelection(int begin_row, int begin_col,
					 int end_row, int end_col)
{
    if ((mySelectionPolicy != SelectMultiple) &&
	(mySelectionPolicy != SelectMultipleRow))
    {
	// Can't extend selections in other modes
	return;
    }

    QicsSelection &sel = findSelectionBlock(begin_row, begin_col);

    if (!sel.isValid())
	return;

    QicsSelectState action;

    if (myCurrentDragAction == QicsSelectTrue)
	action = QicsSelectFalse;
    else if (myCurrentDragAction == QicsSelectFalse)
	action = QicsSelectTrue;
    else if (myCurrentDragAction == QicsSelectTrueRevert)
	action = QicsSelectFalseRevert;
    else
	action = QicsSelectTrueRevert;

    setSelectionProperty(QICS_MIN(begin_row, sel.endRow()),
			 QICS_MIN(begin_col, sel.endColumn()),
			 QICS_MAX(begin_row, sel.endRow()),
			 QICS_MAX(begin_col, sel.endColumn()),
			 action);

    if (myCurrentDragAction == QicsSelectTrue)
	action = QicsSelectTrue;
    else if (myCurrentDragAction == QicsSelectFalse)
	action = QicsSelectFalse;
    else if (myCurrentDragAction == QicsSelectTrueRevert)
	action = QicsSelectTrue;
    else
	action = QicsSelectFalse;

    setSelectionProperty(QICS_MIN(begin_row, end_row),
			 QICS_MIN(begin_col, end_col),
			 QICS_MAX(begin_row, end_row),
			 QICS_MAX(begin_col, end_col),
			 myCurrentDragAction);

    // Update the selection with the new end cell
    sel.setEndCell(end_row, end_col);
}

void QicsSelectionManager::addSelection(int begin_row, int begin_col,
				      int end_row, int end_col)
{
    if ((mySelectionPolicy != SelectMultiple) &&
	(mySelectionPolicy != SelectMultipleRow))
    {
	// Can't have more than one selection in other modes
	if (mySelectionList && mySelectionList->size() > 0)
	    return;
    }

    bool set = (* static_cast<bool *>
                (myStyleManager->getCellProperty(begin_row, begin_col,
                                                 QicsCellStyle::Selected)));

    myCurrentDragAction = (set ? QicsSelectFalseRevert : QicsSelectTrueRevert);
    myCurrentSelection.setSelected(!set);

    myCurrentSelection.setAnchorCell(begin_row, begin_col);
    myCurrentSelection.setEndCell(end_row, end_col);

    setSelectionProperty(QICS_MIN(begin_row, end_row),
			 QICS_MIN(begin_col, end_col),
			 QICS_MAX(begin_row, end_row),
			 QICS_MAX(begin_col, end_col),
			 (set ? QicsSelectFalse : QicsSelectTrue));
}

void QicsSelectionManager::endSelection(int , int ,
				      int , int )
{
    if (!myCurrentSelection.isValid())
	return;

    addToSelectionList(myCurrentSelection);

    // Invalidate the current selection item
    myCurrentSelection.setAnchorRow(-1);

    myCurrentDragAction = QicsSelectTrue;
}

void QicsSelectionManager::deleteSelection(void)
{
    if (mySelectionList)
    {
	QicsSelectionList::iterator iter;

	for (iter = mySelectionList->begin(); iter != mySelectionList->end();
	     ++iter)
	{
	    QicsSelection &sel = *iter;

#ifdef DEBUG_SELECTION2
	    qDebug("invalidateSelection (%d, %d) (%d, %d)\n",
		   sel.anchorRow(), sel.anchorColumn(),
		   sel.endRow(), sel.endColumn());
#endif

	    invalidateSelection(sel.anchorRow(), sel.anchorColumn(),
				sel.endRow(), sel.endColumn());
	}

	delete mySelectionList;

	mySelectionList = 0;
    }

    if (mySelectionActionList)
	delete mySelectionActionList;

    mySelectionActionList = 0;
}

void QicsSelectionManager::invalidateSelection(int begin_row, int begin_col,
					       int end_row, int end_col)
{
    setSelectionProperty(QICS_MIN(begin_row, end_row),
			 QICS_MIN(begin_col, end_col),
			 QICS_MAX(begin_row, end_row),
			 QICS_MAX(begin_col, end_col),
			 QicsSelectFalse);
}

void
QicsSelectionManager::addToSelectionList(QicsSelection &sel)
{
    if (!sel.isValid())
	return;

    if (!mySelectionList)
	mySelectionList = new QicsSelectionList();

    if (!mySelectionActionList)
	mySelectionActionList = new QicsSelectionList();

    mySelectionActionList->push_back(sel);

    if (sel.selected())
	mySelectionList->push_back(sel);
    else
    {
	QicsSelectionList *newlist = new QicsSelectionList();

	QicsSelectionList::iterator iter;
	for (iter = mySelectionList->begin(); iter != mySelectionList->end();
	     ++iter)
	{
	    QicsSelection &s = *iter;

	    if (s.intersects(sel))
	    {
		QicsSelection is = s & sel;

		// =======================
		// |  s                  |
		// |        S1           |
		// |                     |
		// |------==========-----|
		// |      |        |     |
		// |  S3  |  is    | S4  |
		// |      |        |     |
		// |------==========-----|
		// |                     |
		// |       S2            |
		// |                     |
		// =======================

		if (is.topRow() > s.topRow())  // S1
		{
		    QicsSelection s1;

		    s1.setAnchorRow(s.topRow());
		    s1.setEndRow(is.topRow() - 1);
		    s1.setAnchorColumn(s.leftColumn());
		    s1.setEndColumn(s.rightColumn());

		    newlist->push_back(s1);
		}

		if (is.bottomRow() < s.bottomRow())  // S2
		{
		    QicsSelection s2;

		    s2.setAnchorRow(is.bottomRow() + 1);
		    s2.setEndRow(s.bottomRow());
		    s2.setAnchorColumn(s.leftColumn());
		    s2.setEndColumn(s.rightColumn());

		    newlist->push_back(s2);
		}

		if (is.leftColumn() > s.leftColumn())  // S3
		{
		    QicsSelection s3;

		    s3.setAnchorRow(is.topRow());
		    s3.setEndRow(is.bottomRow());
		    s3.setAnchorColumn(s.leftColumn());
		    s3.setEndColumn(is.leftColumn() - 1);

		    newlist->push_back(s3);
		}

		if (is.rightColumn() < s.rightColumn())  // S4
		{
		    QicsSelection s4;

		    s4.setAnchorRow(is.topRow());
		    s4.setEndRow(is.bottomRow());
		    s4.setAnchorColumn(is.rightColumn() + 1);
		    s4.setEndColumn(s.rightColumn());

		    newlist->push_back(s4);
		}
	    }
	    else
		newlist->push_back(s);
	}

	// replace the selected list
	delete mySelectionList;
	mySelectionList = newlist;
    }
}

static QicsSelection invalidSelection;

QicsSelection &QicsSelectionManager::findSelectionBlock(int anchor_row,
							int anchor_col)
{
    if (myCurrentSelection.isValid())
    {
	if ((myCurrentSelection.anchorRow() == anchor_row) &&
	    (myCurrentSelection.anchorColumn() == anchor_col))
	{
	    return (myCurrentSelection);
	}
    }

    QicsSelectionList::iterator iter;

    if (!mySelectionList)
	return invalidSelection;

    for (iter = mySelectionList->begin(); iter != mySelectionList->end(); ++iter)
    {
	QicsSelection &sel = *iter;

	if ((sel.anchorRow() == anchor_row) &&
	    (sel.anchorColumn() == anchor_col))
	{
	    return (sel);
	}
    }

    return invalidSelection;
}

void QicsSelectionManager::setSelectionProperty(const QicsSelection &selection,
						QicsSelectionManager::QicsSelectState sel)
{
    setSelectionProperty(selection.topRow(), selection.leftColumn(),
			 selection.bottomRow(), selection.rightColumn(),
			 sel);
}

void QicsSelectionManager::setSelectionProperty(int begin_row, int begin_col,
				int end_row, int end_col,
				QicsSelectionManager::QicsSelectState sel)
{
    bool set;
    int i, j;

//#define DEBUG_SELECTION1
#ifdef DEBUG_SELECTION1
    qDebug("$$$ setSelectionProperty (%d, %d) to (%d, %d) %d\n",
	   begin_row, begin_col, end_row, end_col, sel);
#endif

    if (!myStyleManager)
	return;

    int lastCol = (myDataModel ? myDataModel->lastColumn() : Qics::QicsLAST_COLUMN);

    // Turn off the property change signal so the grids don't redraw each time
    // through the loop.
    myStyleManager->setReportChanges(false);

    bool skip_rest_of_rows = false;
    for (i = begin_row; i <= end_row && !skip_rest_of_rows; ++i)
    {
	int mRow = myGridInfo->modelRowIndex(i);
	if ((begin_col == 0) && (end_col == Qics::QicsLAST_COLUMN))
	{
	    // This means that we are selecting the entire row

	    setRowSelectionProperty(*myGridInfo, i, sel, myStyleManager);

	    QicsHeaderGridPV::iterator iter;

	    for (iter = myHeaderList.begin(); iter != myHeaderList.end(); ++iter)
	    {
		QicsHeaderGrid *hdr = *iter;

		if (hdr->type() == RowHeader)
		    setRowSelectionProperty(hdr->gridInfo(), i, sel,
					    hdr->gridInfo().styleManager());
	    }
	}
	else
	{
	    for (j = begin_col; (j <= end_col) && (j <= lastCol); ++j)
	    {
		int mCol = myGridInfo->modelColumnIndex(j);
		if ((begin_row == 0) && (end_row == Qics::QicsLAST_ROW))
		{
		    // This means that we are selecting the entire column
		    skip_rest_of_rows = true;

		    setColumnSelectionProperty(*myGridInfo, j, sel, myStyleManager);

		    QicsHeaderGridPV::iterator iter;

		    for (iter = myHeaderList.begin();
			 iter != myHeaderList.end();
			 ++iter)
		    {
			QicsHeaderGrid *hdr = *iter;

			// int modelCol = hdr->gridInfo().modelColumnIndex(j);
			if (hdr->type() == ColumnHeader)
			    setColumnSelectionProperty(hdr->gridInfo(),
					    		j, sel,
							hdr->gridInfo().styleManager());
		    }
		}
		else
		{
		    if (sel == QicsSelectionManager::QicsSelectTrue)
			set = true;
		    else if (sel == QicsSelectionManager::QicsSelectFalse)
			set =  false;
		    else
		    {
			bool cur = mySelectionList->isCellSelected(i, j);

			if (sel == QicsSelectTrueRevert)
			{
			    if (!cur)
				continue;
			    else
				set = true;
			}
			else // (sel == QicsSelectFalseRevert)
			{
			    if (cur)
				continue;
			    else
				set = false;
			}
		    }

		    if (set)
		    {
			bool *val = new bool(true);
			myStyleManager->setCellProperty(mRow, mCol,
							QicsCellStyle::Selected,
							(void *)val);
		    }
		    else
		    {
			// This is an optimization.  Many times, the only property set on
			// a cell will be QicsSelected, just because it was selected at one
			// time.  In order to possibly save space, we CLEAR the property
			// if it is unselected.  The style manager may remove the cell's
			// style altogether if QicsSelected was the only thing set.
			
			// NOTE: This will break if the default value of QicsSelected is
			//       not false!!!!
		    
			myStyleManager->clearCellProperty(mRow, mCol,
							  QicsCellStyle::Selected);
		    }
		}
	    }
	}
    }

    // Now we can turn style manager reporting back on
    myStyleManager->setReportChanges(true);

    // Update the affected region

    QicsRegion region(begin_row, begin_col, end_row, end_col);

    if (myAffectedRegion.isValid())
    {
	myAffectedRegion |= region;
    }
    else
    {
	myAffectedRegion = region;
    }
}

void
QicsSelectionManager::setRowSelectionProperty(QicsGridInfo &info, int row,
					      QicsSelectionManager::QicsSelectState sel,
					      QicsStyleManager *sm)
{
    bool set;
    row = info.modelRowIndex(row);
	  
    if (sel == QicsSelectionManager::QicsSelectTrue)
	set = true;
    else if (sel == QicsSelectionManager::QicsSelectFalse)
	set =  false;
    else
    {
	bool cur = mySelectionList->isRowSelected(row);
	
	if (sel == QicsSelectTrueRevert)
	{
	    if (!cur)
		return;
	    else
		set = true;
	}
	else // (sel == QicsSelectFalseRevert)
	{
	    if (cur)
		return;
	    else
		set = false;
	}
    }

    if (set)
    {
	bool *val = new bool(true);
	sm->setRowProperty(row, QicsCellStyle::Selected, (void *)val, false);
    }
    else
    {
	// This is an optimization.  Many times, the only property set on
	// a row will be QicsSelected, just because it was selected at one
	// time.  In order to possibly save space, we CLEAR the property
	// if it is unselected.  The style manager may remove the row's
	// style altogether if QicsSelected was the only thing set.
	
	// NOTE: This will break if the default value of QicsSelected is
	//       not false!!!!
	
	sm->clearRowProperty(row, QicsCellStyle::Selected);
    }
}

void
QicsSelectionManager::setColumnSelectionProperty(QicsGridInfo &info, int col,
						 QicsSelectionManager::QicsSelectState sel,
						 QicsStyleManager *sm)
{
    bool set;
    col = info.modelColumnIndex(col);

    if (sel == QicsSelectionManager::QicsSelectTrue)
	set = true;
    else if (sel == QicsSelectionManager::QicsSelectFalse)
	set =  false;
    else
    {
	bool cur = mySelectionList->isColumnSelected(col);
	
	if (sel == QicsSelectTrueRevert)
	{
	    if (!cur)
		return;
	    else
		set = true;
	}
	else // (sel == QicsSelectFalseRevert)
	{
	    if (cur)
		return;
	    else
		set = false;
	}
    }

    if (set)
    {
	bool *val = new bool(true);
	sm->setColumnProperty(col, QicsCellStyle::Selected, (void *)val, false);
    }
    else
    {
	// This is an optimization.  Many times, the only property set on
	// a column will be QicsSelected, just because it was selected at one
	// time.  In order to possibly save space, we CLEAR the property
	// if it is unselected.  The style manager may remove the column's
	// style altogether if QicsSelected was the only thing set.
	
	// NOTE: This will break if the default value of QicsSelected is
	//       not false!!!!
	
	sm->clearColumnProperty(col, QicsCellStyle::Selected);
    }
}

void
QicsSelectionManager::announceChanges(bool in_progress)
{
    emit selectionCellsChanged(myAffectedRegion);
    emit selectionListChanged(in_progress);
}

///////////////////////////////////////////////////////////////////////
//////////////////      Insert/Delete Methods       ///////////////////
///////////////////////////////////////////////////////////////////////

void
QicsSelectionManager::insertRows(int num, int start_position)
{
    handleInsertRows(mySelectionList, num, start_position);
    handleInsertRows(mySelectionActionList, num, start_position);
}

void
QicsSelectionManager::handleInsertRows(QicsSelectionList *slist,
				       int num, int start_position)
{
    if (!slist)
	return;

    // Go through each selection in the list and modify
    // the row values if necessary

    QicsSelectionList::iterator iter;

    for (iter = slist->begin(); iter != slist->end(); ++iter)
    {
	QicsSelection &sel = *iter;

	if ((start_position <= sel.anchorRow()) &&
	    (sel.anchorRow() != Qics::QicsLAST_ROW))
	{
	    sel.setAnchorRow(sel.anchorRow() + num);
	}

	if ((start_position <= sel.endRow()) &&
	    (sel.endRow() != Qics::QicsLAST_ROW))
	{
	    sel.setEndRow(sel.endRow() + num);
	}
    }
}

void
QicsSelectionManager::insertColumns(int num, int start_position)
{
    handleInsertColumns(mySelectionList, num, start_position);
    handleInsertColumns(mySelectionActionList, num, start_position);
}

void
QicsSelectionManager::handleInsertColumns(QicsSelectionList *slist,
					  int num, int start_position)
{
    if (!slist)
	return;

    // Go through each selection in the list and modify
    // the column values if necessary

    QicsSelectionList::iterator iter;

    for (iter = slist->begin(); iter != slist->end();
	 ++iter)
    {
	QicsSelection &sel = *iter;

	if ((start_position <= sel.anchorColumn()) &&
	    (sel.anchorColumn() != Qics::QicsLAST_COLUMN))
	{
	    sel.setAnchorColumn(sel.anchorColumn() + num);
	}

	if ((start_position <= sel.endColumn()) &&
	    (sel.endColumn()  != Qics::QicsLAST_COLUMN))
	{
	    sel.setEndColumn(sel.endColumn() + num);
	}
    }
}

void
QicsSelectionManager::deleteRows(int num, int start_position)
{
    handleDeleteRows(mySelectionList, num, start_position);
    handleDeleteRows(mySelectionActionList, num, start_position);
}

void
QicsSelectionManager::handleDeleteRows(QicsSelectionList *slist,
				       int num, int start_position)
{
    if (!slist)
	return;

    // Go through each selection in the list and modify
    // the row values if necessary

    QicsSelectionList::iterator iter;

    for (iter = slist->begin(); iter != slist->end();
	 ++iter)
    {
	QicsSelection &sel = *iter;

	if ((start_position <= sel.anchorRow()) &&
	    (sel.anchorRow() != Qics::QicsLAST_ROW))
	{
	    sel.setAnchorRow(sel.anchorRow() - num);
	}

	if ((start_position <= sel.endRow()) &&
	    (sel.endRow() != Qics::QicsLAST_ROW))
	{
	    sel.setEndRow(sel.endRow() - num);
	}
    }
}

void
QicsSelectionManager::deleteColumns(int num, int start_position)
{
    handleDeleteColumns(mySelectionList, num, start_position);
    handleDeleteColumns(mySelectionActionList, num, start_position);
}

void
QicsSelectionManager::handleDeleteColumns(QicsSelectionList *slist,
					  int num, int start_position)
{
    if (!slist)
	return;

    // Go through each selection in the list and modify
    // the column values if necessary

    QicsSelectionList::iterator iter;

    for (iter = slist->begin(); iter != slist->end();
	 ++iter)
    {
	QicsSelection &sel = *iter;

	if ((start_position <= sel.anchorColumn()) &&
	    (sel.anchorColumn() != Qics::QicsLAST_COLUMN))
	{
	    sel.setAnchorColumn(sel.anchorColumn() - num);
	}

	if ((start_position <= sel.endColumn()) &&
	    (sel.endColumn()  != Qics::QicsLAST_COLUMN))
	{
	    sel.setEndColumn(sel.endColumn() - num);
	}
    }
}


void
QicsSelectionManager::orderChanged(QicsIndexType type, QMemArray<int> vismap)
{
#if notdef
	qDebug("selectionMgr: orderChanged(%d)\n", type);
	for(int i = 0; i < vismap.size(); i++) {
		if(vismap[i] != i) {
			qDebug("%d -> %d (model %d)\n", i, vismap[i],
				type == RowIndex ? 
				   myGridInfo->modelRowToVisualIndex(i) :
				   myGridInfo->modelColumnToVisualIndex(i)
				   );
		}
	}
	dumpList(mySelectionList);
#endif

    if (mySelectionList == 0) return;

    QicsSelectionList *toAdd = new QicsSelectionList();

    QicsSelectionList::iterator iter;
    for (iter = mySelectionList->begin(); iter != mySelectionList->end();
	     ++iter)
    {
	QicsSelection &sel = *iter;

	int top = sel.topRow();
	int left = sel.leftColumn();
	int bottom = sel.bottomRow();
	int right = sel.rightColumn();

	if ((left == 0) && (right == Qics::QicsLAST_COLUMN)) {
	    // row selection
	    if(type == RowIndex) {
		if(top == bottom) {
		    sel.setAnchorRow(vismap[top]);
		    sel.setEndRow(vismap[top]);
		} else {
		    sel.setAnchorRow(vismap[top]);
		    sel.setEndRow(vismap[top]);
		    for(int r = top+1; r <= bottom; r++) {
			toAdd->push_back(QicsSelection(vismap[r], left,
						       vismap[r], right));
		    }
	    	}
	    }
	} else if ((top == 0) && (bottom == Qics::QicsLAST_ROW)) {
	    // column selection
	    if(type == ColumnIndex) {
		if(left == right) {
		    sel.setAnchorColumn(vismap[left]);
		    sel.setEndColumn(vismap[left]);
		} else {
		    sel.setAnchorColumn(vismap[left]);
		    sel.setEndColumn(vismap[left]);
		    for(int c = left+1; c <= right; c++) {
			toAdd->push_back(QicsSelection(top, vismap[c],
						       bottom, vismap[c]));
		    }
		}
	    }
	} else if(top == bottom && left == right) {
	    // single cell - we can always adjust that
	    if(type == RowIndex) {
		sel.setAnchorRow(vismap[top]);
		sel.setEndRow(vismap[top]);
	    } else {
		sel.setAnchorColumn(vismap[left]);
		sel.setEndColumn(vismap[left]);
	    }
	} else {
	    /* It is a region.  This gets tricky.
	     * One rule Chris and I thought of was.
	     * When sorting a set of rows (columns)
	     * Any cell or region that intersects the
	     * sorted set, but is not the entire set, gets
	     * tossed.  The others get mapped.
	     * But, how does that deal with moved rows.
	     */
	    if(type == RowIndex) {
		sel.setAnchorRow(vismap[top]);
		sel.setEndRow(vismap[top]);
		for(int r = top+1; r <= bottom; r++) {
		    toAdd->push_back(QicsSelection(vismap[r], left,
						   vismap[r], right));
		}
	    } else {
		sel.setAnchorColumn(vismap[left]);
		sel.setEndColumn(vismap[left]);
		for(int c = left+1; c <= right; c++) {
		    toAdd->push_back(QicsSelection(top, vismap[c],
						   bottom, vismap[c]));
		}
	    }
	}
    }

    // now we have the additions, splice them in
#if notdef
    qDebug("=== Additions list ==\n");
    dumpList(toAdd);
#endif
    QicsSelectionList::iterator additer;
    for (additer = toAdd->begin(); additer != toAdd->end(); ++additer) {
	mySelectionList->push_back(*additer);
    }
    delete toAdd;

    delete mySelectionActionList;
    mySelectionActionList = 0;

#if notdef
    qDebug("==== NEW LIST IS ==\n");
	dumpList(mySelectionList);
#endif

}
