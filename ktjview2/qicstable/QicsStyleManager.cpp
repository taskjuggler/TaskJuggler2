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


#include <QicsStyleManager.h>
#include <qstring.h>
#include <qobject.h>
#include <QicsDimensionManager.h>
#include <QicsSpanManager.h>

#define SAVE_SPACE

//----------------------------------------------------------------------------
// CONSTRUCTORS AND DESTRUCTORS
//----------------------------------------------------------------------------

QicsStyleManager::QicsStyleManager(QicsGridType type, QWidget *template_widget)
    : QObject() ,
      myType(type),
      myReportChanges(true),
      myBaseSM(0)
{
    myDefaultStyle = new QicsCellStyle(type, true, template_widget);
    myGridStyle = new QicsGridStyle(type, true, template_widget);

    mySpanManager = new QicsSpanManager();
    connect(mySpanManager, SIGNAL(spanChanged(QicsSpan)),
	    this, SIGNAL(spanChanged(QicsSpan)));
}

QicsStyleManager::QicsStyleManager(QicsStyleManager *base_sm) :
    QObject(),
    myType(base_sm->type()),
    myReportChanges(true)
{
    myBaseSM = base_sm;

    myDefaultStyle = new QicsCellStyle(type());
    myGridStyle = new QicsGridStyle(type());

    connect(base_sm, SIGNAL(cellPropertyChanged(QicsRegion,
						QicsCellStyle::QicsCellStyleProperty)),
	    this, SIGNAL(cellPropertyChanged(QicsRegion,
					     QicsCellStyle::QicsCellStyleProperty)));
    connect(base_sm, SIGNAL(gridPropertyChanged(QicsGridStyle::QicsGridStyleProperty)),
	    this, SIGNAL(gridPropertyChanged(QicsGridStyle::QicsGridStyleProperty)));

    // Create a new span manager with the same spans as the base span manager
    mySpanManager = new QicsSpanManager(*(base_sm->spanManager()));

    connect(mySpanManager, SIGNAL(spanChanged(QicsSpan)),
	    this, SIGNAL(spanChanged(QicsSpan)));
}

QicsStyleManager::~QicsStyleManager()
{
    // Go through each Column and delete styles,
    // then delete the columns
    for (int c = 0;
         c < static_cast<int>(myVectorOfColumns.size());
         ++c)
    {
        if (myVectorOfColumns[c])
        {
            // we must have allocated a row vector, so
            // loop through the row (doing some tricky reference
            // stuff to get the right syntax.
            QicsCellStylePV &the_row_vec = *(myVectorOfColumns[c]);
            for (int r = 0;
                 r < static_cast<int>(the_row_vec.size());
                 ++r)
            {
                // see if there is a style for this row.
                if (the_row_vec[r])
                {
                    // delete the style
                    delete (the_row_vec[r]);
                }
            }
            // delete this vector;
            delete (myVectorOfColumns[c]);
        }
    }
    delete myDefaultStyle;
    delete myGridStyle;

    delete mySpanManager;
}

///////////////////////////////////////////////////////////////////////
///////////////////      Get Property Methods       ///////////////////
///////////////////////////////////////////////////////////////////////

// when we look for a CellProperty we need to 1) look in the cell
// then look in the Column, Then look in the Row, if none is found
// then we return the default.
void *
QicsStyleManager::getCellProperty(int row, int col,
				  QicsCellStyle::QicsCellStyleProperty name ) const
{
    void *val;

    if ((val = cellProp(row, col, name)))
	return val;
    else if ((val = columnProp(col, name)))
	return val;
    else if ((val = rowProp(row, name)))
	return val;
    else
    {
	val = defaultProp(name);

	if (val || !myBaseSM)
	    return val;
	else
	    return myBaseSM->getCellProperty(row, col, name);
    }
}

void *
QicsStyleManager::getRowProperty(int row,
				 QicsCellStyle::QicsCellStyleProperty name) const
{
    void *val;

    if ((val = rowProp(row, name)))
	return val;
    else
    {
	val = defaultProp(name);

	if (val || !myBaseSM)
	    return val;
	else
	    return myBaseSM->getRowProperty(row, name);
    }
}

void *
QicsStyleManager::getColumnProperty(int col,
				    QicsCellStyle::QicsCellStyleProperty name) const
{
    void *val;

    if ((val = columnProp(col, name)))
	return val;
    else
    {
	val = defaultProp(name);

	if (val || !myBaseSM)
	    return val;
	else
	    return myBaseSM->getColumnProperty(col, name);
    }
}

