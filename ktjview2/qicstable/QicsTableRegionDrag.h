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


#ifndef _QICSTABLEREGIONDRAG_H
#define _QICSTABLEREGIONDRAG_H

#include <qobject.h>
#include <qdragobject.h>

#include <QicsNamespace.h>
#include <QicsSelection.h>
#include <QicsDataModel.h>

class QicsGridInfo;
class QicsICell;
class QicsDataModel;

///////////////////////////////////////////////////////////////////////////




class QicsTableRegionDrag: public QDragObject, public Qics
{
    Q_OBJECT

protected:

    
    QicsTableRegionDrag(QicsGridInfo *gi, QicsSelectionList *slist,
			QWidget *drag_source, const char *name);

public:
    
    ~QicsTableRegionDrag();

    
    static QicsTableRegionDrag *getDragObject(QicsGridInfo *gi,
					      QicsSelectionList *slist,
					      QicsICell *dragCell,
					      QWidget *dragSource);

    /* required QMimeSource methods */
    const char *format(int i) const;

    QByteArray encodedData(const char *mimetype) const;

    /* Class methods */

    static bool canDecode(const QMimeSource *e);

    
    static bool decode(const QMimeSource *e, QicsDataModel &dm);

private:
    QicsSelectionList *mySelectionList;
    QicsDataModel *myData;
};

#ifndef QICS_MIME_CELLDATA
#define QICS_MIME_CELLDATA	"application/vnd.ics.celldata"
#endif

#endif /* _QICSTABLEREGIONDRAG_H */
