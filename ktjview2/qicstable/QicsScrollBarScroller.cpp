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


#include <QicsScrollBarScroller.h>

QicsScrollBarScroller::QicsScrollBarScroller(QicsIndexType type,
					     QWidget *parent,
					     const char *name)
{
    myType = type;
    myInSetIndex = false;
    myMode = Auto;

    Orientation orientation = (type == RowIndex ? Vertical : Horizontal);
    myScrollBar = new QScrollBar(orientation, parent, name);
    myScrollBar->hide();

    connect(myScrollBar, SIGNAL(valueChanged(int)),
	    this, SLOT(valueChanged(int)));
}

QicsScrollBarScroller::~QicsScrollBarScroller()
{
    delete myScrollBar;
}

void
QicsScrollBarScroller::setIndex(QicsIndexType type, int idx)
{
    if (type == myType)
    {
	// We use this flag so we don't emit the valueChanged() signal
	// in this case.  The scroll manager already knows about this
	// index changed, because it's the one that called this method.
	myInSetIndex = true;
	myScrollBar->setValue(idx);
	myInSetIndex = false;
    }
}

void
QicsScrollBarScroller::setMinIndex(QicsIndexType type, int idx)
{
    if (type == myType)
    {
	myScrollBar->setMinValue(idx);
	checkBounds();
    }
}
    
void
QicsScrollBarScroller::setMaxIndex(QicsIndexType type, int idx)
{
    if (type == myType)
    {
	myScrollBar->setMaxValue(idx);
	checkBounds();
    }
}

void QicsScrollBarScroller::setMode(QicsScrollBarMode m)
{
    myMode = m;
    checkBounds();
}


//////////////////////////////////////////////////////////////////////

void
QicsScrollBarScroller::valueChanged(int val)
{
    if (!myInSetIndex)
	emit indexChanged(myType, val);
}

void
QicsScrollBarScroller::checkBounds(void)
{
    bool show;

    if (myMode == AlwaysOn)
	show = true;
    else if (myMode == AlwaysOff)
	show = false;
    else
	show = (myScrollBar->minValue() < myScrollBar->maxValue());

    if (show)
	myScrollBar->show();
    else
	myScrollBar->hide();
}
