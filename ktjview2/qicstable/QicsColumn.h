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


#ifndef _QICSCOLUMN_H
#define _QICSCOLUMN_H

#include <QicsCellCommon.h>
#include <QicsDataModel.h>

class QicsHeader;

#ifdef CREATE_OBJS_WITH_QICSTABLE
class QicsTable;
#endif

////////////////////////////////////////////////////////////////////////////////



class QicsColumn: public QicsCellCommon
{
Q_OBJECT

    friend class QicsTableCommon;
    friend class QicsHeader;

public:
   


    
    Q_PROPERTY( int columnIndex READ columnIndex WRITE setColumnIndex )

    
    Q_PROPERTY( int modelColumnIndex READ modelColumnIndex )

    
    Q_PROPERTY( int widthInPixels READ widthInPixels WRITE setWidthInPixels )
    
    Q_PROPERTY( int widthInChars READ widthInChars WRITE setWidthInChars )
    
    Q_PROPERTY( int minWidthInPixels READ minWidthInPixels WRITE setMinWidthInPixels )
    
    Q_PROPERTY( int minWidthInChars READ minWidthInChars WRITE setMinWidthInChars )


public:
#ifdef CREATE_OBJS_WITH_QICSTABLE
    
    QicsColumn(int col, QicsTable *table, bool follow_model = true);
#endif

    
    QicsColumn(int column, QicsGridInfo *info, bool follow_model = true,
	       QObject *parent = 0);

    virtual void setInfo(QicsGridInfo *info);

    
    int columnIndex(void) const;

    
    int modelColumnIndex(void) const;

    
    bool isValid(void) const;

    
    QicsDataModelColumn dataValues(int first_row = 0, int last_row = -1) const;

    
    int widthInPixels(void) const;

    
    int widthInChars(void) const;

    
    int minWidthInPixels(void) const;

    
    int minWidthInChars(void) const;

public slots:
    
    void setColumnIndex(int idx);

    
    void setDataValues(QicsDataModelColumn &vals);

    
    void setWidthInPixels(int width);

    
    void setWidthInChars(int width);

    
    void setMinWidthInPixels(int width);

    
    void setMinWidthInChars(int width);

protected:
    
    void init(void);

    virtual void setAttr(QicsCellStyle::QicsCellStyleProperty attr, const void *val);
    virtual void *getAttr(QicsCellStyle::QicsCellStyleProperty attr) const;
    virtual void clearAttr(QicsCellStyle::QicsCellStyleProperty attr);

    virtual void setDMMargin(int margin);
    virtual void setDMFont(const QFont &font);

        int                    myColumn;

        bool myFollowModel;

protected slots:
    void handleModelColumnInsert(int ncols, int pos);
    void handleModelColumnDelete(int ncols, int pos);
    void handleOrderChanged(QicsIndexType type, QMemArray<int> visChange);
    void changeDataModel(QicsDataModel *old_dt, QicsDataModel *new_dt);

private:
#ifdef Q_DISABLE_COPY
    QicsColumn(const QicsColumn& col);
    QicsColumn &operator=(const QicsColumn& col);
#endif
};

#endif /* _QICSCOLUMN_H */
