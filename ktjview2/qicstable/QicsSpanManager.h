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


#ifndef _QICSSPANMANAGER_H
#define _QICSSPANMANAGER_H

#include <QicsNamespace.h>
#include <QicsSpan.h>
#include <QicsRegion.h>

#include <qobject.h>

class QicsGridInfo;


class QicsSpanManager: public QObject, public Qics
{
    Q_OBJECT

public:
    
    QicsSpanManager(void);

    
    QicsSpanManager(const QicsSpanManager &sm);

    ~QicsSpanManager(void);

    
    bool addCellSpan(QicsSpan span);
    
    void removeCellSpan(int start_row, int start_col);

    
    QicsSpanList *cellSpanList(void) const;

    
    bool isSpanner(QicsGridInfo &, int row, int col, QicsRegion &reg_return) const;
    
    bool isSpanned(QicsGridInfo &,int row, int col, QicsRegion &reg_return) const;

    
    QicsRegion maxSpanForRow(QicsGridInfo &gi, int row) const;
    
    QicsRegion maxSpanForColumn(QicsGridInfo &gi, int col) const;

signals:
    
    void spanChanged(QicsSpan span);

protected:
        // are in model coordinates, so we do not have to remap
    // them when sorting
    QicsSpanList myCellSpanList;
    
};

#endif /* _QICSSPANMANAGER_H */
