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


#include <QicsCheckCellDisplay.h>
#include <QicsStyleManager.h>
#include <QicsDataItem.h>
#include <QicsScreenGrid.h>
#include <QicsUtil.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE 
#undef CREATE_OBJS_WITH_QICSTABLE
#endif
#include <QicsCell.h>

#include <qpainter.h>
#include <qstyle.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qapplication.h>

#define QICS_CHECK_INDICATOR_SPACING 5

////////////////////////////////////////////////////////////////////
////////////      QicsCheckCellDisplay Methods        ////////////////
////////////////////////////////////////////////////////////////////

QicsCheckCellDisplay::QicsCheckCellDisplay() :
    QCheckBox(0, 0),
    QicsMovableEntryWidgetCellDisplay()
{
    myRedirectEvent = false;

    connect(this, SIGNAL(stateChanged(int)),
	    this, SLOT(checkStateChanged(int)));
}

QicsCheckCellDisplay::~QicsCheckCellDisplay()
{
}

QWidget *
QicsCheckCellDisplay::newEntryWidget(QicsScreenGrid *)
{
    return (static_cast<QCheckBox *> (this));
}

QRect
QicsCheckCellDisplay::entryWidgetRect(QicsGridInfo *ginfo, int row, int col,
				      QRect cell_rect)
{
    return (displayableCellArea(ginfo, row, col, cell_rect));
}

