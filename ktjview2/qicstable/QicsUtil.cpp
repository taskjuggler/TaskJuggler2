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


#include <QicsNamespace.h>
#include <QicsUtil.h>
#include <QicsStyleManager.h>
#include <QicsSpanManager.h>
#include <QicsGrid.h>
#include <QicsHeaderGrid.h>

#include <qpainter.h>
#include <qstyle.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <qfontmetrics.h>

/////////////////////////////////////////////////////////////////////////////

// Pixmap for "more text"

const char * Qics_arrow_xpm[] = {
"7 9 2 1",
"       c white",
".      c black",
"       ",
" .     ",
" ..    ",
" ....  ",
" ..... ",
" ....  ",
" ..    ",
" .     ",
"       "};

/////////////////////////////////////////////////////////////////////////////
int
qicsHeightOfFont(const QFont &fnt)
{
    QFontMetrics fm(fnt);

    return (fm.height());
}

int
qicsWidthOfFont(const QFont &fnt)
{
    QFontMetrics fm(fnt);

    return (fm.maxWidth());
}

