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


#ifndef _QicsGridStyle_H
#define _QicsGridStyle_H

#include <QicsStyle.h>

#include <qwidget.h>
#include <qvaluevector.h>

////////////////////////////////////////////////////////////////////////////////


class QicsGridStyle: public QicsStyle
{
public:
    
    enum QicsGridStyleProperty { HorizontalGridLinesVisible,
				 VerticalGridLinesVisible,
				 HorizontalGridLineWidth,
				 VerticalGridLineWidth,
				 HorizontalGridLineStyle,
				 VerticalGridLineStyle,
				 HorizontalGridLinePen,
				 VerticalGridLinePen,
				 GridCellClipping,
				 DrawPartialCells,
				 AllowUserResize,
				 AllowUserMove,
				 ClickToEdit,
				 AutoSelectCellContents,
				 EnterTraversalDirection,
				 TabTraversalDirection,
				 GridRepaintBehavior,
				 CellOverflowBehavior,
				 MaxOverflowCells,
				 CurrentCellStyle,
				 CurrentCellBorderWidth,
				 FrameLineWidth,
				 FrameStyle,
				 GridPalette,
				 Viewport,
				 MoreTextPixmap,
				 DragEnabled,
				 LastProperty };

        
        
    QicsGridStyle(QicsGridType type, bool create_defaults = false,
		  QWidget *template_widget = 0);
       
    
    inline void *getValue(QicsGridStyleProperty name) const
	{ return QicsStyle::getValue((int)name); }
    
    inline void setValue(QicsGridStyleProperty name, const void *val) 
	{ QicsStyle::setValue((int) name, val); }

    
    inline void clear(QicsGridStyleProperty prop)
	{ QicsStyle::clear((int) prop); }

protected:        
    
    static const QicsStylePropertyType myGridStyleTypeList[];
};

typedef QValueVector<QicsGridStyle *> QicsGridStylePV;
typedef QValueVector<QicsGridStylePV *> QicsGridStylePVPV;

#endif /*_QicsGridStyle_H --- Do not add anything past this line */
 
