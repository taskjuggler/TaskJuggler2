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


#include <QicsCellStyle.h>
#include <QicsTextCellDisplay.h>

#include <qcursor.h>


const QicsStyle::QicsStylePropertyType QicsCellStyle::myCellStyleTypeList[]={
    QicsT_QPalette,		// Palette
    QicsT_Int,     		// BorderWidth
    QicsT_Int,			// BorderStyle
    QicsT_QPen,			// BorderPen
    QicsT_Int,     		// CellMargin
    QicsT_Boolean, 		// Enabled
    QicsT_Boolean, 		// Current
    QicsT_Boolean, 		// Selected
    QicsT_Boolean, 		// ReadOnly
    QicsT_Int,     		// Alignment
    QicsT_Int,     		// TextFlags
    QicsT_QFont,   		// Font
    QicsT_QCursor,		// Cursor
    QicsT_QicsCellDisplay, 	// CellDisplayer
    QicsT_QicsDataItemFormatter,// Formatter
    QicsT_QPixmap, 		// Pixmap
    QicsT_Int,			// PixmapSpacing
    QicsT_QValidator, 		// Validator
    QicsT_QString, 		// Label
    QicsT_Int,      		// MaxLength
    QicsT_Pointer		// UserData
}; 

QicsCellStyle::QicsCellStyle(QicsGridType type, bool create_defaults,
			     QWidget *template_widget)
{
    myStyleTypeList = QicsCellStyle::myCellStyleTypeList;
    myNumProperties = QicsCellStyle::LastProperty;

    init();

    if (create_defaults)
    {
	if (template_widget)
	{
	    setValue(Palette, new QPalette(template_widget->palette()));
	    setValue(Font, new QFont(template_widget->font()));
	    setValue(Cursor, new QCursor(template_widget->cursor()));
	}

        setValue(BorderWidth, new int(1));

	if (type == TableGrid)
	    setValue(BorderStyle, new QicsLineStyle(Qics::None));
	else
	    setValue(BorderStyle, new QicsLineStyle(Qics::Raised));

	setValue(BorderPen, new QPen());

        setValue(CellMargin, new int(1));

        setValue(Enabled, new bool(true));
        setValue(Current, new bool(false));
	setValue(Selected, new bool(false));
        setValue(ReadOnly, new bool(false));
        setValue(Alignment, new int(Qt::AlignLeft));
        setValue(TextFlags, new int(0));

	setValue(CellDisplayer,
		 static_cast<QicsCellDisplay *> (new QicsTextCellDisplay()));

	setValue(Pixmap, 0);
	setValue(PixmapSpacing, new int(6));

	setValue(Formatter, 0);
	setValue(Validator, 0);

	setValue(Label, new QString());

	setValue(MaxLength, new int(32767));

	setValue(UserData, 0);
     }
}
