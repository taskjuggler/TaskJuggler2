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


#include <QicsCellCommon.h>
#include <QicsStyleManager.h>
#include <QicsDimensionManager.h>
#include <QicsSpanManager.h>
#include <QicsUtil.h>

#include <qpixmap.h>
#include <qcursor.h>
#include <qpen.h>

/////////////////////////////////////////////////////////////////////////////////

QicsCellCommon::QicsCellCommon(QObject *parent) :
    QObject(parent),
    myInfo(0)
{
}

QicsCellCommon::QicsCellCommon(QicsGridInfo *info, QObject *parent) :
    QObject(parent),
    myInfo(info)
{
}

QicsCellCommon::~QicsCellCommon()
{
}


int
QicsCellCommon::margin(void) const
{
    return (* static_cast<int *> (getAttr(QicsCellStyle::CellMargin)));
}

void 
QicsCellCommon::setMargin(int margin)
{
    myInfo->setGlobalRepaintBehavior(RepaintOff);

    // update the dimension manager with the new margin
    setDMMargin(margin);

    setAttr(QicsCellStyle::CellMargin, static_cast<const void *> (&margin));

    myInfo->revertGlobalRepaintBehavior();
}

QFont
QicsCellCommon::font(void) const
{
    return (* static_cast<QFont *> (getAttr(QicsCellStyle::Font)));
}

void
QicsCellCommon::setFont(const QFont &font)
{
    myInfo->setGlobalRepaintBehavior(RepaintOff);

    // update the dimension manager with the new font
    setDMFont(font);

    setAttr(QicsCellStyle::Font, static_cast<const void *>(&font));

    myInfo->revertGlobalRepaintBehavior();
}

QPalette
QicsCellCommon::palette(void) const
{
    return (* static_cast<QPalette *> (getAttr(QicsCellStyle::Palette)));
}

void 
QicsCellCommon::setPalette(const QPalette &pal)
{
    setAttr(QicsCellStyle::Palette, static_cast<const void *> (&pal));
}

QColor
QicsCellCommon::foregroundColor(void) const
{
    QPalette pal = palette();

    return (pal.active().foreground());
}

void
QicsCellCommon::setForegroundColor(const QColor &color)
{
    QPalette pal = palette();

    pal.setColor(QPalette::Active, QColorGroup::Foreground, color);
    pal.setColor(QPalette::Inactive, QColorGroup::Foreground, color);
    pal.setColor(QPalette::Disabled, QColorGroup::Foreground, color);

    pal.setColor(QPalette::Active, QColorGroup::Text, color);
    pal.setColor(QPalette::Inactive, QColorGroup::Text, color);
    pal.setColor(QPalette::Disabled, QColorGroup::Text, color);

    setPalette(pal);
}

QColor
QicsCellCommon::backgroundColor(void) const
{
    QPalette pal = palette();

    return (pal.active().background());
}

void
QicsCellCommon::setBackgroundColor(const QColor &color)
{
    QPalette pal = palette();

    pal.setColor(QPalette::Active, QColorGroup::Background, color);
    pal.setColor(QPalette::Inactive, QColorGroup::Background, color);
    pal.setColor(QPalette::Disabled, QColorGroup::Background, color);

    pal.setColor(QPalette::Active, QColorGroup::Base, color);
    pal.setColor(QPalette::Inactive, QColorGroup::Base, color);
    pal.setColor(QPalette::Disabled, QColorGroup::Base, color);

    setPalette(pal);
}

QColor
QicsCellCommon::selectedForegroundColor(void) const
{
    QPalette pal = palette();

    return (pal.active().highlightedText());
}

void
QicsCellCommon::setSelectedForegroundColor(const QColor &color)
{
    QPalette pal = palette();

    pal.setColor(QPalette::Active, QColorGroup::HighlightedText, color);
    pal.setColor(QPalette::Inactive, QColorGroup::HighlightedText, color);
    pal.setColor(QPalette::Disabled, QColorGroup::HighlightedText, color);

    setPalette(pal);
}

QColor
QicsCellCommon::selectedBackgroundColor(void) const
{
    QPalette pal = palette();

    return (pal.active().highlight());
}

void
QicsCellCommon::setSelectedBackgroundColor(const QColor &color)
{
    QPalette pal = palette();

    pal.setColor(QPalette::Active, QColorGroup::Highlight, color);
    pal.setColor(QPalette::Inactive, QColorGroup::Highlight, color);
    pal.setColor(QPalette::Disabled, QColorGroup::Highlight, color);

    setPalette(pal);
}

bool
QicsCellCommon::readOnly(void) const
{
     return (* static_cast<bool *> (getAttr(QicsCellStyle::ReadOnly)));
}

void
QicsCellCommon::setReadOnly(bool b)
{
    setAttr(QicsCellStyle::ReadOnly, static_cast<const void *> (&b));
}

QPixmap
QicsCellCommon::pixmap(void) const
{
    QPixmap *pix = static_cast<QPixmap *> (getAttr(QicsCellStyle::Pixmap));

    if (pix)
	return (*pix);
    else
	return QPixmap();
}

