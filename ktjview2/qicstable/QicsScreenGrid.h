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


#ifndef _QicsScreenGrid_H
#define _QicsScreenGrid_H

#include <QicsGrid.h>
#include <QicsSpan.h>

#include <qframe.h>
#include <qdragobject.h>
#include <qvaluevector.h>

class QicsGridToolTip;

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


class QicsScreenGrid: public QFrame, public QicsGrid {
    Q_OBJECT

public:
    
    QicsScreenGrid(QWidget *w, 
	     QicsGridInfo &info, 
	     int top_row = 0,
	     int left_column = 0);

    
    virtual ~QicsScreenGrid();

    void setViewport(const QicsRegion &reg);

    
    int lastPageRows(void);
    
    int lastPageColumns(void);

    
    inline int topRow(void) const
	{ return myTopRow; }
    
    inline int bottomRow(void) const
	{ return myBottomRow; }

    
    inline int leftColumn(void) const
	{ return myLeftColumn; }
    
    inline int rightColumn(void) const
	{ return myRightColumn; }

    
    QicsICell cellAt(int x, int y, bool nearest) const;

    
    int rowAt(int y, bool nearest) const;

    
    int columnAt(int x, bool nearest) const;

    
    void scrollLeft(int num);
    
    void scrollRight(int num);
    
    void scrollUp(int num);
    
    void scrollDown(int num);

    
    inline bool isHeightFixed(void) const { return myHeightFixed; }
    
    inline bool isWidthFixed(void) const { return myWidthFixed; }

    
    void fixHeightToViewport(bool set);
    
    void fixWidthToViewport(bool set);

    
    int visibleRows(void) const;
    
    int visibleColumns(void) const;

    virtual bool requestCellOverflow(const QicsRegion &cur_area,
				     const QRect &cur_rect,
				     QicsRegion &new_area,
				     QRect &new_rect);

    
    inline QicsICell currentCell(void) const
	{ return gridInfo().currentCell(); }

    
    QString tooltipText(const QicsICell &cell) const;

    virtual void orderRowsBy(int column,
			     QicsSortOrder order = Qics::Ascending,
			     DataItemComparator func = 0);

    virtual void orderColumnsBy(int row,
				QicsSortOrder order = Qics::Ascending,
				DataItemComparator func = 0);


    ///////// Traversal Methods ///////////

    
    virtual bool handleTraversalKeys(QKeyEvent *ke);

public slots:

    
    void setTopRow(int value);
    
    
    void setLeftColumn(int value);

    
    virtual void reset(void);
    
    
    virtual void recomputeAndDraw(void);
    
    
    virtual void resetAndDraw(void);

    
    virtual void redraw(void);
    
    virtual void redraw(QicsRegion region);

    
    virtual void redraw(QicsSpan span);

    
    void setVisibleRows(int num);
    
    void setVisibleColumns(int num);

    
    virtual void handleCellPropertyChange(QicsRegion region,
					  QicsCellStyle::QicsCellStyleProperty prop);

    
    virtual void handleGridPropertyChange(QicsGridStyle::QicsGridStyleProperty prop);

    
    void drawHeaderResizeBar(int idx, int pos, QicsHeaderType type);

    
    void traverse(QicsScrollDirection dir);

    
    virtual bool traverseToCell(int row, int col, bool select_cell = true);

    
    virtual void traverseToBeginningOfTable(void);
    
    virtual void traverseToEndOfTable(void);
    
    virtual void traverseToBeginningOfRow(void);
    
    virtual void traverseToEndOfRow(void);

    
    virtual void traverseLeft(void);
    
    virtual void traverseRight(void);
    
    virtual void traverseUp(void);
    
    virtual void traverseDown(void);

    
    bool editCell(int row, int col);
    
    bool editCurrentCell(void);
    
    void uneditCurrentCell(void);

    
    inline bool editable(void) const { return myEditable; }

    
    void setEditable(bool b);

protected:
    
    virtual void dropEvent(QDropEvent *event);


    
    virtual void dropAt(QDropEvent *event, const QicsICell &cell);

signals:
    
