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
#include <QicsDataModelDefault.h>
#include <QicsStyleManager.h>
#include <QicsSelectionManager.h>
#include <QicsDimensionManager.h>
#include <QicsMappedDimensionManager.h>
#include <QicsSorter.h>
#include <QicsScreenGrid.h>
#include <QicsHeaderGrid.h>
#include <QicsTableRegionDrag.h>

#include <assert.h>

//////////////////////////////////////////////////////////////////////////////

QicsGridInfo::QicsGridInfo(QicsGridType type)
    : myType(type),
      myCellValue(0)
{
    myDT = 0;
    mySM = 0;
    myDM = 0;
    mySelM = 0;
    
    myRowOrdering = new QicsSorter(RowIndex, 0);
    myColumnOrdering = new QicsSorter(ColumnIndex, 0);
    myRowOrderOwner = this;
    myColumnOrderOwner = this;

    myCutCopySelection = 0;
}

QicsGridInfo::~QicsGridInfo()
{
    if(myRowOrderOwner == this) delete myRowOrdering;
    if(myColumnOrderOwner == this) delete myColumnOrdering;
    myRowOrderOwner = 0;
    myColumnOrderOwner = 0;
}

QicsDataModel *
QicsGridInfo::dataModel(void) const
{
    return myDT;
}

void 
QicsGridInfo::setDataModel(QicsDataModel *dt)
{
    QicsDataModel *oldDT = myDT;
    myDT = dt;

    if (oldDT)
    {
	// remove all connections

	if (mySM)
	    disconnect(oldDT, 0, mySM, 0);

	if (myDM)
	    disconnect(oldDT, 0, myDM, 0);

	for (QicsScreenGridPV::iterator iter = myGrids.begin();
	     iter != myGrids.end();
	     ++iter)
	{
	    disconnect(oldDT, 0, *iter, 0);
	}
    }

    if ((myRowOrderOwner == 0) || (myRowOrderOwner == this))
    {
	delete myRowOrdering;
	myRowOrdering = new QicsSorter(RowIndex, myDT);
	myRowOrderOwner = this;
    }

    if ((myColumnOrderOwner == 0) || (myColumnOrderOwner == this))
    {
	delete myColumnOrdering;
	myColumnOrdering = new QicsSorter(ColumnIndex, myDT);
	myColumnOrderOwner = this;
    }

    connectDTtoSM();
    connectDTtoDM();

    if (myDT)
    {
	connect(myDT, SIGNAL(modelChanged(QicsRegion)),
		this, SLOT(redrawModel(QicsRegion)));
    }

    for (QicsScreenGridPV::iterator iter = myGrids.begin();
	 iter != myGrids.end();
	 ++iter)
    {
	connectDTtoGrid(*iter);
    }

    emit dataModelChanged(oldDT, myDT);
}

QicsStyleManager *
QicsGridInfo::styleManager(void) const
{
    return mySM;
}

void
QicsGridInfo::setStyleManager(QicsStyleManager *sm)
{
    QicsStyleManager *oldSM = mySM;
    mySM = sm;

    if (oldSM)
    {
	disconnect(myDT, 0, oldSM, 0);

	for (QicsScreenGridPV::iterator iter = myGrids.begin();
	     iter != myGrids.end();
	     ++iter)
	{
	    disconnect(oldSM, 0, *iter, 0);
	}
    }

    connectDTtoSM();

    for (QicsScreenGridPV::iterator iter = myGrids.begin();
	 iter != myGrids.end();
	 ++iter)
    {
	connectSMtoGrid(*iter);
    }

    emit styleManagerChanged(oldSM, mySM);
}

/*!
 * We return the mapped DM so that the caller can use visual
 * coordinates yet have the right thing happen
 */
QicsMappedDimensionManager *
QicsGridInfo::mappedDM(void) const
{
    return myMappedDM;
}

/*!
 * We return the mapped DM so that the caller can use visual
 * coordinates yet have the right thing happen
 */
