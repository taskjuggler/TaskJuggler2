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


#ifndef _QicsDataModelDefault_H
#define _QicsDataModelDefault_H

#include <QicsDataModel.h>
#include <QicsDataItem.h>
#include <qstring.h>
#include <qvaluevector.h>

typedef QValueVector<QicsDataItem *> QicsDataItemPV;
typedef QValueVector<QicsDataItemPV *> QicsDataItemPVPV;



class QicsDataModelDefault: public QicsDataModel {
    Q_OBJECT
public:
    QicsDataModelDefault(int num_rows = 0, int num_cols = 0);
    virtual ~QicsDataModelDefault();

    virtual const QicsDataItem *item(int, int) const;
    virtual QicsDataModelRow rowItems(int, int, int) const;
    virtual QicsDataModelColumn columnItems(int, int, int) const;

    virtual void setColumnItems(int, const QicsDataModelColumn &v);
    virtual void setRowItems(int, const QicsDataModelRow &v);

    
    static QicsDataModel *create(int num_rows = 0 , int num_cols = 0);

public slots:
#if !defined(__GNUC__) || (__GNUC__ >= 3)
    // need this or else setItem() hides other setItem methods in superclass
    using QicsDataModel::setItem;
#endif

    virtual void setItem(int row, int col, const QicsDataItem &item);

    virtual void insertColumns(int, int);
    virtual void insertRows(int, int);
    virtual void addColumns(int cols);
    virtual void addRows(int rows);
        
    virtual void clearItem (int, int);
    virtual void clearModel(void);
    virtual void deleteRow (int);
    virtual void deleteColumn (int);
    virtual void deleteRows(int num_rows, int start_row);
    virtual void deleteColumns(int num_cols, int start_col);
    
protected:
    
            
    virtual void clearRow(int rownum);
    
    
    QicsDataItemPVPV myVectorOfRowPointers; 
};

#endif /*_QicsDataModelDefault_H --- Do not add anything past this line */

