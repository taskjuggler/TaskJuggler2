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


#include <QicsTableRegionDrag.h>
#include <QicsGridInfo.h>
#include <QicsSelectionManager.h>
#include <QicsICell.h>
#include <QicsDataModelDefault.h>

#include <qdatastream.h>

////////////////////////////////////////////////////////////////////////////////

QicsTableRegionDrag::QicsTableRegionDrag(QicsGridInfo *gi, QicsSelectionList *slist,
					 QWidget *drag_source, const char *name) :
    QDragObject(drag_source, name)
{
    mySelectionList = new QicsSelectionList(*slist);

    // we need to determine the topmost row index and the leftmost
    // column index of the selection, as well as the max size of the selections

    int top = QicsLAST_ROW;
    int left = QicsLAST_COLUMN;
    int bottom = 0;
    int right = 0;

    QicsSelectionList::iterator iter;
    for (iter = mySelectionList->begin();
	 iter != mySelectionList->end();
	 ++iter)
    {
	QicsSelection &sel = *iter;

	if (sel.topRow() < top)
	    top = sel.topRow();
	if (sel.leftColumn() < left)
	    left = sel.leftColumn();
	if (sel.bottomRow() > bottom)
	    bottom = sel.bottomRow();
	if (sel.rightColumn() > right)
	    right = sel.rightColumn();
    }	

    myData = new QicsDataModelDefault((bottom - top + 1), (right - left + 1));

    // now go through all the selections and copy the data

    for (iter = mySelectionList->begin();
	 iter != mySelectionList->end();
	 ++iter)
    {
	QicsSelection &sel = *iter;

	for (int i = sel.topRow(); i <= sel.bottomRow(); ++i)
	{
	    for (int j = sel.leftColumn(); j <= sel.rightColumn(); ++j)
	    {
		int mr = gi->modelRowIndex(i);
		int mc = gi->modelColumnIndex(j);

		const QicsDataItem *itm = gi->dataModel()->item(mr, mc);
		
		if (itm)
		    myData->setItem((i - top), (j - left), *itm);
	    }
	}
    }
}


QicsTableRegionDrag::~QicsTableRegionDrag()
{
    delete myData;
    delete mySelectionList;
}

QicsTableRegionDrag *
QicsTableRegionDrag::getDragObject(QicsGridInfo *gi, QicsSelectionList *slist,
				   QicsICell *, QWidget *dragSource)
{
    QicsTableRegionDrag *ret = 
	    new QicsTableRegionDrag(gi, slist, dragSource, 0);

    return ret;
}


    /* required QMimeSource methods */
/*
 * \reimp
 */

const char *QicsTableRegionDrag::format(int i) const
{
    if ( i == 0 )
        return QICS_MIME_CELLDATA;
    else if ( i == 1 )
        return "text/plain";

    return 0;
}

/*!
    Returns \b true if \a e can be decoded by the QicsTableRegionDrag, otherwise
    return \b false.
*/

bool QicsTableRegionDrag::canDecode(const QMimeSource* e)
{
    const char* fmt;
    for (int i = 0; (fmt = e->format(i)); ++i)
    {
	if (!strcmp(fmt, QICS_MIME_CELLDATA))
	    return true;
    }

    return false;
}




QByteArray
QicsTableRegionDrag::encodedData(const char *mimetype) const
{
    QByteArray retval;

    if(strcmp(mimetype, "text/plain")==0)
    {
	// this may happen when dropped on another application
    }
    if(!strcmp(mimetype, QICS_MIME_CELLDATA))
    {
	QDataStream ds(retval, IO_WriteOnly);

	// encode number of selections
	ds << (static_cast<Q_INT32> (mySelectionList->size()));

	// we want to encode the topmost row index and the leftmost
	// column index of the selection, because the decoder
	// may want to use this information, and it's much easier
	// to determine it here than in the decoder.

	int top = QicsLAST_ROW;
	int left = QicsLAST_COLUMN;

	QicsSelectionList::iterator iter;
	for (iter = mySelectionList->begin();
	     iter != mySelectionList->end();
	     ++iter)
	{
	    QicsSelection &sel = *iter;

	    if (sel.topRow() < top)
		top = sel.topRow();
	    if (sel.leftColumn() < left)
		left = sel.leftColumn();
	}	

	// encode topmost row and leftmost column of the selections
	ds << top;
	ds << left;

	// now go through all the selections and encode them
	for (iter = mySelectionList->begin();
	     iter != mySelectionList->end();
	     ++iter)
	{
	    QicsSelection &sel = *iter;

	    // encode number of rows and columns in the selection
	    ds << sel.numRows();
	    ds << sel.numColumns();

	    // encode the starting cell of the selection
	    ds << sel.topRow();
	    ds << sel.leftColumn();

	    for (int i = sel.topRow(); i <= sel.bottomRow(); ++i)
	    {
		for (int j = sel.leftColumn(); j <= sel.rightColumn(); ++j)
		{
		    const QicsDataItem *itm = myData->item((i - top), (j - left));

		    if (itm)
			itm->encode(ds);
		    else
			ds << QString("empty");
		}
	    }
	}
    }

    return retval;
}

bool
QicsTableRegionDrag::decode(const QMimeSource *e, QicsDataModel &dm)
{
    bool okdrag = false;

    if (e->provides(QICS_MIME_CELLDATA))
    {
	QByteArray ba = e->encodedData(QICS_MIME_CELLDATA);
	QDataStream ds(ba, IO_ReadOnly);

	// get the number of selections
	int nsels;
	ds >> nsels;

	// get the topmost row and leftmost column of the selections
	int top, left;
	ds >> top;
	ds >> left;

	for (int seln = 0; seln < nsels; ++seln)
	{
	    // get the number of rows and columns
	    int nrows, ncols;
	    ds >> nrows;
	    ds >> ncols;

	    // get the starting cell of the selection
	    int srow, scol;
	    ds >> srow;
	    ds >> scol;

	    // make sure the data model is big enough

	    int dm_row_index = srow - top;
	    int dm_col_index = scol - left;

	    int needed_rows = (dm_row_index + nrows);
	    if (needed_rows > dm.numRows())
	    {
		dm.addRows(needed_rows - dm.numRows());
	    }
	    int needed_cols = (dm_col_index + ncols);
	    if (needed_cols > dm.numColumns())
	    {
		dm.addColumns(needed_cols - dm.numColumns());
	    }

	    // go through each cell and set the value in the data model
	    for (int i = 0; i < nrows; ++i)
	    {
		for (int j = 0; j < ncols; ++j)
		{
		    QicsDataItem *itm = QicsDataItem::decode(ds);

		    if (itm)
		    {
			dm.setItem(dm_row_index + i, dm_col_index + j,
				   *itm);
		    }
		    else
		    {
			dm.clearItem(dm_row_index + i, dm_col_index + j);
		    }

		    delete itm;
		}
	    }
	}

	okdrag = true;
    }

    return okdrag;
}

