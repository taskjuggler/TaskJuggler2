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


#include <QicsRowColumnDrag.h>
#include <QicsGridInfo.h>
#include <QicsSelectionManager.h>
#include <QicsICell.h>

QicsRowColumnDrag::QicsRowColumnDrag(QicsGridInfo *gi, Qics::QicsIndexType _type,
				     QWidget *_dragSource, const char *_name) :
    QDragObject(_dragSource, _name),
    myGridInfo(gi),
    myIndexType(_type)
{
}


QicsRowColumnDrag::~QicsRowColumnDrag()
{
}

/*!
 * the foundry which looks at the grid info and the selection
 * data and squirrels away the selected row or column indices
 */

QicsRowColumnDrag *
QicsRowColumnDrag::getDragObject(QicsGridInfo *gi,
				 Qics::QicsIndexType type,
				 QicsICell *dragCell,
				 QWidget *dragSource)
{
    QicsRowColumnDrag *ret = 
	    new QicsRowColumnDrag(gi, type, dragSource, 0);

    QicsSelectionList *slist = gi->selectionManager()->selectionList();

    // qDebug("Dragging from %d,%d\n", dragCell->row(), dragCell->column());

    if(slist == 0) {
        // qDebug("slist is 0\n");
	if(dragCell == 0) return 0;

	// We have no selection, but a start cell, drag from here.

	ret->items.resize(1);
	ret->items[0] = (type == RowIndex ? dragCell->row() :
			dragCell->column());
	return ret;
    }

    QicsSelectionList::iterator iter;
    int	nitems = 0;
    bool okdrag = false;

    for (iter = slist->begin(); iter != slist->end(); ++iter) {
	QicsSelection &sel = *iter;

	int row = sel.topRow();
	int col = sel.leftColumn();
	int erow = sel.bottomRow();
	int ecol = sel.rightColumn();
	QicsRegion reg(row, col, erow, ecol);

	if(type == RowIndex) {
	    // skip things that are not row selections
	    if(!(col == 0) && (ecol == Qics::QicsLAST_COLUMN)) continue;
	    if (dragCell != 0 && reg.contains(*dragCell)) {
		okdrag = true;
	    }

	    for( ; row <= erow; row++) {
	    	ret->items.resize(nitems+1);
		ret->items[nitems++] = gi->modelRowIndex(row);
	    }

	} else {
	    // skip things that are not column selections
	    if(!(row == 0) && (erow == Qics::QicsLAST_ROW)) continue;

	    if (dragCell != 0 && reg.contains(*dragCell)) {
		okdrag = true;
	    }

	    for( ; col <= ecol; col++) {
	    	ret->items.resize(nitems+1);
		ret->items[nitems++] = gi->modelColumnIndex(col);
	    }
	}
    }

    if(nitems == 0) {
	    delete ret;
	    ret = 0;
    }
    return ret;
}


    /* required QMimeSource methods */
/*
 * \reimp
 */

const char *QicsRowColumnDrag::format(int i) const
{
    if ( i == 0 ) {
	if(myIndexType == RowIndex) {
	    return QICS_MIME_ROWLIST;
	} else if(myIndexType == ColumnIndex) {
	    return QICS_MIME_COLUMNLIST;
	}
    } else if ( i == 1 ) {
        return QICS_MIME_CELLDATA;
    } else if ( i == 2 ) {
        return "text/plain";
    }
    return 0;
}

/*!
    Returns \b true if \a e can be decoded by the QicsRowColumnDrag, otherwise
    return \b false.
*/

bool QicsRowColumnDrag::canDecode(const QMimeSource* e, QicsIndexType kind )
{
    if( kind == RowIndex ) {
	return e->provides(QICS_MIME_ROWLIST);
    } else if( kind == ColumnIndex ) {
	return e->provides(QICS_MIME_COLUMNLIST);
    }
    if(e->provides(QICS_MIME_ROWLIST) ||
       e->provides(QICS_MIME_COLUMNLIST) ||
       e->provides(QICS_MIME_CELLDATA) ) {
	return true;
    }
    return false;
}




QByteArray
QicsRowColumnDrag::encodedData(const char *mimetype) const
{
    QByteArray retval;
    QDataStream ds(retval, IO_WriteOnly);

    if(strcmp(mimetype, "text/plain")==0) {
	// this may happen when dropped on another application
	
	ds << "drop data as text ";
	ds << ((myIndexType == RowIndex) ? "rows" : "columns");

	for(unsigned int i = 0; i < items.count(); i++)
	    ds << items.at(i);
	ds << "\n";

	return retval;
    }
    if(strcmp(mimetype, QICS_MIME_ROWLIST)==0 ||
       strcmp(mimetype, QICS_MIME_COLUMNLIST)==0)
    {
	ds << items.count();

	for(unsigned int i = 0; i < items.count(); i++)
	    ds << items.at(i);

	return retval;
    }

    qDebug("TODO: must implement QicsRowColumnDrag::encodeData(%s)\n", mimetype);
    return retval;
}


/*!
 * Decodes the data which is stored (encoded) in \a e and, if
 *  successful, fills TODO with the selected data.
 *
 * \return \b true if there was some data, \b false otherwise.
 */

static void decodeArray(QByteArray &in, QMemArray<int> &out)
{
    QDataStream ds(in, IO_ReadOnly);
    int	nitems;

    // get the number of items
    ds >> nitems;

    out.resize(nitems);

    for (int i = 0; i < nitems; ++i)
	ds >> out[i];
}

bool
QicsRowColumnDrag::decode(const QMimeSource *mimesource,
			  QicsIndexType type,
			  QMemArray<int> &results)
{
    if (type == RowIndex && mimesource->provides(QICS_MIME_ROWLIST))
    {
	QByteArray ba = mimesource->encodedData( QICS_MIME_ROWLIST );

	if (ba.size() > 0)
	{
	    decodeArray(ba, results);
	    return true;
	}
    }
    else if (type == ColumnIndex && mimesource->provides(QICS_MIME_COLUMNLIST))
    {
	QByteArray ba = mimesource->encodedData( QICS_MIME_COLUMNLIST );

	if (ba.size() > 0)
	{
	    decodeArray(ba, results);
	    return true;
	}
    }

    return false;
}

bool
QicsRowColumnDrag::decodeCellData(const QMimeSource *)
{
#if TODO
    QByteArray ba = e->encodedData( QICS_MIME_CELLDATA );
    if ( ba.size() != 0) {
	qDebug("TODO: must implement QicsRowColumnDrag::decode for celldata\n");
	// return true;
    }
#endif

    return false;
}

