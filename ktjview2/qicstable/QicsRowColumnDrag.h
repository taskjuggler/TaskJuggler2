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


#ifndef _QICSROWDRAG_H
#define _QICSROWDRAG_H

#include <qobject.h>
#include <qdragobject.h>
#include <qmemarray.h>

#include <QicsNamespace.h>

class QicsGridInfo;
class QicsICell;

///////////////////////////////////////////////////////////////////////////




class QicsRowColumnDrag: public QDragObject, public Qics
{
    Q_OBJECT


protected:

    
    QicsRowColumnDrag(QicsGridInfo *, Qics::QicsIndexType _type,
		      QWidget *dragSource, const char *name);

public:
    
    ~QicsRowColumnDrag();

    
    static QicsRowColumnDrag *getDragObject(QicsGridInfo *gi,
		    Qics::QicsIndexType type, QicsICell *dragCell,
		    QWidget *dragSource);

    /* required QMimeSource methods */
    const char *format(int i) const;

    QByteArray encodedData(const char *mimetype) const;


public:
    /* Class methods */

    static bool canDecode(const QMimeSource *e, QicsIndexType axistype);


    
    static bool decode(const QMimeSource *e, QicsIndexType axistype,
		       QMemArray<int> &results);

    
    static bool decodeCellData(const QMimeSource *e);


private:
    QicsGridInfo	*myGridInfo;
    Qics::QicsIndexType myIndexType;
    QMemArray<int>	items;
};


#define QICS_MIME_ROWLIST	"application/vnd.ics.rowlist"
#define QICS_MIME_COLUMNLIST	"application/vnd.ics.columnlist"
#define QICS_MIME_CELLDATA	"application/vnd.ics.celldata"

#endif /* _QICSROWDRAG_H */
