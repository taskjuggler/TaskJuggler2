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


#include <QicsCellDisplay.h>
#include <QicsScreenGrid.h>
#include <QicsStyleManager.h>
#include <QicsSpanManager.h>
#include <QicsUtil.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE 
#undef CREATE_OBJS_WITH_QICSTABLE
#endif
#include <QicsCell.h>
#include <QicsMainGrid.h>
#include <QicsRow.h>
#include <QicsColumn.h>

#include <qpainter.h>
#include <qstyle.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <qfontmetrics.h>



QicsCellDisplay::QicsCellDisplay()
{
    myCell = new QicsCell(-1, -1, 0, false, 0);
    myRow = new QicsRow(-1, 0, false, 0);
    myColumn = new QicsColumn(-1, 0, false, 0);
    myGrid = new QicsMainGrid(0);
}

QicsCellDisplay::~QicsCellDisplay()
{
    delete myCell;
    delete myRow;
    delete myColumn;
    delete myGrid;
}

bool
QicsCellDisplay::handleMouseEvent(QicsScreenGrid *, int, int, QMouseEvent *)
{
    return false;
}

bool
QicsCellDisplay::handleKeyEvent(QicsScreenGrid *, int, int, QKeyEvent *)
{
    return false;
}

QString
QicsCellDisplay::tooltipText(QicsGridInfo *, int, int,
			     const QicsDataItem *, const QRect &) const
{
    return QString();
}

void
QicsCellDisplay::drawBackground(QicsGridInfo *info, int, int,
				const QRect &rect, const QColorGroup &cg,
				QPainter *painter,
				bool is_current, bool is_selected)
{
    QColor bg;

    myGrid->setInfo(info);

    // only do the special background if drawing in spreadsheet style
    is_current = is_current && (myGrid->currentCellStyle() == Qics::Spreadsheet);

    if (is_current || !is_selected)
    {
	if (info->gridType() == TableGrid)
	    bg = cg.base();
	else
	    bg = cg.background();
    }
    else
    {
	bg = cg.highlight();
    }

    painter->fillRect(rect, bg);
}

void
QicsCellDisplay::drawBorder(QicsGridInfo *info, int row, int col,
			    const QRect &rect, const QColorGroup &cg,
			    QPainter *painter,
			    bool is_current, bool)
{
    myGrid->setInfo(info);

    // only do the special border if drawing in spreadsheet style
    is_current = is_current && (myGrid->currentCellStyle() == Qics::Spreadsheet);

    if (is_current)
    {
	int bw = myGrid->currentCellBorderWidth();

	// have to offset if we have thick lines
	int tl_offset = bw / 2;
	int br_offset = 0 - ((bw % 2) ? 1 : 0);

	QRect nrect(rect);
	nrect.addCoords(tl_offset, tl_offset, br_offset, br_offset);

	QPen old_pen = painter->pen();

	painter->setPen(QPen(cg.foreground(), bw));

	painter->drawRect(nrect);

	painter->setPen(old_pen);
    }
    else
    {
	myCell->setInfo(info);
	myCell->setRowIndex(row);
	myCell->setColumnIndex(col);

	QicsLineStyle style = myCell->borderStyle();

	if (style != None)
	{
	    int line_width = myCell->borderWidth();

	    if (style == Plain)
	    {
		// have to offset if we have thick lines
		int tl_offset = line_width / 2;
		int br_offset = 0 - ((line_width % 2) ? 1 : 0);
		
		QRect nrect(rect);
		nrect.addCoords(tl_offset, tl_offset, br_offset, br_offset);

		QPen pen = myCell->borderPen();
		pen.setWidth(line_width);

		QPen old_pen = painter->pen();
		painter->setPen(pen);
		painter->drawRect(nrect);
		painter->setPen(old_pen);
	    }
	    else
	    {
		qDrawShadePanel(painter, rect, cg, (style == Sunken),
				line_width, 0);
	    }
	}
    }
}

bool
QicsCellDisplay::isCellSelected(QicsGridInfo *info, int row, int col)
{
    QicsRegion reg;
    QicsStyleManager *sm = info->styleManager();

    if ((info->gridType() == Qics::RowHeaderGrid) &&
	sm->spanManager()->isSpanner(*info, row, col, reg))
    {
	myRow->setInfo(info);

	for (int i = reg.startRow(); i <= reg.endRow(); ++i)
	{
	    myRow->setRowIndex(i);

	    if (!myRow->selected())
		return false;
	}

	return true;
    }
    else if ((info->gridType() == Qics::ColumnHeaderGrid) &&
	     sm->spanManager()->isSpanner(*info, row, col, reg))
    {
	myColumn->setInfo(info);

	for (int i = reg.startColumn(); i <= reg.endColumn(); ++i)
	{
	    myColumn->setColumnIndex(i);

	    if (!myColumn->selected())
		return false;
	}

	return true;
    }
    else
    {
	myCell->setInfo(info);
	myCell->setRowIndex(row);
	myCell->setColumnIndex(col);

	return (myCell->selected());
    }
}