void QicsCheckCellDisplay::displayCell(QicsGrid *grid, int row, int col,
				       const QicsDataItem *itm,
				       const QRect &rect, QPainter *painter)
{
    QicsGridInfo *ginfo = &(grid->gridInfo());

    bool for_printer = painter->device()->isExtDev();

    // First, let's retrieve some cell and grid properties.
    // We dont want to query more than we need becasue each query is
    // potentially expensive.

    myCell->setInfo(ginfo);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    bool enabled = myCell->enabled();
    bool is_current = myCell->isCurrent();
    int cell_alignment = myCell->alignment();
    int flags = cell_alignment | myCell->textFlags();

    // setup some stuff
    painter->setFont(myCell->font());
    QColorGroup cg = cellColorGroup(ginfo, row, col, for_printer);

    QColor fg;
    if (is_current || !isCellSelected(ginfo, row, col))
    {
	if (ginfo->gridType() == TableGrid)
	    fg = cg.text();
	else
	    fg = cg.foreground();
    }
    else
	fg = cg.highlightedText();

    // Finally, we get to draw.  First, draw the background and the border.
    drawBackground(ginfo, row, col, rect, cg, painter, is_current, myCell->selected());
    drawBorder(ginfo, row, col, rect, cg, painter, is_current, myCell->selected());

    QRect cr = displayableCellArea(ginfo, row, col, rect);

    int mrow = ginfo->modelRowIndex(row);
    int mcol = ginfo->modelColumnIndex(col);

    // the stuff to display
    QString qs = textToDisplay(ginfo, row, col, mrow, mcol, itm);

    QStyle &style = QApplication::style();

    QSize sz = QSize(style.pixelMetric(QStyle::PM_IndicatorWidth),
		     style.pixelMetric(QStyle::PM_IndicatorHeight));

    QStyle::SFlags style_flags = QStyle::Style_Default;
    if (checkState(ginfo, row, col, mrow, mcol, itm))
	style_flags |= QStyle::Style_On;
    else
	style_flags |= QStyle::Style_Off;
    
    if (enabled)
	style_flags |= QStyle::Style_Enabled;

    if (is_current)
	style_flags |= QStyle::Style_HasFocus;

    style.drawPrimitive(QStyle::PE_Indicator,
			painter,     // painter
			QRect(cr.left(),
			      cr.top() + (cr.height() - sz.height()) / 2,
			      sz.width(),
			      sz.height()),
			cg,
			style_flags);

    
    int new_x = cr.left() + sz.width() + QICS_CHECK_INDICATOR_SPACING;
    int new_width = cr.width() - (sz.width() + QICS_CHECK_INDICATOR_SPACING);

    // Draw the pixmap

#ifdef QICS_CHECKCELL_DRAW_PIXMAP
    QPixmap pix = pixmapToDisplay(ginfo, row, col, mrow, mcol, itm);

    if (!pix.isNull())
    {
	QRect pix_rect(new_x, cr.top(), new_width, cr.height());;
	new_x += (pix.width() + myCell->pixmapSpacing());
	new_width -= (pix.width() + 6);

	// Because of a Qt bug, we need to intersect the cell area
	// with any clip mask that may be set on the painter.

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
#endif

    if (!qs.isEmpty())
    {
	QRect str_rect(new_x, cr.top(), new_width, cr.height());

	style.drawItem(painter,     // painter
		       str_rect,
//		       (AlignLeft | AlignVCenter),
		       flags,
		       cg,
		       enabled,
		       0,
		       qs,
		       -1,
		       &fg);
    }
}

void
QicsCheckCellDisplay::startEdit(QicsScreenGrid *grid, int row, int col,
				const QicsDataItem *itm)
{
    QicsGridInfo *ginfo = &(grid->gridInfo());
    QicsEntryWidgetInfo *info = getInfoFromGrid(grid);
    QCheckBox *widget = static_cast<QCheckBox *> (info->widget());

    if (widget->parent() != grid)
    {
	widget->reparent(grid, QPoint(0,0), false);
    }

    info->setRow(ginfo->modelRowIndex(row));
    info->setColumn(ginfo->modelColumnIndex(col));

    // Setup widget (colors, fonts, values etc) for this cell

    myCell->setInfo(ginfo);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    widget->setFont(myCell->font());

    // Unfortunately, we really want to use the Base color as our
    // background color, so we need to whack this a bit...

    QPalette p = myCell->palette();
    p.setColor(QPalette::Active, QColorGroup::Background,
	       p.active().base());
    p.setColor(QPalette::Inactive, QColorGroup::Background,
	       p.inactive().base());
    p.setColor(QPalette::Disabled, QColorGroup::Background,
	       p.disabled().base());

    widget->setPalette(p);

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

    int mrow = ginfo->modelRowIndex(row);
    int mcol = ginfo->modelColumnIndex(col);

    widget->setText(textToDisplay(ginfo, row, col, mrow, mcol, itm));
#ifdef QICS_CHECKCELL_DRAW_PIXMAP
    widget->setPixmap(pixmapToDisplay(ginfo, row, col, mrow, mcol, itm));
#endif
    widget->setChecked(checkState(ginfo, row, col, mrow, mcol, itm));
}

bool
QicsCheckCellDisplay::handleMouseEvent(QicsScreenGrid *grid, int row, int col,
				       QMouseEvent *me)
{
    QicsICell cur_cell = grid->currentCell();

    if (!cur_cell.isValid() ||
	((cur_cell.row() != row) || (cur_cell.column() != col)))
    {
	grid->editCell(row, col);
    }

    QRect r = grid->cellDimensions(row, col, false);
    QPoint p(me->x() - r.left(), me->y() - r.top());
    QMouseEvent new_event(me->type(), p, me->button(), me->state());

    QicsEntryWidgetInfo *info = getInfoFromGrid(grid);
    QApplication::sendEvent(info->widget(), &new_event);

    return true;
}

void
QicsCheckCellDisplay::keyPressEvent(QKeyEvent *ke)
{
    QicsScreenGrid *grid = static_cast<QicsScreenGrid *> (this->parent());

    bool eatEvent = false;

    if (ke->type() == QEvent::KeyPress)
    {
	// special processing for key press

	if (grid->handleTraversalKeys(ke))
	    eatEvent = true;
    }

    if (!eatEvent)
	QCheckBox::keyPressEvent(ke);
}

QSize
QicsCheckCellDisplay::sizeHint(QicsGrid *grid, int row, int col,
			       const QicsDataItem *itm)
{
    QicsGridInfo *ginfo = &(grid->gridInfo());

    myCell->setInfo(ginfo);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    QStyle &style = QApplication::style();

    QString text = textToDisplay(ginfo, row, col,
			       ginfo->modelRowIndex(row),
			       ginfo->modelColumnIndex(col), itm);
    
    int xoffset = 0;

    QSize sz = QSize(style.pixelMetric(QStyle::PM_IndicatorWidth),
		     style.pixelMetric(QStyle::PM_IndicatorHeight));

    // spacing between indicator and text
    sz.setWidth(sz.width() + QICS_CHECK_INDICATOR_SPACING);

    xoffset += sz.width();

#ifdef QICS_CHECKCELL_DRAW_PIXMAP
    // get the pixmap dimensions

    QPixmap pix = pixmapToDisplay(ginfo, row, col,
				  ginfo->modelRowIndex(row),
				  ginfo->modelColumnIndex(col),
				  itm);
    if (!pix.isNull())
    {
	int pix_margin = myCell->pixmapSpacing();
	sz.setWidth(sz.width() + pix.width() + pix_margin);
	sz.setHeight(QICS_MAX(sz.height(), pix.height());

	xoffset += pix.width() + pix_margin;
    }
#endif

    int tflags = myCell->alignment() | myCell->textFlags();
    QFontMetrics fm(myCell->font());
    bool wordbreak = tflags | Qt::WordBreak;

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

//////////////// Subclassable methods //////////////////////

bool
QicsCheckCellDisplay::checkState(QicsGridInfo *, int, int, int, int,
				 const QicsDataItem *)
{
    
    return false;
}

QString
QicsCheckCellDisplay::textToDisplay(QicsGridInfo *ginfo, int row, int col,
				    int, int, const QicsDataItem *)
{
    myCell->setInfo(ginfo);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    return (myCell->label());
}

QPixmap
QicsCheckCellDisplay::pixmapToDisplay(QicsGridInfo *ginfo, int row, int col,
				      int, int, const QicsDataItem *)
{
    myCell->setInfo(ginfo);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    return (myCell->pixmap());
}

void
QicsCheckCellDisplay::valueChanged(QicsGridInfo *, int, int, int, int, bool)
{
}

////////////// Slots /////////////////

void
QicsCheckCellDisplay::checkStateChanged(int state)
{
    const QCheckBox *widget = static_cast<const QCheckBox *> (sender());
    QicsEntryWidgetInfo *info = getInfoFromEntry(widget);
    QicsGridInfo *ginfo = &(info->grid()->gridInfo());

    valueChanged(ginfo,
		 ginfo->visualRowIndex(info->row()),
		 ginfo->visualColumnIndex(info->column()),
		 info->row(),
		 info->column(),
		 (state == 2 ? true : false));
}
