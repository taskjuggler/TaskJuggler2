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


#include <QicsSpanManager.h>
#include <QicsGridInfo.h>


QicsSpanManager::QicsSpanManager(void) :
    QObject()
{
}

QicsSpanManager::QicsSpanManager(const QicsSpanManager &sm) :
    QObject()
{
    QicsSpanList::const_iterator iter;

    for (iter = sm.myCellSpanList.begin();
	 iter != sm.myCellSpanList.end();
	 ++iter)
    {
	myCellSpanList.push_back(*iter);
    }
}

QicsSpanManager::~QicsSpanManager(void)
{
}


/*
 * Remember, the region is model coordinate based, not visual.
 * This means it is not really a region from start to end, but
 * a different beast, of start + length
 */
bool
QicsSpanManager::addCellSpan(QicsSpan span)
{
    QicsSpanList::iterator iter;

    for (iter = myCellSpanList.begin(); iter != myCellSpanList.end(); ++iter)
    {
	if (span.intersects(*iter))
	    return false;
    }

    myCellSpanList.push_back(span);

    emit spanChanged(span);

    return true;
}

void
QicsSpanManager::removeCellSpan(int start_row, int start_col)
{
    QicsSpanList::iterator iter;

    for (iter = myCellSpanList.begin(); iter != myCellSpanList.end(); ++iter)
    {
	QicsSpan r = *iter;

	if ((r.row() == start_row) && (r.column() == start_col))
	{
	    myCellSpanList.erase(iter);
	    emit spanChanged(r);
	    return;
	}
    }
}

QicsSpanList *
QicsSpanManager::cellSpanList(void) const
{
	// TODO - map this back to visual coordinates
    return new QicsSpanList(myCellSpanList);
}

bool
QicsSpanManager::isSpanner(QicsGridInfo &gi, int _row, int _col,
			   QicsRegion &reg_return) const
{
    QicsSpanList::const_iterator iter;

    int row = gi.modelRowIndex(_row);
    int col = gi.modelColumnIndex(_col);

    for (iter = myCellSpanList.begin(); iter != myCellSpanList.end(); ++iter)
    {
	const QicsSpan &r = *iter;

	if ((r.row() == row) && (r.column() == col))
	{
	    // Return the span in VISUAL coordinates

		// TODO - make this return a Span
	    reg_return = QicsRegion(_row, _col,
				    _row + r.height()-1, _col + r.width() - 1);
	    return true;
	}
    }

    return false;
}

/* The conversions between visual and model here are subtle to get
 * the right visual effect.   One choice would be to map the incoming
 * coords to the model, and see that intersects any cell.  This mostly
 * works, but the problem is that it's wrong.   Let's say our column
 * contains rows with the numbers 0 to 1000.   We have a span on cell
 * in row 6 for 3 rows (6, 7 and 8).  Sort it lexically, so row 6 is
 * now followed by row 60 and then 600.  model[6] appears at visual[556]
 * so, visual[557] is in the span.   It is obvious when you put this
 * all together
 * 	visual[667] > model[7] is in the span model[6 for 3]  -> WRONG
 * 	visual[557] > model[60] is not in the span model[6 for 3] - > WRONG
 *
 * The proper way to compute this is convert the model coordinates
 * in the spans back to visual and see if the incoming visual row
 * is in the new span
 * 	model[6 for 3] -> visual[556 for 3], visual 557 is in span -> RIGHT
 *
 * You would not believe the number of times I forget this and kept
 * flipping between the two methods.   Learn from my pain
 * - tony
 */

bool
QicsSpanManager::isSpanned(QicsGridInfo &gi, int row, int col,
			   QicsRegion &reg_return) const
{
    QicsSpanList::const_iterator iter;
    for (iter = myCellSpanList.begin(); iter != myCellSpanList.end(); ++iter)
    {
	const QicsSpan &r = *iter;

	int top = gi.visualRowIndex(r.row());
	int left = gi.visualColumnIndex(r.column());
	// qDebug("span model[%d,%d] -> vis:[%d,%d]\n", r.row(), r.col(), top, left);

	if(!((row == top) && (col == left))) {
		// It is not the same cell
		
		if( (top <= row) && (row < top+r.height()) &&
		    (left <= col) && (col < left+r.width())) {
#ifdef notdef
			qDebug("v[%d,%d] is spanned by m[%d, %d]+%d,%d\n",
					row, col, r.row(), r.col(),
					r.height(), r.width());
#endif
			// Return the span in VISUAL coordinates

			// reg_return = r;
			// TODO - make this return a Span
			reg_return = QicsRegion(top, left,
					top + r.height()-1, left + r.width() - 1);
			return true;
		}
	}
    }

    return false;
}

QicsRegion
QicsSpanManager::maxSpanForRow(QicsGridInfo &gi, int row) const
{
    // Note: input and return in visual units
    QicsRegion retval(row, 0, row, Qics::QicsLAST_COLUMN);
    int	height = 1;

    int modelrow = gi.modelRowIndex(row);
    QicsSpanList::const_iterator iter;
    for (iter = myCellSpanList.begin(); iter != myCellSpanList.end(); ++iter)
    {
	const QicsSpan &r = *iter;

	if (r.row() == modelrow)
	{
	    if (height < r.height()) height = r.height();
	}
    }

    retval.setEndRow(row + height - 1);
    return retval;
}

QicsRegion
QicsSpanManager::maxSpanForColumn(QicsGridInfo &gi, int col) const
{
    // Note: input and return in visual units
    QicsRegion retval(0, col, Qics::QicsLAST_ROW, col);
    int	width = 1;

    int modelcolumn = gi.modelColumnIndex(col);
    QicsSpanList::const_iterator iter;
    for (iter = myCellSpanList.begin(); iter != myCellSpanList.end(); ++iter)
    {
	const QicsSpan &r = *iter;

	if (r.column() == modelcolumn)
	{
	    if (width < r.width()) width = r.width();
	}
    }

    retval.setEndColumn(col + width - 1);
    return retval;
}

bool QicsSpan::intersects(QicsSpan &span) const
{
	/* compute interection.  Consider the 9 regions
	 * 	A B C
	 * 	D E F
	 * 	G H I
	 * where this span is in the center at E.
	 * We just make sure the other span does not lie in
	 * an of the other 8 regions.
	 */
	int myEndrow = this->_row + this->_nrows - 1;
	int otherEndrow = span._row + span._nrows  - 1;
	// Eliminate G H I
	if(span._row > myEndrow) return false;
	// Eliminate A B C
	if(this->_row > otherEndrow) return false;

	int myEndcol = this->_col + this->_ncols - 1;
	int otherEndcol = span._col + span._ncols  - 1;
	// Eliminate C F I
	if(span._col > myEndcol) return false;
	// Eliminate A D G
	if(this->_col > otherEndcol) return false;

	// the other guy lies somewhere in E, so it hits me
	return true;
}