QColorGroup
QicsCellDisplay::cellColorGroup(QicsGridInfo *info,
				int row, int col, bool for_printer)
{
    myCell->setInfo(info);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    QPalette pal = myCell->palette();
    bool enabled = myCell->enabled();

    QColorGroup cg = (enabled ? pal.active() : pal.disabled());

    // If we are printing, we'd like to replace the default background with
    // white.
    if (for_printer)
    {
	myGrid->setInfo(info);

	pal = myGrid->palette();

	QColorGroup def_cg = (enabled ? pal.active() : pal.disabled());

	if (cg.background() == def_cg.background())
	{
	    cg.setColor(QColorGroup::Background, Qt::white);
	}
    }

    return cg;
}

QRect
QicsCellDisplay::displayableCellArea(QicsGridInfo *info,
				     int row, int col,
				     const QRect &cr_full, 
				     bool consider_margin,
				     bool consider_border) const
{
    int line_width = 0;
    int cell_margin = 0;

    myCell->setInfo(info);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    if (consider_border)
	line_width = myCell->borderWidth();

    if (consider_margin)
	cell_margin = myCell->margin();

    // compute the area we will be drawing in by subtracting the border and margin
    QRect cr = QRect(cr_full.x() + line_width + cell_margin,
		     cr_full.y() + line_width + cell_margin,
		     cr_full.width() - (line_width *2) - (cell_margin *2),
		     cr_full.height() - (line_width *2) - (cell_margin *2) );

    return cr;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////     QicsNoWidgetCellDisplay      ////////////////////////
/////////////////////////////////////////////////////////////////////////////

QicsNoWidgetCellDisplay::QicsNoWidgetCellDisplay() :
    QicsCellDisplay()
{
}

QicsNoWidgetCellDisplay::~QicsNoWidgetCellDisplay()
{
}

void
QicsNoWidgetCellDisplay::moveEdit(QicsScreenGrid *, int, int, const QRect &)
{
}

void
QicsNoWidgetCellDisplay::hideEdit(QicsScreenGrid *)
{
}

void
QicsNoWidgetCellDisplay::startEdit(QicsScreenGrid *, int, int,
				   const QicsDataItem *)
{
}

void
QicsNoWidgetCellDisplay::endEdit(QicsScreenGrid *, int, int)
{
}

/////////////////////////////////////////////////////////////////////////////
//////////////     QicsMovableEntryWidgetCellDisplay      /////////////////////
/////////////////////////////////////////////////////////////////////////////

#include <qwidget.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qdrawutil.h>
#include <qpixmap.h>

QicsMovableEntryWidgetCellDisplay::QicsMovableEntryWidgetCellDisplay() :
    QicsCellDisplay()
{
    myEntryList.setAutoDelete(true);
}

QicsMovableEntryWidgetCellDisplay::~QicsMovableEntryWidgetCellDisplay()
{
}

void
QicsMovableEntryWidgetCellDisplay::moveEdit(QicsScreenGrid *grid,
					    int row, int col,
					    const QRect &rect)
{
    QWidget *widget = getInfoFromGrid(grid)->widget();
    QicsGridInfo *ginfo = &(grid->gridInfo());

    widget->setGeometry(entryWidgetRect(ginfo, row, col, rect));
    widget->show();
    widget->setFocus();
}


void
QicsMovableEntryWidgetCellDisplay::hideEdit(QicsScreenGrid *grid)
{
    QicsEntryWidgetInfo *info = getInfoFromGrid(grid);

    info->widget()->hide();

    info->widget()->move(-1000, -1000);

    info->setRow(-1);
    info->setColumn(-1);
}

void
QicsMovableEntryWidgetCellDisplay::endEdit(QicsScreenGrid *, int, int)
{
}

QicsMovableEntryWidgetCellDisplay::QicsEntryWidgetInfo *
QicsMovableEntryWidgetCellDisplay::getInfoFromGrid(QicsScreenGrid *grid)
{
    QicsEntryWidgetInfo *info = 0;

    for (uint i = 0; i < myEntryList.count(); ++i)
    {
	QicsEntryWidgetInfo *cur = myEntryList.at(i);
	if (cur->grid() == grid)
	{
	    info = cur;
	    break;
	}
    }

    if (!info)
    {
	QWidget *widget = newEntryWidget(grid);

	info = new QicsEntryWidgetInfo();
	info->setWidget(widget);
	info->setGrid(grid);

	myEntryList.append(info);
    }

    return info;
}


QicsMovableEntryWidgetCellDisplay::QicsEntryWidgetInfo *
QicsMovableEntryWidgetCellDisplay::getInfoFromEntry(const QWidget *widget)
{
    QicsEntryWidgetInfo *info = 0;

    for (uint i = 0; i < myEntryList.count(); ++i)
    {
	QicsEntryWidgetInfo *cur = myEntryList.at(i);
	if (cur->widget() == widget)
	{
	    info = cur;
	    break;
	}
    }

    return info;
}

QRect
QicsMovableEntryWidgetCellDisplay::entryWidgetRect(QicsGridInfo *, int, int,
						   QRect cell_rect)
{
    return cell_rect;
}