QicsDimensionManager *
QicsGridInfo::dimensionManager(void) const
{
    return myDM;
}

void 
QicsGridInfo::setDimensionManager(QicsDimensionManager *dm)
{
    QicsDimensionManager *oldDM = myDM;
    myDM = dm;
    myMappedDM = new QicsMappedDimensionManager(dm, this);

    if (oldDM)
    {
	disconnect(myDT, 0, oldDM, 0);

	for (QicsScreenGridPV::iterator iter = myGrids.begin();
	     iter != myGrids.end();
	     ++iter)
	{
	    disconnect(oldDM, 0, *iter, 0);
	}
    }

    connectDTtoDM();


    for (QicsScreenGridPV::iterator iter = myGrids.begin();
	 iter != myGrids.end();
	 ++iter)
    {
	connectDMtoGrid(*iter);
    }

    emit dimensionManagerChanged(oldDM, myDM);
}

QicsSelectionManager *
QicsGridInfo::selectionManager(void) const
{
    return (mySelM);
}

void
QicsGridInfo::setSelectionManager(QicsSelectionManager *sel_m)
{
    QicsSelectionManager *oldSelM = mySelM;
    mySelM = sel_m;

    if (oldSelM)
    {
	for (QicsScreenGridPV::iterator iter = myGrids.begin();
	     iter != myGrids.end();
	     ++iter)
	{
	    disconnect(oldSelM, 0, *iter, 0);
	}
    }

    for (QicsScreenGridPV::iterator iter = myGrids.begin();
	 iter != myGrids.end();
	 ++iter)
    {
	connectSelMtoGrid(*iter);
    }

    if ((myRowOrderOwner == this) && myRowOrdering)
    {
        connect(myRowOrdering,SIGNAL(orderChanged(QicsIndexType,QMemArray<int>)),
		mySelM, SLOT(orderChanged(QicsIndexType,QMemArray<int>)));
    }
    if ((myColumnOrderOwner == this) && myColumnOrdering)
    {
        connect(myColumnOrdering, SIGNAL(orderChanged(QicsIndexType,QMemArray<int>)),
		mySelM, SLOT(orderChanged(QicsIndexType,QMemArray<int>)));
    }

    emit selectionManagerChanged(oldSelM, mySelM);
}

void
QicsGridInfo::connectGrid(QicsScreenGrid *grid)
{
    if (grid)
    {
	connectDTtoGrid(grid);
	connectSMtoGrid(grid);
	connectDMtoGrid(grid);
	connectSelMtoGrid(grid);

	myGrids.push_back(grid);

	connect(this,
		SIGNAL(dataModelChanged(QicsDataModel *, QicsDataModel *)),
		grid,
		SLOT(recomputeAndDraw()));
	connect(this,
		SIGNAL(styleManagerChanged(QicsStyleManager *,
					   QicsStyleManager *)),
		grid,
		SLOT(recomputeAndDraw()));
	connect(this,
		SIGNAL(dimensionManagerChanged(QicsDimensionManager *,
					       QicsDimensionManager *)),
		grid,
		SLOT(recomputeAndDraw()));
	connect(this, SIGNAL(modelReordered(QicsIndexType)),
		grid, SLOT(recomputeAndDraw()));

	connect(grid, SIGNAL(destroyed(QObject *)),
		this, SLOT(removeGrid(QObject *)));

	emit gridAdded(grid);
    }
}

void
QicsGridInfo::disconnectGrid(QicsScreenGrid *grid)
{
    if (grid)
    {
	disconnect(myDT, 0, grid, 0);
	disconnect(mySM, 0, grid, 0);
	disconnect(myDM, 0, grid, 0);
	disconnect(mySelM, 0, grid, 0);
	disconnect(grid, 0, this, 0);

	removeGrid(grid);

	emit gridRemoved(grid);
    }
}

