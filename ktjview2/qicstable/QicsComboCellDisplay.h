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


#ifndef _QICSCOMBOCELLDISPLAY_H
#define _QICSCOMBOCELLDISPLAY_H

#include <QicsCellDisplay.h>

#include <qpixmap.h>
#include <qcombobox.h>




class QicsComboCellDisplay: public QComboBox,
					public QicsMovableEntryWidgetCellDisplay
{
Q_OBJECT

public:
    
    QicsComboCellDisplay();

    virtual ~QicsComboCellDisplay();
    virtual void displayCell(QicsGrid *grid, int row, int col,
			     const QicsDataItem *itm,
			     const QRect &rect, QPainter *painter);
    virtual void startEdit(QicsScreenGrid *, int row, int col,
			   const QicsDataItem *itm);
    inline virtual bool isEmpty(QicsGridInfo *, int, int,
				const QicsDataItem *) const
	{ return false; }

#if !defined(__GNUC__) || (__GNUC__ >= 3)
    using QComboBox::sizeHint;
#endif
    virtual QSize sizeHint(QicsGrid *grid, int row, int col,
			   const QicsDataItem *itm);

    virtual bool handleMouseEvent(QicsScreenGrid *grid, int row, int col,
				  QMouseEvent *me);

    
    bool addValueToList(void) const;

    
    void setAddValueToList(bool set);

protected:
    virtual QWidget *newEntryWidget(QicsScreenGrid *grid);

    virtual void keyPressEvent(QKeyEvent *ke);

    
    virtual QString textToDisplay(QicsGridInfo *ginfo, int row, int col,
				  int model_row, int model_col,
				  const QicsDataItem *itm) const;

    
    virtual void valueChanged(QicsGridInfo *info, int row, int col,
			      int model_row, int model_col,
			      const QString &val);

        bool myAddValueToList;

protected slots:
    
    virtual void itemSelected(const QString &val);

private:
    
    QWidget *myComboHolder;

    
    QComboBox *myFakeCombo;

    
    bool myRedirectEvent;
};

#endif /* _QICSCOMBOCELLDISPLAY_H */