void *
QicsStyleManager::getDefaultProperty(QicsCellStyle::QicsCellStyleProperty name) const
{
    void *val = defaultProp(name);

    if (val || !myBaseSM)
	return val;
    else
	return myBaseSM->getDefaultProperty(name);
}

void *
QicsStyleManager::cellProp(int row, int col,
			   QicsCellStyle::QicsCellStyleProperty prop) const
{
    if ((row >= 0) && (col >= 0) &&
	(static_cast<int>(myVectorOfColumns.size()) > col) &&
        (myVectorOfColumns[col]) &&
        (static_cast<int>(myVectorOfColumns[col]->size()) > row))
    {
        QicsCellStylePV &the_row_vec = *(myVectorOfColumns[col]);
        if (the_row_vec[row])
        {
	    return( the_row_vec[row]->getValue(prop));
        }
    }

    return 0;
}

void *
QicsStyleManager::rowProp(int row, QicsCellStyle::QicsCellStyleProperty prop) const
{
    // If we have a row
    if ((row >= 0) && (static_cast<int> (myVectorOfRowStyles.size()) > row) &&
        (myVectorOfRowStyles[row]))
    {
        return (myVectorOfRowStyles[row]->getValue(prop));
    }

    return 0;
}

void *
QicsStyleManager::columnProp(int col, QicsCellStyle::QicsCellStyleProperty prop) const
{
    // If we have a Col
    if ((col >= 0) && (static_cast<int> (myVectorOfColumnStyles.size()) > col) &&
        (myVectorOfColumnStyles[col] != 0))
    {
        return (myVectorOfColumnStyles[col]->getValue(prop));
    }

    return 0;
}

void *
QicsStyleManager::defaultProp(QicsCellStyle::QicsCellStyleProperty prop) const
{
     return (myDefaultStyle->getValue(prop));
}

///////////////////////////////////////////////////////////////////////
///////////////////      Set Property Methods       ///////////////////
///////////////////////////////////////////////////////////////////////


// This method will expand the internal table to just the right
// size to add this cell property if there isnt a cell there.
// always presume that the wonderful cell world is very small.
// we only need to build the table as big as we absolutly must have
// it.
void
QicsStyleManager::setCellProperty(int row, int col,
				  QicsCellStyle::QicsCellStyleProperty name,
				  const void *val)
{
    // Verify that there is a cell there in the first place, If not
    // expand the table to reference it.
    if(static_cast<int>(myVectorOfColumns.size()) <= col)
    {
        // Ok, there arnt even the required number of columns
        // in this vector. lets expand the number of columns.
        // This Doesnt give us rows, just places (columns) to
        // put them
        //SLOW ME DOWN BABY, lets speed this up.
        myVectorOfColumns.resize(col+10);
    }
    // See if we have a ROW located here, it might just be null
    // so we will have to create one.
    if (!myVectorOfColumns[col])
    {
       //Ok this just gives us a row vector, it has no real rows
       //in it.
       myVectorOfColumns[col]= new QicsCellStylePV();
    }
    // Now lets verify that we have enough rows,
    if (static_cast<int>(myVectorOfColumns[col]->size()) <=row)
    {
        //Ok, not enough rows, lets add enough +10.
        // SLOW ME DOWN BABY
        myVectorOfColumns[col]->resize(row+10);
    }
    // lets see if we have a cell at this Place, if there
    // is no Style there, then we will need to create one,
    // then set its values.

    // This funny cast seems needed, put the pointer into
    // an easy to read reference.
    QicsCellStylePV &the_row_vec = *(myVectorOfColumns[col]);
    if (!the_row_vec[row])
    {
        the_row_vec[row] = new QicsCellStyle(type());
    }


    // by this point we have verified that either
    // everything existed to get here with out memory
    // errors, or we have created everything that we need
    // just set it and be done with it.
    the_row_vec[row]->setValue(name,val);

    if (myReportChanges)
	emit cellPropertyChanged(QicsRegion(row, col, row, col), name);
}

