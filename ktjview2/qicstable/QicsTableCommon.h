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


#ifndef _QicsTABLECOMMON_H
#define _QicsTABLECOMMON_H

#include <QicsGridCommon.h>
#include <QicsHeader.h>
#include <QicsRow.h>
#include <QicsColumn.h>
#include <QicsMainGrid.h>
#include <QicsRowHeader.h>
#include <QicsColumnHeader.h>
#include <QicsCell.h>

/////////////////////////////////////////////////////////////////////////////////



class QicsTableCommon: public QicsGridCommon
{
    Q_OBJECT

    friend class QicsTable;

public:

    Q_ENUMS( QicsTableDisplayOption )

   


    
    Q_PROPERTY( QicsTableDisplayOption topHeaderVisible READ topHeaderVisible WRITE setTopHeaderVisible )

    
    Q_PROPERTY( QicsTableDisplayOption bottomHeaderVisible READ bottomHeaderVisible WRITE setBottomHeaderVisible )

    
    Q_PROPERTY( QicsTableDisplayOption leftHeaderVisible READ leftHeaderVisible WRITE setLeftHeaderVisible )

    
    Q_PROPERTY( QicsTableDisplayOption rightHeaderVisible READ rightHeaderVisible WRITE setRightHeaderVisible )

    
    Q_PROPERTY( int tableMargin READ tableMargin WRITE setTableMargin )

    
    Q_PROPERTY( int tableSpacing READ tableSpacing WRITE setTableSpacing )

    
    Q_PROPERTY( int gridSpacing READ gridSpacing WRITE setGridSpacing )

    
    Q_PROPERTY( bool columnHeaderUsesModel READ columnHeaderUsesModel WRITE setColumnHeaderUsesModel )

    
    Q_PROPERTY( bool rowHeaderUsesModel READ rowHeaderUsesModel WRITE setRowHeaderUsesModel )



public:
    
    QicsTableCommon(QObject *parent = 0, bool forward_signals = false);

    
    virtual ~QicsTableCommon();

    /////////////////// Data Model methods

    
    QicsDataModel *dataModel(void) const;
    
    virtual void setDataModel(QicsDataModel *dm);

    /////////////////// Grid Access classes

    
    QicsRow &rowRef(int rownum);
    
    const QicsRow &rowRef(int rownum) const;

    
    QicsRow *row(int rownum, bool follow_model = true);
    
    const QicsRow *row(int rownum, bool follow_model = true) const;

    
    QicsColumn &columnRef(int colnum);
    
    const QicsColumn &columnRef(int colnum) const;

    
    QicsColumn *column(int colnum, bool follow_model = true);
    
    const QicsColumn *column(int colnum, bool follow_model = true) const;

    
    QicsCell &cellRef(int rownum, int colnum);
    
    const QicsCell &cellRef(int rownum, int colnum) const;

    
    QicsCell *cell(int rownum, int colnum, bool follow_model = true);
    
    const QicsCell *cell(int rownum, int colnum, bool follow_model = true) const;

    
    QicsMainGrid &mainGridRef(void);
    
    const QicsMainGrid &mainGridRef(void) const;

    
    QicsMainGrid *mainGrid(void);
    
    const QicsMainGrid *mainGrid(void) const;

    
    QicsRowHeader &rowHeaderRef(void);
    
    const QicsRowHeader &rowHeaderRef(void) const;

    
    QicsRowHeader *rowHeader(void);
    
    const QicsRowHeader *rowHeader(void) const;

    
    QicsColumnHeader &columnHeaderRef(void);
    
    const QicsColumnHeader &columnHeaderRef(void) const;

    
    QicsColumnHeader *columnHeader(void);
    
    const QicsColumnHeader *columnHeader(void) const;
    
