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


#include <QicsGridCommon.h>
#include <QicsStyleManager.h>
#include <QicsSpanManager.h>
#include <QicsDimensionManager.h>
#include <QicsGridInfo.h>
#include <QicsScreenGrid.h>
#include <QicsUtil.h>

#include <qpen.h>
#include <qpixmap.h>

////////////////////////////////////////////////////////////////////////////////

QicsGridCommon::QicsGridCommon(QicsGridInfo *info, QObject *parent,
			       bool forward_signals) :
    QicsCellCommon(info, parent),
    myForwardSignals(forward_signals)
{
}

QicsGridCommon::QicsGridCommon(QObject *parent, bool forward_signals) :
    QicsCellCommon(parent),
    myForwardSignals(forward_signals)
{
}

QicsGridCommon::~QicsGridCommon()
{
}


void
QicsGridCommon::setAttr(QicsCellStyle::QicsCellStyleProperty attr, const void *val)
{
    styleManager().setDefaultProperty(attr, val);
}

void *
QicsGridCommon::getAttr(QicsCellStyle::QicsCellStyleProperty attr) const
{
    return (styleManager().getDefaultProperty(attr));
}

void
QicsGridCommon::clearAttr(QicsCellStyle::QicsCellStyleProperty attr)
{
    styleManager().setDefaultProperty(attr, static_cast<void *> (0));
}

void
QicsGridCommon::setGridAttr(QicsGridStyle::QicsGridStyleProperty attr, const void *val)
{
    styleManager().setGridProperty(attr, val);
}

void *
QicsGridCommon::getGridAttr(QicsGridStyle::QicsGridStyleProperty attr) const
{
    return (styleManager().getGridProperty(attr));
}

void
QicsGridCommon::clearGridAttr(QicsGridStyle::QicsGridStyleProperty attr)
{
    styleManager().setGridProperty(attr, static_cast<void *> (0));
}

//////////////////////////////////////////////////////////////////

void
QicsGridCommon::initSignals(void)
{
    if (!myForwardSignals)
	return;

    if (myInfo)
    {
	QicsGridInfo::QicsScreenGridPV grids = myInfo->grids();
	QicsGridInfo::QicsScreenGridPV::iterator iter;

	for (iter = grids.begin(); iter != grids.end(); ++iter)
	    connectGrid(*iter);

	connect(myInfo, SIGNAL(gridAdded(QicsScreenGrid *)),
		this, SLOT(connectGrid(QicsScreenGrid *)));
	connect(myInfo, SIGNAL(gridRemoved(QicsScreenGrid *)),
		this, SLOT(disconnectGrid(QicsScreenGrid *)));
	connect(myInfo, SIGNAL(cellValueChanged(int, int)),
		this, SIGNAL(valueChanged(int, int)));
    }
}

void
QicsGridCommon::setInfo(QicsGridInfo *info)
{
    if (info == myInfo)
	return;

    if (myInfo && myForwardSignals)
    {
	QicsGridInfo::QicsScreenGridPV grids = myInfo->grids();
	QicsGridInfo::QicsScreenGridPV::iterator iter;

	for (iter = grids.begin(); iter != grids.end(); ++iter)
	    disconnectGrid(*iter);
    }

    QicsCellCommon::setInfo(info);

    initSignals();
}

void
QicsGridCommon::connectGrid(QicsScreenGrid *grid)
{
    connect(grid, SIGNAL(pressed(int, int, int, const QPoint &)),
	    this, SLOT(handleGridPress(int, int, int, const QPoint &)));
    connect(grid, SIGNAL(clicked(int, int, int, const QPoint &)),
	    this, SLOT(handleGridClick(int, int, int, const QPoint &)));
    connect(grid, SIGNAL(doubleClicked(int, int, int, const QPoint &)),
	    this, SLOT(handleGridDoubleClick(int, int, int, const QPoint &)));
}

void
QicsGridCommon::disconnectGrid(QicsScreenGrid *grid)
{
    disconnect(grid, 0, this, 0);
}

