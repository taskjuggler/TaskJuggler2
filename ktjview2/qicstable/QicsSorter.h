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


#ifndef _QICSSORTER_H
#define _QICSSORTER_H

#include <qobject.h>
#include <qmemarray.h>

#include <QicsDataModel.h>
#include <QicsDataItem.h>

///////////////////////////////////////////////////////////////////////////




class QicsSorter: public QObject, public Qics
{
    Q_OBJECT

public:

    
    QicsSorter(Qics::QicsIndexType _type, QicsDataModel *dm);

    
    ~QicsSorter();


    
    void sort(int otherAxisIndex,
	      QicsSortOrder order = Qics::Ascending,
	      int from = 0, int to = -1,
	      DataItemComparator func = NULL);

    
    int visualToModel(int x);

    
    int modelToVisual(int x);

    
    void moveItems(int target, const QMemArray<int> &itms);


protected slots:
    
    void insertElements(int num, int start_position);

    
    void appendElements(int num);

    
    void deleteElements(int num, int start_position);




protected:
    
    void integrityCheck(void);

private:
    
    void expandTo(int how_many);

    
    void fillVisualToModelMap(void);

    
    void fillModelToVisualMap(void);

    
    void flushModelToVisualMap(void);

    
    void installNewOrder(int *neworder, int newlen);


    ////////////////////////////////////////////////////////////////////////
    //
    // Signals
    //
    ////////////////////////////////////////////////////////////////////////

signals:

    
    void orderChanged(QicsIndexType type, QMemArray<int> map);


    ////////////////////////////////////////////////////////////////////////
    //
    // Data members
    //
    ////////////////////////////////////////////////////////////////////////

/* private data members */
private:

    
    Qics::QicsIndexType	myType;

    
    QicsDataModel *myDM;

    
    QMemArray<int>	order;

    
    QMemArray<int>	modelToVisualMap;


public:
    friend class QicsSorter_sortHelper;
};

#endif /* _QICSSORTER_H */
