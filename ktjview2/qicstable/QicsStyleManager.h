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


#ifndef _QICSSTYLEMANAGER_H
#define _QICSSTYLEMANAGER_H

#include <qobject.h>

#include <QicsNamespace.h>
#include <QicsCellStyle.h>
#include <QicsGridStyle.h>
#include <QicsRegion.h>
#include <QicsSpan.h>


class QString;
class QicsSpanManager;


class QicsStyleManager : public QObject, public Qics
{
    Q_OBJECT
public:
    
    QicsStyleManager(QicsGridType type, QWidget *template_widget);

    
    QicsStyleManager(QicsStyleManager *base_sm);

    
    ~QicsStyleManager();

    inline QicsGridType type(void) const
	{ return myType; }

    
    void *getCellProperty(int row, int col,
			  QicsCellStyle::QicsCellStyleProperty name) const;

    
    void *getRowProperty(int row,
			 QicsCellStyle::QicsCellStyleProperty name) const;

    
    void *getColumnProperty(int col,
			    QicsCellStyle::QicsCellStyleProperty name) const;

    
    void *getDefaultProperty(QicsCellStyle::QicsCellStyleProperty name) const;

    
    void setCellProperty(int row, int col,
			 QicsCellStyle::QicsCellStyleProperty name,
			 const void *val);

    
    void setRowProperty(int row,
			QicsCellStyle::QicsCellStyleProperty name,
			const void *val,
			bool override = true);

    
    void setColumnProperty(int col,
			   QicsCellStyle::QicsCellStyleProperty name,
			   const void *val,
			   bool override = true);

    
    void setDefaultProperty(QicsCellStyle::QicsCellStyleProperty name,
			    const void *val);
   
    
    void clearCellPropertyInRow(int row,
				QicsCellStyle::QicsCellStyleProperty name);

    
    void clearRowProperty(int row,
			  QicsCellStyle::QicsCellStyleProperty name);
    
    
    void clearCellPropertyInColumn(int col,
				   QicsCellStyle::QicsCellStyleProperty name);

    
    void clearColumnProperty(int col,
			     QicsCellStyle::QicsCellStyleProperty name);
    
     
    void clearCellProperty(int row, int col,
			   QicsCellStyle::QicsCellStyleProperty name);


    
    void setGridProperty(QicsGridStyle::QicsGridStyleProperty name,
			 const void *val);
    
    void *getGridProperty(QicsGridStyle::QicsGridStyleProperty name) const;

    
    inline void setReportChanges(bool report) { myReportChanges = report; }
    
    inline bool isReportingChanges(void) const
	{ return myReportChanges; }

    
    inline QicsSpanManager *spanManager(void) const
	{ return mySpanManager; }

public slots:
    
    void insertRows(int num, int start_position);
    
    void insertColumns(int num, int start_position);
    
    void deleteRows(int num, int start_position);
    
    void deleteColumns(int num, int start_position);

signals:
    
    void cellPropertyChanged(QicsRegion reg,
			     QicsCellStyle::QicsCellStyleProperty prop);
    
    void gridPropertyChanged(QicsGridStyle::QicsGridStyleProperty prop);

    
    void spanChanged(QicsSpan reg);
    
protected:

    
    void *cellProp(int row, int col,
		   QicsCellStyle::QicsCellStyleProperty prop) const;

    
    void *rowProp(int row, QicsCellStyle::QicsCellStyleProperty prop) const;

    
    void *columnProp(int col, QicsCellStyle::QicsCellStyleProperty prop) const;
    
    void *defaultProp(QicsCellStyle::QicsCellStyleProperty prop) const;

    
    void replaceStyle(QicsCellStylePV *pv, int indx, QicsCellStyle *style);
    
    void clearStyleGivenVectorOfRows(QicsCellStylePV & row_vec, int row,  
                                     QicsCellStyle::QicsCellStyleProperty name,
                                     bool save_space);

        QicsGridType       myType;

        QicsCellStyle      *myDefaultStyle;

        QicsGridStyle      *myGridStyle;
    
        QicsCellStylePV    myVectorOfRowStyles;
    
        QicsCellStylePV    myVectorOfColumnStyles;

        QicsCellStylePVPV myVectorOfColumns;
    
        bool myReportChanges;

        QicsSpanManager *mySpanManager;

        QicsStyleManager *myBaseSM;
private:
};

#endif /* _QICSSTYLEMANAGER_H */
