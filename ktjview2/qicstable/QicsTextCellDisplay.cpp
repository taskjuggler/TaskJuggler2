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


#include <QicsTextCellDisplay.h>
#include <QicsStyleManager.h>
#include <QicsDataItem.h>
#include <QicsScreenGrid.h>
#include <QicsDataItemFormatter.h>
#include <QicsUtil.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE 
#undef CREATE_OBJS_WITH_QICSTABLE
#endif
#include <QicsCell.h>
#include <QicsMainGrid.h>

#include <qapplication.h>
#include <qdrawutil.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <qvalidator.h>
#include <qfontmetrics.h>


//////////////////////////////////////////////////////////////////////
////////////      QicsTextCellDisplay Methods         ////////////////
//////////////////////////////////////////////////////////////////////

QicsTextCellDisplay::QicsTextCellDisplay():
    QObject(),
    QicsMovableEntryWidgetCellDisplay()
{
    myLastEvent = 0;
}

QicsTextCellDisplay::~QicsTextCellDisplay()
{
}

void QicsTextCellDisplay::displayCell(QicsGrid *grid, int row, int col,
				      const QicsDataItem *itm,
				      const QRect &rect, QPainter *painter)
{
    QicsGridInfo *ginfo = &(grid->gridInfo());

    bool for_printer = painter->device()->isExtDev();
	
    // First, let's retrieve some cell and grid properties.
    // We dont want to query more than we need because each query is
    // potentially expensive.

    // The cell properties
    myCell->setInfo(ginfo);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    bool is_selected = isCellSelected(ginfo, row, col);
    bool enabled = myCell->enabled();
    bool is_current = myCell->isCurrent();
    int cell_alignment = myCell->alignment();
    int flags = cell_alignment | myCell->textFlags();
    QFont font = myCell->font();

    // The Grid Properties
    myGrid->setInfo(ginfo);

    QicsGridCellClipping clipping = myGrid->gridCellClipping();

    //
    // Get the pixmap and text(if any) to display
    //
    QPixmap pix = pixmapToDisplay(ginfo, row, col, itm);
    QString qs = textToDisplay(ginfo, row, col, itm);

    //
    // Figure out all the dimensions
    //
    int pix_width = (pix.isNull() ? 0 : pix.width() + myCell->pixmapSpacing());

    // This region technically may not be accurate, if this cell is a spanner.
    // However, it won't matter, because if/when we ask for an overflow,
    // the grid will say no because (row, col) is a spanner cell.
    QicsRegion area(row, col, row, col);

    QRect cr_full(rect);

    // Ok, now it's time to determine the rectangle that we are going to draw
    // into.  Yes, a rect has been passed to this method, but that's only
    // the starting point.  We want to check if we can display the entire
    // string in our given rect.  If not, we will try to overflow into the
    // next cell.

    bool ready_to_draw = false;
    bool accept_overflow = false;
    bool can_display_all = true;

    //
    // No need to try to overflow if this cell is empty.
    //
    if (qs.isEmpty())
    {
	ready_to_draw = true;
    }

    while (!ready_to_draw)
    {
	if (!canDisplayAll(ginfo, cr_full, row, col, qs, flags, font, pix))
	{
	    // Not enough room, so we ask for more...

	    QicsRegion new_area;
	    QRect new_rect;

	    if (grid->requestCellOverflow(area, cr_full, new_area, new_rect))
	    {
		// more room is available
		area = new_area;
		cr_full = new_rect;
		accept_overflow = true;
	    }
	    else
	    {
		// the grid won't give us any more space, so we're done
		ready_to_draw = true;
		can_display_all = false;
	    }
	}
	else
	    // we fit, so we're ready to go
	    ready_to_draw = true;
    }

    // This notifies the grid that we will accept the proposed overflow.
    if (accept_overflow)
	grid->acceptCellOverflow(area);

    // setup some stuff
    painter->setFont(font);
    QColorGroup cg = cellColorGroup(ginfo, row, col, for_printer);

    // get the right foreground and background colors
    QColor bg, fg;
    if (is_current || !is_selected)
    {
	if (ginfo->gridType() == TableGrid)
	{
	    bg = cg.base();
	    fg = cg.text();
	}
	else
	{
	    bg = cg.background();
	    fg = cg.foreground();
	}
    }
    else
    {
	bg = cg.highlight();
	fg = cg.highlightedText();
    }

    // Finally, we get to draw.  First, draw the background and the border.
    drawBackground(ginfo, row, col, cr_full, cg, painter,
		   is_current, is_selected);
    drawBorder(ginfo, row, col, cr_full, cg, painter,
	       is_current, is_selected);

    // We only display the text and/or pixmap if the entire cell can be drawn or
    // partial cell display is allowed
    if (can_display_all || (clipping != NoDisplayOnPartial))
    {
	QRect cr = displayableCellArea(ginfo, row, col, cr_full);

	//
	// Draw the pixmap...
	//
	if (!pix.isNull())
	{
	    // Because of a Qt bug, we need to "intersect" the cell area
	    // with any clip mask that may be set on the painter.

	    QRect pix_rect = cr;
	    QRegion reg = painter->clipRegion();

	    if (!reg.isNull())
	    {
		pix_rect.setRight(QICS_MIN(cr.right(), reg.boundingRect().right()));
		pix_rect.setBottom(QICS_MIN(cr.bottom(), reg.boundingRect().bottom()));
	    }

	    // If we have both a pixmap and a string, the pixmap is always
	    // displayed on the left
	    
	    int alignment;
	    
	    if (!qs.isEmpty())
		// preserve the vertical alignment, change horizontal
		// alignment to left
		alignment = (cell_alignment & AlignVertical_Mask) | Qt::AlignLeft;
	    else
		alignment = cell_alignment;

	    QApplication::style().drawItem(painter, 
					   pix_rect,
					   alignment,
					   cg,
					   enabled,
					   (pix.isNull() ? 0 : &pix),
					   QString(),
					   -1,
					   &fg);

	    // And, because of another Qt bug, we have to reset the painter's
	    // clip region after drawing the pixmap (drawItem() appears to
	    // modify the clip region)

	    if (!reg.isNull())
		painter->setClipRegion(reg);
	}

	//
	// Draw the text...
	//
	if (!qs.isEmpty())
	{
	    QRect str_rect(QPoint(cr.left() + pix_width, cr.top()),
			   cr.bottomRight());

	    QApplication::style().drawItem(painter, 
					   str_rect,
					   flags,
					   cg,
					   enabled,
					   0,
					   qs,
					   -1,
					   &fg);

	    if (!can_display_all && (clipping == UseClippedSymbol))
	    {
		// mark that some of the data isn't displayed...

		QPixmap clip_pix = myGrid->moreTextPixmap();

		if (!clip_pix.isNull())
		{
		    QRect marker_size(cr.x() + cr.width() - clip_pix.width(), cr.y(),
				      clip_pix.width(), cr.height());

		    painter->fillRect(marker_size, bg);
		    QApplication::style().drawItem(painter,
						   marker_size,
						   Qt::AlignCenter,
						   cg,
						   enabled,
						   &clip_pix,
						   QString(),
						   -1,
						   &fg);
		}
	    }
	}
    }
}


