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


#include <QicsTable.h>
#include <QicsTableGrid.h>
#include <QicsDataModelDefault.h>
#include <QicsHeaderGrid.h>
#include <QicsScrollManager.h>
#include <QicsDimensionManager.h>
#include <QicsSelectionManager.h>
#include <QicsTextCellDisplay.h>
#include <QicsScrollBarScroller.h>

#include <QicsUtil.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qlayout.h>

/////////////////////////////////////////////////////////////////////////////////

char QicsTable_version_string[] = QICSTABLE_VERSION;

/////////////////////////////////////////////////////////////////////////////////

#define LEFT_VHEADER_IDX    0
#define RIGHT_VHEADER_IDX   1
#define TOP_VHEADER_IDX     0
#define MIDDLE_VHEADER_IDX  1
#define BOTTOM_VHEADER_IDX  2

#define LEFT_TOP_HEADER     (myVHeaders[TOP_VHEADER_IDX][LEFT_VHEADER_IDX])
#define LEFT_HEADER         (myVHeaders[MIDDLE_VHEADER_IDX][LEFT_VHEADER_IDX])
#define LEFT_BOTTOM_HEADER  (myVHeaders[BOTTOM_VHEADER_IDX][LEFT_VHEADER_IDX])
#define RIGHT_TOP_HEADER    (myVHeaders[TOP_VHEADER_IDX][RIGHT_VHEADER_IDX])
#define RIGHT_HEADER        (myVHeaders[MIDDLE_VHEADER_IDX][RIGHT_VHEADER_IDX])
#define RIGHT_BOTTOM_HEADER (myVHeaders[BOTTOM_VHEADER_IDX][RIGHT_VHEADER_IDX])


#define LEFT_HHEADER_IDX    0
#define MIDDLE_HHEADER_IDX  1
#define RIGHT_HHEADER_IDX   2
#define TOP_HHEADER_IDX     0
#define BOTTOM_HHEADER_IDX  1

#define TOP_LEFT_HEADER     (myHHeaders[TOP_HHEADER_IDX][LEFT_HHEADER_IDX])
#define TOP_HEADER          (myHHeaders[TOP_HHEADER_IDX][MIDDLE_HHEADER_IDX])
#define TOP_RIGHT_HEADER    (myHHeaders[TOP_HHEADER_IDX][RIGHT_HHEADER_IDX])
#define BOTTOM_LEFT_HEADER  (myHHeaders[BOTTOM_HHEADER_IDX][LEFT_HHEADER_IDX])
#define BOTTOM_HEADER       (myHHeaders[BOTTOM_HHEADER_IDX][MIDDLE_HHEADER_IDX])
#define BOTTOM_RIGHT_HEADER (myHHeaders[BOTTOM_HHEADER_IDX][RIGHT_HHEADER_IDX])


//
// Indices for the myGrids 2-dimensional array

#define GRID_TOP_IDX     0
#define GRID_MIDDLE_IDX  1
#define GRID_BOTTOM_IDX  2
#define GRID_LEFT_IDX    0
#define GRID_RIGHT_IDX   2

//
// The grids
//

#define TOP_LEFT_GRID       (myGrids[GRID_TOP_IDX][GRID_LEFT_IDX]) 
#define TOP_MIDDLE_GRID     (myGrids[GRID_TOP_IDX][GRID_MIDDLE_IDX]) 
#define TOP_RIGHT_GRID      (myGrids[GRID_TOP_IDX][GRID_RIGHT_IDX]) 
#define MIDDLE_LEFT_GRID    (myGrids[GRID_MIDDLE_IDX][GRID_LEFT_IDX]) 
#define MAIN_GRID           (myGrids[GRID_MIDDLE_IDX][GRID_MIDDLE_IDX])
#define MIDDLE_RIGHT_GRID   (myGrids[GRID_MIDDLE_IDX][GRID_RIGHT_IDX]) 
#define BOTTOM_LEFT_GRID    (myGrids[GRID_BOTTOM_IDX][GRID_LEFT_IDX]) 
#define BOTTOM_MIDDLE_GRID  (myGrids[GRID_BOTTOM_IDX][GRID_MIDDLE_IDX]) 
#define BOTTOM_RIGHT_GRID   (myGrids[GRID_BOTTOM_IDX][GRID_RIGHT_IDX]) 


//
// Indices for the main layout
//

#define LAYOUT_LEFT_TITLE_IDX 0
#define LAYOUT_GRID_LAYOUT_IDX 1
#define LAYOUT_RIGHT_TITLE_IDX 2

#define LAYOUT_TOP_TITLE_IDX 0
#define LAYOUT_BOTTOM_TITLE_IDX 2

//
// Indices for the "grids" grid layout
//

#define GRID_LAYOUT_LEFT_SCROLLBAR_IDX 0
#define GRID_LAYOUT_LEFT_HDR_IDX 1
#define GRID_LAYOUT_LEFT_FROZEN_GRID_IDX 2
#define GRID_LAYOUT_MAIN_GRID_IDX 3
#define GRID_LAYOUT_RIGHT_FROZEN_GRID_IDX 4
#define GRID_LAYOUT_RIGHT_HDR_IDX 5
#define GRID_LAYOUT_RIGHT_SCROLLBAR_IDX 6

#define GRID_LAYOUT_TOP_SCROLLBAR_IDX 0
#define GRID_LAYOUT_TOP_HDR_IDX 1
#define GRID_LAYOUT_TOP_FROZEN_GRID_IDX 2
#define GRID_LAYOUT_BOTTOM_FROZEN_GRID_IDX 4
#define GRID_LAYOUT_BOTTOM_HDR_IDX 5
#define GRID_LAYOUT_BOTTOM_SCROLLBAR_IDX 6

/////////////////////////////////////////////////////////////////////////////////



QicsTable::QicsTable(QicsDataModel *model, QWidget *parent, const char *name) :
    QWidget(parent, name)
{
    init(model, 0, 0, 0, 0);
}

QicsTable::QicsTable(QicsDataModel::Foundry rhdmf,
		     QicsDataModel::Foundry chdmf,
		     QicsTableGrid::Foundry tf,
		     QicsHeaderGrid::Foundry hf,
		     QicsDataModel *model,
		     QWidget *parent, const char *name) :
    QWidget(parent, name)
{
    init(model, rhdmf, chdmf, tf, hf);
}

void
QicsTable::init(QicsDataModel *model,
		QicsDataModel::Foundry rhdmf,
		QicsDataModel::Foundry chdmf,
		QicsTableGrid::Foundry tf,
		QicsHeaderGrid::Foundry hf)
{
  

    myNumTopFrozenRows = 0;
    myNumBottomFrozenRows = 0;
    myNumLeftFrozenColumns = 0;
    myNumRightFrozenColumns = 0;
    myTopTitleWidget = 0;
    myBottomTitleWidget = 0;
    myLeftTitleWidget = 0;
    myRightTitleWidget = 0;
    myTopLeftCornerWidget = 0;
    myTopRightCornerWidget = 0;
    myBottomLeftCornerWidget = 0;
    myBottomRightCornerWidget = 0;

    myTableCommon = new QicsTableCommon(this);

    if (rhdmf)
	myRowHeaderDMFoundry = rhdmf;
    else
	myRowHeaderDMFoundry = QicsDataModelDefault::create;

    if (chdmf)
	myColumnHeaderDMFoundry = chdmf;
    else
	myColumnHeaderDMFoundry = QicsDataModelDefault::create;

    if (tf)
	myTableGridFoundry = tf;
    else
	myTableGridFoundry = QicsTableGrid::createGrid;

    if (hf)
	myHeaderGridFoundry = hf;
    else
	myHeaderGridFoundry = QicsHeaderGrid::createGrid;

    initDataModels(model);
    initObjects();
    initGridInfoObjects();

    // we need to keep the three data models in sync so that when
    // rows and columns are inserted or deleted in the main data model,
    // the header data models do the same.
    gridInfo().controlRowsOf(&rhGridInfo());
    gridInfo().controlColumnsOf(&chGridInfo());

    // use the default main grid viewport as the initial value
    myFullViewport = mainGridViewport();

    // create the subwidgets for the table
    initDisplay();
    
    configureFrozen();
}

QicsTable::~QicsTable()
{
    delete myTableCommon->myRHDataModel;
    delete myTableCommon->myCHDataModel;

    delete styleManager();
    delete rhStyleManager();
    delete chStyleManager();

    delete dimensionManager();
    delete rhDimensionManager();
    delete chDimensionManager();

    delete mySelectionManager;
    delete myScrollManager;
}

