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


#ifndef _QICSGRIDINFO_H
#define _QICSGRIDINFO_H

#include <QicsNamespace.h>
#include <QicsDataItem.h>
#include <QicsICell.h>
#include <QicsSelection.h>
#include <QicsSorter.h>

#include <qobject.h>
#include <qdragobject.h>
#include <qmemarray.h>
#include <qvaluevector.h>

class QicsDataModel;
class QicsStyleManager;
class QicsDimensionManager;
class QicsMappedDimensionManager;
class QicsSelectionManager;
class QicsSorter;
class QicsGrid;
class QicsScreenGrid;




class QicsGridInfo: public QObject, public Qics
{
    Q_OBJECT

public:
    
    typedef QValueVector<QicsScreenGrid *> QicsScreenGridPV;

    
    QicsGridInfo(QicsGridType type);
    
    virtual ~QicsGridInfo();

    
    inline QicsGridType gridType(void) const { return myType; }

    
    QicsDataModel *dataModel(void) const;
    
    void setDataModel(QicsDataModel *dm);

    
    QicsStyleManager *styleManager(void) const;
    
    void setStyleManager(QicsStyleManager *sm);

    
    QicsMappedDimensionManager *mappedDM(void) const;

    
    QicsDimensionManager *dimensionManager(void) const;

    
    void setDimensionManager(QicsDimensionManager *dm);

    
    QicsSelectionManager *selectionManager(void) const;
    
    void setSelectionManager(QicsSelectionManager *sel_m);

    
    inline QicsScreenGridPV grids(void) const { return myGrids; }
    
    void connectGrid(QicsScreenGrid *grid);
    
    void disconnectGrid(QicsScreenGrid *grid);

    
    virtual const QicsDataItem *cellValue(int row, int col) const;

    
    Qics::QicsRepaintBehavior gridRepaintBehavior(void) const;
    
    
    void setGlobalRepaintBehavior(QicsRepaintBehavior behavior);
    
    void revertGlobalRepaintBehavior(void);

    
    void setGridRepaintBehavior(QicsRepaintBehavior behavior);
    
    void revertGridRepaintBehavior(void);

    
    virtual void controlColumnsOf(QicsGridInfo *grid);

    
    virtual void controlRowsOf(QicsGridInfo *grid);

    
    inline int modelColumnIndex(int column) const {
	return myColumnOrdering->visualToModel(column);
    }

    
    inline int modelRowIndex(int row) const {
	return myRowOrdering->visualToModel(row);
    }

    
    inline int visualColumnIndex(int column) const {
	return myColumnOrdering->modelToVisual(column);
    }

    
    inline int visualRowIndex(int row) const {
	return myRowOrdering->modelToVisual(row);
    }

    
    QicsRegion modelRegion(QicsRegion vis_region) const;

    
    QicsRegion visualRegion(QicsRegion model_region) const;

    
    void orderRowsBy(int column,
		     QicsSortOrder order = Qics::Ascending,
		     int from = 0, int to = -1,
		     DataItemComparator func = 0);

    
    void orderColumnsBy(int row,
			QicsSortOrder order = Qics::Ascending,
			int from = 0, int to = -1,
			DataItemComparator func = 0);

    
    const QicsSorter *rowOrdering() const { return myRowOrdering; }
    
    const QicsSorter *columnOrdering() const { return myColumnOrdering; }

    
    virtual QDragObject *cutCopyData(QWidget *drag_widget = 0,
				     QicsICell *ref_cell = 0);

    
    virtual void finishCut(bool removeData);

    
    void paste(QMimeSource *ms, const QicsICell &cell);

    
    void overlay(const QicsDataModel &dm, const QicsICell &start_cell,
		 bool expand_model = false, bool clear_if_empty = true);

    
    QicsICell currentCell(void) const;

    
    virtual void setCurrentCell(const QicsICell &cell);

signals:
    
    void dataModelChanged(QicsDataModel *old_dm, QicsDataModel *new_dm);
    
    void styleManagerChanged(QicsStyleManager *old_sm,
			     QicsStyleManager *new_sm);
    
    void dimensionManagerChanged(QicsDimensionManager *old_dm,
				 QicsDimensionManager *new_dm);
    
    void selectionManagerChanged(QicsSelectionManager *old_sm,
				 QicsSelectionManager *new_sm);

    
    void gridAdded(QicsScreenGrid *grid);

    
    void gridRemoved(QicsScreenGrid *grid);

    
    void globalSetRepaintBehaviorRequest(QicsRepaintBehavior behavior);
    
    void globalRevertRepaintBehaviorRequest(void);

    
    void modelReordered(QicsIndexType);

    
    void currentCellChanged(int new_row, int new_col);

    
    void cellValueChanged(int row, int col);
      
public slots:
    
    void deleteRows(int how_many, int starting_position);

    
    void insertRow(int where);

    
    void moveRows(int target, const QMemArray<int> &rows);

    
    void moveColumns(int target, const QMemArray<int> &cols);

    
    void deleteColumns(int how_many, int starting_position);

    
    void insertColumn(int where);

    
    void redrawAllGrids();

    
    virtual void redrawModel(QicsRegion r);

    
    virtual void setCellValue(int row, int col, const QicsDataItem &itm);

    
    virtual void setCurrentCellValue(const QicsDataItem &itm);

protected slots:
    
    void removeGrid(QObject *obj);

protected:
    
    void connectDTtoSM(void);
    
    void connectDTtoDM(void);

    
    void connectDTtoGrid(QicsScreenGrid *grid);
    
    void connectSMtoGrid(QicsScreenGrid *grid);
    
    void connectDMtoGrid(QicsScreenGrid *grid);
    
    void connectSelMtoGrid(QicsScreenGrid *grid);

    
    virtual QicsSelectionList *cutCopySelection(QicsICell *ref_cell);

        QicsGridType myType;

        QicsDataModel *myDT;
        QicsStyleManager *mySM;
        QicsDimensionManager *myDM;
        QicsMappedDimensionManager *myMappedDM;
            QicsSelectionManager *mySelM;

        QicsScreenGridPV myGrids;

        QicsDataInt myCellValue;

        QicsRepaintBehavior myPrevGridRepaintBehavior;

        QicsSelectionList *myCutCopySelection;

        QicsICell myCurrentCell;

private:
        QicsSorter *myRowOrdering;

        QicsSorter *myColumnOrdering;
         
        QicsGridInfo *myRowOrderOwner;

        QicsGridInfo *myColumnOrderOwner;

};

#endif /* _QICSGRIDINFO_H */
