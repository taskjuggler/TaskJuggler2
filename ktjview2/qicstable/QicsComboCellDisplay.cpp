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


#include <QicsComboCellDisplay.h>
#include <QicsStyleManager.h>
#include <QicsDataItem.h>
#include <QicsScreenGrid.h>
#include <QicsUtil.h>
#include <QicsDataItemFormatter.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE 
#undef CREATE_OBJS_WITH_QICSTABLE
#endif
#include <QicsCell.h>

#include <qpainter.h>
#include <qstyle.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qapplication.h>


////////////////////////////////////////////////////////////////////
////////////      QicsComboCellDisplay Methods        ////////////////
////////////////////////////////////////////////////////////////////

QicsComboCellDisplay::QicsComboCellDisplay() :
    QComboBox(),
    QicsMovableEntryWidgetCellDisplay()
{
    myRedirectEvent = false;
    myAddValueToList = true;

    connect(this, SIGNAL(activated(const QString &)),
	    this, SLOT(itemSelected(const QString &)));

    myComboHolder = new QWidget();
    myFakeCombo = new QComboBox(myComboHolder);
    myFakeCombo->hide();
}

QicsComboCellDisplay::~QicsComboCellDisplay()
{
    delete myComboHolder;  // will also delete myFakeCombo
}

QWidget *
QicsComboCellDisplay::newEntryWidget(QicsScreenGrid *)
{
    return (this);
}

void QicsComboCellDisplay::displayCell(QicsGrid *grid, int row, int col,
				       const QicsDataItem *itm,
				       const QRect &rect, QPainter *painter)
{
    QicsGridInfo *ginfo = &(grid->gridInfo());

    bool for_printer = painter->device()->isExtDev();

    int mrow = ginfo->modelRowIndex(row);
    int mcol = ginfo->modelColumnIndex(col);
	
    // First, let's retrieve some cell and grid properties.
    // We dont want to query more than we need becasue each query is
    // potentially expensive.

    myCell->setInfo(ginfo);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    bool enabled = myCell->enabled();
    int flags = myCell->alignment() | myCell->textFlags();

    // setup some stuff
    painter->setFont(myCell->font());
    QColorGroup cg = cellColorGroup(ginfo, row, col, for_printer);

    QRect cr_full(rect);

    // draw the combo box

    QRect cr = displayableCellArea(ginfo, row, col, cr_full, false, false);
    QString qs = textToDisplay(ginfo, row, col, mrow, mcol, itm);

    myFakeCombo->resize(cr.width(), cr.height());

    QStyle::SFlags sflags = QStyle::Style_Default;
    if (enabled)
	sflags |= QStyle::Style_Enabled;

    painter->save();

    painter->translate(cr.left(), cr.top());

    QStyle &style = QApplication::style();

    style.drawComplexControl(QStyle::CC_ComboBox, painter, myFakeCombo,
			     myFakeCombo->rect(), cg, sflags);

    QRect trect = style.querySubControlMetrics(QStyle::CC_ComboBox,
					       myFakeCombo,
					       QStyle::SC_ComboBoxEditField);

    style.drawItem(painter, trect, flags, cg, enabled, 0, qs, -1, &(cg.foreground()));

    painter->restore();
}

void
QicsComboCellDisplay::startEdit(QicsScreenGrid *grid, int row, int col,
				const QicsDataItem *itm)
{
    QicsGridInfo *ginfo = &(grid->gridInfo());
    QicsEntryWidgetInfo *info = getInfoFromGrid(grid);
    QComboBox *widget = static_cast<QComboBox *> (info->widget());

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
    widget->setPalette(myCell->palette());

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

    QString txt = textToDisplay(ginfo,
				row, col, 
				ginfo->modelRowIndex(row),
				ginfo->modelColumnIndex(col),
				itm);

    bool found = false;
    for (int i = 0; i < widget->count(); ++i)
    {
	if (widget->text(i) == txt)
	{
	    widget->setCurrentItem(i);
	    found = true;
	    break;
	}
    }

    if (!found && myAddValueToList)
    {
	widget->insertItem(txt);
	widget->setCurrentItem(widget->count() - 1);
    }
}

bool
QicsComboCellDisplay::handleMouseEvent(QicsScreenGrid *grid, int row, int col,
				       QMouseEvent *me)
{
    QicsICell cur_cell = grid->currentCell();

    if (!cur_cell.isValid() ||
	((cur_cell.row() != row) || (cur_cell.column() != col)))
    {
	grid->editCell(row, col);
	QApplication::sendEvent(static_cast<QComboBox *> (this), me);

	return true;
    }

    return false;
}

void
QicsComboCellDisplay::keyPressEvent(QKeyEvent *ke)
{
    QicsScreenGrid *grid = static_cast<QicsScreenGrid *> (this->parent());

    bool eatEvent = false;

    if (grid->handleTraversalKeys(ke))
	eatEvent = true;

    if (!eatEvent)
	QComboBox::keyPressEvent(ke);
}

QSize
QicsComboCellDisplay::sizeHint(QicsGrid *grid, int row, int col,
			       const QicsDataItem *itm)
{
    QicsGridInfo *ginfo = &(grid->gridInfo());

    int mrow = ginfo->modelRowIndex(row);
    int mcol = ginfo->modelColumnIndex(col);
	
    myCell->setInfo(ginfo);
    myCell->setRowIndex(row);
    myCell->setColumnIndex(col);

    QString qs = textToDisplay(ginfo, row, col, mrow, mcol, itm);

    myFakeCombo->setFont(myCell->font());

    myFakeCombo->insertItem(qs);
    myFakeCombo->setCurrentItem(myFakeCombo->count() - 1);

    return (myFakeCombo->sizeHint());
}
//////////////// Subclassable methods //////////////////////

QString
QicsComboCellDisplay::textToDisplay(QicsGridInfo *ginfo, int row, int col,
				    int, int, const QicsDataItem *itm) const
{
    myCell->setInfo(ginfo);
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

    return (qs);
}

void
QicsComboCellDisplay::valueChanged(QicsGridInfo *, int, int, int, int,
				   const QString &val)
{
    QicsEntryWidgetInfo *info = getInfoFromEntry(this);

    QicsDataString itm(val);

    info->grid()->gridInfo().setCurrentCellValue(itm);
}

bool
QicsComboCellDisplay::addValueToList(void) const
{
    return myAddValueToList;
}

void
QicsComboCellDisplay::setAddValueToList(bool set)
{
    myAddValueToList = set;
}

////////////// Slots /////////////////

void
QicsComboCellDisplay::itemSelected(const QString &val)
{
    QicsEntryWidgetInfo *info = getInfoFromEntry(this);
    QicsGridInfo *ginfo = &(info->grid()->gridInfo());

    valueChanged(ginfo,
		 ginfo->visualRowIndex(info->row()),
		 ginfo->visualColumnIndex(info->column()),
		 info->row(),
		 info->column(),
		 val);
}

#include "QicsComboCellDisplay.moc"