void
QicsTable::initObjects(void)
{
    // the main style manager
    setStyleManager(new QicsStyleManager(TableGrid, this));
    // separate style managers for headers
    setRHStyleManager(new QicsStyleManager(RowHeaderGrid, this));
    setCHStyleManager(new QicsStyleManager(ColumnHeaderGrid, this));

    // set the default font in the style managers
    styleManager()->setDefaultProperty(QicsCellStyle::Font,
				       static_cast<void *>
				       (new QFont(QWidget::font())));
    rhStyleManager()->setDefaultProperty(QicsCellStyle::Font,
					 static_cast<void *>
					 (new QFont(QWidget::font())));
    chStyleManager()->setDefaultProperty(QicsCellStyle::Font,
					 static_cast<void *> 
					 (new QFont(QWidget::font())));

    // dimension managers - the main dimension manager first...
    setDimensionManager(new QicsDimensionManager(*styleManager()));
    // followed by a row header DM that passes on row height queries
    // to the main DM...
    setRHDimensionManager(new QicsDimensionManager(*rhStyleManager()));
    rhDimensionManager()->setControllingRowDimensionManager(dimensionManager());
    // and finally a column header DM that passes on column width queries
    // to the main DM.
    setCHDimensionManager(new QicsDimensionManager(*chStyleManager()));
    chDimensionManager()->setControllingColumnDimensionManager(dimensionManager());

    // keeps track of selections in the table
    mySelectionManager = new QicsSelectionManager();
    mySelectionManager->setDataModel(dataModel());
    mySelectionManager->setStyleManager(styleManager());
    mySelectionManager->setGridInfo(&gridInfo());

    connect(mySelectionManager, SIGNAL(selectionListChanged(bool)),
	    this, SIGNAL(selectionListChanged(bool)));

    // handles all the scrolling
    myScrollManager = new QicsScrollManager();
}

void
QicsTable::initDataModels(QicsDataModel *dm)
{
    // separate data models for headers
    if (dm)
    {
	myTableCommon->myRHDataModel =
	    (*myRowHeaderDMFoundry)(dm->numRows(), 1);
	myTableCommon->myCHDataModel =
	    (*myColumnHeaderDMFoundry)(1, dm->numColumns());
    }
    else
    {
	myTableCommon->myRHDataModel = 0;
	myTableCommon->myCHDataModel = 0;
    }

    gridInfo().setDataModel(dm);
    rhGridInfo().setDataModel(myTableCommon->myRHDataModel);
    chGridInfo().setDataModel(myTableCommon->myCHDataModel);
}

void
QicsTable::initGridInfoObjects(void)
{
    // set up the grid info for the main table grid
    gridInfo().setSelectionManager(mySelectionManager);
    connect(&gridInfo(),
	    SIGNAL(globalSetRepaintBehaviorRequest(QicsRepaintBehavior)),
	    this,
	    SLOT(setRepaintBehavior(QicsRepaintBehavior)));
    connect(&gridInfo(),
	    SIGNAL(globalRevertRepaintBehaviorRequest(void)),
	    this,
	    SLOT(revertRepaintBehavior(void)));
    connect(&gridInfo(),
	    SIGNAL(cellValueChanged(int, int)),
	    this,
	    SIGNAL(valueChanged(int, int)));

    // now set up grid info for the row headers
    rhGridInfo().setSelectionManager(mySelectionManager);
    connect(&rhGridInfo(),
	    SIGNAL(globalSetRepaintBehaviorRequest(QicsRepaintBehavior)),
	    this,
	    SLOT(setRepaintBehavior(QicsRepaintBehavior)));
    connect(&rhGridInfo(),
	    SIGNAL(globalRevertRepaintBehaviorRequest(void)),
	    this, SLOT(revertRepaintBehavior(void)));

    // finally, set up grid info for the column headers
    chGridInfo().setSelectionManager(mySelectionManager);
    connect(&chGridInfo(),
	    SIGNAL(globalSetRepaintBehaviorRequest(QicsRepaintBehavior)),
	    this,
	    SLOT(setRepaintBehavior(QicsRepaintBehavior)));
    connect(&chGridInfo(),
	    SIGNAL(globalRevertRepaintBehaviorRequest(void)),
	    this, SLOT(revertRepaintBehavior(void)));
}

void
QicsTable::initDisplay()
{
    int r;

    for (r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
            myGrids[r][c] = 0;
    }

    for(r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 2; ++c)
        {
            // the r and C here is misleading as the dimensions are different.
            // the first [] is rows, the second [] is cols (as always)
            myHHeaders[c][r] =0;
            myVHeaders [r][c] = 0 ;
        }
    }

    myMasterGrid = new QGridLayout(this,
				   3, 3,
				   myTableCommon->tableMargin(),
				   myTableCommon->tableSpacing(),
				   "masterGrid");
    myMasterGrid->setRowStretch(LAYOUT_GRID_LAYOUT_IDX, 1);
    myMasterGrid->setColStretch(LAYOUT_GRID_LAYOUT_IDX, 1);
    myMasterGrid->setResizeMode(QLayout::FreeResize);

    myGridLayout = new QGridLayout(7, 7,
				   myTableCommon->gridSpacing());
    myGridLayout->setRowStretch(GRID_LAYOUT_MAIN_GRID_IDX, 1);
    myGridLayout->setColStretch(GRID_LAYOUT_MAIN_GRID_IDX, 1);

    // Scrollers

    myRowScroller = new QicsScrollBarScroller(RowIndex,
					      this);
    myColumnScroller = new QicsScrollBarScroller(ColumnIndex,
						 this);

    myScrollManager->connectScroller(myRowScroller);
    myScrollManager->connectScroller(myColumnScroller);

    // create the main grid, this one never goes away..
    MAIN_GRID = createGrid(GRID_LAYOUT_MAIN_GRID_IDX,
			   GRID_LAYOUT_MAIN_GRID_IDX);

    MAIN_GRID->setVisibleRows(10);
    MAIN_GRID->setVisibleColumns(10);

    myScrollManager->setPrimaryGrid(MAIN_GRID);
    myScrollManager->connectGrid(MAIN_GRID, true, true);

    connect(&gridInfo(), SIGNAL(currentCellChanged(int, int)),
	    this, SIGNAL(currentCellChanged(int, int)));

    // Now set up the master grid
    myMasterGrid->addLayout(myGridLayout, LAYOUT_GRID_LAYOUT_IDX,
			    LAYOUT_GRID_LAYOUT_IDX);

    myGridLayout->addWidget(myRowScroller->widget(),
			    GRID_LAYOUT_MAIN_GRID_IDX,
			    GRID_LAYOUT_RIGHT_SCROLLBAR_IDX);
    myGridLayout->addWidget(myColumnScroller->widget(),
			    GRID_LAYOUT_BOTTOM_SCROLLBAR_IDX,
			    GRID_LAYOUT_MAIN_GRID_IDX);

    myGridLayout->addRowSpacing(GRID_LAYOUT_TOP_HDR_IDX, 5);
    myGridLayout->addRowSpacing(GRID_LAYOUT_BOTTOM_HDR_IDX, 5);
    myGridLayout->addColSpacing(GRID_LAYOUT_LEFT_HDR_IDX, 5);
    myGridLayout->addColSpacing(GRID_LAYOUT_RIGHT_HDR_IDX, 5);

    QSize sz = myColumnScroller->widget()->sizeHint();
    myGridLayout->addRowSpacing(GRID_LAYOUT_BOTTOM_SCROLLBAR_IDX,
				sz.height());
    sz = myRowScroller->widget()->sizeHint();
    myGridLayout->addColSpacing(GRID_LAYOUT_RIGHT_SCROLLBAR_IDX,
				sz.width());

    connect(MAIN_GRID, SIGNAL(scrollRequest(QicsScrollDirection, int)),
            this, SLOT(scroll(QicsScrollDirection, int)));
}

void
QicsTable::setDataModel(QicsDataModel *dm)
{
    initDataModels(dm);
    initGridInfoObjects();
    mySelectionManager->setDataModel(dataModel());

    // we need to keep the three data models in sync so that when
    // rows and columns are inserted or deleted in the main data model,
    // the header data models do the same.
    gridInfo().controlRowsOf(&rhGridInfo());
    gridInfo().controlColumnsOf(&chGridInfo());
}

QicsTableGrid *
QicsTable::createGrid(int grid_row, int grid_col)
{
    QicsTableGrid *grid = (*myTableGridFoundry)(this, gridInfo(), 0, 0);

    gridInfo().connectGrid(grid);

    connect(grid, SIGNAL(pressed(int, int, int, const QPoint &)),
	    this, SLOT(handleGridPress(int, int, int, const QPoint &)));
    connect(grid, SIGNAL(clicked(int, int, int, const QPoint &)),
	    this, SLOT(handleGridClick(int, int, int, const QPoint &)));
    connect(grid, SIGNAL(doubleClicked(int, int, int, const QPoint &)),
	    this, SLOT(handleGridDoubleClick(int, int, int, const QPoint &)));
    
    myGridLayout->addWidget(grid, grid_row, grid_col);
    grid->show();

    return grid;
}

QicsHeaderGrid *
QicsTable::createHeader(QicsHeaderType type, int grid_row, int grid_col,
			int alignment)
{
    QicsGridInfo &info = (type == RowHeader ? rhGridInfo() :
			  chGridInfo());

    QicsHeaderGrid *hdr = (*myHeaderGridFoundry)(this, info, type);
    info.connectGrid(hdr);

    connect(hdr, SIGNAL(scrollRequest(QicsScrollDirection, int)),
            this, SLOT(scroll(QicsScrollDirection, int)));

    connect(hdr, SIGNAL(gripDoubleClicked(int, int, QicsHeaderType)),
	    this, SLOT(handleHeaderDoubleClick(int, int, QicsHeaderType)));

    myGridLayout->addWidget(hdr, grid_row, grid_col, alignment);
    hdr->show();

    return (hdr);
}

