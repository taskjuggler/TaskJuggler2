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


#ifndef _QICSWIDGETCELLDISPLAY_H
#define _QICSWIDGETCELLDISPLAY_H

#include <QicsCellDisplay.h>

#include <qwidget.h>
#include <qpainter.h>

class QicsDataItem;




class QicsWidgetCellDisplay: public QicsCellDisplay {
public:
    
    QicsWidgetCellDisplay(QWidget *widget);

    
    virtual ~QicsWidgetCellDisplay();

    
    virtual void displayCell(QicsGrid *, int row, int col,
			     const QicsDataItem *itm, const QRect &rect,
			     QPainter *painter);

    
    inline virtual void startEdit(QicsScreenGrid *, int, int,
				  const QicsDataItem *) {;}

    
    inline virtual void moveEdit(QicsScreenGrid *, int, int, const QRect &) {;}

    inline virtual void hideEdit(QicsScreenGrid *) { myWidget->hide(); }

    
    inline virtual void endEdit(QicsScreenGrid *, int, int) {;}

    virtual QSize sizeHint(QicsGrid *grid, int row, int col,
			   const QicsDataItem *itm);

    inline virtual bool editWhenCurrent(void) const { return false; }

    inline virtual bool needsVisibilityNotification(void) const { return true; }

    inline virtual bool isEmpty(QicsGridInfo *, int, int,
				const QicsDataItem *) const
	{ return false; }

protected:
        QWidget *myWidget;
};

#endif /* _QICSWIDGETCELLDISPLAY_H */
