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


#ifndef _QicsTableGrid_H
#define _QicsTableGrid_H

#include <QicsScreenGrid.h>

////////////////////////////////////////////////////////////////////////


class QicsTableGrid: public QicsScreenGrid {
    Q_OBJECT

public:
    
    QicsTableGrid(QWidget *w, 
		  QicsGridInfo &info,
		  int top_row = 0,
		  int left_column = 0);

    
    virtual ~QicsTableGrid();

    
    virtual void reset(void);

    
    typedef QicsTableGrid *(*Foundry)(QWidget *w, QicsGridInfo &info,
				      int top_row, int left_column);

    
    static QicsTableGrid *createGrid(QWidget *w, QicsGridInfo &info,
				     int top_row = 0, int left_column = 0);

public slots:
    virtual void handleGridPropertyChange(QicsGridStyle::QicsGridStyleProperty prop);

protected:

    
    virtual bool event( QEvent *e );

    virtual void handleMousePressEvent(const QicsICell &cell, QMouseEvent *m);
    virtual void handleMouseReleaseEvent(const QicsICell &cell, QMouseEvent *m);
    virtual void handleMouseDoubleClickEvent(const QicsICell &cell, QMouseEvent *m);
    virtual void handleMouseMoveEvent(const QicsICell &cell, QMouseEvent *m);

    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);

    virtual void dragMoveEvent(QDragMoveEvent *event);

    virtual bool canDropAt(QDragMoveEvent *event, const QicsICell &cell) const;
    virtual void dropAt(QDropEvent *event, const QicsICell &cell);

    virtual void selectCell(int row, int col);

    
    void setSelectionAnchorCell(QicsICell *cell);

    
    void setSelectionCurrentCell(QicsICell *cell);

    
    void beginSelection(int row, int col);
    
    void endSelection(int row, int col);
    
    void dragSelection(int row, int col);
    
    void extendSelection(int row, int col);
    
    void addSelection(int row, int col);
    
    void clearSelection(void);
    
    void reportSelection(int row, int col, QicsSelectionType stype);

    
    QicsICell *mySelectionCurrentCell;

    
    QicsICell *mySelectionAnchorCell;

    
    bool myDoingSelectionFlag;
};

#endif /* _QicsTableGrid_H --- Do not add anything past this line */
 