void QicsTable::configureFrozen(void)
{
    // Go through the regions listed below. The center 0,0 doesn't
    // need to be checked, as it's always on. Various flags determine
    // what can be shown in any other region.

    //  frozen L             frozen R
    // ------------------------------
    // | Reg1  |   reg2    |  reg3  |  frozen Top
    // ------------------------------
    // | reg5  |           |  reg6  |
    // ------------------------------
    // | reg7  |  reg8     |  reg9  |  Frozen Bottom
    // ------------------------------

    //check reg1
    if ((myNumLeftFrozenColumns > 0) &&
        (myNumTopFrozenRows > 0))
    {
        if(!TOP_LEFT_GRID)
        {
            TOP_LEFT_GRID = createGrid(GRID_LAYOUT_TOP_FROZEN_GRID_IDX,
				       GRID_LAYOUT_LEFT_FROZEN_GRID_IDX);
        }
    }
    else
    {
        if (TOP_LEFT_GRID)
        {
	    delete TOP_LEFT_GRID;
	    TOP_LEFT_GRID = 0;
        }
    }

    // check reg 2
    if (myNumTopFrozenRows > 0)
    {
        if (!LEFT_TOP_HEADER && leftHeaderVisible())
	{
	    LEFT_TOP_HEADER = createHeader(RowHeader,
					   GRID_LAYOUT_TOP_FROZEN_GRID_IDX,
					   GRID_LAYOUT_LEFT_HDR_IDX,
					   Qt::AlignRight);
	}
	else if (LEFT_TOP_HEADER && !leftHeaderVisible())
	{
	    delete LEFT_TOP_HEADER;
	    LEFT_TOP_HEADER = 0;
	}

        if (!RIGHT_TOP_HEADER && rightHeaderVisible())
        {
            RIGHT_TOP_HEADER = createHeader(RowHeader,
					    GRID_LAYOUT_TOP_FROZEN_GRID_IDX,
					    GRID_LAYOUT_RIGHT_HDR_IDX,
					    Qt::AlignLeft);
        }
	else if (RIGHT_TOP_HEADER && !rightHeaderVisible())
	{
	    delete RIGHT_TOP_HEADER;
	    RIGHT_TOP_HEADER = 0;
	}

        if (!TOP_MIDDLE_GRID)
        {
            TOP_MIDDLE_GRID = createGrid(GRID_LAYOUT_TOP_FROZEN_GRID_IDX,
					 GRID_LAYOUT_MAIN_GRID_IDX);
	    myScrollManager->connectGrid(TOP_MIDDLE_GRID, false, true);
        }
    }
    else
    {
        if (LEFT_TOP_HEADER)
        {
            delete LEFT_TOP_HEADER;
            LEFT_TOP_HEADER = 0;
        }
        if (RIGHT_TOP_HEADER)
        {
            delete RIGHT_TOP_HEADER;
            RIGHT_TOP_HEADER = 0;
        }
        if (TOP_MIDDLE_GRID)
        {
            delete TOP_MIDDLE_GRID;
	    TOP_MIDDLE_GRID = 0;
        }
    }
    // check reg 3
    if ((myNumTopFrozenRows > 0) && (myNumRightFrozenColumns > 0))
    {
        if (!TOP_RIGHT_GRID)
        {
            TOP_RIGHT_GRID = createGrid(GRID_LAYOUT_TOP_FROZEN_GRID_IDX,
					GRID_LAYOUT_RIGHT_FROZEN_GRID_IDX);
        }
    }
    else
    {
        if (TOP_RIGHT_GRID)
        {
            delete TOP_RIGHT_GRID;
	    TOP_RIGHT_GRID = 0;
        }
    }

    // check Reg5
    if (myNumLeftFrozenColumns > 0)
    {
        if(!TOP_LEFT_HEADER && topHeaderVisible() )
        {
            TOP_LEFT_HEADER = createHeader(ColumnHeader,
					   GRID_LAYOUT_TOP_HDR_IDX,
					   GRID_LAYOUT_LEFT_FROZEN_GRID_IDX,
					   Qt::AlignBottom);
        }
	else if (TOP_LEFT_HEADER && !topHeaderVisible())
	{
	    delete TOP_LEFT_HEADER;
	    TOP_LEFT_HEADER = 0;
	}

        if(!BOTTOM_LEFT_HEADER && bottomHeaderVisible() )
        {
            BOTTOM_LEFT_HEADER = createHeader(ColumnHeader,
					      GRID_LAYOUT_BOTTOM_HDR_IDX,
					      GRID_LAYOUT_LEFT_FROZEN_GRID_IDX,
					      Qt::AlignTop);
        }
	else if (BOTTOM_LEFT_HEADER && !bottomHeaderVisible())
	{
	    delete BOTTOM_LEFT_HEADER;
	    BOTTOM_LEFT_HEADER = 0;
	}

        if(!MIDDLE_LEFT_GRID)
        {
            MIDDLE_LEFT_GRID = createGrid(GRID_LAYOUT_MAIN_GRID_IDX,
					  GRID_LAYOUT_LEFT_FROZEN_GRID_IDX);
	    myScrollManager->connectGrid(MIDDLE_LEFT_GRID, true, false);
        }
    }
    else
    {
        if (TOP_LEFT_HEADER)
        {
            delete TOP_LEFT_HEADER;
            TOP_LEFT_HEADER = 0;
        }
        if (BOTTOM_LEFT_HEADER)
        {
            delete BOTTOM_LEFT_HEADER;
            BOTTOM_LEFT_HEADER = 0;
        }
        if (MIDDLE_LEFT_GRID)
        {
            delete MIDDLE_LEFT_GRID;
	    MIDDLE_LEFT_GRID = 0;
        }
    }

    // check Reg6
    if (myNumRightFrozenColumns > 0)
    {
        if (!TOP_RIGHT_HEADER && topHeaderVisible())
        {
            TOP_RIGHT_HEADER = createHeader(ColumnHeader,
					    GRID_LAYOUT_TOP_HDR_IDX,
					    GRID_LAYOUT_RIGHT_FROZEN_GRID_IDX,
					    Qt::AlignBottom);
        }
	else if (TOP_RIGHT_HEADER && !topHeaderVisible())
	{
	    delete TOP_RIGHT_HEADER;
	    TOP_RIGHT_HEADER = 0;
	}

        if (!BOTTOM_RIGHT_HEADER && bottomHeaderVisible())
        {
            BOTTOM_RIGHT_HEADER = createHeader(ColumnHeader,
					       GRID_LAYOUT_BOTTOM_HDR_IDX,
					       GRID_LAYOUT_RIGHT_FROZEN_GRID_IDX,
					       Qt::AlignTop);
        }
	else if (BOTTOM_RIGHT_HEADER && !bottomHeaderVisible())
	{
	    delete BOTTOM_RIGHT_HEADER;
	    BOTTOM_RIGHT_HEADER = 0;
	}

        if (!MIDDLE_RIGHT_GRID)
        {
            MIDDLE_RIGHT_GRID = createGrid(GRID_LAYOUT_MAIN_GRID_IDX,
					   GRID_LAYOUT_RIGHT_FROZEN_GRID_IDX);
	    myScrollManager->connectGrid(MIDDLE_RIGHT_GRID, true, false);
        }
    }
    else
    {
        if (TOP_RIGHT_HEADER)
        {
            delete TOP_RIGHT_HEADER;
            TOP_RIGHT_HEADER = 0;
        }
        if (BOTTOM_RIGHT_HEADER)
        {
            delete BOTTOM_RIGHT_HEADER;
            BOTTOM_RIGHT_HEADER = 0;
        }
        if (MIDDLE_RIGHT_GRID)
        {
            delete MIDDLE_RIGHT_GRID;
	    MIDDLE_RIGHT_GRID = 0;
        }
    }
    // chech reg7
    if ((myNumLeftFrozenColumns > 0) &&
        (myNumBottomFrozenRows > 0))
    {
        if(!BOTTOM_LEFT_GRID)
        {
            BOTTOM_LEFT_GRID = createGrid(GRID_LAYOUT_BOTTOM_FROZEN_GRID_IDX,
					  GRID_LAYOUT_LEFT_FROZEN_GRID_IDX);
        }
    }
    else
    {
        if(BOTTOM_LEFT_GRID)
        {
            delete BOTTOM_LEFT_GRID;
	    BOTTOM_LEFT_GRID = 0;
        }

    }

    // check reg8
    if (myNumBottomFrozenRows > 0)
    {
        if (!LEFT_BOTTOM_HEADER && leftHeaderVisible())
        {
            LEFT_BOTTOM_HEADER = createHeader(RowHeader,
					      GRID_LAYOUT_BOTTOM_FROZEN_GRID_IDX,
					      GRID_LAYOUT_LEFT_HDR_IDX,
					      Qt::AlignRight);
        }
	else if (LEFT_BOTTOM_HEADER && !leftHeaderVisible())
	{
	    delete LEFT_BOTTOM_HEADER;
	    LEFT_BOTTOM_HEADER = 0;
	}

        if (!RIGHT_BOTTOM_HEADER && rightHeaderVisible())
        {
            RIGHT_BOTTOM_HEADER = createHeader(RowHeader,
					       GRID_LAYOUT_BOTTOM_FROZEN_GRID_IDX,
					       GRID_LAYOUT_RIGHT_HDR_IDX,
					       Qt::AlignLeft);
        }
	else if (RIGHT_BOTTOM_HEADER && !rightHeaderVisible())
	{
	    delete RIGHT_BOTTOM_HEADER;
	    RIGHT_BOTTOM_HEADER = 0;
	}

        if (!BOTTOM_MIDDLE_GRID)
        {
            BOTTOM_MIDDLE_GRID = createGrid(GRID_LAYOUT_BOTTOM_FROZEN_GRID_IDX,
					    GRID_LAYOUT_MAIN_GRID_IDX);
	    myScrollManager->connectGrid(BOTTOM_MIDDLE_GRID, false, true);
        }
    }
    else
    {
        if(LEFT_BOTTOM_HEADER)
        {
            delete LEFT_BOTTOM_HEADER;
            LEFT_BOTTOM_HEADER = 0;
        }
        if(RIGHT_BOTTOM_HEADER)
        {
            delete RIGHT_BOTTOM_HEADER;
            RIGHT_BOTTOM_HEADER = 0;
        }
        if(BOTTOM_MIDDLE_GRID)
        {
            delete BOTTOM_MIDDLE_GRID;
	    BOTTOM_MIDDLE_GRID = 0;
        }
    }
    // check reg9
    if ((myNumRightFrozenColumns > 0) &&
        (myNumBottomFrozenRows > 0))
    {
        if(!BOTTOM_RIGHT_GRID)
        {
            BOTTOM_RIGHT_GRID = createGrid(GRID_LAYOUT_BOTTOM_FROZEN_GRID_IDX,
					   GRID_LAYOUT_RIGHT_FROZEN_GRID_IDX);
        }
    }
    else
    {
        if(BOTTOM_RIGHT_GRID)
        {
            delete BOTTOM_RIGHT_GRID;
	    BOTTOM_RIGHT_GRID = 0;
        }
    }

    if (topHeaderVisible())
    {
        if (!TOP_HEADER)
        {
            TOP_HEADER = createHeader(ColumnHeader,
				      GRID_LAYOUT_TOP_HDR_IDX,
				      GRID_LAYOUT_MAIN_GRID_IDX,
				      Qt::AlignBottom);

	    myScrollManager->connectGrid(TOP_HEADER, false, true);

            connect(TOP_HEADER, SIGNAL(sizeChange(int, int, int, QicsHeaderType)),
                    MAIN_GRID, SLOT(recomputeAndDraw()));

            connect(TOP_HEADER, SIGNAL(resizeInProgress(int, int, QicsHeaderType)),
                    MAIN_GRID, SLOT(drawHeaderResizeBar(int, int, QicsHeaderType)));
        }
    }
    else
    {
        if (TOP_HEADER)
        {
            delete TOP_HEADER;
            TOP_HEADER = 0;
        }
    }

    if (bottomHeaderVisible())
    {
        if (!BOTTOM_HEADER)
        {
            BOTTOM_HEADER = createHeader(ColumnHeader,
					 GRID_LAYOUT_BOTTOM_HDR_IDX,
					 GRID_LAYOUT_MAIN_GRID_IDX,
					 Qt::AlignTop);

	    myScrollManager->connectGrid(BOTTOM_HEADER, false, true);

            connect(BOTTOM_HEADER, SIGNAL(sizeChange(int, int, int, QicsHeaderType)),
                    MAIN_GRID, SLOT(recomputeAndDraw()));

            connect(BOTTOM_HEADER, SIGNAL(resizeInProgress(int, int, QicsHeaderType)),
                    MAIN_GRID, SLOT(drawHeaderResizeBar(int, int, QicsHeaderType)));
         }
    }
    else
    {
        if (BOTTOM_HEADER)
        {
            delete BOTTOM_HEADER;
            BOTTOM_HEADER = 0;
        }
    }

    if (leftHeaderVisible())
    {
        if(!LEFT_HEADER)
        {
            LEFT_HEADER = createHeader(RowHeader,
				       GRID_LAYOUT_MAIN_GRID_IDX,
				       GRID_LAYOUT_LEFT_HDR_IDX,
				       Qt::AlignRight);


	    myScrollManager->connectGrid(LEFT_HEADER, true, false);

            connect(LEFT_HEADER, SIGNAL(sizeChange(int, int, int, QicsHeaderType)),
                    MAIN_GRID, SLOT(recomputeAndDraw()));

            connect(LEFT_HEADER, SIGNAL(resizeInProgress(int, int, QicsHeaderType)),
                    MAIN_GRID, SLOT(drawHeaderResizeBar(int, int, QicsHeaderType)));
        }
    }
    else
    {
        if (LEFT_HEADER)
        {
            delete LEFT_HEADER;
            LEFT_HEADER =0;
        }
    }

    if (rightHeaderVisible())
    {
        if (!RIGHT_HEADER)
        {
            RIGHT_HEADER = createHeader(RowHeader,
					GRID_LAYOUT_MAIN_GRID_IDX,
					GRID_LAYOUT_RIGHT_HDR_IDX,
					Qt::AlignLeft);

	    myScrollManager->connectGrid(RIGHT_HEADER, true, false);

            connect(RIGHT_HEADER, SIGNAL(sizeChange(int, int, int, QicsHeaderType)),
                    MAIN_GRID, SLOT(recomputeAndDraw()));

            connect(RIGHT_HEADER, SIGNAL(resizeInProgress(int, int, QicsHeaderType)),
                    MAIN_GRID, SLOT(drawHeaderResizeBar(int, int, QicsHeaderType)));
        }
    }
    else
    {
        if (RIGHT_HEADER)
        {
            delete RIGHT_HEADER;
            RIGHT_HEADER = 0;
        }
    }

    //
    // Configure viewports
    //

    QicsRegion vp, cvp, tvp;

    // Begin by setting all of the frozen grids' viewports to the
    // full viewport of the table

    for(int i = 0; i < 3; ++i)
    {
	for (int j = 0; j < 3; ++j)
	{
	    if ((i != GRID_MIDDLE_IDX) || (j != GRID_MIDDLE_IDX))
	    {
		QicsGrid *g = myGrids[i][j];

		if (g) g->setViewport(viewport());
	    }
	}
    }

    // Constrain the frozen grids' viewports based on the
    // number of frozen rows and columns

    if (myNumTopFrozenRows > 0)
    {
	for(int i = 0; i < 3; ++i)
	{
	    QicsTableGrid *grid = myGrids[GRID_TOP_IDX][i];

	    if (grid)
	    {
		if (i == GRID_MIDDLE_IDX)
		    vp = MAIN_GRID->viewport();
		else
		    vp = grid->viewport();

		tvp = viewport();

		vp.setStartRow(tvp.startRow());
		vp.setEndRow(tvp.startRow() + myNumTopFrozenRows - 1);

		grid->setViewport(vp);
		grid->fixHeightToViewport(true);
		grid->setTopRow(vp.startRow());
	    }
	}
    }

    if (myNumBottomFrozenRows > 0)
    {
	for(int i = 0; i < 3; ++i)
	{
	    QicsTableGrid *grid = myGrids[GRID_BOTTOM_IDX][i];

	    if (grid)
	    {
		if (i == GRID_MIDDLE_IDX)
		    vp = MAIN_GRID->viewport();
		else
		    vp = grid->viewport();

		cvp = grid->currentViewport();
		tvp = viewport();

		vp.setStartRow(cvp.endRow() - myNumBottomFrozenRows + 1);
		vp.setEndRow(tvp.endRow());

		grid->setViewport(vp);
		grid->fixHeightToViewport(true);
		grid->setTopRow(vp.startRow());
	    }
	}
    }

    if (myNumLeftFrozenColumns > 0)
    {
	for(int i = 0; i < 3; ++i)
	{
	    QicsTableGrid *grid = myGrids[i][GRID_LEFT_IDX];

	    if (grid)
	    {
		if (i == GRID_MIDDLE_IDX)
		    vp = MAIN_GRID->viewport();
		else
		    vp = grid->viewport();

		tvp = viewport();

		vp.setStartColumn(tvp.startColumn());
		vp.setEndColumn(tvp.startColumn() + myNumLeftFrozenColumns - 1);

		grid->setViewport(vp);
		grid->fixWidthToViewport(true);
		grid->setLeftColumn(vp.startColumn());
	    }
	}
    }

    if (myNumRightFrozenColumns > 0)
    {
	for(int i = 0; i < 3; ++i)
	{
	    QicsTableGrid *grid = myGrids[i][GRID_RIGHT_IDX];

	    if (grid)
	    {
		if (i == GRID_MIDDLE_IDX)
		    vp = MAIN_GRID->viewport();
		else
		    vp = grid->viewport();

		cvp = grid->currentViewport();
		tvp = viewport();

		vp.setStartColumn(cvp.endColumn() - myNumRightFrozenColumns + 1);
		vp.setEndColumn(tvp.endColumn());

		grid->setViewport(vp);
		grid->fixWidthToViewport(true);
		grid->setLeftColumn(vp.startColumn());
	    }
	}
    }

    // Configure headers

    // Begin by setting all of the frozen headers' viewports to the
    // full viewport of the appropriate header

    {
	QicsRegion row_hdr_vp = rowHeaderRef().viewport();

	for (int i = 0; i < 2; ++i)
	{
	    QicsHeaderGrid *hdr = myVHeaders[TOP_VHEADER_IDX][i];
	    if (hdr)
		hdr->setViewport(row_hdr_vp);
	    
	    hdr = myVHeaders[BOTTOM_VHEADER_IDX][i];
	    if (hdr)
		hdr->setViewport(row_hdr_vp);
	}
    }

    {
	QicsRegion col_hdr_vp = columnHeaderRef().viewport();

	for (int i = 0; i < 2; ++i)
	{
	    QicsHeaderGrid *hdr = myHHeaders[i][LEFT_HHEADER_IDX];
	    if (hdr)
		hdr->setViewport(col_hdr_vp);
	    
	    hdr = myHHeaders[i][RIGHT_HHEADER_IDX];
	    if (hdr)
		hdr->setViewport(col_hdr_vp);
	}
    }

    // Constrain the frozen headers' viewports based on the
    // number of frozen rows and columns


    if (myNumTopFrozenRows > 0)
    {
	for (int i = 0; i < 2; ++i)
	{
	    QicsHeaderGrid *hdr = myVHeaders[TOP_VHEADER_IDX][i];
	    if (hdr)
	    {
		vp = hdr->viewport();
		tvp = viewport();

		vp.setStartRow(tvp.startRow());
		vp.setEndRow(tvp.startRow() + myNumTopFrozenRows - 1);

		hdr->setViewport(vp);
		hdr->setTopRow(vp.startRow());
	    }
	}
    }

    if (myNumBottomFrozenRows > 0)
    {
	for (int i = 0; i < 2; ++i)
	{
	    QicsHeaderGrid *hdr = myVHeaders[BOTTOM_VHEADER_IDX][i];
	    if (hdr)
	    {
		vp = hdr->viewport();
		tvp = viewport();
		cvp = MAIN_GRID->currentViewport();

		vp.setStartRow(cvp.endRow() + 1);
		vp.setEndRow(tvp.endRow());

		hdr->setViewport(vp);
		hdr->setTopRow(vp.startRow());
	    }
	}
    }

    if (myNumLeftFrozenColumns > 0)
    {
	for (int i = 0; i < 2; ++i)
	{
	    QicsHeaderGrid *hdr = myHHeaders[i][LEFT_HHEADER_IDX];
	    if (hdr)
	    {
		vp = hdr->viewport();
		tvp = viewport();

		vp.setStartColumn(tvp.startColumn());
		vp.setEndColumn(tvp.startColumn() + myNumLeftFrozenColumns - 1);

		hdr->setViewport(vp);
		hdr->setLeftColumn(vp.startColumn());
	    }
	}
    }

    if (myNumRightFrozenColumns > 0)
    {
	for (int i = 0; i < 2; ++i)
	{
	    QicsHeaderGrid *hdr = myHHeaders[i][RIGHT_HHEADER_IDX];
	    if (hdr)
	    {
		vp = hdr->viewport();
		tvp = viewport();
		cvp = MAIN_GRID->currentViewport();

		vp.setStartColumn(cvp.endColumn() + 1);
		vp.setEndColumn(tvp.endColumn());

		hdr->setViewport(vp);
		hdr->setLeftColumn(vp.startColumn());
	    }
	}
    }
}

