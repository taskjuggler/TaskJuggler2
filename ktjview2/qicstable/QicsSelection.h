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


#ifndef _QICSSELECTION_H
#define _QICSSELECTION_H

#include <qvaluevector.h>

#ifndef QICS_MIN
# define QICS_MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef QICS_MAX
# define QICS_MAX(a,b) ((a) > (b) ? (a) : (b))
#endif




// Represents a simple, rectangular selection



class QicsSelection
{
public:
    
    QicsSelection();

    
    QicsSelection(int anchor_row, int anchor_col,
		  int end_row, int end_col, bool select_state = true);

    QicsSelection(const QicsSelection &sel);

    
    inline void setSelected(bool set)
	{ mySelectState = set; }

    
    inline bool selected(void) const
	{ return mySelectState; }

    
    bool isValid(void) const;

    
    void setAnchorRow(int anchor_row);
    
    void setAnchorColumn(int anchor_col);
    
    void setAnchorCell(int anchor_row, int anchor_col);

    
    void setEndRow(int end_row);
    
    void setEndColumn(int end_col);
    
    void setEndCell(int end_row, int end_col);

    
    inline int anchorRow(void) const { return myAnchorRow; }
    
    inline int anchorColumn(void) const { return myAnchorCol; }

    
    inline int endRow(void) const { return myEndRow; }
    
    inline int endColumn(void) const { return myEndCol; }

    
    inline int topRow(void) const { return QICS_MIN(myAnchorRow, myEndRow); }
    
    inline int leftColumn(void) const { return QICS_MIN(myAnchorCol, myEndCol); }

    
    inline int bottomRow(void) const { return QICS_MAX(myAnchorRow, myEndRow); }
    
    inline int rightColumn(void) const { return QICS_MAX(myAnchorCol, myEndCol); }

    
    inline int numColumns(void) const { return (rightColumn() - leftColumn() + 1); }

    
    inline int numRows(void) const { return (bottomRow() - topRow() + 1); }

    
    void setEntireRow(int row);

    
    void setEntireColumn(int col);

    
    bool intersects(const QicsSelection &sel);

    
    QicsSelection operator&(const QicsSelection &sel);

    
    QicsSelection intersect(const QicsSelection &sel);

    
    friend bool operator==(const QicsSelection &r1,
                                       const QicsSelection &r2);
    
    friend bool operator!=(const QicsSelection &r1,
				       const QicsSelection &r2);
protected:
        bool mySelectState;

        int myAnchorRow;
        int myAnchorCol;
        int myEndRow;
        int myEndCol;
private:

};

// A complex selection may have more than one area, hence the separate class



class QicsSelectionList : public QValueVector<QicsSelection>
{
public:
    
    QicsSelectionList();

    
    bool isCellSelected(int row, int col) const;

    
    bool isRowSelected(int row) const;

    
    bool isColumnSelected(int col) const;
};

#endif /* _QICSSELECTION_H */