void
QicsTextCellDisplay::startEdit(QicsScreenGrid *grid, int row, int col,
			       const QicsDataItem *itm)
{
    QicsGridInfo *ginfo = &(grid->gridInfo());
    QicsEntryWidgetInfo *info = getInfoFromGrid(grid);
    QLineEdit *widget = static_cast<QLineEdit *> (info->widget());

    info->setRow(ginfo->modelRowIndex(row));
    info->setColumn(ginfo->modelColumnIndex(col));

    info->setItem(itm ? itm->clone() : 0);

    // Setup widget (colors, fonts, values etc) for this cell

    widget->clear();

    myCell->setInfo(ginfo);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    myGrid->setInfo(ginfo);

    widget->setFont(myCell->font());
    widget->setPalette(myCell->palette());
    widget->setValidator(myCell->validator());
    widget->setMaxLength(myCell->maxLength());

    // This is a hack to fix the "flashing" problem that occurs when the
    // widget moves between cells.  Without this, there is a brief instant
    // where the old cell value is visible before the new value is painted.
    // You can't just call erase() because Qt is too smart -- it doesn't bother
    // to erase windows that are hidden.  So, we move the widget off screen,
    // show it, erase it, and hide it again.

    widget->move(-1000, -1000);
    widget->show();
    widget->erase();
    widget->hide();


    // Set the text of the entry widget to the underlying value
    QString valString;

    if (itm)
	valString = itm->string();
    else
	valString = "";

    widget->setText(valString);

    if (myGrid->autoSelectCellContents())
	widget->selectAll();
}

