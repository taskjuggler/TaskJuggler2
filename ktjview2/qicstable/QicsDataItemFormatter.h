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


#ifndef _QicsDataItemFormatter_H
#define _QicsDataItemFormatter_H

#include <QicsDataItem.h>

#include <qcstring.h>
#include <qvaluevector.h>



class QicsDataItemFormatter
{
public:
    
    QicsDataItemFormatter();

    
    virtual ~QicsDataItemFormatter();

    
    virtual QString format(const QicsDataItem &itm) const = 0;
};

/////////////////////////////////////////////////////////////////////

typedef struct {
    QicsDataItemType type;
    QCString *format;
} QicsFormatPair;


typedef QValueVector<QicsFormatPair> QicsFormatPairV;


class QicsDataItemSprintfFormatter: public QicsDataItemFormatter
{
public:
    
    QicsDataItemSprintfFormatter();
    
    virtual ~QicsDataItemSprintfFormatter();

    virtual QString format(const QicsDataItem &itm) const;

    
    void addFormatString(QicsDataItemType type, const char *format_string);
    
    void removeFormatString(QicsDataItemType type);

protected:
    QicsFormatPairV myFormats;
};

#endif /*_QicsDataItemFormatter_H --- Do not add anything past this line */
