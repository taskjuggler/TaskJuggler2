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


#ifndef _QICSSPAN_H
#define _QICSSPAN_H

#include <qvaluevector.h>





class QicsSpan
{
  public:
    
    QicsSpan() :
	_row(-1), _col(-1), _nrows(0), _ncols(0) {}
    
    QicsSpan(int begin_row, int begin_col, int nrows, int ncols)
        : 
	_row(begin_row), _col(begin_col), _nrows(nrows), _ncols(ncols) {}

    
    inline int row(void) const     { return _row; }
    
    inline int column(void) const  { return _col; }
    // Kept in for backward compatibility reasons
    inline int col(void) const  { return column(); }
    
    inline int height(void) const  { return _nrows; }
    
    inline int width(void) const  { return _ncols; }

    
    inline bool isValid(void) const
	{ return ((_row < 0) || (_col < 0) || (_nrows <= 0) || (_ncols <= 0)); }

    
    inline bool containsCell(int row, int col) const
	{
	    return (isValid() && (_row <= row) && (row < _row + _nrows) &&
		    (_col <= col) && (col < _col + _ncols));
	}

    
    bool intersects(QicsSpan &s) const;

private:
    int _row, _col, _nrows, _ncols;

    friend class QicsSpanManager;
};


typedef QValueVector<QicsSpan> QicsSpanList;

#endif /*_FILE_H --- Do not add anything past this line */
 