const QicsDataItem *
QicsGridInfo::cellValue(int row, int col) const
{
    const QicsDataItem *itm = 0;

    if (myDT)
	itm = myDT->item(modelRowIndex(row), modelColumnIndex(col));

    if (!itm)
    {
	int idx = -1;

	if (gridType() == RowHeaderGrid)
	    idx = row;
	else if (gridType() == ColumnHeaderGrid)
	    idx = col;

	if (idx >= 0)
	{
	    QicsGridInfo *self = const_cast<QicsGridInfo *> (this);

	    self->myCellValue = QicsDataInt(idx);
	    itm = &(self->myCellValue);
	}
    }

    return itm;
}


Qics::QicsRepaintBehavior
QicsGridInfo::gridRepaintBehavior(void) const
{
    return (* (static_cast<Qics::QicsRepaintBehavior *> 
	       (mySM->getGridProperty(QicsGridStyle::GridRepaintBehavior))));
}

void
QicsGridInfo::setGlobalRepaintBehavior(QicsRepaintBehavior behavior)
{
    emit globalSetRepaintBehaviorRequest(behavior);
}

void
QicsGridInfo::revertGlobalRepaintBehavior(void)
{
    emit globalRevertRepaintBehaviorRequest();
}

void
QicsGridInfo::setGridRepaintBehavior(QicsRepaintBehavior behavior)
{
    myPrevGridRepaintBehavior =  * static_cast<QicsRepaintBehavior *>
	(mySM->getGridProperty(QicsGridStyle::GridRepaintBehavior));

    QicsRepaintBehavior *val = new QicsRepaintBehavior(behavior);

    mySM->setGridProperty(QicsGridStyle::GridRepaintBehavior,
			  static_cast<void *> (val));
}