void
QicsGridCommon::handleGridPress(int row, int col, int button, const QPoint &pos)
{
    if (!myInfo)
	return;

    QicsScreenGrid *grid = 0;

    QicsGridInfo::QicsScreenGridPV grids = myInfo->grids();
    QicsGridInfo::QicsScreenGridPV::iterator iter;

    for (iter = grids.begin(); iter != grids.end(); ++iter)
    {
	if (*iter == sender())
	    {
		grid = *iter;
		break;
	    }
    }

    if (grid)
    {
	emit pressed(row, col, button, grid->mapToParent(pos));
    }
}

void
QicsGridCommon::handleGridClick(int row, int col, int button, const QPoint &pos)
{
    if (!myInfo)
	return;

    QicsScreenGrid *grid = 0;

    QicsGridInfo::QicsScreenGridPV grids = myInfo->grids();
    QicsGridInfo::QicsScreenGridPV::iterator iter;

    for (iter = grids.begin(); iter != grids.end(); ++iter)
    {
	if (*iter == sender())
	    {
		grid = *iter;
		break;
	    }
    }

    if (grid)
    {
	emit clicked(row, col, button, grid->mapToParent(pos));
    }
}

void
QicsGridCommon::handleGridDoubleClick(int row, int col, int button,
				      const QPoint &pos)
{
    if (!myInfo)
	return;

    QicsScreenGrid *grid = 0;

    QicsGridInfo::QicsScreenGridPV grids = myInfo->grids();
    QicsGridInfo::QicsScreenGridPV::iterator iter;

    for (iter = grids.begin(); iter != grids.end(); ++iter)
    {
	if (*iter == sender())
	    {
		grid = *iter;
		break;
	    }
    }

    if (grid)
    {
	emit doubleClicked(row, col, button, grid->mapToParent(pos));
    }
}

//////////////////////////////////////////////////////////////////

void
QicsGridCommon::setDMMargin(int margin)
{
    dimensionManager().setDefaultMargin(margin);
}

void
QicsGridCommon::setDMFont(const QFont &font)
{
    dimensionManager().setDefaultFont(font);
}

QicsRegion
QicsGridCommon::viewport(void) const
{
    return (* static_cast<QicsRegion *>
	    (getGridAttr(QicsGridStyle::Viewport)));
}

void
QicsGridCommon::setViewport(const QicsRegion &vp)
{
    setGridAttr(QicsGridStyle::Viewport, static_cast<const void *> (&vp));
}

bool
QicsGridCommon::addCellSpan(QicsSpan span)
{
    return styleManager().spanManager()->addCellSpan(span);
}

void
QicsGridCommon::removeCellSpan(int start_row, int start_col)
{
    styleManager().spanManager()->removeCellSpan(start_row, start_col);
}

QicsSpanList *
QicsGridCommon::cellSpanList(void)
{
    return styleManager().spanManager()->cellSpanList();
}

Qics::QicsGridCellClipping
QicsGridCommon::gridCellClipping(void) const
{
    return (* static_cast<QicsGridCellClipping *>
	    (getGridAttr(QicsGridStyle::GridCellClipping)));
}

void
QicsGridCommon::setGridCellClipping(QicsGridCellClipping c)
{
    setGridAttr(QicsGridStyle::GridCellClipping, static_cast<const void *> (&c));
}

bool
QicsGridCommon::drawPartialCells(void) const
{
    return (* static_cast<bool *>
	    (getGridAttr(QicsGridStyle::DrawPartialCells)));
}

void
QicsGridCommon::setDrawPartialCells(bool b)
{
    setGridAttr(QicsGridStyle::DrawPartialCells, static_cast<const void *> (&b));
}

bool
QicsGridCommon::horizontalGridLinesVisible(void) const
{
    return (* static_cast<bool *>
	    (getGridAttr(QicsGridStyle::HorizontalGridLinesVisible)));
}

void
QicsGridCommon::setHorizontalGridLinesVisible(bool b)
{
    setGridAttr(QicsGridStyle::HorizontalGridLinesVisible,
		static_cast<const void *> (&b));
}

