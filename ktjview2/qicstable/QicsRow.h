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


#ifndef _QICSROW_H
#define _QICSROW_H

#include <QicsCellCommon.h>
#include <QicsDataModel.h>

class QicsHeader;

#ifdef CREATE_OBJS_WITH_QICSTABLE
class QicsTable;
#endif

////////////////////////////////////////////////////////////////////////////////



class QicsRow: public QicsCellCommon
{
Q_OBJECT

    friend class QicsTableCommon;
    friend class QicsHeader;

public:
   


    
    Q_PROPERTY( int rowIndex READ rowIndex WRITE setRowIndex )

    
    Q_PROPERTY( int modelRowIndex READ modelRowIndex )

    
    Q_PROPERTY( int heightInPixels READ heightInPixels WRITE setHeightInPixels )
    
    Q_PROPERTY( int heightInChars READ heightInChars WRITE setHeightInChars )
    
    Q_PROPERTY( int minHeightInPixels READ minHeightInPixels WRITE setMinHeightInPixels )
    
    Q_PROPERTY( int minHeightInChars READ minHeightInChars WRITE setMinHeightInChars )


public:
#ifdef CREATE_OBJS_WITH_QICSTABLE
    
    QicsRow(int row, QicsTable *table, bool follow_model = true);
#endif

    
    QicsRow(int row, QicsGridInfo *info, bool follow_model = true,
	    QObject *parent = 0);

    virtual void setInfo(QicsGridInfo *info);

    
    int rowIndex(void) const;

    
    int modelRowIndex(void) const;

    
    bool isValid(void) const;

    
    QicsDataModelRow dataValues(int first_col = 0, int last_col = -1) const;

    
    int heightInPixels(void) const;

    
    int heightInChars(void) const;

    
    int minHeightInPixels(void) const;

    
    int minHeightInChars(void) const;

public slots:
    
    void setRowIndex(int idx);

    
    void setDataValues(QicsDataModelRow &vals);

    
    void setHeightInPixels(int height);

    
    void setHeightInChars(int height);

    
    void setMinHeightInPixels(int height);

    
    void setMinHeightInChars(int height);

protected:
    
    void init(void);

    virtual void setAttr(QicsCellStyle::QicsCellStyleProperty attr, const void *val);
    virtual void *getAttr(QicsCellStyle::QicsCellStyleProperty attr) const;
    virtual void clearAttr(QicsCellStyle::QicsCellStyleProperty attr);

    virtual void setDMMargin(int margin);
    virtual void setDMFont(const QFont &font);

        int                    myRow;

        bool myFollowModel;

protected slots:
    void handleModelRowInsert(int nrows, int pos);
    void handleModelRowDelete(int nrows, int pos);
    void handleOrderChanged(QicsIndexType type, QMemArray<int> visChange);
    void changeDataModel(QicsDataModel *old_dt, QicsDataModel *new_dt);

private:
#ifdef Q_DISABLE_COPY
    QicsRow(const QicsRow& row);
    QicsRow &operator=(const QicsRow& row);
#endif
};

#endif /* _QICSROW_H */