void
QicsCellCommon::setPixmap(const QPixmap &p)
{
    if (p.isNull())
    {
	clearAttr(QicsCellStyle::Pixmap);
    }
    else
    {
	setAttr(QicsCellStyle::Pixmap, static_cast<const void *> (&p));
    }
}

int
QicsCellCommon::pixmapSpacing(void) const
{
    return (* static_cast<int *> (getAttr(QicsCellStyle::PixmapSpacing)));
}

void
QicsCellCommon::setPixmapSpacing(int sp)
{
    setAttr(QicsCellStyle::PixmapSpacing, static_cast<const void *> (&sp));
}

QicsCellDisplay *
QicsCellCommon::displayer(void) const
{
    return (static_cast<QicsCellDisplay *>
	    (getAttr(QicsCellStyle::CellDisplayer)));
}

void
QicsCellCommon::setDisplayer(QicsCellDisplay *dsp)
{
    setAttr(QicsCellStyle::CellDisplayer, static_cast<const void *> (dsp));
}

QicsDataItemFormatter *
QicsCellCommon::formatter(void) const
{
    return (static_cast<QicsDataItemFormatter *> 
	    (getAttr(QicsCellStyle::Formatter)));
}

void
QicsCellCommon::setFormatter(QicsDataItemFormatter *fmt)
{
    setAttr(QicsCellStyle::Formatter, static_cast<const void *>(fmt));
}

int
QicsCellCommon::alignment(void) const
{
    return (* static_cast<int *> (getAttr(QicsCellStyle::Alignment)));
}

void
QicsCellCommon::setAlignment(int flags)
{
    setAttr(QicsCellStyle::Alignment, static_cast<const void *> (&flags));
}

bool
QicsCellCommon::enabled(void) const
{
    return (* static_cast<bool *> (getAttr(QicsCellStyle::Enabled)));
}

void
QicsCellCommon::setEnabled(bool b)
{
    setAttr(QicsCellStyle::Enabled, static_cast<const void *> (&b));
}

bool
QicsCellCommon::selected(void) const
{
    return (* static_cast<bool *> (getAttr(QicsCellStyle::Selected)));
}

int
QicsCellCommon::textFlags(void) const
{
    return (* static_cast<int *> (getAttr(QicsCellStyle::TextFlags)));
}

void
QicsCellCommon::setTextFlags(int flags)
{
    setAttr(QicsCellStyle::TextFlags, static_cast<const void *> (&flags));
}

QValidator *
QicsCellCommon::validator(void) const
{
    return (static_cast<QValidator *> (getAttr(QicsCellStyle::Validator)));
}

void
QicsCellCommon::setValidator(QValidator *v)
{
    setAttr(QicsCellStyle::Validator, static_cast<const void *> (v));
}

QString
QicsCellCommon::label(void) const
{
    return (* static_cast<QString *> (getAttr(QicsCellStyle::Label)));
}

void 
QicsCellCommon::setLabel(const QString &label)
{
    setAttr(QicsCellStyle::Label, static_cast<const void *> (&label));
}

int
QicsCellCommon::maxLength(void) const
{
    return (* static_cast<int *> (getAttr(QicsCellStyle::MaxLength)));
}

void
QicsCellCommon::setMaxLength(int len)
{
    setAttr(QicsCellStyle::MaxLength, static_cast<const void *> (&len));
}

const QCursor &
QicsCellCommon::cursor(void) const
{
    return (* static_cast<QCursor *> (getAttr(QicsCellStyle::Cursor)));
}

void
QicsCellCommon::setCursor(const QCursor &c)
{
    setAttr(QicsCellStyle::Cursor, static_cast<const void *> (&c));

}

void
QicsCellCommon::unsetCursor(void)
{
    clearAttr(QicsCellStyle::Cursor);
}

int
QicsCellCommon::borderWidth(void) const
{
    return (* static_cast<int *> (getAttr(QicsCellStyle::BorderWidth)));
}

void
QicsCellCommon::setBorderWidth(int bw)
{
    setAttr(QicsCellStyle::BorderWidth, static_cast<const void *> (&bw));
}

Qics::QicsLineStyle
QicsCellCommon::borderStyle(void) const
{
    return (* static_cast<QicsLineStyle *>
	    (getAttr(QicsCellStyle::BorderStyle)));
}

void
QicsCellCommon::setBorderStyle(QicsLineStyle bs)
{
    setAttr(QicsCellStyle::BorderStyle, static_cast<const void *> (&bs));
}

QPen
QicsCellCommon::borderPen(void) const
{
    return (* static_cast<QPen *> (getAttr(QicsCellStyle::BorderPen)));
}

void
QicsCellCommon::setBorderPen(const QPen &pen)
{
    setAttr(QicsCellStyle::BorderPen, static_cast<const void *> (&pen));
}

void *
QicsCellCommon::userData(void) const
{
    return (getAttr(QicsCellStyle::UserData));
}

void
QicsCellCommon::setUserData(void *data)
{
    setAttr(QicsCellStyle::UserData, static_cast<const void *> (data));
}

