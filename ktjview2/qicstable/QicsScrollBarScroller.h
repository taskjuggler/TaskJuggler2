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


#ifndef _QICSSCROLLBARSCROLLER_H
#define _QICSSCROLLBARSCROLLER_H

#include <QicsScroller.h>
#include <qscrollbar.h>



class QicsScrollBarScroller: public QicsScroller
{
    Q_OBJECT

public:
    
    QicsScrollBarScroller(QicsIndexType type, QWidget *parent,
			  const char *name = 0);
    
    virtual ~QicsScrollBarScroller();

    
    inline QScrollBar *widget(void) const { return myScrollBar; }

    
    inline QicsScrollBarMode mode(void) const { return myMode; }

    
    void setMode(QicsScrollBarMode m);

public slots:
    
    inline virtual int index(void) const { return myScrollBar->value(); }
    
    virtual void setIndex(QicsIndexType type, int idx);

    
    inline virtual int minIndex(void) const { return myScrollBar->minValue(); }
    
    virtual void setMinIndex(QicsIndexType type, int idx);

    
    inline virtual int maxIndex(void) const { return myScrollBar->maxValue(); }
    
    virtual void setMaxIndex(QicsIndexType type, int idx);

protected slots:
    
    void valueChanged(int val);

protected:
    
    void checkBounds(void);

    /// the scrollbar widget
    QScrollBar *myScrollBar;
    /// the type of this scroller
    QicsIndexType myType;
    /// flag used in setIndex to avoid recursive signals
    bool myInSetIndex;
    /// controls the mode of the scrollbar
    QicsScrollBarMode myMode;
};


#endif /* _QICSSCROLLBARSCROLLER_H */