void
QicsTable::setTopRow(int row)
{
    myScrollManager->setRowIndex(row);
}

void
QicsTable::setLeftColumn(int col)
{
    myScrollManager->setColumnIndex(col);
}

int
QicsTable::visibleRows(void) const
{
    return MAIN_GRID->visibleRows();
}

int
QicsTable::visibleColumns(void) const
{
    return MAIN_GRID->visibleColumns();
}

void
QicsTable::setVisibleRows(int num)
{
    MAIN_GRID->setVisibleRows(num);
}

void
QicsTable::setVisibleColumns(int num)
{
    MAIN_GRID->setVisibleColumns(num);
}

QScrollBar *
QicsTable::horizontalScrollBar(void) const
{
    return myColumnScroller->widget();
}

QScrollBar *
QicsTable::verticalScrollBar(void) const
{
    return myRowScroller->widget();
}

void
QicsTable::setTopTitleWidget(QWidget *w)
{
    if (myTopTitleWidget)
	delete myTopTitleWidget;

    myTopTitleWidget = w;

    if (w)
	myMasterGrid->addWidget(w, LAYOUT_TOP_TITLE_IDX,
				LAYOUT_GRID_LAYOUT_IDX);
}

void
QicsTable::setBottomTitleWidget(QWidget *w)
{
    if (myBottomTitleWidget)
	delete myBottomTitleWidget;

    myBottomTitleWidget = w;

    if (w)
	myMasterGrid->addWidget(w, LAYOUT_BOTTOM_TITLE_IDX,
				LAYOUT_GRID_LAYOUT_IDX);
}

