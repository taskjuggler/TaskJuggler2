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


#ifndef _QICSMAINGRID_H
#define _QICSMAINGRID_H

#include <QicsGridCommon.h>

#ifdef CREATE_OBJS_WITH_QICSTABLE
class QicsTable;
#endif

/////////////////////////////////////////////////////////////////////////////////



class QicsMainGrid: public QicsGridCommon
{
    Q_OBJECT

public:
#ifdef CREATE_OBJS_WITH_QICSTABLE
    
    QicsMainGrid(QicsTable *table);
#endif

    
    QicsMainGrid(QicsGridInfo *info, QObject *parent = 0,
		 bool forward_signals = false);

    virtual ~QicsMainGrid();

private:
#ifdef Q_DISABLE_COPY
    QicsMainGrid(const QicsMainGrid& mg);
    QicsMainGrid &operator=(const QicsMainGrid& mg);
#endif
};

#endif /* _QICSMAINGRID_H */