    /////////////////// PROPERTY GETS

    
    inline QicsTableDisplayOption topHeaderVisible(void) const
	{ return myTopHeaderVisible; }

    
    inline QicsTableDisplayOption bottomHeaderVisible(void) const
	{ return myBottomHeaderVisible; }

    
    inline QicsTableDisplayOption leftHeaderVisible(void) const
	{ return myLeftHeaderVisible; }

    
    inline QicsTableDisplayOption rightHeaderVisible(void) const
	{ return myRightHeaderVisible; }

    
    inline int tableMargin(void) const
	{ return myTableMargin; }

    
    inline int tableSpacing(void) const
	{ return myTableSpacing; }

    
    inline int gridSpacing(void) const
	{ return myGridSpacing; }

    
    inline bool columnHeaderUsesModel(void) const
	{ return myColumnHeaderUsesModel; } ;

    
    inline bool rowHeaderUsesModel(void) const
	{ return myRowHeaderUsesModel; } ;

    
    void sortRows(int column,
		  QicsSortOrder order = Qics::Ascending,
		  int from = 0, int to = -1,
		  DataItemComparator func = 0);

    
    void sortColumns(int row,
		     QicsSortOrder order = Qics::Ascending,
		     int from = 0, int to = -1,
		     DataItemComparator func = 0);

    
    void moveRows(int target_row, const QMemArray<int> &rows);

    
    void moveColumns(int target_col, const QMemArray<int> &cols);

public slots:
    
    void setTopHeaderVisible(QicsTableDisplayOption);

    
    void setBottomHeaderVisible(QicsTableDisplayOption);

    
    void setLeftHeaderVisible(QicsTableDisplayOption);

    
    void setRightHeaderVisible(QicsTableDisplayOption);

    
    void setTableMargin(int margin);

    
    void setTableSpacing(int spacing);

    
    void setGridSpacing(int spacing);

    
    void setRowHeaderUsesModel(bool b);

    
     void setColumnHeaderUsesModel(bool b);

    
    void deleteColumn(int column);
    
    void addColumns(int howMany);
    
    void insertColumn(int column);
    
    void deleteRow(int row);
    
    void addRows(int rows);
    
    void insertRow(int row);

protected:
    virtual void setAttr(QicsCellStyle::QicsCellStyleProperty attr, const void *val);
    virtual void setGridAttr(QicsGridStyle::QicsGridStyleProperty attr,
			     const void *val);
    virtual void clearGridAttr(QicsGridStyle::QicsGridStyleProperty attr);

    const QicsGridInfo &gridInfo(void) const { return myMainGridInfo; }
    const QicsGridInfo &rhGridInfo(void) const { return myRowHeaderGridInfo; }
    const QicsGridInfo &chGridInfo(void) const { return myColumnHeaderGridInfo; }

    QicsGridInfo &gridInfo(void)  { return myMainGridInfo; }
    QicsGridInfo &rhGridInfo(void)  { return myRowHeaderGridInfo; }
    QicsGridInfo &chGridInfo(void)  { return myColumnHeaderGridInfo; }

        QicsGridInfo myMainGridInfo;
        QicsGridInfo myRowHeaderGridInfo;
        QicsGridInfo myColumnHeaderGridInfo;

        QicsDataModel *myRHDataModel;
        QicsDataModel *myCHDataModel;

    QicsCell *myCell;
    QicsRow *myRow;
    QicsColumn *myColumn;
    QicsMainGrid *myMainGrid;
    QicsRowHeader *myRowHeader;
    QicsColumnHeader *myColumnHeader;

        QicsTableDisplayOption myTopHeaderVisible;
        QicsTableDisplayOption myBottomHeaderVisible;
        QicsTableDisplayOption myLeftHeaderVisible;
        QicsTableDisplayOption myRightHeaderVisible;

        int myTableMargin;
        int myTableSpacing;
        int myGridSpacing;

        bool myRowHeaderUsesModel;
        bool myColumnHeaderUsesModel;
};

#endif /* _QICSTABLECOMMON_H */
