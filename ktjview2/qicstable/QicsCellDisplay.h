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


#ifndef _QICSCELLDISPLAY_H
#define _QICSCELLDISPLAY_H

#include <QicsNamespace.h>
#include <QicsDataItem.h>
#include <QicsGridInfo.h>

#include <qpainter.h>
#include <qstring.h>
#include <qevent.h>
#include <qptrlist.h>

class QicsStyleManager;
class QicsDataItem;
class QicsGrid;
class QicsScreenGrid;

class QicsCell;
class QicsRow;
class QicsColumn;
class QicsMainGrid;

// The abstract base class for all cell display objects



class QicsCellDisplay: public Qics {
public:
    QicsCellDisplay();

    
    virtual ~QicsCellDisplay();

    
    virtual void displayCell(QicsGrid *grid, int row, int col,
			     const QicsDataItem *itm,
			     const QRect &rect, QPainter *painter) = 0;

    
    virtual void startEdit(QicsScreenGrid *grid, int row, int col,
			   const QicsDataItem *itm) = 0;

    
    virtual void moveEdit(QicsScreenGrid *grid, int row, int col,
			  const QRect &rect) = 0;

    
    virtual void hideEdit(QicsScreenGrid *grid) = 0;

    
    virtual void endEdit(QicsScreenGrid *grid, int row, int col) = 0;

    
    virtual bool handleMouseEvent(QicsScreenGrid *grid, int row, int col,
				  QMouseEvent *me);

    
    virtual bool handleKeyEvent(QicsScreenGrid *grid, int row, int col,
				QKeyEvent *ke);

    
    virtual QSize sizeHint(QicsGrid *grid, int row, int col,
			   const QicsDataItem *itm) = 0;

    
    virtual bool editWhenCurrent(void) const = 0;

    
    inline virtual bool needsVisibilityNotification(void) const { return false; }

    
    virtual bool isEmpty(QicsGridInfo *info, int row, int col,
			 const QicsDataItem *itm) const = 0;

    
    virtual QString tooltipText(QicsGridInfo *info, int row, int col,
				const QicsDataItem *itm, const QRect &rect) const;

protected:
    
    virtual void drawBackground(QicsGridInfo *info, int row, int col,
				const QRect &rect, const QColorGroup &cg,
				QPainter *painter,
				bool is_current = false, bool is_selected = false);

    
    virtual void drawBorder(QicsGridInfo *info, int row, int col,
			    const QRect &rect, const QColorGroup &cg,
			    QPainter *painter,
			    bool is_current = false, bool is_selected = false);

    
    virtual bool isCellSelected(QicsGridInfo *info, int row, int col);

    
    virtual QColorGroup cellColorGroup(QicsGridInfo *info,
				       int row, int col,
				       bool for_printer = false);

    
    virtual QRect displayableCellArea(QicsGridInfo *ginfo,
				      int row, int col,
				      const QRect &cr_full, 
				      bool consider_margin = true,
				      bool consider_border = true) const;

protected:
    QicsCell     *myCell;
    QicsRow      *myRow;
    QicsColumn   *myColumn;
    QicsMainGrid *myGrid;
};

////////////////////////////////////////////////////////////////////////////////////




// An abstract class for all cell display objects that draw each cell,
// and use a single entry widget

class QicsNoWidgetCellDisplay: public QicsCellDisplay {
public:
    
    QicsNoWidgetCellDisplay();

    
    virtual ~QicsNoWidgetCellDisplay();

    
    virtual void startEdit(QicsScreenGrid *, int row, int col,
			   const QicsDataItem *);
    
    virtual void moveEdit(QicsScreenGrid *, int row, int col, const QRect &rect);

    
    virtual void endEdit(QicsScreenGrid *grid, int row, int col);

    
    virtual void hideEdit(QicsScreenGrid *);

    inline virtual bool editWhenCurrent(void) const { return false; }
};

////////////////////////////////////////////////////////////////////////////////////



// An abstract class for all cell display objects that draw each cell,
// and use a single entry widget

class QicsMovableEntryWidgetCellDisplay: public QicsCellDisplay {
public:
    
    QicsMovableEntryWidgetCellDisplay();

    
    virtual ~QicsMovableEntryWidgetCellDisplay();

    virtual void moveEdit(QicsScreenGrid *, int row, int col, const QRect &rect);
    virtual void endEdit(QicsScreenGrid *, int row, int col);
    virtual void hideEdit(QicsScreenGrid *);

    inline virtual bool editWhenCurrent(void) const { return true; }

protected:
// A helper class for QicsMovableEntryWidgetCellDisplay


    class QicsEntryWidgetInfo {
    public:
	QicsEntryWidgetInfo() { myWidget = 0; myGrid = 0; myItem = 0;
	                        myRow = -1; myCol = -1; }
	~QicsEntryWidgetInfo() { delete myItem; }

	
	inline QWidget *widget(void) const { return myWidget; }
	
	inline void setWidget(QWidget *w) { myWidget = w; }
	
	inline QicsScreenGrid *grid(void) const { return myGrid; }
	
	inline void setGrid(QicsScreenGrid *grid) { myGrid = grid; }
	
	inline int row(void) const { return myRow; }
	
	inline void setRow(int row) { myRow = row; }
	
	inline int column(void) const { return myCol; }
	
	inline void setColumn(int col) { myCol = col; }
	
	inline QicsDataItem *item(void) const { return myItem; }
	
	inline void setItem(QicsDataItem *item) { delete myItem; myItem = item; }

    protected:
	QWidget *myWidget;
	QicsScreenGrid *myGrid;
	int myRow;
	int myCol;
	QicsDataItem *myItem;
    };

    typedef QPtrList<QicsEntryWidgetInfo> QicsEntryWidgetInfoPL;

    
    virtual QWidget *newEntryWidget(QicsScreenGrid *grid) = 0;
    
    QicsEntryWidgetInfo *getInfoFromGrid(QicsScreenGrid *grid);
    
    QicsEntryWidgetInfo *getInfoFromEntry(const QWidget *widget);

    
    virtual QRect entryWidgetRect(QicsGridInfo *ginfo, int row, int col,
				  QRect cell_rect);

        QicsEntryWidgetInfoPL myEntryList;
};

#endif /* _QICSCELLDISPLAY_H */