void
QicsGridInfo::revertGridRepaintBehavior(void)
{
    QicsRepaintBehavior *val = new QicsRepaintBehavior(myPrevGridRepaintBehavior);

    mySM->setGridProperty(QicsGridStyle::GridRepaintBehavior,
			  static_cast<void *> (val));
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void
QicsGridInfo::removeGrid(QObject *obj)
{
    for (QicsScreenGridPV::iterator iter = myGrids.begin();
	 iter != myGrids.end();
	 ++iter)
    {
	if (*iter == obj)
	{
	    myGrids.erase(iter);
	    mySelM->removeHeader(obj);
	    break;
	}
    }
}

void
QicsGridInfo::connectDTtoSM(void)
{
    if (myDT && mySM)
    {
	connect(myDT, SIGNAL(rowsInserted(int, int)),
		mySM, SLOT(insertRows(int, int)));
	connect(myDT, SIGNAL(columnsInserted(int, int)),
		mySM, SLOT(insertColumns(int, int)));
	connect(myDT, SIGNAL(rowsDeleted(int, int)),
		mySM, SLOT(deleteRows(int, int)));
	connect(myDT, SIGNAL(columnsDeleted(int, int)),
		mySM, SLOT(deleteColumns(int, int)));
    }
}

void
QicsGridInfo::connectDTtoDM(void)
{
    if (myDT && myDM)
    {
	connect(myDT, SIGNAL(rowsInserted(int, int)),
		myDM, SLOT(insertRows(int, int)));
	connect(myDT, SIGNAL(columnsInserted(int, int)),
		myDM, SLOT(insertColumns(int, int)));
	connect(myDT, SIGNAL(rowsDeleted(int, int)),
		myDM, SLOT(deleteRows(int, int)));
	connect(myDT, SIGNAL(columnsDeleted(int, int)),
		myDM, SLOT(deleteColumns(int, int)));
    }
}

void
QicsGridInfo::connectDTtoGrid(QicsScreenGrid *grid)
{
    if (myDT && grid)
    {
	connect(myDT, SIGNAL(modelSizeChanged(int, int)),
		grid, SLOT(resetAndDraw()));
    }
}

void
QicsGridInfo::connectSMtoGrid(QicsScreenGrid *grid)
{
    if (mySM && grid)
    {
	connect(mySM,
		SIGNAL(cellPropertyChanged(QicsRegion,
					   QicsCellStyle::QicsCellStyleProperty)),
		grid,
		SLOT(handleCellPropertyChange(QicsRegion,
					      QicsCellStyle::QicsCellStyleProperty)));

	connect(mySM,
		SIGNAL(gridPropertyChanged(QicsGridStyle::QicsGridStyleProperty)),
		grid,
		SLOT(handleGridPropertyChange(QicsGridStyle::QicsGridStyleProperty)));

	connect(mySM,
		SIGNAL(spanChanged(QicsSpan)),
		grid,
		SLOT(redraw(QicsSpan)));
    }
}

void
QicsGridInfo::connectDMtoGrid(QicsScreenGrid *grid)
{
    if (myDM && grid)
    {
	connect(myDM,
		SIGNAL(dimensionChanged()),
		grid,
		SLOT(recomputeAndDraw()));
    }
}

void
QicsGridInfo::connectSelMtoGrid(QicsScreenGrid *grid)
{
    if (mySelM && grid)
    {
	connect(mySelM, SIGNAL(selectionCellsChanged(QicsRegion)),
		grid, SLOT(redraw(QicsRegion)));

	if (grid->inherits("QicsHeaderGrid"))
	{
	    QicsHeaderGrid *hg = static_cast<QicsHeaderGrid *> (grid);

	    mySelM->addHeader(hg);
	}
    }
}

void QicsGridInfo::deleteRows(int how_many, int start)
{
    if (!myDT)
	return;

    // Applications should not be calling delete rows
    // on anything but the master
    assert(myRowOrderOwner == this);

    /* As we delete rows from the model, it will
     * signal the Sorter, and thus order[start]
     * will be different each iteration.
     */

    // TODO - we probabaly should do something to prevent redraw
    // while this is going on
    while(how_many-- > 0) {
	myDT->deleteRows(1, modelRowIndex(start));
    }
}

void QicsGridInfo::insertRow(int where)
{
    if (!myDT)
	return;

    // Applications should not be calling insertRow
    // on anything but the master
    assert(myRowOrderOwner == this);

    /* Insert at end is easy */
    if(where < 0) {
	myDT->addRows(1);
	return;
    }

    /* Insert in the middle is a littler trickier.  We
     * must insert into the model using model indices.
     */
    int data_where = modelRowIndex(where);
    myDT->insertRows(1, data_where);

    /* now the data model tells the sorters for any grids
     * looking at it that we inserted a row, but in model
     * dimensions, so the sorter has to go through gyrations,
     * but that is in QicsSorter.C, so we do not see the pain
     * here.
     */
}

void QicsGridInfo::deleteColumns(int how_many, int start)
{
    if (!myDT)
	return;

    // Applications should not be calling delete rows
    // on anything but the master
    assert(myColumnOrderOwner == this);

    /* As we delete rows from the model, it will
     * signal the Sorter, and thus order[start]
     * will be different each iteration.
     */
    // TODO - we probabaly should do something to prevent redraw
    // while this is going on
    while(how_many-- > 0) {
	myDT->deleteColumns(1, modelColumnIndex(start));
    }
}


void QicsGridInfo::insertColumn(int where)
{
    if (!myDT)
	return;

    // Applications should not be calling insertColumn
    // on anything but the master
    assert(myColumnOrderOwner == this);

    /* Insert at end is easy */
    if(where < 0) {
	myDT->addColumns(1);
	return;
    }

    /* Insert in the middle is a littler trickier.  We
     * must insert into the model using model indices.
     */
    int data_where = modelColumnIndex(where);
    myDT->insertColumns(1, data_where);

    /* now the data model tells the sorters for any grids
     * looking at it that we inserted a row, but in model
     * dimensions, so the sorter has to go through gyrations,
     * but that is in QicsSorter.C, so we do not see the pain
     * here.
     */
}

void
QicsGridInfo::controlColumnsOf(QicsGridInfo *slave)
{
    if (myDT)
    {
	connect(myDT, SIGNAL(columnsInserted(int, int)),
		slave->myDT, SLOT(insertColumns(int, int)));
	connect(myDT, SIGNAL(columnsAdded(int)),
		slave->myDT, SLOT(addColumns(int)));
	connect(myDT, SIGNAL(columnsDeleted(int, int)),
		slave->myDT, SLOT(deleteColumns(int, int)));
    }

    connect(this, SIGNAL(modelReordered(QicsIndexType)),
	    slave, SLOT(redrawAllGrids()));

    /* I am not really happy with the master modifying
     * the slave.  What we should have is a helper function
     * named "becomeColumnSlaveOf()", which does this part.
     * But then we have another special method, which should
     * only be called from this method.  C++ has no way to
     * enforce that contract other than documentation.
     * The angst of it all.   Maybe I'm just nit-picking.
     * -tony
     */
    if(slave->myColumnOrderOwner == slave) {
	delete slave->myColumnOrdering;
    }
    slave->myColumnOrdering = myColumnOrdering;
    // slave->myColumnOrderOwner = myColumOrderOwner;
    slave->myColumnOrderOwner = this;
}

void
QicsGridInfo::controlRowsOf(QicsGridInfo *slave)
{
    if (myDT)
    {
	connect(myDT, SIGNAL(rowsInserted(int, int)),
		slave->myDT, SLOT(insertRows(int, int)));
	connect(myDT, SIGNAL(rowsAdded(int)),
		slave->myDT, SLOT(addRows(int)));
	connect(myDT, SIGNAL(rowsDeleted(int, int)),
		slave->myDT, SLOT(deleteRows(int, int)));
    }

    connect(this, SIGNAL(modelReordered(QicsIndexType)),
	    slave, SLOT(redrawAllGrids()));

    if(slave->myRowOrderOwner == slave) {
	delete slave->myRowOrdering;
    }
    slave->myRowOrdering = myRowOrdering;
    // TODO: decide which one of these choices is right
    // slave->myRowOrderOwner = myRowOrderOwner;
    slave->myRowOrderOwner = this;
}

void
QicsGridInfo::orderRowsBy(int column, QicsSortOrder order, int from, int to,
			  DataItemComparator func)
{    
    myRowOrdering->sort(modelColumnIndex(column), order, from, to, func);
    emit modelReordered(Qics::RowIndex);
}

void
QicsGridInfo::orderColumnsBy(int row, QicsSortOrder order, int from, int to,
			     DataItemComparator func)
{    
    myColumnOrdering->sort(modelRowIndex(row), order, from, to, func);
    emit modelReordered(Qics::ColumnIndex);
}

void QicsGridInfo::moveRows(int target, const QMemArray<int> &rows)
{
    if(this->myRowOrderOwner == this) {
	myRowOrdering->moveItems(target, rows);
	emit modelReordered(Qics::RowIndex);
    } else {
	this->myRowOrderOwner->moveRows(target, rows);
    }
}

void QicsGridInfo::moveColumns(int target, const QMemArray<int> &cols)
{
    if(this->myColumnOrderOwner == this) {
	myColumnOrdering->moveItems(target, cols);
	emit modelReordered(Qics::ColumnIndex);
    } else {
	this->myColumnOrderOwner->moveColumns(target, cols);
    }
}

QicsRegion
QicsGridInfo::modelRegion(QicsRegion vis_region) const
{
    QicsRegion mr(vis_region);

    // don't translate if the region contains all rows
    if ((vis_region.startRow() != 0) ||
	(vis_region.endRow() != QicsLAST_ROW))
    {
	mr.setStartRow(modelRowIndex(vis_region.startRow()));
	mr.setEndRow(modelRowIndex(vis_region.endRow()));
    }

    // don't translate if the region contains all columns
    if ((vis_region.startColumn() != 0) ||
	(vis_region.endColumn() != QicsLAST_COLUMN))
    {
	mr.setStartColumn(modelColumnIndex(vis_region.startColumn()));
	mr.setEndColumn(modelColumnIndex(vis_region.endColumn()));
    }

    return mr;
}

QicsRegion
QicsGridInfo::visualRegion(QicsRegion model_region) const
{
    QicsRegion vr(model_region);

    // don't translate if the region contains all rows
    if ((model_region.startRow() != 0) ||
	(model_region.endRow() != QicsLAST_ROW))
    {
	vr.setStartRow(visualRowIndex(model_region.startRow()));
	vr.setEndRow(visualRowIndex(model_region.endRow()));
    }

    // don't translate if the region contains all columns
    if ((model_region.startColumn() != 0) ||
	(model_region.endColumn() != QicsLAST_COLUMN))
    {
	vr.setStartColumn(visualColumnIndex(model_region.startColumn()));
	vr.setEndColumn(visualColumnIndex(model_region.endColumn()));
    }

    return vr;
}

void
QicsGridInfo::redrawAllGrids()
{
	for (QicsScreenGridPV::iterator iter = myGrids.begin();
	     iter != myGrids.end();
	     ++iter)
	{
		(*iter)->recomputeAndDraw();
	}
}

void
QicsGridInfo::redrawModel(QicsRegion r)
{

    for(int vrow = r.startRow(); vrow <= r.endRow(); vrow++)
    {
	int mrow = visualRowIndex(vrow);
	for(int vcol = r.startColumn(); vcol <= r.endColumn(); vcol++)
	{
	    int mcol = visualColumnIndex(vcol);
	    QicsRegion rv(mrow, mcol, mrow, mcol);

	    for(QicsScreenGridPV::iterator iter = myGrids.begin();
		iter != myGrids.end();
		++iter)
	    {
		    (*iter)->redraw(rv);
    	    }
	}
    }
}



////////////////////////////////////////////////////////////////////////
//////////////////  Clipboard / Drag and Drop Methods  /////////////////
////////////////////////////////////////////////////////////////////////

QDragObject *
QicsGridInfo::cutCopyData(QWidget *widget, QicsICell *ref_cell)
{
    if (myCutCopySelection)
	delete myCutCopySelection;

    myCutCopySelection = cutCopySelection(ref_cell);

    if (myCutCopySelection && myCutCopySelection->size() > 0)
    {
	return QicsTableRegionDrag::getDragObject(this, myCutCopySelection,
						  ref_cell, widget);
    }

    return 0;
}

QicsSelectionList *
QicsGridInfo::cutCopySelection(QicsICell *ref_cell)
{
    QicsSelectionList *return_list = 0;

    QicsSelectionList *slist = mySelM->selectionList();

    if (ref_cell && slist)
    {
	/*
	 * We only grab the part of the selection list that contains ref_cell
	 *
	 * This is simply because we can't think of any good semantics
	 * right now for dragging discontiguous regions into a cell, or
	 * into anywhere for that matter.  We need some API for the
	 * application to make sense of things like this.
	 *
	 * TODO: provide that API for extended selections
	 *
	 */
	
	QicsSelectionList::iterator iter;

	for (iter = slist->begin(); iter != slist->end(); ++iter)
	{
	    QicsSelection &sel = *iter;

	    QicsRegion reg(sel.topRow(), sel.leftColumn(),
			   sel.bottomRow(), sel.rightColumn());

	    if (reg.contains(*ref_cell))
	    {
		return_list = new QicsSelectionList();

		return_list->push_back(QicsSelection(reg.startRow(),
						     reg.startColumn(),
						     reg.endRow(),
						     reg.endColumn()));
		break;
	    }
	}

	delete slist;
    }
    else
    {
	//
	// We take the entire selection list
	//

	return_list = slist;
    }

    return (return_list);
}

void
QicsGridInfo::finishCut(bool remove_data)
{
    if (!myCutCopySelection || !dataModel())
	return;

    if (remove_data)
    {
	QicsSelectionList::iterator iter;
	
	for (iter = myCutCopySelection->begin();
	     iter != myCutCopySelection->end();
	     ++iter)
	{
	    QicsSelection &sel = *iter;

	    for (int i = sel.topRow(); i <= sel.bottomRow(); ++i)
	    {
		for (int j = sel.leftColumn(); j <= sel.rightColumn(); ++j)
		{
		    dataModel()->clearItem(modelRowIndex(i),
					   modelColumnIndex(j));
		}
	    }
	}
    }
    
    delete myCutCopySelection;
    myCutCopySelection = 0;
}

void 
QicsGridInfo::paste(QMimeSource *ms, const QicsICell &cell)
{
    if (!dataModel())
	return;

    QicsDataModelDefault tmp_dm;
    QString text;

    if (QicsTableRegionDrag::decode(ms, tmp_dm))
    {
	overlay(tmp_dm, cell, true);
    }
    else if (QTextDrag::decode(ms, text))
    {
	QicsDataString itm(text);

	dataModel()->setItem(modelRowIndex(cell.row()),
			     modelColumnIndex(cell.column()), itm);
    }
}

void
QicsGridInfo::overlay(const QicsDataModel &dm, const QicsICell &start_cell,
		       bool expand_model, bool clear_if_empty)
{
    for (int i = 0; i < dm.numRows(); ++i)
    {
	int view_row = start_cell.row() + i;

	if (!expand_model && (view_row >= dataModel()->numRows()))
	    break;

	for (int j = 0; j < dm.numColumns(); ++j)
	{
	    int view_col = start_cell.column() + j;

	    if (!expand_model && (view_col >= dataModel()->numColumns()))
		break;

	    const QicsDataItem *itm = dm.item(i, j);

	    if (itm)
	    {
		dataModel()->setItem(modelRowIndex(view_row),
				     modelColumnIndex(view_col),
				     *itm);
	    }
	    else if (clear_if_empty)
	    {
		dataModel()->clearItem(modelRowIndex(view_row),
				       modelColumnIndex(view_col));
	    }
	}
    }
}

QicsICell
QicsGridInfo::currentCell(void) const
{
    QicsICell cell;

    // Create a cell in visual coordinates, as that's what the grids expect
    
    if (myCurrentCell.isValid())
    {
	cell.setRow(visualRowIndex(myCurrentCell.row()));
	cell.setColumn(visualColumnIndex(myCurrentCell.column()));
    }

    return cell;
}

void
QicsGridInfo::setCurrentCell(const QicsICell &vis_cell)
{
    // Yes, we have to do this.
    QicsICell old_cell = myCurrentCell;
    myCurrentCell = QicsICell();

    if (old_cell.isValid() && (gridType() == TableGrid))
    {
	styleManager()->clearCellProperty(old_cell.row(),
					  old_cell.column(),
					  QicsCellStyle::Current);
    }

    QicsICell cell;

    // We store this in model coordinates, so row/column reordering works

    if (vis_cell.isValid())
    {
	int mr = modelRowIndex(vis_cell.row());
	int mc = modelColumnIndex(vis_cell.column());

	bool enabled = (* static_cast<bool*>
			(styleManager()->getCellProperty(mr, mc,
							 QicsCellStyle::Enabled)));

	if (enabled)
	{
	    cell.setRow(mr);
	    cell.setColumn(mc);

	    if (gridType() == TableGrid)
	    {
		styleManager()->setCellProperty(mr, mc, QicsCellStyle::Current,
						static_cast<void *> (new bool(true)));
	    }
	}
    }

    myCurrentCell = cell;

    emit currentCellChanged(vis_cell.row(), vis_cell.column());
}

void 
QicsGridInfo::setCellValue(int row, int col, const QicsDataItem &itm)
{
    if (dataModel())
	dataModel()->setItem(modelRowIndex(row), modelColumnIndex(col), itm);

    emit cellValueChanged(row, col);
}

void
QicsGridInfo::setCurrentCellValue(const QicsDataItem &itm)
{
    if (myCurrentCell.isValid())
	setCellValue(visualRowIndex(myCurrentCell.row()),
		     visualColumnIndex(myCurrentCell.column()),
		     itm);
}
