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


#ifndef _QICSSCROLLER_H
#define _QICSSCROLLER_H

#include <QicsNamespace.h>
#include <qobject.h>

 
class QicsScroller: public QObject, public Qics
{
    Q_OBJECT

public:
    
    QicsScroller() {;}
    
    virtual ~QicsScroller() {;}

signals:
    
    void indexChanged(QicsIndexType type, int idx);

public slots:
    
    virtual int index(void) const = 0;
    
    virtual void setIndex(QicsIndexType type, int idx) = 0;

    
    virtual int minIndex(void) const = 0;
    
    virtual void setMinIndex(QicsIndexType type, int idx) = 0;

    
    virtual int maxIndex(void) const = 0;
    
    virtual void setMaxIndex(QicsIndexType type, int idx) = 0;
};

#endif /* _QICSSCROLLER_H */