void
QicsTable::setLeftTitleWidget(QWidget *w)
{
    if (myLeftTitleWidget)
	delete myLeftTitleWidget;

    myLeftTitleWidget = w;

    if (w)
	myMasterGrid->addWidget(w, LAYOUT_GRID_LAYOUT_IDX,
				LAYOUT_LEFT_TITLE_IDX);
}

void
QicsTable::setRightTitleWidget(QWidget *w)
{
    if (myRightTitleWidget)
	delete myRightTitleWidget;

    myRightTitleWidget = w;

    if (w)
	myMasterGrid->addWidget(w, LAYOUT_GRID_LAYOUT_IDX,
				LAYOUT_RIGHT_TITLE_IDX);
}

void
QicsTable::setTopLeftCornerWidget(QWidget *w)
{
    if (myTopLeftCornerWidget)
	delete myTopLeftCornerWidget;

    myTopLeftCornerWidget = w;

    if (w)
	myGridLayout->addWidget(w, GRID_LAYOUT_TOP_HDR_IDX,
				GRID_LAYOUT_LEFT_HDR_IDX);
}

void
QicsTable::setTopRightCornerWidget(QWidget *w)
{
    if (myTopRightCornerWidget)
	delete myTopRightCornerWidget;

    myTopRightCornerWidget = w;

    if (w)
	myGridLayout->addWidget(w, GRID_LAYOUT_TOP_HDR_IDX,
				GRID_LAYOUT_RIGHT_HDR_IDX);
}

void
QicsTable::setBottomLeftCornerWidget(QWidget *w)
{
    if (myBottomLeftCornerWidget)
	delete myBottomLeftCornerWidget;

    myBottomLeftCornerWidget = w;

    if (w)
	myGridLayout->addWidget(w, GRID_LAYOUT_BOTTOM_HDR_IDX,
				GRID_LAYOUT_LEFT_HDR_IDX);
}

void
QicsTable::setBottomRightCornerWidget(QWidget *w)
{
    if (myBottomRightCornerWidget)
	delete myBottomRightCornerWidget;

    myBottomRightCornerWidget = w;

    if (w)
	myGridLayout->addWidget(w, GRID_LAYOUT_BOTTOM_HDR_IDX,
				GRID_LAYOUT_RIGHT_HDR_IDX);
}


// computes the intersection of the data model dimensions and the
// viewport dimensions
QicsRegion
QicsTable::currentViewport(void) const
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

void
QicsTable::toggleRowHeightExpansion(int row)
{
    if (dimensionManager()->overriddenRowHeight(row) >= 0)
    {
	dimensionManager()->resetRowHeight(row);
    }
    else
    {
	QicsRegion vp = currentViewport();

	int h = 0;

	QicsCell cell(row, 0, this, false);

	for (int i = vp.startColumn(); i <= vp.endColumn(); ++i)
	{
	    cell.setColumnIndex(i);
	    QicsCellDisplay *cd = cell.displayer();
	    const QicsDataItem *itm = cell.dataValue();
		
	    h = QICS_MAX(h, cd->sizeHint(MAIN_GRID, row, i, itm).height());
	}
    
	dimensionManager()->overrideRowHeight(row, h);
    }
}

void
QicsTable::toggleColumnWidthExpansion(int col)
{
    if (dimensionManager()->overriddenColumnWidth(col) >= 0)
    {
	dimensionManager()->resetColumnWidth(col);
    }
    else
    {
	QicsRegion vp = currentViewport();

	int w = 0;

	QicsCell cell(0, col, this, false);

	for (int i = vp.startRow(); i <= vp.endRow(); ++i)
	{
	    cell.setRowIndex(i);
	    QicsCellDisplay *cd = cell.displayer();
	    const QicsDataItem *itm = cell.dataValue();

	    w = QICS_MAX(w, cd->sizeHint(MAIN_GRID, i, col, itm).width());
	}
    
	dimensionManager()->overrideColumnWidth(col, w);
    }
}

////////////////////////////////////////////////////////////////////
///////////       Grid Access Methods                 //////////////
////////////////////////////////////////////////////////////////////

QicsRow &
QicsTable::rowRef(int rownum)
{
    return myTableCommon->rowRef(rownum);
}

