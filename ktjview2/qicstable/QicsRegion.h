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


#ifndef _QicsRegion_H
#define _QicsRegion_H

#include <qrect.h>
#include <qvaluevector.h>

#include <QicsICell.h>




class QicsRegion : public QRect
{
  public:
    
    QicsRegion(int begin_row, int begin_col, int end_row, int end_col)
        : QRect(begin_col, begin_row, end_col-begin_col+1, end_row-begin_row+1) {}

    
    QicsRegion(const QicsICell &begin_cell, const QicsICell &end_cell)
        : QRect(begin_cell.column(), begin_cell.row(),
		end_cell.column() - begin_cell.column() + 1,
		end_cell.row() - begin_cell.row() + 1) {}

    
    QicsRegion(void) : QRect() {}
  
    
    inline QicsICell startCell(void) const
	{ return QicsICell(startRow(), startColumn()); }

    
    inline QicsICell endCell(void) const
	{ return QicsICell(endRow(), endColumn()); }

    
    inline int startRow(void) const     { return top(); }
    
    inline int endRow(void) const       { return bottom(); }
    
    inline int startColumn(void) const  { return left(); }
    
    inline int endColumn(void) const    { return right(); }
    
    inline int numColumns(void) const   { return width(); }
    
    inline int numRows(void) const      { return height(); }

    
    inline void setStartCell(const QicsICell &cell)
	{ setTop(cell.row()); setLeft(cell.column()); }

    
    inline void setEndCell(const QicsICell &cell)
	{ setBottom(cell.row()); setRight(cell.column()); }

    
    inline void setStartRow(int val)    { setTop(val); }
    
    inline void setEndRow(int val)      { setBottom(val); }
    
    inline void setStartColumn(int val) { setLeft(val); }
    
    inline void setEndColumn(int val)   { setRight(val); }

    
    inline bool containsCell(const QicsICell &cell) const
	{ return contains(cell.column(), cell.row()); }

    
    inline bool containsCell(int row, int col) const
	{ return contains(col, row); }

    inline QicsRegion intersect(const QicsRegion &reg) const
	{
	    QRect r = QRect::intersect(reg);
	    return QicsRegion(r.top(), r.left(), r.bottom(), r.right());
	}
};


typedef QValueVector<QicsRegion> QicsRegionV;

#endif /*_FILE_H --- Do not add anything past this line */
 