void
QicsStyleManager::setRowProperty(int row,
				 QicsCellStyle::QicsCellStyleProperty name,
				 const void *val, bool override)
{
    // First, try to see if there is something to set:
    if (static_cast<int>(myVectorOfRowStyles.size()) <= row )
    {
        // SLOW ME Down baby, revisit for speed.
        myVectorOfRowStyles.resize(row+10);
    }

    // now see if there is a style there
    if (!myVectorOfRowStyles[row])
    {
        myVectorOfRowStyles[row] = new QicsCellStyle(type());
        assert(myVectorOfRowStyles[row]);
    }

    // now see if the style has a value, if so, clear it there is a new
    // one comming
    if (myVectorOfRowStyles[row]->getValue(name))
    {
        myVectorOfRowStyles[row]->clear(name);
    }

    // set it, we should have created all we need by this point
    myVectorOfRowStyles[row]->setValue(name, val);

    // unset any cell styles for this row

    QicsCellStylePVPV::iterator iter;

    // go through each cell in the row and unset this property
    if(override) {
	for (iter = myVectorOfColumns.begin();
	     iter != myVectorOfColumns.end();
	     ++iter)
	{
	    QicsCellStylePV *colvec = *iter;

	    if (colvec && (static_cast<int> (colvec->size()) > row) && 
	        (*colvec)[row])
	    {
		((*colvec)[row])->clear(name);
	    }
	}
    }

    if (myReportChanges)
	emit cellPropertyChanged(QicsRegion(row, 0, row,
					    Qics::QicsLAST_COLUMN), name);
}

void
QicsStyleManager::setColumnProperty(int col,
				    QicsCellStyle::QicsCellStyleProperty name,
				    const void *val, bool override)
{
    // First, try to see if there is something to set:
    if (static_cast<int>(myVectorOfColumnStyles.size()) <= col )
    {
        // SLOW ME DOWN BABY (think about this with speed)
        myVectorOfColumnStyles.resize(col+10);
    }

    // now see if there is a style there
    if (!myVectorOfColumnStyles[col])
    {
        myVectorOfColumnStyles[col] = new QicsCellStyle(type());
        assert(myVectorOfColumnStyles[col]);
    }

    // now see if the style has a value, if so, clear it there is a new
    // one comming
    if (myVectorOfColumnStyles[col]->getValue(name))
    {
        myVectorOfColumnStyles[col]->clear(name);
    }
    // set it, we should have created all we need by this point
    myVectorOfColumnStyles[col]->setValue(name, val);


    // go through each cell in the column and unset this property

    if(override) {
      if (static_cast<int> (myVectorOfColumns.size()) > col)
      {
	QicsCellStylePV *colvec = myVectorOfColumns[col];

	if (colvec)
	{
	    QicsCellStylePV::iterator iter;

	    for (iter = (*colvec).begin(); iter != (*colvec).end(); ++iter)
	    {
		QicsCellStyle *style = *iter;
		
		if (style)
		    style->clear(name);
	    }
	}
      }
    }

    if (myReportChanges)
	emit cellPropertyChanged(QicsRegion(0, col, Qics::QicsLAST_ROW, col),
				 name);
}

void
QicsStyleManager::setDefaultProperty(QicsCellStyle::QicsCellStyleProperty name,
				     const void *val)
{
    myDefaultStyle->setValue(name, val);


    QicsCellStylePVPV::iterator iter;

    // go through each cell in the table and unset this property if it exists

    for (iter = myVectorOfColumns.begin();
	 iter != myVectorOfColumns.end();
	 ++iter)
    {
	QicsCellStylePV *colvec = *iter;

	if (colvec)
	{
	    QicsCellStylePV::iterator iter2;

	    for (iter2 = (*colvec).begin(); iter2 != (*colvec).end(); ++iter2)
	    {
		QicsCellStyle *style = *iter2;
		
		if (style)
		    style->clear(name);
	    }
	}
    }

    // now go through the list of row styles and remove the property

    QicsCellStylePV::iterator iter3;

    for (iter3 = myVectorOfRowStyles.begin();
	 iter3 != myVectorOfRowStyles.end();
	 ++iter3)
    {
	QicsCellStyle *style = *iter3;
	
	if (style)
	    style->clear(name);
    }

    // finally, go through the list of column styles and remove the property

    for (iter3 = myVectorOfColumnStyles.begin();
	 iter3 != myVectorOfColumnStyles.end();
	 ++iter3)
    {
	QicsCellStyle *style = *iter3;
	
	if (style)
	    style->clear(name);
    }

    // report the change
    if (myReportChanges)
	emit cellPropertyChanged(QicsRegion(0, 0, Qics::QicsLAST_ROW,
					    Qics::QicsLAST_COLUMN), name);
}


///////////////////////////////////////////////////////////////////////
///////////////////     Clear Property Methods      ///////////////////
///////////////////////////////////////////////////////////////////////

void
QicsStyleManager::clearCellPropertyInRow(int row,
					 QicsCellStyle::QicsCellStyleProperty name)
{
    // loop through all the possible Columns
    for (int c = 0;
         c < static_cast<int>(myVectorOfColumns.size());
         ++c)
    {
        // If there is a column here, then check the row
        if (myVectorOfColumns[c] &&
            static_cast<int>(myVectorOfColumns[c]->size()) >= row)
        {

            clearStyleGivenVectorOfRows(*(myVectorOfColumns[c]),
                                        row,
                                        name,
#ifdef SAVE_SACE
                                        true
#else
                                        false
#endif
                                        );
        }
    }
}

