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


#ifndef _QicsCellStyle_H
#define _QicsCellStyle_H

#include <QicsStyle.h>
#include <qwidget.h>
#include <qvaluevector.h>


class QicsCellStyle: public QicsStyle
{
public:
    
    enum QicsCellStyleProperty { Palette = 0,
				 BorderWidth,
				 BorderStyle,
				 BorderPen,
				 CellMargin,
				 Enabled,
				 Current,
				 Selected,       
				 ReadOnly,
				 Alignment,
				 TextFlags,
				 Font,
				 Cursor,
				 CellDisplayer,
				 Formatter,
				 Pixmap,
				 PixmapSpacing,
				 Validator,
				 Label,
				 MaxLength,
				 UserData,
				 LastProperty };
    
    
        
    QicsCellStyle(QicsGridType type, bool create_default = false,
		  QWidget *template_widget = 0);
       
    
    inline void *getValue(QicsCellStyleProperty name) const
	{ return QicsStyle::getValue((int)name); }
    
    inline void setValue(QicsCellStyleProperty name, const void *val) 
	{ QicsStyle::setValue((int) name, val); }

    
    inline void clear(QicsCellStyleProperty prop)
	{ QicsStyle::clear((int) prop); }

protected:        
    
    static const QicsStylePropertyType myCellStyleTypeList[];
};

typedef QValueVector<QicsCellStyle *> QicsCellStylePV;
typedef QValueVector<QicsCellStylePV *> QicsCellStylePVPV;

#endif /*_QicsCellStyle_H --- Do not add anything past this line */
 