void
QicsTextCellDisplay::endEdit(QicsScreenGrid *grid, int, int)
{
    setValue(getInfoFromGrid(grid));
}

bool 
QicsTextCellDisplay::handleKeyEvent(QicsScreenGrid *grid, int row, int col,
				    QKeyEvent *ke)
{
    if (myLastEvent == ke)
	return false;

    QicsEntryWidgetInfo *info = getInfoFromGrid(grid);
    QicsGridInfo *ginfo = &(grid->gridInfo());

    if ((info->row() == ginfo->modelRowIndex(row)) && 
	(info->column() == ginfo->modelRowIndex(col)))
    {
	QApplication::sendEvent(info->widget(), ke);
        return true;
    }

     return false;
}

QSize
QicsTextCellDisplay::sizeHint(QicsGrid *grid, int row, int col,
			      const QicsDataItem *itm)
{
    QicsGridInfo *ginfo = &(grid->gridInfo());
    
    QPixmap pix = pixmapToDisplay(ginfo, row, col, itm);
    QString text = textToDisplay(ginfo, row, col, itm);

    myCell->setInfo(ginfo);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    int tflags = myCell->alignment() | myCell->textFlags();
    QFontMetrics fm(myCell->font());
    bool wordbreak = tflags | Qt::WordBreak;

    QSize sz(0,0);

    int xoffset = 0;

    // get the pixmap dimensions
    if (!pix.isNull())
    {
	xoffset = pix.width() + myCell->pixmapSpacing();

	sz.setWidth(sz.width() + xoffset);
	sz.setHeight(sz.height() + pix.height());
    }

    if (!wordbreak && (text.find('\n') == -1))
    {
	// If we aren't breaking lines, or if there's no newline in the
	// string, this is easy...

	sz.setWidth(sz.width() + fm.width(text));
	sz.setHeight(sz.height() + fm.height());
    }
    else
    {
	// If we do expect more than one line, then we really don't
	// care how wide we are.  So we say that we want to be as
	// wide as the column is now.  Then, once we have that width
	// fixed, we calculate how tall the cell must be in order to
	// display the whole string.

	QRect cr = QRect(0, 0, myCell->widthInPixels(), myCell->heightInPixels());
	QRect dar = displayableCellArea(ginfo, row, col, cr);
	
	// the area left for the string
	QRect str_rect(QPoint(dar.left() + xoffset, 0),
		       QPoint(dar.right(), 0));

	QRect br = fm.boundingRect(str_rect.left(), str_rect.top(),
				   str_rect.width(), str_rect.height(),
				   tflags, text);

	sz.setWidth(sz.width() + br.width());
	sz.setHeight(QICS_MAX(sz.height(), br.height()));
    }

    // add the space for the cell border
    int edge_size = myCell->borderWidth() + myCell->margin();

    sz.setWidth(sz.width() + (2 * edge_size));
    sz.setHeight(sz.height() + (2 * edge_size));

    return sz.expandedTo(QApplication::globalStrut());
}

bool
QicsTextCellDisplay::isEmpty(QicsGridInfo *info, int row, int col,
			     const QicsDataItem *itm) const
{
    QPixmap pix = pixmapToDisplay(info, row, col, itm);
    QString text = textToDisplay(info, row, col, itm);

    return (pix.isNull() && text.isEmpty());
}

QString
QicsTextCellDisplay::tooltipText(QicsGridInfo *info, int row, int col,
				 const QicsDataItem *itm, const QRect &rect) const
{
    QString ttext;
    QPixmap pix = pixmapToDisplay(info, row, col, itm);
    QString text = textToDisplay(info, row, col, itm);

    myCell->setInfo(info);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    if (!canDisplayAll(info, rect, row, col, text,
		       (myCell->alignment() | myCell->textFlags()),
		       myCell->font(), pix))
	ttext = text;

    return (ttext);
}

QWidget *
QicsTextCellDisplay::newEntryWidget(QicsScreenGrid *grid)
{
    QLineEdit *widget = new QLineEdit(grid);

    widget->setFrameStyle(QFrame::NoFrame);

    widget->installEventFilter(this);

    return (widget);
}