const QicsRow &
QicsTable::rowRef(int rownum) const
{
    return myTableCommon->rowRef(rownum);
}

QicsRow *
QicsTable::row(int rownum, bool follow_model)
{
    return myTableCommon->row(rownum, follow_model);
}

const QicsRow *
QicsTable::row(int rownum, bool follow_model) const
{
    return myTableCommon->row(rownum, follow_model);
}

QicsColumn &
QicsTable::columnRef(int colnum)
{
    return myTableCommon->columnRef(colnum);
}

const QicsColumn &
QicsTable::columnRef(int colnum) const
{
    return myTableCommon->columnRef(colnum);
}

QicsColumn *
QicsTable::column(int colnum, bool follow_model)
{
    return myTableCommon->column(colnum, follow_model);
}

const QicsColumn *
QicsTable::column(int colnum, bool follow_model) const
{
    return myTableCommon->column(colnum, follow_model);
}

QicsCell &
QicsTable::cellRef(int rownum, int colnum)
{
    return myTableCommon->cellRef(rownum, colnum);
}

const QicsCell &
QicsTable::cellRef(int rownum, int colnum) const
{
    return myTableCommon->cellRef(rownum, colnum);
}

QicsCell *
QicsTable::cell(int rownum, int colnum, bool follow_model)
{
    return myTableCommon->cell(rownum, colnum, follow_model);
}

QicsMainGrid &
QicsTable::mainGridRef(void)
{
    return myTableCommon->mainGridRef();
}

const QicsMainGrid &
QicsTable::mainGridRef(void) const
{
    return myTableCommon->mainGridRef();
}

QicsMainGrid *
QicsTable::mainGrid(void)
{
    return myTableCommon->mainGrid();
}

const QicsMainGrid *
QicsTable::mainGrid(void) const
{
    return myTableCommon->mainGrid();
}

QicsRowHeader &
QicsTable::rowHeaderRef(void)
{
    return myTableCommon->rowHeaderRef();
}

const QicsRowHeader &
QicsTable::rowHeaderRef(void) const
{
    return myTableCommon->rowHeaderRef();
}

QicsRowHeader *
QicsTable::rowHeader(void)
{
    return myTableCommon->rowHeader();
}

const QicsRowHeader *
QicsTable::rowHeader(void) const
{
    return myTableCommon->rowHeader();
}

QicsColumnHeader &
QicsTable::columnHeaderRef(void)
{
    return myTableCommon->columnHeaderRef();
}

const QicsColumnHeader &
QicsTable::columnHeaderRef(void) const
{
    return myTableCommon->columnHeaderRef();
}

QicsColumnHeader *
QicsTable::columnHeader(void)
{
    return myTableCommon->columnHeader();
}

const QicsColumnHeader *
QicsTable::columnHeader(void) const
{
    return myTableCommon->columnHeader();
}

////////////////////////////////////////////////////////////////////
///////////         Property Methods                  //////////////
////////////////////////////////////////////////////////////////////

///////// Cell Attribute methods

int
QicsTable::margin(void) const
{
    return myTableCommon->margin();
}

void 
QicsTable::setMargin(int margin)
{
    myTableCommon->setMargin(margin);
}

QColor
QicsTable::selectedForegroundColor(void) const
{
    return myTableCommon->selectedForegroundColor();
}

void
QicsTable::setSelectedForegroundColor(const QColor &color)
{
    myTableCommon->setSelectedForegroundColor(color);
}

QColor
QicsTable::selectedBackgroundColor(void) const
{
    return myTableCommon->selectedBackgroundColor();
}

void
QicsTable::setSelectedBackgroundColor(const QColor &color)
{
    myTableCommon->setSelectedBackgroundColor(color);
}

bool
QicsTable::readOnly(void) const
{
    return myTableCommon->readOnly();
}

void
QicsTable::setReadOnly(bool b)
{
    myTableCommon->setReadOnly(b);
}

QPixmap
QicsTable::pixmap(void) const
{
    return myTableCommon->pixmap();
}

void
QicsTable::setPixmap(const QPixmap &p)
{
    myTableCommon->setPixmap(p);
}

int
QicsTable::pixmapSpacing(void) const
{
    return myTableCommon->pixmapSpacing();
}

void
QicsTable::setPixmapSpacing(int sp)
{
    myTableCommon->setPixmapSpacing(sp);
}

QicsCellDisplay *
QicsTable::displayer(void) const
{
    return myTableCommon->displayer();
}

void
QicsTable::setDisplayer(QicsCellDisplay *dsp)
{
    myTableCommon->setDisplayer(dsp);
}

QicsDataItemFormatter *
QicsTable::formatter(void) const
{
    return myTableCommon->formatter();
}

void
QicsTable::setFormatter(QicsDataItemFormatter *f)
{
    myTableCommon->setFormatter(f);
}

int
QicsTable::alignment(void) const
{
    return myTableCommon->alignment();
}

void
QicsTable::setAlignment(int flags)
{
    myTableCommon->setAlignment(flags);
}

int
QicsTable::textFlags(void) const
{
    return myTableCommon->textFlags();
}

void
QicsTable::setTextFlags(int flags)
{
    myTableCommon->setTextFlags(flags);
}

QValidator *
QicsTable::validator(void) const
{
    return myTableCommon->validator();
}

void
QicsTable::setValidator(QValidator *v)
{
    myTableCommon->setValidator(v);
}

QString
QicsTable::label(void) const
{
    return myTableCommon->label();
}

void 
QicsTable::setLabel(const QString &label)
{
    myTableCommon->setLabel(label);
}

int
QicsTable::maxLength(void) const
{
    return myTableCommon->maxLength();
}

void
QicsTable::setMaxLength(int len)
{
    myTableCommon->setMaxLength(len);
}

int
QicsTable::borderWidth(void) const
{
    return myTableCommon->borderWidth();
}

void
QicsTable::setBorderWidth(int bw)
{
    myTableCommon->setBorderWidth(bw);
}

Qics::QicsLineStyle
QicsTable::borderStyle(void) const
{
    return myTableCommon->borderStyle();
}

void
QicsTable::setBorderStyle(QicsLineStyle bs)
{
    myTableCommon->setBorderStyle(bs);
}

QPen
QicsTable::borderPen(void) const
{
    return myTableCommon->borderPen();
}

void
QicsTable::setBorderPen(const QPen &pen)
{
    myTableCommon->setBorderPen(pen);
}

///////// Grid Attribute methods

QicsRegion
QicsTable::viewport(void) const
{
    return myFullViewport;
}

void
QicsTable::setViewport(const QicsRegion &vp)
{
    myFullViewport = vp;
    setMainGridViewport(vp);
}

QicsRegion
QicsTable::mainGridViewport(void) const
{
    return myTableCommon->viewport();
}

void
QicsTable::setMainGridViewport(const QicsRegion &vp)
{
    QicsRegion old_region;
    QicsRegion new_region;

    // Set the row header viewport
    QicsHeader *rh = rowHeader();
    old_region = rh->viewport();
    new_region = QicsRegion(vp.startRow(), old_region.startColumn(), 
			    vp.endRow(), old_region.endColumn());
    rh->setViewport(new_region);
    delete rh;

    // Set the column header viewport
    QicsHeader *ch = columnHeader();
    old_region = ch->viewport();
    new_region = QicsRegion(old_region.startRow(), vp.startColumn(),
			    old_region.endRow(), vp.endColumn());
    ch->setViewport(new_region);
    delete ch;

    // Set the main viewport
    myTableCommon->setViewport(vp);
}

bool
QicsTable::addCellSpan(QicsSpan span)
{
    return myTableCommon->addCellSpan(span);
}

void
QicsTable::removeCellSpan(int start_row, int start_col)
{
    myTableCommon->removeCellSpan(start_row, start_col);
}

QicsSpanList *
QicsTable::cellSpanList(void)
{
    return myTableCommon->cellSpanList();
}

Qics::QicsGridCellClipping
QicsTable::gridCellClipping(void) const
{
    return myTableCommon->gridCellClipping();
}

void
QicsTable::setGridCellClipping(Qics::QicsGridCellClipping c)
{
    myTableCommon->setGridCellClipping(c);
}

bool
QicsTable::drawPartialCells(void) const
{
    return myTableCommon->drawPartialCells();
}

void
QicsTable::setDrawPartialCells(bool b)
{
    myTableCommon->setDrawPartialCells(b);
}

bool
QicsTable::horizontalGridLinesVisible(void) const
{
    return myTableCommon->horizontalGridLinesVisible();
}

void
QicsTable::setHorizontalGridLinesVisible(bool b)
{
    myTableCommon->setHorizontalGridLinesVisible(b);
}

bool
QicsTable::verticalGridLinesVisible(void) const
{
    return myTableCommon->verticalGridLinesVisible();
}

void
QicsTable::setVerticalGridLinesVisible(bool b)
{
    myTableCommon->setVerticalGridLinesVisible(b);
}

int
QicsTable::horizontalGridLineWidth(void) const
{
    return myTableCommon->horizontalGridLineWidth();
}

void
QicsTable::setHorizontalGridLineWidth(int w)
{
    myTableCommon->setHorizontalGridLineWidth(w);
}

