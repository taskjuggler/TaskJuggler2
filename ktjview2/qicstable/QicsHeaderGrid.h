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


#ifndef _QICSHEADERGRID_H
#define _QICSHEADERGRID_H

#include <QicsScreenGrid.h>
#include <QicsDataItem.h>


class QicsHeaderGrid: public QicsScreenGrid {
    Q_OBJECT

public:
    
    QicsHeaderGrid(QWidget *w, QicsGridInfo &info, 
		   QicsHeaderType type);
 
    
    virtual ~QicsHeaderGrid();

    
    inline QicsHeaderType type(void) const { return myType; }

    virtual QicsRegion currentViewport(void) const;

    
    inline int gripThreshold(void) const { return myGripThreshold; }
    
    inline void setGripThreshold(int gt) { myGripThreshold = gt; }

    
    typedef QicsHeaderGrid *(*Foundry)(QWidget *w, QicsGridInfo &info,
				       QicsHeaderType type);

    
    static QicsHeaderGrid *createGrid(QWidget *w, QicsGridInfo &info,
				      QicsHeaderType type);

signals:
    
    void sizeChange(int idx, int old_size, int new_size, QicsHeaderType type);

    
    void resizeInProgress(int idx, int position, QicsHeaderType type);

    
    void gripDoubleClicked(int idx, int button, QicsHeaderType type);

public slots:
    virtual void handleGridPropertyChange(QicsGridStyle::QicsGridStyleProperty prop);

protected:
    virtual QSize sizeHint(void) const;

    virtual void handleMousePressEvent(const QicsICell &cell, QMouseEvent *m);
    virtual void handleMouseReleaseEvent(const QicsICell &cell, QMouseEvent *m);
    virtual void handleMouseMoveEvent(const QicsICell &cell, QMouseEvent *m);
    virtual void handleMouseDoubleClickEvent(const QicsICell &, QMouseEvent *m);

    virtual void keyPressEvent (QKeyEvent *ke);

    void dragMoveEvent(QDragMoveEvent *event);

    
    virtual bool canDropAt(QDragMoveEvent *event, const QicsICell &cell) const;

    virtual void dropAt(QDropEvent *event, const QicsICell &cell);

    
    bool isWithinResizeThreshold(int x, int y, int *in_cell, int *close_cell) const;

    
    bool setResizeCursor( QMouseEvent *m );

    
    void handleCellResize( QMouseEvent *m );
    
    void finishResize(void);

    virtual void selectCell(int row, int col);

    
    void beginSelection(int row, int col);
    
    void endSelection(int row, int col);
    
    void extendSelection(int row, int col);
    
    void addSelection(int row, int col);
    
    void clearSelection(void);
    
    void dragSelection( QMouseEvent *m );
    
    void reportSelection(int row, int col, QicsSelectionType stype);

    void startDrag(QDragObject::DragMode mode);

    void reset();

        QicsHeaderType myType;

        int myGripThreshold;

        bool myMouseInGrip;

        bool myDoingResize;
    
        bool myDoingSelectionFlag;

        /// when myCanResize=true)
    int myExpandingCell;
        /// (valid only when myCanResize=true)
    int myExpandingCellSize;
        /// (valid only when myCanResize=true)
    int myExpandingCellStartPosition;
        /// (valid only when myCanResize=true)
    int myExpandingCellCurrentPosition;

        int myAnchorIndex;
        int mySelectionIndex;
};


#endif /* _QICSHEADERGRID_H */