bool
QicsGridCommon::verticalGridLinesVisible(void) const
{
    return (* static_cast<bool *>
	    (getGridAttr(QicsGridStyle::VerticalGridLinesVisible)));
}

void
QicsGridCommon::setVerticalGridLinesVisible(bool b)
{
    setGridAttr(QicsGridStyle::VerticalGridLinesVisible,
		static_cast<const void *> (&b));
}

int
QicsGridCommon::horizontalGridLineWidth(void) const
{
    return (* static_cast<int *>
	    (getGridAttr(QicsGridStyle::HorizontalGridLineWidth)));
}

void
QicsGridCommon::setHorizontalGridLineWidth(int w)
{
    setGridAttr(QicsGridStyle::HorizontalGridLineWidth,
		static_cast<const void *> (&w));
}

int
QicsGridCommon::verticalGridLineWidth(void) const
{
    return (* static_cast<int *>
	    (getGridAttr(QicsGridStyle::VerticalGridLineWidth)));
}

void
QicsGridCommon::setVerticalGridLineWidth(int w)
{
    setGridAttr(QicsGridStyle::VerticalGridLineWidth,
		static_cast<const void *> (&w));
}

Qics::QicsLineStyle
QicsGridCommon::horizontalGridLineStyle(void) const
{
    return (* static_cast<QicsLineStyle *>
	    (getGridAttr(QicsGridStyle::HorizontalGridLineStyle)));
}

void
QicsGridCommon::setHorizontalGridLineStyle(QicsLineStyle style)
{
    setGridAttr(QicsGridStyle::HorizontalGridLineStyle,
		static_cast<const void *> (&style));
}

Qics::QicsLineStyle
QicsGridCommon::verticalGridLineStyle(void) const
{
    return (* static_cast<QicsLineStyle *>
	    (getGridAttr(QicsGridStyle::VerticalGridLineStyle)));
}

void
QicsGridCommon::setVerticalGridLineStyle(QicsLineStyle style)
{
    setGridAttr(QicsGridStyle::VerticalGridLineStyle,
		static_cast<const void *> (&style));
}

QPen
QicsGridCommon::horizontalGridLinePen(void) const
{
    return (* static_cast<QPen *>
	    (getGridAttr(QicsGridStyle::HorizontalGridLinePen)));
}

void
QicsGridCommon::setHorizontalGridLinePen(const QPen &pen)
{
    setGridAttr(QicsGridStyle::HorizontalGridLinePen,
		static_cast<const void *> (&pen));
}

QPen
QicsGridCommon::verticalGridLinePen(void) const
{
    return (* static_cast<QPen *>
	    (getGridAttr(QicsGridStyle::VerticalGridLinePen)));
}

void
QicsGridCommon::setVerticalGridLinePen(const QPen &pen)
{
    setGridAttr(QicsGridStyle::VerticalGridLinePen,
		static_cast<const void *> (&pen));
}

Qics::QicsCellOverflowBehavior
QicsGridCommon::cellOverflowBehavior(void) const
{
    return (* static_cast<QicsCellOverflowBehavior *>
	    (getGridAttr(QicsGridStyle::CellOverflowBehavior)));
}

void
QicsGridCommon::setCellOverflowBehavior(QicsCellOverflowBehavior b)
{
    setGridAttr(QicsGridStyle::CellOverflowBehavior,
		static_cast<const void *> (&b));
}

int
QicsGridCommon::maxOverflowCells(void) const
{
    return (* static_cast<int *>
	    (getGridAttr(QicsGridStyle::MaxOverflowCells)));
}

void
QicsGridCommon::setMaxOverflowCells(int num)
{
    setGridAttr(QicsGridStyle::MaxOverflowCells, static_cast<const void *> (&num));
}

int
QicsGridCommon::frameLineWidth(void) const
{
    return (* static_cast<int *>
	    (getGridAttr(QicsGridStyle::FrameLineWidth)));
}

void
QicsGridCommon::setFrameLineWidth(int lw)
{
    setGridAttr(QicsGridStyle::FrameLineWidth, static_cast<const void *> (&lw));
}

