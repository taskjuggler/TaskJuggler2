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


#include <QicsWidgetCellDisplay.h>
#include <QicsStyleManager.h>
#include <QicsDataItem.h>
#include <QicsScreenGrid.h>
#include <QicsUtil.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE 
#undef CREATE_OBJS_WITH_QICSTABLE
#endif
#include <QicsCell.h>

#include <qapplication.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>


//////////////////////////////////////////////////////////////////////
///////////      QicsWidgetCellDisplay Methods         ///////////////
//////////////////////////////////////////////////////////////////////

QicsWidgetCellDisplay::QicsWidgetCellDisplay(QWidget *widget):
    QicsCellDisplay(),
    myWidget(widget)
{
    myWidget->hide();
}

QicsWidgetCellDisplay::~QicsWidgetCellDisplay()
{
}

void
QicsWidgetCellDisplay::displayCell(QicsGrid *grid,  int row, int col,
				   const QicsDataItem *, const QRect &rect,
				   QPainter *painter)
{
    QicsGridInfo *ginfo = &(grid->gridInfo());

    myCell->setInfo(ginfo);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    bool is_current = myCell->isCurrent();

    bool for_printer = painter->device()->isExtDev();
	
    QColorGroup cg = cellColorGroup(ginfo, row, col, for_printer);

    // First draw the background and border
    drawBackground(ginfo, row, col, rect, cg, painter, is_current);
    drawBorder(ginfo, row, col, rect, cg, painter, is_current);

    // Now figure out how big the widget should be
    QRect cr = displayableCellArea(ginfo, row, col, rect);

    if (painter->device()->isExtDev())
    {
	// We are trying to print, so grab a pixmap of the widget
	// and draw it to the printer.

	QPixmap pix = QPixmap::grabWidget(myWidget);
	QString qs;

	QApplication::style().drawItem(painter, cr, 0, cg, true, &pix, qs);
    }
    else
    {
	QicsScreenGrid *sgrid = static_cast<QicsScreenGrid *> (grid);

	if (myWidget->parentWidget() != sgrid)
	    myWidget->reparent(sgrid, QPoint(0,0), false);

	myWidget->setGeometry(cr);
	myWidget->show();
    }
}


QSize
QicsWidgetCellDisplay::sizeHint(QicsGrid *, int, int, const QicsDataItem *)
{
    return myWidget->sizeHint();
}