QString
QicsTextCellDisplay::textToDisplay(QicsGridInfo *info, int row, int col,
				   const QicsDataItem *itm) const
{
    // NOTE:: the data item pointer could be 0!

    myCell->setInfo(info);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    QString qs = myCell->label();

    if (qs.isEmpty() && itm)
    {
	QicsDataItemFormatter *formatter = myCell->formatter();

	if (formatter)
	{
	    qs = formatter->format(*itm);
	}
	else
	    qs = QString(itm->string());
    }

    return qs;
}

QPixmap
QicsTextCellDisplay::pixmapToDisplay(QicsGridInfo *info, int row, int col,
				     const QicsDataItem *) const
{
    // NOTE:: the data item pointer could be 0!

    myCell->setInfo(info);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    return (myCell->pixmap());
}

bool
QicsTextCellDisplay::canDisplayAll(QicsGridInfo *info,
				   const QRect &rect, int row, int col,
				   const QString &text, int text_flags,
				   const QFont &font,
				   const QPixmap &pix) const
{
    myCell->setInfo(info);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    int pix_width;
    int pix_height;

    // get the pixmap dimensions
    if (pix.isNull())
    {
	pix_width = 0;
	pix_height = 0;
    }
    else
    {
	pix_width = pix.width() + myCell->pixmapSpacing();
	pix_height = pix.height();
    }

    QRect cr = displayableCellArea(info, row, col, rect);
	
    // the area left for the string
    QRect str_rect(QPoint(cr.left() + pix_width, cr.top()),
		   cr.bottomRight());

    QFontMetrics fm(font);

    QRect br = fm.boundingRect(str_rect.left(), str_rect.top(),
			       str_rect.width(), str_rect.height(),
			       text_flags, text);

    return ((br.width() <= (str_rect.width() + 1)) &&
	    (br.height() <= (str_rect.height() + 1)));
}

bool QicsTextCellDisplay::eventFilter( QObject *o, QEvent *e )
{
    QicsEntryWidgetInfo *info = getInfoFromEntry((static_cast<QLineEdit *> (o)));
    QicsScreenGrid *grid = info->grid();

    bool eatEvent = false;

    if ( e->type() == QEvent::KeyPress ) {

	// special processing for key press
	QKeyEvent *k = static_cast<QKeyEvent *> (e);

	if (grid->handleTraversalKeys(k))
	    eatEvent = true;
	else
	{
	    switch (k->key())
	    {
	    case Qt::Key_Escape:
		resetValue(info);
		eatEvent = true;
		break;
	    }
	}
    }

    myLastEvent = e;

    if (eatEvent)
	return (true);
    else
	return QObject::eventFilter( o, e );
}

bool
QicsTextCellDisplay::setValue(QicsEntryWidgetInfo *info)
{
    if (!info)
	return false;

    QicsScreenGrid *grid = info->grid();

    QLineEdit *widget = static_cast<QLineEdit *> (info->widget());

    QString text = widget->text();

    // If the value hasn't changed, return false.
    if (info->item() && (info->item()->string() == text))
	return false;

    int pos = 0;

    bool valid = true;
    if (widget->validator() && (!widget->validator()->validate(text, pos)))
	valid = false;

    if (valid)
    {
	QicsDataItem *item;

	if (info->item())
	{
	    info->item()->setString(text);
	    item = info->item();
	}
	else
	    // XXX try to figure out what kind of data item  this is?
	    item = new QicsDataString(text);

	grid->gridInfo().setCurrentCellValue(*item);

	return true;
    }
    else
	return false;
}

void
QicsTextCellDisplay::resetValue(QicsEntryWidgetInfo *info)
{
    if (!info)
	return;

    QString valString;

    if (info->item())
	valString = info->item()->string();
    else
	valString = "";

    QLineEdit *widget = static_cast<QLineEdit *> (info->widget());
    widget->setText(valString);

    QicsStyleManager *sm = info->grid()->gridInfo().styleManager();
    const bool auto_select = * static_cast<bool*>
	(sm->getGridProperty(QicsGridStyle::AutoSelectCellContents));

    if (auto_select)
	widget->selectAll();
}

#include "QicsTextCellDisplay.moc"
