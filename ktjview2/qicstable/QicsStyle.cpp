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


#include <QicsStyle.h>
#include <qstring.h>
#include <qcolor.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <qpen.h>
#include <qvalidator.h>
#include <QicsCellDisplay.h>
#include <QicsTextCellDisplay.h>
#include <QicsDataItemFormatter.h>
#include <QicsUtil.h>
#include <QicsRegion.h>

////////////////////////////////////////////////////////////////////////////////

QicsStyle::QicsStyle()
{
    mySetCount = 0;
}

void
QicsStyle::init(void)
{
    myProperties = new void*[myNumProperties];


    for (int prop = 0; prop < myNumProperties; ++prop)
    {
	myProperties[prop]=0;
    }
}

QicsStyle::~QicsStyle()
{
    for (int i =0; i < myNumProperties; ++i)
    {
        if (myProperties[i])
        {
            clear(i);
        }
    }

    delete [] myProperties;
}

bool
QicsStyle::isEmpty(void) const
{
    return (mySetCount == 0);
}

void
QicsStyle::setValue(int prop, const void *val)
{
    if (!getValue(prop))
	++mySetCount;

    void *newval;

    if (val)
    {
	switch(myStyleTypeList[prop])
	{
	case QicsT_Int:
	    newval = new int(*(static_cast<const int *> (val)));
	    break;
	case QicsT_Float:
	    newval = new float(*(static_cast<const float *>(val)));
	    break;
	case QicsT_QString:
	    newval = new QString(*(static_cast<const QString *>(val)));
	    break;
	case QicsT_QColor:
	    newval = new QColor(*(static_cast<const QColor *>(val)));
	    break;
	case QicsT_Boolean:
	    newval = new bool(*(static_cast<const bool *> (val)));
	    break;
	case QicsT_QWidget:
	    // We don't copy this
	    newval = const_cast<void *> (val);
	    break;
	case QicsT_QFont:
	    newval = new QFont(*(static_cast<const QFont *> (val)));
	    break;
	case QicsT_QColorGroup:
	    newval = new QColorGroup(*(static_cast<const QColorGroup *> (val)));
	    break;
	case QicsT_QPalette:
	    newval = new QPalette(*(static_cast<const QPalette *> (val)));
	    break;
	case QicsT_QCursor:
	    newval = new QCursor(*(static_cast<const QCursor *> (val)));
	    break;
	case QicsT_QicsCellDisplay:
	    // We don't copy this
	    newval = const_cast<void *> (val);
	    break;
	case QicsT_QicsDataItemFormatter:
	    // We don't copy this
	    newval = const_cast<void *> (val);
	    break;
	case QicsT_QPixmap:
	    newval = new QPixmap(*(static_cast<const QPixmap *> (val)));
	    break;
	case QicsT_QValidator:
	    // We don't copy this
	    newval = const_cast<void *> (val);
	    break;
	case QicsT_QPen:
	    newval = new QPen(*(static_cast<const QPen *> (val)));
	    break;
	case QicsT_QicsRegion:
	    newval = new QicsRegion(*(static_cast<const QicsRegion *> (val)));
	    break;
	case QicsT_Pointer:
	    // We don't copy this
	    newval = const_cast<void *> (val);
	    break;
	default:
	    newval = 0;
	}
    }
    else
    {
	newval = 0;
    }

    myProperties[prop] = newval;
}

void
QicsStyle::clear(int prop)
{
    if (!getValue(prop)) return;

    switch(myStyleTypeList[prop])
    {
    case QicsT_Int:
	delete (static_cast<int *>(myProperties[prop]));
	break;
    case QicsT_Float:
	delete (static_cast<float *>(myProperties[prop]));
	break;
    case QicsT_QString:
	delete (static_cast<QString *>(myProperties[prop]));
	break;
    case QicsT_QColor:
	delete (static_cast<QColor *>(myProperties[prop]));
	break;
    case QicsT_Boolean:
	delete (static_cast<bool *> (myProperties[prop]));
	break;
    case QicsT_QWidget:
	// We don't free this
	break;
    case QicsT_QFont:
	delete (static_cast<QFont *> (myProperties[prop]));
	break;
    case QicsT_QColorGroup:
	delete (static_cast<QColorGroup *> (myProperties[prop]));
	break;
    case QicsT_QPalette:
	delete (static_cast<QPalette *> (myProperties[prop]));
	break;
    case QicsT_QCursor:
	delete (static_cast<QCursor *> (myProperties[prop]));
	break;
    case QicsT_QicsCellDisplay:
	// We don't free this
	break;
    case QicsT_QicsDataItemFormatter:
	// We don't free this
	break;
    case QicsT_QPixmap:
	delete (static_cast<QPixmap *> (myProperties[prop]));
	break;
    case QicsT_QValidator:
	// We don't free this
	break;
    case QicsT_QPen:
	delete (static_cast<QPen *> (myProperties[prop]));
	break;
    case QicsT_QicsRegion:
	delete (static_cast<QicsRegion *> (myProperties[prop]));
	break;
    case QicsT_Pointer:
	// We don't free this
	break;
    default: ;

    };
    myProperties[prop] =0;

    --mySetCount;
}

