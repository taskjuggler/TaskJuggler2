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


#ifndef _QICSICELL_H
#define _QICSICELL_H

#include <qpoint.h>
#include <qvaluelist.h>
#include <qvaluevector.h>



class QicsICell : public QPoint
{
  public:
    
    QicsICell(int row, int col)
        : QPoint(col, row) {}

    
    QicsICell(void) : QPoint(-1, -1) {}
  
    
    inline int row(void) const     { return y(); }
    
    inline int column(void) const  { return x(); }

    
    inline void setRow(int row)    { setY(row); }
    
    inline void setColumn(int col)   { setX(col); }

    
    inline bool isValid(void) const { return ((x() >= 0) && (y() >= 0)); }
};

typedef QValueVector<QicsICell> QicsICellV;
typedef QValueVector<QicsICell *> QicsICellPV;
typedef QValueList<QicsICell> QicsICellQVL;

#endif /* _QICSICELL_H */