int
QicsTable::verticalGridLineWidth(void) const
{
    return myTableCommon->verticalGridLineWidth();
}

void
QicsTable::setVerticalGridLineWidth(int w)
{
    myTableCommon->setVerticalGridLineWidth(w);
}

Qics::QicsLineStyle
QicsTable::horizontalGridLineStyle(void) const
{
    return myTableCommon->horizontalGridLineStyle();
}

void
QicsTable::setHorizontalGridLineStyle(Qics::QicsLineStyle style)
{
    myTableCommon->setHorizontalGridLineStyle(style);
}

Qics::QicsLineStyle
QicsTable::verticalGridLineStyle(void) const
{
    return myTableCommon->verticalGridLineStyle();
}

void
QicsTable::setVerticalGridLineStyle(Qics::QicsLineStyle style)
{
    myTableCommon->setVerticalGridLineStyle(style);
}

QPen
QicsTable::horizontalGridLinePen(void) const
{
    return myTableCommon->horizontalGridLinePen();
}

void
QicsTable::setHorizontalGridLinePen(const QPen &pen)
{
    myTableCommon->setHorizontalGridLinePen(pen);
}

QPen
QicsTable::verticalGridLinePen(void) const
{
    return myTableCommon->verticalGridLinePen();
}

void
QicsTable::setVerticalGridLinePen(const QPen &pen)
{
    myTableCommon->setVerticalGridLinePen(pen);
}

Qics::QicsCellOverflowBehavior
QicsTable::cellOverflowBehavior(void) const
{
    return myTableCommon->cellOverflowBehavior();
}

void
QicsTable::setCellOverflowBehavior(Qics::QicsCellOverflowBehavior b)
{
    myTableCommon->setCellOverflowBehavior(b);
}

int
QicsTable::maxOverflowCells(void) const
{
    return myTableCommon->maxOverflowCells();
}

void
QicsTable::setMaxOverflowCells(int num)
{
    myTableCommon->setMaxOverflowCells(num);
}

int
QicsTable::frameLineWidth(void) const
{
    return myTableCommon->frameLineWidth();
}

void
QicsTable::setFrameLineWidth(int lw)
{
    myTableCommon->setFrameLineWidth(lw);
}

int
QicsTable::frameStyle(void) const
{
    return myTableCommon->frameStyle();
}

void
QicsTable::setFrameStyle(int style)
{
    myTableCommon->setFrameStyle(style);
}

Qics::QicsCurrentCellStyle
QicsTable::currentCellStyle(void) const
{
    return myTableCommon->currentCellStyle();
}

void
QicsTable::setCurrentCellStyle(QicsCurrentCellStyle s)
{
    myTableCommon->setCurrentCellStyle(s);
}

int
QicsTable::currentCellBorderWidth(void) const
{
    return myTableCommon->currentCellBorderWidth();
}

void
QicsTable::setCurrentCellBorderWidth(int w)
{
    myTableCommon->setCurrentCellBorderWidth(w);
}

bool
QicsTable::clickToEdit(void) const
{
    return myTableCommon->clickToEdit();
}

void
QicsTable::setClickToEdit(bool b)
{
    myTableCommon->setClickToEdit(b);
}

bool
QicsTable::autoSelectCellContents(void) const
{
    return myTableCommon->autoSelectCellContents();
}

void
QicsTable::setAutoSelectCellContents(bool b)
{
    myTableCommon->setAutoSelectCellContents(b);
}

Qt::Orientation
QicsTable::enterTraversalDirection(void) const
{
    return myTableCommon->enterTraversalDirection();
}

void
QicsTable::setEnterTraversalDirection(Qt::Orientation dir)
{
    myTableCommon->setEnterTraversalDirection(dir);
}

Qt::Orientation
QicsTable::tabTraversalDirection(void) const
{
    return myTableCommon->tabTraversalDirection();
}

void
QicsTable::setTabTraversalDirection(Qt::Orientation dir)
{
    myTableCommon->setTabTraversalDirection(dir);
}

QPixmap
QicsTable::moreTextPixmap(void) const
{
    return myTableCommon->moreTextPixmap();
}

void
QicsTable::setMoreTextPixmap(const QPixmap &pix)
{
    myTableCommon->setMoreTextPixmap(pix);
}

bool
QicsTable::dragEnabled(void) const
{
    return myTableCommon->dragEnabled();
}

void
QicsTable::setDragEnabled(bool b)
{
    myTableCommon->setDragEnabled(b);
}

///////////////////
///////////////////

int 
QicsTable::topRow(void) const
{
    return (MAIN_GRID->topRow());
}

int 
QicsTable::bottomRow(void) const
{
    return (MAIN_GRID->bottomRow());
}

int 
QicsTable::leftColumn(void) const
{
    return (MAIN_GRID->leftColumn());
}

int 
QicsTable::rightColumn(void) const
{
    return (MAIN_GRID->rightColumn());
}

QicsCell *
QicsTable::currentCell(void)
{
    QicsICell grid_cell = gridInfo().currentCell();

    return (new QicsCell(grid_cell.row(),
			 grid_cell.column(),
			 &gridInfo()));
}

const QicsCell *
QicsTable::currentCell(void) const
{
    QicsICell grid_cell = gridInfo().currentCell();

    return (new QicsCell(grid_cell.row(),
			 grid_cell.column(),
			 &gridInfo()));
}

void
QicsTable::setCurrentCell(int row, int col)
{
    MAIN_GRID->traverseToCell(row, col);
}

bool 
QicsTable::editCurrentCell(void)
{
    return (MAIN_GRID->editCurrentCell());
}

void
QicsTable::uneditCurrentCell(void)
{
    MAIN_GRID->uneditCurrentCell();
}

bool 
QicsTable::editCell(int row, int col)
{
    return (MAIN_GRID->editCell(row, col));
}

Qics::QicsRepaintBehavior
QicsTable::repaintBehavior(void) const
{
    return (gridInfo().gridRepaintBehavior());
}

void
QicsTable::setRepaintBehavior(QicsRepaintBehavior r)
{
    gridInfo().setGridRepaintBehavior(r);
    rhGridInfo().setGridRepaintBehavior(r);
    chGridInfo().setGridRepaintBehavior(r);
}

Qics::QicsScrollBarMode
QicsTable::hScrollBarMode(void) const
{
    return myColumnScroller->mode();
}

Qics::QicsScrollBarMode
QicsTable::vScrollBarMode(void) const
{
    return myRowScroller->mode();
}

void
QicsTable::setHScrollBarMode(QicsScrollBarMode m)
{
    myColumnScroller->setMode(m);
}

void
QicsTable::setVScrollBarMode(QicsScrollBarMode m)
{
    myRowScroller->setMode(m);
}

void
QicsTable::sortRows(int colnum, Qics::QicsSortOrder order,
		    int from, int to,
		    DataItemComparator func)
{
    myTableCommon->sortRows(colnum, order, from, to, func);
}

void
QicsTable::sortColumns(int rownum, Qics::QicsSortOrder order,
		       int from, int to,
		       DataItemComparator func)
{
    myTableCommon->sortColumns(rownum, order, from, to, func);
}

void
QicsTable::moveRows(int target_row, const QMemArray<int> &rows)
{
    myTableCommon->moveRows(target_row, rows);
}

void
QicsTable::moveColumns(int target_col, const QMemArray<int> &cols)
{
    myTableCommon->moveColumns(target_col, cols);
}

bool
QicsTable::rowHeaderUsesModel(void) const
{
    return myTableCommon->rowHeaderUsesModel();
}

void 
QicsTable::setRowHeaderUsesModel(bool b) 
{
    myTableCommon->setRowHeaderUsesModel(b);
    configureFrozen();
}

bool
QicsTable::columnHeaderUsesModel(void) const
{
    return myTableCommon->columnHeaderUsesModel();
}

void 
QicsTable::setColumnHeaderUsesModel(bool b) 
{ 
    myTableCommon->setColumnHeaderUsesModel(b);
    configureFrozen();
}

////////////////////////////////////////////////////////////////////
///////////               Selection methods           //////////////
////////////////////////////////////////////////////////////////////

QicsSelectionList *
QicsTable::selectionList(void) const
{
    return mySelectionManager->selectionList();
}

QicsSelectionList *
QicsTable::selectionActionList(void) const
{
    return mySelectionManager->selectionActionList();
}

void
QicsTable::setSelectionList(QicsSelectionList &sel_list)
{
    mySelectionManager->setSelectionList(sel_list);
}

void
QicsTable::clearSelectionList(void)
{
    mySelectionManager->clearSelectionList();
}

void
QicsTable::addSelection(QicsSelection &selection)
{
    mySelectionManager->addSelection(selection);
}

Qics::QicsSelectionPolicy
QicsTable::selectionPolicy(void) const
{
    return mySelectionManager->selectionPolicy();
}

void
QicsTable::setSelectionPolicy(QicsSelectionPolicy policy)
{
    mySelectionManager->setSelectionPolicy(policy);
}

////////////////////////////////////////////////////////////////////////

#define DISPLAY_OPT(b) ((b) ? DisplayAlways : DisplayNever)

bool
QicsTable::topHeaderVisible(void) const
{
    return (myTableCommon->topHeaderVisible() != DisplayNever);
}

