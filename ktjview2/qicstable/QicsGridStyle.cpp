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


#include <qpixmap.h>
#include <qbitmap.h>
#include <qpen.h>

#include <QicsGridStyle.h>
#include <QicsNamespace.h>
#include <QicsRegion.h>
#include <QicsUtil.h>


const QicsStyle::QicsStylePropertyType QicsGridStyle::myGridStyleTypeList[]={
    QicsT_Boolean,	// HorizontalGridLinesVisible
    QicsT_Boolean,	// VerticalGridLinesVisible
    QicsT_Int,		// HorizontalGridLineWidth
    QicsT_Int,		// VerticalGridLineWidth
    QicsT_Int,		// HorizontalGridLineStyle
    QicsT_Int,		// VerticalGridLineStyle
    QicsT_QPen,		// HorizontalGridLinePen
    QicsT_QPen,		// VerticalGridLinePen
    QicsT_Int,		// GridCellClipping
    QicsT_Boolean,	// DrawPartialCells
    QicsT_Boolean,	// AllowUserResize
    QicsT_Boolean,	// AllowUserMove
    QicsT_Boolean,	// ClickToEdit
    QicsT_Boolean,	// AutoSelectCellContents,
    QicsT_Int,	        // EnterTraversalDirection,
    QicsT_Int,		// TabTraversalDirection
    QicsT_Int,		// GridRepaintBehavior
    QicsT_Int,		// CellOverflowBehavior
    QicsT_Int,		// MaxOverflowCells
    QicsT_Int,		// CurrentCellStyle
    QicsT_Int,		// CurrentCellBorderWidth
    QicsT_Int,		// FrameLineWidth
    QicsT_Int,		// FrameStyle
    QicsT_QPalette,	// GridPalette
    QicsT_QicsRegion,   // Viewport
    QicsT_QPixmap,      // MoreTextPixmap
    QicsT_Boolean	// DragEnabled
}; 

QicsGridStyle::QicsGridStyle(QicsGridType type, bool create_defaults,
			     QWidget *template_widget)
{
    myStyleTypeList = QicsGridStyle::myGridStyleTypeList;
    myNumProperties = QicsGridStyle::LastProperty;

    init();

    // Initialize the style values

    if (create_defaults)
    {
	if (template_widget)
	{
	    setValue(GridPalette, new QPalette(template_widget->palette()));
	}

	setValue(GridCellClipping,
		 new QicsGridCellClipping(Qics::AllowPartial));

	setValue(HorizontalGridLinesVisible, new bool(true));
	setValue(VerticalGridLinesVisible, new bool(true));

	setValue(HorizontalGridLineWidth, new int(1));
	setValue(VerticalGridLineWidth, new int(1));

	setValue(HorizontalGridLineStyle, new QicsLineStyle(Qics::Plain));
	setValue(VerticalGridLineStyle, new QicsLineStyle(Qics::Plain));

	setValue(HorizontalGridLinePen, new QPen());
	setValue(VerticalGridLinePen, new QPen());

	setValue(DrawPartialCells, new bool(true));

	setValue(AllowUserResize, new bool(true));
	setValue(AllowUserMove, new bool(false));
	setValue(ClickToEdit, new bool(false));

	setValue(AutoSelectCellContents, new bool(true));
	setValue(EnterTraversalDirection, new Qt::Orientation(Qt::Vertical));
	setValue(TabTraversalDirection, new Qt::Orientation(Qt::Horizontal));

	setValue(GridRepaintBehavior, new Qics::QicsRepaintBehavior(Qics::RepaintOn));
	setValue(CellOverflowBehavior, new Qics::QicsCellOverflowBehavior(Qics::Clip));
	setValue(MaxOverflowCells, new int(10));
	setValue(CurrentCellStyle, new Qics::QicsCurrentCellStyle(Qics::Spreadsheet));
	setValue(CurrentCellBorderWidth, new int(2));

	setValue(FrameLineWidth, new int(2));

	if (type == TableGrid)
	    setValue(FrameStyle, new int(QFrame::StyledPanel | QFrame::Sunken));
	else
	    setValue(FrameStyle, new int(QFrame::NoFrame | QFrame::Plain));

	setValue(Viewport, new QicsRegion(0,
					  0,
					  Qics::QicsLAST_ROW,
					  Qics::QicsLAST_COLUMN));

	QPixmap *mtp = new QPixmap(Qics_arrow_xpm);
	mtp->setMask(mtp->createHeuristicMask(true));
	setValue(MoreTextPixmap, mtp);

	setValue(DragEnabled, new bool(true));
    }
}
