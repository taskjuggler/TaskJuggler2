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


#ifndef _QICSGRIDCOMMON_H
#define _QICSGRIDCOMMON_H

#include <QicsCellCommon.h>
#include <QicsGridStyle.h>
#include <QicsSpan.h>

/////////////////////////////////////////////////////////////////////////////////



class QicsGridCommon: public QicsCellCommon {
Q_OBJECT

public:
    Q_ENUMS( QicsCurrentCellStyle )
    Q_ENUMS( QicsGridCellClipping )

   


    
    Q_PROPERTY( int frameLineWidth READ frameLineWidth WRITE setFrameLineWidth )

    
    Q_PROPERTY( int frameStyle READ frameStyle WRITE setFrameStyle )

    
    Q_PROPERTY( bool horizontalGridLinesVisible  READ horizontalGridLinesVisible WRITE setHorizontalGridLinesVisible )

    
    Q_PROPERTY( bool verticalGridLinesVisible  READ verticalGridLinesVisible WRITE setVerticalGridLinesVisible )

    
    Q_PROPERTY( int horizontalGridLineWidth READ horizontalGridLineWidth WRITE setHorizontalGridLineWidth )

    
    Q_PROPERTY( int verticalGridLineWidth READ verticalGridLineWidth WRITE setVerticalGridLineWidth )

    
    Q_PROPERTY( QicsLineStyle horizontalGridLineStyle  READ horizontalGridLineStyle WRITE setHorizontalGridLineStyle )

    
    Q_PROPERTY( QicsLineStyle verticalGridLineStyle  READ verticalGridLineStyle WRITE setVerticalGridLineStyle )

    
    Q_PROPERTY( QicsCellOverflowBehavior cellOverflowBehavior READ cellOverflowBehavior WRITE setCellOverflowBehavior )

    
    Q_PROPERTY( int maxOverflowCells READ maxOverflowCells WRITE setMaxOverflowCells )

    
    Q_PROPERTY( bool drawPartialCells READ drawPartialCells WRITE setDrawPartialCells )

    
    Q_PROPERTY( QicsGridCellClipping gridCellClipping READ gridCellClipping WRITE setGridCellClipping )

    
    Q_PROPERTY( QicsCurrentCellStyle currentCellStyle READ currentCellStyle WRITE setCurrentCellStyle )

    
    Q_PROPERTY( int currentCellBorderWidth READ currentCellBorderWidth WRITE setCurrentCellBorderWidth )

    
    Q_PROPERTY( bool clickToEdit READ clickToEdit WRITE setClickToEdit )

    
    Q_PROPERTY( bool autoSelectCellContents READ autoSelectCellContents WRITE setAutoSelectCellContents )

    
    Q_PROPERTY( Orientation enterTraversalDirection READ enterTraversalDirection WRITE setEnterTraversalDirection )

    
    Q_PROPERTY( Orientation tabTraversalDirection READ tabTraversalDirection WRITE setTabTraversalDirection )

    
    Q_PROPERTY( QPixmap moreTextPixmap READ moreTextPixmap WRITE setMoreTextPixmap )

    
    Q_PROPERTY( QPalette gridPalette READ gridPalette WRITE setGridPalette )

    
    Q_PROPERTY( bool dragEnabled READ dragEnabled WRITE setDragEnabled )


public:
    
    QicsGridCommon(QObject *parent = 0, bool forward_signals = false);

    
    QicsGridCommon(QicsGridInfo *info, QObject *parent = 0,
		   bool forward_signals = false);

    
    virtual ~QicsGridCommon();

