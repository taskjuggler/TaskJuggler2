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


#ifndef _QicsDataModel_H
#define _QicsDataModel_H

#include <qvaluevector.h>
#include <qstring.h>
#include <qobject.h>

#include <QicsNamespace.h>
#include <QicsDataItem.h>
#include <QicsRegion.h>




typedef QValueVector<const QicsDataItem *>         QicsDataModelColumn;


typedef QValueVector<const QicsDataItem *>         QicsDataModelRow;




class QicsDataModel: public QObject, public Qics {
    Q_OBJECT
public:
    
    QicsDataModel(int num_rows = 0 , int num_cols = 0);

    
    virtual ~QicsDataModel(void);

    
    inline int numRows(void) const { return myNumRows; }

    
    inline int numColumns(void) const { return myNumColumns; }
        
    
    inline int lastRow(void) const { return (myNumRows - 1); }

    
    inline int lastColumn(void) const { return (myNumColumns - 1); }

    
    inline bool contains(int row, int col) const
	{ return ((col > lastColumn() ||
		   row > lastRow() ||
		   row < 0 ||
		   col < 0)?false: true );}
    
    
    virtual const QicsDataItem *item(int row, int col) const = 0;

    
    virtual QString itemString(int row, int col) const;

    
    const QicsDataItem *operator()(int row, int col) { return item(row, col); }

    
    virtual QicsDataModelRow rowItems(int row,
				      int first_col = 0,
				      int last_col = -1) const = 0;

    
    virtual QicsDataModelColumn columnItems(int col,
					    int first_row = 0,
					    int last_row = -1) const = 0;

    
    virtual void setRowItems(int row, const QicsDataModelRow &v) = 0;

    
    virtual void setColumnItems(int col, const QicsDataModelColumn &v) = 0;
    
    
    inline bool emitSignals(void) const { return myEmitSignalsFlag; }
        
    
    inline void setEmitSignals(bool b)  { myEmitSignalsFlag = b; }
        
    
    void readASCII(QTextStream &stream, const char separator = ';',
		   int start_row = 0, int start_col = 0,
		   bool clear_model = false);

    
    void writeASCII(QTextStream &stream, const char separator = ';',
		    int start_row = 0, int start_col = 0,
		    int nrows = -1, int ncols = -1);

    
    typedef QicsDataModel *(*Foundry)(int num_rows, int num_cols);

public slots:    
    
    virtual void setItem(int row, int col, const QicsDataItem &item) = 0;

    
    inline virtual void setItem(int row, int col, int val)
	{ setItem(row, col, QicsDataInt(val)); }

    
    inline virtual void setItem(int row, int col, long val)
	{ setItem(row, col, QicsDataLong(val)); }

    
    inline virtual void setItem(int row, int col, float val)
	{ setItem(row, col, QicsDataFloat(val)); }

    
    inline virtual void setItem(int row, int col, double val)
	{ setItem(row, col, QicsDataDouble(val)); }

    
    inline virtual void setItem(int row, int col, const QString &val)
	{ setItem(row, col, QicsDataString(val)); }

    
    inline virtual void setItem(int row, int col, const QDate &val)
	{ setItem(row, col, QicsDataDate(val)); }

    
    inline virtual void setItem(int row, int col, const QTime &val)
	{ setItem(row, col, QicsDataTime(val)); }

    
    inline virtual void setItem(int row, int col, const QDateTime &val)
	{ setItem(row, col, QicsDataDateTime(val)); }

    
    virtual void insertRows(int number_of_rows, int starting_position)=0;

    
    virtual void insertColumns(int number_of_columns,
			       int starting_position)=0;

    
    virtual void addRows(int rows) =0;
    
     
    virtual void addColumns(int num) = 0;
        
    
    virtual void deleteRow(int row)  = 0;

    
    virtual void deleteRows(int num_rows, int start_row)  = 0;

    
    virtual void deleteColumn(int col) = 0;

    
    virtual void deleteColumns(int num_cols, int start_col) = 0;

    
    virtual void clearItem(int row, int col)  = 0;

    
    virtual void clearModel(void) = 0;
         
 signals:
    
    void modelChanged(QicsRegion reg);
    
    
    void modelSizeChanged(int new_number_of_rows, int new_number_of_columns);

    
    void rowsInserted(int number_of_rows, int starting_position);
    
    void columnsInserted(int number_of_columns, int starting_position);

    
    void rowsDeleted(int number_of_rows, int starting_position);
    
    void columnsDeleted(int number_of_columns, int starting_position);

    
    void rowsAdded(int number_of_rows);
    
    void columnsAdded(int number_of_columns);
         
protected:
    
    inline void setNumRows(int nrows) { myNumRows = nrows; }
    
    inline void setNumColumns(int ncols) { myNumColumns = ncols; }

        int myNumRows;
        int myNumColumns;
        
        bool myEmitSignalsFlag; 

private:
    
    QicsDataModel(const QicsDataModel&):QObject() 
	{/*DON'T DO THIS */}
    
    QicsDataModel& operator= (const QicsDataModel& ) 
	{/*DON'T DO THIS */return *this;}
};

#endif /*_QicsDataModel_H --- Do not add anything past this line */