    void pressed(int row, int col, int button, const QPoint &pos);

    
    void clicked(int row, int col, int button, const QPoint &pos);

    
    void doubleClicked(int row, int col, int button, const QPoint &pos);

    
    void scrollRequest(QicsScrollDirection direction, int num);

    
    void newBoundsInfo();

protected:
    
    inline void setCurrentCell(const QicsICell &cell)
	{ gridInfo().setCurrentCell(cell); }

    // XXX doc this
    virtual void layout(void);

    
    virtual void computeCellPositions(void);

    
    virtual void drawContents(QPainter *painter);

    
    virtual void paintRegion(const QRect &rect, QPainter *painter);

    
    virtual void paintRegion(const QicsRegion &region, QPainter *painter);

    virtual void drawCell(int row, int col, int x, int y,
			  bool look_for_overflower, QPainter *painter);
        
    virtual bool prepareToDraw(int row, int col, const QRect &rect,
			       QPainter *painter);

    
    virtual void computeLastPage(void);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    
    virtual void resizeEvent( QResizeEvent *r);

    
    virtual void mousePressEvent( QMouseEvent *m );

    
    virtual void mouseReleaseEvent( QMouseEvent *m );

    
    virtual void mouseDoubleClickEvent( QMouseEvent *m );

    
    virtual void mouseMoveEvent( QMouseEvent *m );

    
    virtual void keyPressEvent (QKeyEvent *ke);

    
    virtual void keyReleaseEvent (QKeyEvent *ke);

    
    virtual void handleMousePressEvent(const QicsICell &cell, QMouseEvent *m);

    
    virtual void handleMouseReleaseEvent(const QicsICell &cell, QMouseEvent *m);

    
    virtual void handleMouseDoubleClickEvent(const QicsICell &cell, QMouseEvent *m);

    
    virtual void handleMouseMoveEvent(const QicsICell &cell, QMouseEvent *m);

    
    virtual void handleKeyPressEvent(const QicsICell &cell, QKeyEvent *k);

    
    virtual void handleKeyReleaseEvent(const QicsICell &cell, QKeyEvent *k);

    
    bool isCellVisible(int row, int col) const;

    
    void makeCellFullyVisible(int row, int col);

    
    void updateViewport(void);

    
    void updateLineWidth(void);

    
    void updateFrameStyle(void);

    
    void traverse(Qt::Orientation orient, bool forward);

    
    virtual void selectCell(int, int) {;}

    
    void placeEntryWidget(void);

    
    virtual void prepareDrag(const QicsICell &cell);

    
    virtual void startDrag(QDragObject::DragMode mode);

    
    virtual void finishDrag(QDragObject::DragMode mode, bool remove, QWidget *target);

    
    QDragObject *dragObject(QDragObject::DragMode mode);

    
    virtual bool canDropAt(QDragMoveEvent *event, const QicsICell &cell) const;

    //////////////// Data Members /////////////////

    
    QicsGridToolTip *myGridToolTip;

    
    int myTopRow;
        
    
    int myLeftColumn;

    
    int myBottomRow;
    
    int myRightColumn;

    
    int myReqVisibleRows;
    
    int myReqVisibleColumns;

    
    int myLastResizeLinePosition;

    
    bool myNeedsRecomputeCellsFlag;
    
    bool myNeedsRecomputeLastPageFlag;

    
    bool myNeedsRepaintFlag;

    
    int myLastPageRows;
    
    int myLastPageColumns;

    
    QicsICellV myCellsToNotify;

    
    bool myHeightFixed;
    
    bool myWidthFixed;

    
    int myTraversalRow;
    
    int myTraversalColumn;

    /* \internal
     * Flag which indicates whether the current cell is being edited
     * (i.e. there is an entry widget currently displayed for that cell).
     */
    bool myEditingCurrentCell;

    
    bool myEditable;

    
    bool myPlacingEntryWidgetFlag;

    
    QicsICell myPressedCell;

    
    QicsICell *myDragCell;
};

#endif /* _QicsScreenGrid_H --- Do not add anything past this line */
