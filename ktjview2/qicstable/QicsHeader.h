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


#ifndef _QicsHeader_H
#define _QicsHeader_H

#include <QicsGridCommon.h>

#include <QicsRow.h>
#include <QicsColumn.h>
#include <QicsCell.h>




class QicsHeader: public QicsGridCommon {
Q_OBJECT

public:
   


    
    Q_PROPERTY( bool allowUserResize READ allowUserResize WRITE setAllowUserResize )

    
    Q_PROPERTY( bool allowUserMove READ allowUserMove WRITE setAllowUserMove )


public:
    
    QicsHeader(QicsGridInfo *info, QObject *parent = 0,
	       bool forward_signals = false);

    
    virtual ~QicsHeader();

    
    virtual QicsRow &rowRef(int rownum) const;
    
    virtual QicsRow *row(int idx, bool follow_model = true) const;

    
    virtual QicsColumn &columnRef(int colnum) const;
    
    virtual QicsColumn *column(int idx, bool follow_model = true) const;

    
    virtual QicsCell &cellRef(int rownum, int colnum) const;
    
    virtual QicsCell *cell(int row, int col, bool follow_model = true) const;

    
    bool allowUserResize(void) const;

    
    bool allowUserMove(void) const;

public slots:
    
    void setAllowUserResize(bool b);

    
    void setAllowUserMove(bool b);

signals:
    
    void sizeChange(int idx, int old_size, int new_size, QicsHeaderType type);

    
    void resizeInProgress(int idx, int position, QicsHeaderType type);

    
    void gripDoubleClicked(int idx, int button, QicsHeaderType type);

protected slots:
    void connectGrid(QicsScreenGrid *grid);

protected:
    QicsCell *myCell;
    QicsRow *myRow;
    QicsColumn *myColumn;
};

#endif /* _QicsHeader_H --- Do not add anything past this line */
 
