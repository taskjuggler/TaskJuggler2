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


#ifndef _QICSTEXTCELLDISPLAY_H
#define _QICSTEXTCELLDISPLAY_H

#include <QicsCellDisplay.h>

#include <qpainter.h>
#include <qlineedit.h>



class QicsGrid;
class QicsGridInfo;


class QicsTextCellDisplay: public QObject,
				       public QicsMovableEntryWidgetCellDisplay
{
Q_OBJECT

public:
    
    QicsTextCellDisplay();
    virtual ~QicsTextCellDisplay();

    virtual void displayCell(QicsGrid *grid, int row, int col,
			     const QicsDataItem *itm,
			     const QRect &rect, QPainter *painter);
    virtual void startEdit(QicsScreenGrid *, int row, int col,
			   const QicsDataItem *itm);

    virtual void endEdit(QicsScreenGrid *, int row, int col);

    inline virtual bool editWhenCurrent(void) const { return false; }

    virtual bool isEmpty(QicsGridInfo *info, int row, int col,
			 const QicsDataItem *itm) const;

    
    virtual bool handleKeyEvent(QicsScreenGrid *grid, int row, int col,
				QKeyEvent *ke);

    virtual QSize sizeHint(QicsGrid *grid, int row, int col,
			   const QicsDataItem *itm);

    virtual QString tooltipText(QicsGridInfo *info, int row, int col,
				const QicsDataItem *itm, const QRect &rect) const; 

protected:
    QWidget *newEntryWidget(QicsScreenGrid *grid);

    
    virtual QString textToDisplay(QicsGridInfo *info, int row, int col,
				  const QicsDataItem *itm) const;

    
    virtual QPixmap pixmapToDisplay(QicsGridInfo *info, int row, int col,
				    const QicsDataItem *itm) const;

    
    virtual bool canDisplayAll(QicsGridInfo *info,
			       const QRect &rect, int row, int col,
			       const QString &text, int text_flags,
			       const QFont &font,
			       const QPixmap &pix) const;

    
    virtual bool eventFilter(QObject *o, QEvent *e);

    
    virtual bool setValue(QicsEntryWidgetInfo *info);

    
    virtual void resetValue(QicsEntryWidgetInfo *info);

        QEvent *myLastEvent;
};

#endif /* _QICSTEXTCELLDISPLAY_H */