void
QicsStyleManager::clearRowProperty(int row,
				   QicsCellStyle::QicsCellStyleProperty name)
{
     if (static_cast<int>(myVectorOfRowStyles.size()) >= row )
     {
        if(myVectorOfRowStyles[row])
        {
            // It might me Marginly faster to check if there is something
            // to clear, as the check (getValue) is inline and clear will
            // cost a function call.
            myVectorOfRowStyles[row]->clear(name);
        }
        // else there is no style at that row node.
     }
     // else, there is no property.
}
void
QicsStyleManager::clearCellPropertyInColumn(int col, 
					    QicsCellStyle::QicsCellStyleProperty name)
{
    if((static_cast<int>(myVectorOfColumns.size()) >= col) &&
       (myVectorOfColumns[col]))
    {
        QicsCellStylePV &the_row_vec = *(myVectorOfColumns[col]);
        for (int r = 0;
            r < (static_cast<int>(the_row_vec.size()));
            ++r)
        {
            clearStyleGivenVectorOfRows(the_row_vec,
                                        r,
                                        name,
#ifdef SAVE_SACE
                                        true
#else
                                        false
#endif
                                        );
        }
    }
}

void
QicsStyleManager::clearColumnProperty(int col,
				      QicsCellStyle::QicsCellStyleProperty name)
{
   if (static_cast<int>(myVectorOfColumnStyles.size()) >= col )
     {
        if(myVectorOfColumnStyles[col])
        {
            // It might me Marginly faster to check if there is something
            // to clear, as the check (getValue) is inline and clear will
            // cost a function call.
            myVectorOfColumnStyles[col]->clear(name);
        }
        // else there is no style at that row node.
     }
     // else, there is no property.
}


void
QicsStyleManager::clearCellProperty(int row, int col,
				    QicsCellStyle::QicsCellStyleProperty name)

{
   if((static_cast<int>(myVectorOfColumns.size()) >= col) &&
       (myVectorOfColumns[col]))
   {
       QicsCellStylePV &the_row_vec = *(myVectorOfColumns[col]);
       if ((static_cast<int>(the_row_vec.size())) >= row)
       {
            clearStyleGivenVectorOfRows(the_row_vec,
                                        row,
                                        name,
#ifdef SAVE_SPACE
                                        true
#else
                                        false
#endif
                                        );

       }

       // report the change
       if (myReportChanges)
	   emit cellPropertyChanged(QicsRegion(row, col, row, col), name);
   }
}

void 
QicsStyleManager::clearStyleGivenVectorOfRows(QicsCellStylePV &row_vec, 
					      int row,
					      QicsCellStyle::QicsCellStyleProperty name,
					      bool save_space)
{
    if (row_vec[row])
    {
        row_vec[row]->clear(name);
        if (save_space)
        {
            // nuke it if we can, save the space..
            if (row_vec[row]->isEmpty())
            {
                delete row_vec[row];
                row_vec[row] = 0;
            }
        }
    }
}
void
QicsStyleManager::replaceStyle(QicsCellStylePV *, int , QicsCellStyle *)
{

}

///////////////////////////////////////////////////////////////////////
//////////////////      Insert/Delete Methods       ///////////////////
///////////////////////////////////////////////////////////////////////

void
QicsStyleManager::insertRows(int num, int start_position)
{
    // make sure we are in bounds..
    // and no one is trying to clobber us with a bad number of rows.
    if ((start_position < 0) || (num <= 0))
	return;

    // First, we do the cell styles

    QicsCellStylePVPV::iterator iter;

    // iterate through all columns
    for (iter = myVectorOfColumns.begin();
	 iter != myVectorOfColumns.end();
	 ++iter)
    {
	QicsCellStylePV *column = *iter;

	// for insertions out of the current size of the column, do nothing
	if (!column || (start_position >= static_cast<int> (column->size())))
	    continue;

	QicsCellStylePV::iterator pos = column->begin();
	pos += start_position;

	column->insert(pos, static_cast<size_t> (num), 0);
    }

    // Next, the row styles

    if (start_position < static_cast<int> (myVectorOfRowStyles.size()))
    {
	QicsCellStylePV::iterator pos = myVectorOfRowStyles.begin();
	pos += start_position;

	myVectorOfRowStyles.insert(pos, static_cast<size_t> (num), 0);
    }
}

