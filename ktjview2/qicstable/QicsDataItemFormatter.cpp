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


#include <QicsDataItemFormatter.h>

QicsDataItemFormatter::QicsDataItemFormatter()
{
}

QicsDataItemFormatter::~QicsDataItemFormatter()
{
}

////////////////////////////////////////////////////////////////////////

QicsDataItemSprintfFormatter::QicsDataItemSprintfFormatter()
{
}

QicsDataItemSprintfFormatter::~QicsDataItemSprintfFormatter()
{
}

QString
QicsDataItemSprintfFormatter::format(const QicsDataItem &itm) const
{
    for (QicsFormatPairV::const_iterator iter = myFormats.begin();
	 iter != myFormats.end(); ++iter)
    {
	const QicsFormatPair &fp = *iter;

	if (fp.type == itm.type())
	{
	    const char *fmt = *(fp.format);

	    return itm.format(fmt);
	}
    }

    return itm.string();
}

void
QicsDataItemSprintfFormatter::addFormatString(QicsDataItemType type,
					      const char *format_string)
{
    for (QicsFormatPairV::iterator iter = myFormats.begin();
	 iter != myFormats.end(); ++iter)
    {
	QicsFormatPair &fp = *iter;

	if (fp.type == type)
	{
	    delete fp.format;
	    fp.format = new QCString(format_string);
	    return;
	}
    }

    QicsFormatPair new_fp;
    new_fp.type = type;
    new_fp.format = new QCString(format_string);

    myFormats.push_back(new_fp);
}

void
QicsDataItemSprintfFormatter::removeFormatString(QicsDataItemType type)
{
    for (QicsFormatPairV::iterator iter = myFormats.begin();
	 iter != myFormats.end(); ++iter)
    {
	QicsFormatPair &fp = *iter;

	if (fp.type == type)
	{
	    delete fp.format;
	    myFormats.erase(iter);
	    return;
	}
    }
}