int
QicsGridCommon::frameStyle(void) const
{
    return (* static_cast<int *>
	    (getGridAttr(QicsGridStyle::FrameStyle)));
}

void
QicsGridCommon::setFrameStyle(int style)
{
    setGridAttr(QicsGridStyle::FrameStyle, static_cast<const void *> (&style));
}

Qics::QicsCurrentCellStyle
QicsGridCommon::currentCellStyle(void) const
{
    return (* static_cast<QicsCurrentCellStyle *>
	    (getGridAttr(QicsGridStyle::CurrentCellStyle)));
}

void
QicsGridCommon::setCurrentCellStyle(QicsCurrentCellStyle s)
{
    setGridAttr(QicsGridStyle::CurrentCellStyle,
		static_cast<const void *> (&s));
}

int
QicsGridCommon::currentCellBorderWidth(void) const
{
    return (* static_cast<int *>
	    (getGridAttr(QicsGridStyle::CurrentCellBorderWidth)));
}

void
QicsGridCommon::setCurrentCellBorderWidth(int w)
{
    setGridAttr(QicsGridStyle::CurrentCellBorderWidth,
		static_cast<const void *> (&w));
}

bool
QicsGridCommon::clickToEdit(void) const
{
    return (* static_cast<bool *>
	    (getGridAttr(QicsGridStyle::ClickToEdit)));
}

void
QicsGridCommon::setClickToEdit(bool b)
{
    setGridAttr(QicsGridStyle::ClickToEdit, static_cast<const void *> (&b));
}

bool
QicsGridCommon::autoSelectCellContents(void) const
{
    return (* static_cast<bool *>
	    (getGridAttr(QicsGridStyle::AutoSelectCellContents)));
}

void
QicsGridCommon::setAutoSelectCellContents(bool b)
{
    setGridAttr(QicsGridStyle::AutoSelectCellContents,
		static_cast<const void *> (&b));
}

Qt::Orientation
QicsGridCommon::enterTraversalDirection(void) const
{
    return (* static_cast<Orientation *>
	    (getGridAttr(QicsGridStyle::EnterTraversalDirection)));
}

void
QicsGridCommon::setEnterTraversalDirection(Orientation dir)
{
    setGridAttr(QicsGridStyle::EnterTraversalDirection,
		static_cast<const void *> (&dir));
}

Qt::Orientation
QicsGridCommon::tabTraversalDirection(void) const
{
    return (* static_cast<Orientation *>
	    (getGridAttr(QicsGridStyle::TabTraversalDirection)));
}

void
QicsGridCommon::setTabTraversalDirection(Orientation dir)
{
    setGridAttr(QicsGridStyle::TabTraversalDirection,
		static_cast<const void *> (&dir));
}

QPixmap
QicsGridCommon::moreTextPixmap(void) const
{
    QPixmap *pix = static_cast<QPixmap *>
	(getGridAttr(QicsGridStyle::MoreTextPixmap));

    if (pix)
	return (*pix);
    else
	return QPixmap();
}

void
QicsGridCommon::setMoreTextPixmap(const QPixmap &pix)
{
    const QPixmap *val;

    if (pix.isNull())
	val = 0;
    else
	val = &pix;

    setGridAttr(QicsGridStyle::MoreTextPixmap, static_cast<const void *> (val));
}

QPalette
QicsGridCommon::gridPalette(void) const
{
    return (* static_cast<QPalette *>
	    (getGridAttr(QicsGridStyle::GridPalette)));
}

void
QicsGridCommon::setGridPalette(const QPalette &pal)
{
    setGridAttr(QicsGridStyle::GridPalette, static_cast<const void *> (&pal));
}

bool
QicsGridCommon::dragEnabled(void) const
{
    return (* static_cast<bool *>
	    (getGridAttr(QicsGridStyle::DragEnabled)));
}

void
QicsGridCommon::setDragEnabled(bool b)
{
    setGridAttr(QicsGridStyle::DragEnabled, static_cast<const void *> (&b));
}

#include "QicsGridCommon.moc"