    virtual void setInfo(QicsGridInfo *info);

    
    virtual QicsRegion viewport(void) const;

    
    bool addCellSpan(QicsSpan span);

    
    QicsSpanList *cellSpanList(void);

    
    bool horizontalGridLinesVisible(void) const;

    
    bool verticalGridLinesVisible(void) const;

    
    int horizontalGridLineWidth(void) const;

    
    int verticalGridLineWidth(void) const;

    
    QicsLineStyle horizontalGridLineStyle(void) const;

    
    QicsLineStyle verticalGridLineStyle(void) const;

    
    QPen horizontalGridLinePen(void) const;

    
    QPen verticalGridLinePen(void) const;

    
    bool drawPartialCells() const;

    
    QicsGridCellClipping gridCellClipping() const;

    
    QicsCellOverflowBehavior cellOverflowBehavior(void) const;

    
    int maxOverflowCells(void) const;

    
    int frameLineWidth(void) const;

    
    int frameStyle(void) const;

    
    QicsCurrentCellStyle currentCellStyle(void) const;

    
    int currentCellBorderWidth(void) const;

    
    bool clickToEdit(void) const;

    
    bool autoSelectCellContents(void) const;

    
    Orientation enterTraversalDirection(void) const;

    
    Orientation tabTraversalDirection(void) const;

    
    QPixmap moreTextPixmap(void) const;

    
    QPalette gridPalette(void) const;

    
    bool dragEnabled(void) const;

public slots:
    
    virtual void setViewport(const QicsRegion &vp);

    
    void removeCellSpan(int start_row, int start_col);

    
    void setHorizontalGridLinesVisible(bool b);

    
    void setVerticalGridLinesVisible(bool b);

    
    void setHorizontalGridLineWidth(int w);

    
    void setVerticalGridLineWidth(int w);

    
    void setHorizontalGridLineStyle(QicsLineStyle style);

    
    void setVerticalGridLineStyle(QicsLineStyle style);
      
    
    void setHorizontalGridLinePen(const QPen &pen);

    
    void setVerticalGridLinePen(const QPen &pen);

    
    void setDrawPartialCells(bool b);

    
    void setGridCellClipping(QicsGridCellClipping c);

    
    void setCellOverflowBehavior(QicsCellOverflowBehavior b);

    
    void setMaxOverflowCells(int num);

    
    void setFrameStyle(int style);

    
    void setFrameLineWidth(int lw);

    
    void setCurrentCellStyle(QicsCurrentCellStyle s);

    
    void setCurrentCellBorderWidth(int w);

    
    void setClickToEdit(bool b);

    
    void setAutoSelectCellContents(bool b);

    
    void setEnterTraversalDirection(Orientation dir);

    
    void setTabTraversalDirection(Orientation dir);

    
    void setMoreTextPixmap(const QPixmap &pix);

    
    void setGridPalette(const QPalette &pal);

    
    void setDragEnabled(bool b);

signals:
    
    void pressed(int row, int col, int button, const QPoint &pos);

    
    void clicked(int row, int col, int button, const QPoint &pos);

    
    void doubleClicked(int row, int col, int button, const QPoint &pos);

    
    void valueChanged(int row, int col);

protected:
    virtual void setAttr(QicsCellStyle::QicsCellStyleProperty attr, const void *val);
    virtual void *getAttr(QicsCellStyle::QicsCellStyleProperty attr) const;
    virtual void clearAttr(QicsCellStyle::QicsCellStyleProperty attr);

    
    virtual void setGridAttr(QicsGridStyle::QicsGridStyleProperty attr,
			     const void *val);
    
    virtual void *getGridAttr(QicsGridStyle::QicsGridStyleProperty attr) const;
    
    virtual void clearGridAttr(QicsGridStyle::QicsGridStyleProperty attr);

    virtual void setDMMargin(int margin);
    virtual void setDMFont(const QFont &font);

    
    void initSignals(void);

    
    bool myForwardSignals;

protected slots:
    
    virtual void connectGrid(QicsScreenGrid *grid);

    
    virtual void disconnectGrid(QicsScreenGrid *grid);

    
    void handleGridPress(int row, int col, int button, const QPoint &pos);

    

    void handleGridClick(int row, int col, int button, const QPoint &pos);

    
    void handleGridDoubleClick(int row, int col, int button, const QPoint &pos);

private:
#ifdef Q_DISABLE_COPY
    QicsGridCommon(const QicsGridCommon& gc);
    QicsGridCommon &operator=(const QicsGridCommon& gc);
#endif
}; 

#endif /* _QICSGRIDCOMMON_H */
