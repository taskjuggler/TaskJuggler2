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


#ifndef _QicsGrid_H
#define _QicsGrid_H

#include <qframe.h>
#include <qpainter.h>
#include <qptrlist.h>

#include <QicsGridInfo.h>
#include <QicsRegion.h>
#include <QicsCellStyle.h>
#include <QicsGridStyle.h>
#include <QicsCellDisplay.h>
#include <QicsICell.h>


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#include <qvaluevector.h>

class QicsPositionList
{
public:
    typedef const int *iterator;

    QicsPositionList()  { setFirstIndex(0); init(); }
    QicsPositionList(int base)  { setFirstIndex(base); init(); }
    
    inline int firstIndex(void) const  { return myBase; }
    inline void setFirstIndex(int base)  { myBase = base; }

    inline int lastIndex(void) const { return myBase + size() - 1; }

    inline void push_back(const int &val)
	{ myPositions.push_back(val); }

    inline int at(int idx) const
	{ return (myPositions.at(idx - myBase)); }

    inline int operator[](int idx) const { return at(idx); }

    inline void clear(void) { myPositions.clear(); }

    inline int size(void) const { return myPositions.size(); }

    inline iterator begin(void) const { return myPositions.begin(); }
    inline iterator end(void) const { return myPositions.end(); }
    
protected:
    void init(void) { myPositions.reserve(50); }
    QValueVector<int> myPositions;
    int myBase; 
};

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



class QicsGrid: public Qics {

public:
    
    QicsGrid(QicsGridInfo &info);
    
    virtual ~QicsGrid();

    
    inline QicsGridInfo &gridInfo(void) const { return myInfo; }

    
    virtual void setViewport(const QicsRegion &reg);

    
    virtual QicsRegion viewport(void) const;

    
    bool isCellValid(int row, int col) const;

    
    QRect cellDimensions(int row, int col, bool with_spans) const;

    
    QRect cellDimensions(const QicsICell &cell, bool with_spans) const
	{ return (cellDimensions(cell.row(), cell.column(), with_spans)); }

    
    virtual QicsRegion currentViewport(void) const;

    
    virtual bool requestCellOverflow(const QicsRegion &cur_area,
				     const QRect &cur_rect,
				     QicsRegion &new_area,
				     QRect &new_rect);

    
    virtual void acceptCellOverflow(QicsRegion &area);

    
    int modelColumnIndex(int column) const;

    
    int modelRowIndex(int row) const;

    
    virtual void orderRowsBy(int column,
			     QicsSortOrder order = Ascending,
			     DataItemComparator func = 0);

    
    virtual void orderColumnsBy(int row,
				QicsSortOrder order = Ascending,
				DataItemComparator func = 0);

    
    void dumpPositions(void) const;

protected:

    
    inline QicsDataModel *dataModel(void) const
	{ return (myInfo.dataModel()); }

    
    inline QicsStyleManager &styleManager(void) const
	{ return (*(myInfo.styleManager())); }

    
    inline QicsDimensionManager &dimensionManager(void) const
	{ return (*(myInfo.dimensionManager())); }

    
    inline QicsMappedDimensionManager &mappedDM(void) const
	{ return (*(myInfo.mappedDM())); }

    
    inline QicsSelectionManager &selectionManager(void) const
	{ return (*(myInfo.selectionManager())); }


    
    virtual void drawRegion(const QicsRegion &region, QPainter *painter);

    
    virtual void drawGridLines(const QicsRegion &reg, QPainter *painter);

    
    virtual void drawCell(int row, int col, int x, int y,
			  bool look_for_overflower, QPainter *painter);

    
    virtual bool prepareToDraw(int row, int col, const QRect &rect,
			       QPainter *painter);
    
    
    virtual const QicsDataItem *cellValue(int row, int col) const;

    
    QicsICell computeCellPositions(const QRect &bounds,
				   const QicsICell &start);

    
    QicsCellDisplay *cellDisplay(int row, int col) const;

    //////////////// Data Members /////////////////

    
    QicsGridInfo &myInfo;
        
    
    QicsRegion myViewport;

    QicsPositionList myRowPositions;
    
    QicsPositionList myColumnPositions;

    
    QicsICellQVL myAlreadyDrawnCells;

    
    QicsRegionV myOverflows;

    
    QicsMainGrid *myMainGrid;

    
    QicsCell *myCell;
};

#endif /* _QicsGrid_H --- Do not add anything past this line */
