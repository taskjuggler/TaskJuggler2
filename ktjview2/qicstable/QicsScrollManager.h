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


#ifndef _QICSSCROLLMANAGER_H
#define _QICSSCROLLMANAGER_H

#include <QicsNamespace.h>
#include <qobject.h>

//////////////////////////////////////////////////////////////////////////////

class QicsScroller;
class QicsScreenGrid;




class QicsScrollManager: public QObject, public Qics
{
    Q_OBJECT

public:
    
    QicsScrollManager();
    
    virtual ~QicsScrollManager();

    
    void connectScroller(QicsScroller *scroller);
    
    void disconnectScroller(QicsScroller *scroller);

    
    inline QicsScreenGrid *primaryGrid(void) const { return myPrimaryGrid; }
    
    void setPrimaryGrid(QicsScreenGrid *grid);

    
    void connectGrid(QicsScreenGrid *grid, bool control_rows, bool control_columns);
    
    void disconnectGrid(QicsScreenGrid *grid);

    
    inline int rowIndex(void) const { return myRowIndex; }
    
    inline int columnIndex(void) const { return myColumnIndex; }

public slots:
    
    void setRowMinValue(int val);
    
    void setRowMaxValue(int val);

    
    void setColumnMinValue(int val);
    
    void setColumnMaxValue(int val);

    
    void setRowIndex(int idx);
    
    void setColumnIndex(int idx);

signals:
    
    void rowIndexChanged(int idx);
    
    void columnIndexChanged(int idx);

    
    void indexChanged(QicsIndexType type, int val);

    
    void minIndexChanged(QicsIndexType type, int val);
    
    void maxIndexChanged(QicsIndexType type, int val);

protected:
    /// the primary grid widget
    QicsScreenGrid *myPrimaryGrid;
    /// the current topmost displayed row
    int myRowIndex;
    /// the current leftmost displayed column
    int myColumnIndex;

protected slots:
    
    void setIndex(QicsIndexType type, int idx);
    
    void gridBoundsChanged(void);
    
    void gridDestroyed(QObject *obj);

};

#endif /* _QICSSCROLLMANAGER_H */