void
QicsStyleManager::insertColumns(int num, int start_position)
{
    // make sure we are in bounds..
    // and no one is trying to clobber us with a bad number of columns.

    if ((start_position < 0) || (num <= 0))
	return;

    // First, we do the cell styles

    if (start_position < static_cast<int> (myVectorOfColumns.size()))
    {
	QicsCellStylePVPV::iterator pos = myVectorOfColumns.begin();
	pos += start_position;

	myVectorOfColumns.insert(pos, static_cast<size_t> (num), 0);
    }

    // Next, the column styles

    if (start_position < static_cast<int> (myVectorOfColumnStyles.size()))
    {
	QicsCellStylePV::iterator pos = myVectorOfColumnStyles.begin();
	pos += start_position;

	myVectorOfColumnStyles.insert(pos, static_cast<size_t> (num), 0);
    }
}

void
QicsStyleManager::deleteRows(int num, int start_position)
{
    // make sure we are in bounds..
    // and no one is trying to clobber us with a bad number of rows.
    if ((start_position < 0) || (num <= 0))
	return;

    // First, we do the cell styles

    QicsCellStylePVPV::iterator iter;

    // iterate through all columns
    for (iter = myVectorOfColumns.begin();
	 iter != myVectorOfColumns.end();
	 ++iter)
    {
	QicsCellStylePV *column = *iter;

	// for insertions out of the current size of the column, do nothing
	if (!column || (start_position >= static_cast<int> (column->size())))
	    continue;

	QicsCellStylePV::iterator start_pos = column->begin();
	start_pos += start_position;

	QicsCellStylePV::iterator end_pos;
	if ((start_position + num) >= static_cast<int> (column->size()))
	{
	    end_pos = column->end();
	}
	else
	{
	    end_pos= column->begin();
	    end_pos += (start_position + num);
	}

	column->erase(start_pos, end_pos);
    }

    // Next, the row styles

    if (start_position < static_cast<int> (myVectorOfRowStyles.size()))
    {
	QicsCellStylePV::iterator start_pos = myVectorOfRowStyles.begin();
	start_pos += start_position;

	QicsCellStylePV::iterator end_pos;
	if ((start_position + num) >= 
	    static_cast<int> (myVectorOfRowStyles.size()))
	{
	    end_pos = myVectorOfRowStyles.end();
	}
	else
	{
	    end_pos= myVectorOfRowStyles.begin();
	    end_pos += (start_position + num);
	}

	myVectorOfRowStyles.erase(start_pos, end_pos);
    }
}

void
QicsStyleManager::deleteColumns(int num, int start_position)
{
    // make sure we are in bounds..
    // and no one is trying to clobber us with a bad number of columns.

    if ((start_position < 0) || (num <= 0))
	return;

    // First, we do the cell styles

    if (start_position < static_cast<int> (myVectorOfColumns.size()))
    {
	QicsCellStylePVPV::iterator start_pos = myVectorOfColumns.begin();
	start_pos += start_position;

	QicsCellStylePVPV::iterator end_pos;
	if ((start_position + num) >= 
	    static_cast<int> (myVectorOfColumns.size()))
	{
	    end_pos = myVectorOfColumns.end();
	}
	else
	{
	    end_pos= myVectorOfColumns.begin();
	    end_pos += (start_position + num);
	}

	myVectorOfColumns.erase(start_pos, end_pos);
    }

    // Next, the column styles

    if (start_position < static_cast<int> (myVectorOfColumnStyles.size()))
    {
	QicsCellStylePV::iterator start_pos = myVectorOfColumnStyles.begin();
	start_pos += start_position;

	QicsCellStylePV::iterator end_pos;
	if ((start_position + num) >= 
	    static_cast<int> (myVectorOfColumnStyles.size()))
	{
	    end_pos = myVectorOfColumnStyles.end();
	}
	else
	{
	    end_pos= myVectorOfColumnStyles.begin();
	    end_pos += (start_position + num);
	}

	myVectorOfColumnStyles.erase(start_pos, end_pos);
    }
}

////////////////////////////////////////////////////////////////////////

void
QicsStyleManager::setGridProperty(QicsGridStyle::QicsGridStyleProperty name,
				  const void *val)
{
    myGridStyle->setValue(name, val);

    emit gridPropertyChanged(name);
}

void *
QicsStyleManager::getGridProperty(QicsGridStyle::QicsGridStyleProperty name) const
{
    void *val = myGridStyle->getValue(name);

    if (!val)
	val = myBaseSM->getGridProperty(name);

    return val;
}

#include "QicsStyleManager.moc"