void 
QicsTable::setTopHeaderVisible(bool b)
{
    myTableCommon->setTopHeaderVisible(DISPLAY_OPT(b));
    configureFrozen();
}

bool
QicsTable::bottomHeaderVisible(void) const
{
    return (myTableCommon->bottomHeaderVisible() != DisplayNever);
}

void 
QicsTable::setBottomHeaderVisible(bool b)
{
    myTableCommon->setBottomHeaderVisible(DISPLAY_OPT(b));
    configureFrozen();
}

bool
QicsTable::leftHeaderVisible(void) const
{
    return (myTableCommon->leftHeaderVisible() != DisplayNever);
}

void 
QicsTable::setLeftHeaderVisible(bool b)
{
    myTableCommon->setLeftHeaderVisible(DISPLAY_OPT(b));
    configureFrozen();
}

bool
QicsTable::rightHeaderVisible(void) const
{
    return (myTableCommon->rightHeaderVisible() != DisplayNever);
}

void 
QicsTable::setRightHeaderVisible(bool b)
{
    myTableCommon->setRightHeaderVisible(DISPLAY_OPT(b));
    configureFrozen();
}

int
QicsTable::tableMargin(void) const
{
    return (myTableCommon->tableMargin());
}

int
QicsTable::tableSpacing(void) const
{
    return (myTableCommon->tableSpacing());
}

int
QicsTable::gridSpacing(void) const
{
    return (myTableCommon->gridSpacing());
}

void
QicsTable::setTableMargin(int margin)
{
    myTableCommon->setTableMargin(margin);
    myMasterGrid->setMargin(margin);
}

void
QicsTable::setTableSpacing(int spacing)
{
    myTableCommon->setTableSpacing(spacing);
    myMasterGrid->setSpacing(spacing);
}

void
QicsTable::setGridSpacing(int spacing)
{
    myTableCommon->setGridSpacing(spacing);
    myGridLayout->setSpacing(spacing);
}


void
QicsTable::scroll(QicsScrollDirection dir, int num)
{

    switch (dir)
    {
    case ScrollUp:
        myScrollManager->setRowIndex(myScrollManager->rowIndex() - num);
        break;

    case ScrollDown:
        myScrollManager->setRowIndex(myScrollManager->rowIndex() + num);
        break;

    case ScrollLeft:
        myScrollManager->setColumnIndex(myScrollManager->columnIndex() - num);
        break;

    case ScrollRight:
        myScrollManager->setColumnIndex(myScrollManager->columnIndex() + num);
        break;
    default:
	break;
    }
}

void 
QicsTable::freezeTopRows(int num_rows)
{
    if (num_rows <= 0)
    {
	unfreezeTopRows();
	return;
    }

    myNumTopFrozenRows = num_rows;

    QicsRegion vp = MAIN_GRID->viewport();
    vp.setStartRow(vp.startRow() + num_rows);

    setMainGridViewport(vp);
    configureFrozen();
}

void 
QicsTable::freezeBottomRows(int num_rows)
{
    if (num_rows <= 0)
    {
	unfreezeBottomRows();
	return;
    }

    myNumBottomFrozenRows = num_rows;

    QicsRegion vp = MAIN_GRID->currentViewport();
    vp.setEndRow(vp.endRow() - num_rows);

    setMainGridViewport(vp);
    configureFrozen();
}

void 
QicsTable::freezeLeftColumns(int num_cols)
{
    if (num_cols <= 0)
    {
	unfreezeLeftColumns();
	return;
    }

    myNumLeftFrozenColumns = num_cols;

    QicsRegion vp = MAIN_GRID->viewport();
    vp.setStartColumn(vp.startColumn() + num_cols);

    setMainGridViewport(vp);
    configureFrozen();
}

void 
QicsTable::freezeRightColumns(int num_cols)
{
    if (num_cols <= 0)
    {
	unfreezeRightColumns();
	return;
    }

    myNumRightFrozenColumns = num_cols;

    QicsRegion vp = MAIN_GRID->currentViewport();
    vp.setEndColumn(vp.endColumn() - num_cols);

    setMainGridViewport(vp);
    configureFrozen();
}

void 
QicsTable::unfreezeTopRows(void)
{
    myNumTopFrozenRows = 0;
    configureFrozen();
    setViewport(viewport());
}

void 
QicsTable::unfreezeBottomRows(void)
{
    myNumBottomFrozenRows = 0;
    configureFrozen();
    setViewport(viewport());
}

void 
QicsTable::unfreezeLeftColumns(void)
{
    myNumLeftFrozenColumns = 0;
    configureFrozen();
    setViewport(viewport());
}

void 
QicsTable::unfreezeRightColumns(void)
{
    myNumRightFrozenColumns = 0;
    configureFrozen();
    setViewport(viewport());
}

void
QicsTable::revertRepaintBehavior(void)
{
    gridInfo().revertGridRepaintBehavior();
    rhGridInfo().revertGridRepaintBehavior();
    chGridInfo().revertGridRepaintBehavior();
}

void
QicsTable::handleHeaderDoubleClick(int idx, int button, QicsHeaderType type)
{
    if (button == LeftButton)
    {
	if (type == RowHeader)
	    toggleRowHeightExpansion(idx);
	else
	    toggleColumnWidthExpansion(idx);
    }
}

void
QicsTable::addRows(int how_many)
{
    myTableCommon->addRows(how_many);
}
void
QicsTable::insertRow(int row)
{
    myTableCommon->insertRow(row);
}
void
QicsTable::deleteRow(int row)
{
    myTableCommon->deleteRow(row);
}

void
QicsTable::addColumns(int how_many)
{
    myTableCommon->addColumns(how_many);
}
void
QicsTable::insertColumn(int col)
{
    myTableCommon->insertColumn(col);
}
void
QicsTable::deleteColumn(int col)
{
    myTableCommon->deleteColumn(col);
}



////////////////////////////////////////////////////////////////////////

void
QicsTable::cut(void)
{
    QApplication::clipboard()->setData(gridInfo().cutCopyData());

    gridInfo().finishCut(true);
}

void
QicsTable::copy(void)
{
    QApplication::clipboard()->setData(gridInfo().cutCopyData());

    gridInfo().finishCut(false);
}

void
QicsTable::paste(void)
{
    QicsICell cur_cell = gridInfo().currentCell();

    if (cur_cell.isValid())
	gridInfo().paste(QApplication::clipboard()->data(), cur_cell);
}

////////////////////////////////////////////////////////////////////////

void
QicsTable::handleGridPress(int row, int col, int button, const QPoint &pos)
{
    QicsScreenGrid *grid = 0;

    for (int i = 0; i <= GRID_BOTTOM_IDX; ++i)
    {
	for (int j = 0; j <= GRID_RIGHT_IDX; ++j)
	{
	    if (myGrids[i][j] == sender())
	    {
		grid = myGrids[i][j];
		break;
	    }
	}
    }

    if (grid)
    {
	emit pressed(row, col, button, grid->mapToParent(pos));
    }
}

void
QicsTable::handleGridClick(int row, int col, int button, const QPoint &pos)
{
    QicsScreenGrid *grid = 0;

    for (int i = 0; i <= GRID_BOTTOM_IDX; ++i)
    {
	for (int j = 0; j <= GRID_RIGHT_IDX; ++j)
	{
	    if (myGrids[i][j] == sender())
	    {
		grid = myGrids[i][j];
		break;
	    }
	}
    }

    if (grid)
    {
	emit clicked(row, col, button, grid->mapToParent(pos));
    }
}

void
QicsTable::handleGridDoubleClick(int row, int col, int button,
				 const QPoint &pos)
{
    QicsScreenGrid *grid = 0;

    for (int i = 0; i <= GRID_BOTTOM_IDX; ++i)
    {
	for (int j = 0; j <= GRID_RIGHT_IDX; ++j)
	{
	    if (myGrids[i][j] == sender())
	    {
		grid = myGrids[i][j];
		break;
	    }
	}
    }

    if (grid)
    {
	emit doubleClicked(row, col, button, grid->mapToParent(pos));
    }
}

////////////////////////////////////////////////////////////////////////
//////////////////     Traversal Methods     ///////////////////////////
////////////////////////////////////////////////////////////////////////

bool
QicsTable::traverseToCell(int row, int col)
{
    return (MAIN_GRID->traverseToCell(row, col));
}

void
QicsTable::traverseToBeginningOfTable(void)
{
    MAIN_GRID->traverseToBeginningOfTable();
}

void
QicsTable::traverseToEndOfTable(void)
{
    MAIN_GRID->traverseToEndOfTable();
}

void
QicsTable::traverseLeft(void)
{
    MAIN_GRID->traverseLeft();
}

void
QicsTable::traverseRight(void)
{
    MAIN_GRID->traverseRight();
}

void
QicsTable::traverseUp(void)
{
    MAIN_GRID->traverseUp();
}

void
QicsTable::traverseDown(void)
{
    MAIN_GRID->traverseDown();
}

void
QicsTable::traverseToBeginningOfRow(void)
{
    MAIN_GRID->traverseToBeginningOfRow();
}

void
QicsTable::traverseToEndOfRow(void)
{
    MAIN_GRID->traverseToEndOfRow();
}
