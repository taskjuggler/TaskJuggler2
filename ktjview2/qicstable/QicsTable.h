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


#ifndef _QicsTABLE_H
#define _QicsTABLE_H

#include <QicsTableCommon.h>
#include <QicsSelection.h>
#include <QicsRegion.h>
#include <QicsSpan.h>
#include <QicsDataItem.h>
#include <QicsHeaderGrid.h>
#include <QicsTableGrid.h>

#include <qscrollbar.h>
#include <qlayout.h>

// Forwards to reduce dependencies..
class QicsScrollManager;
class QicsScrollBarScroller;
class QicsStyleManager;
class QicsDimensionManager;
class QicsSelectionManager;

/////////////////////////////////////////////////////////////////////////////////




class QicsTable:  public QWidget, public Qics
{
    Q_OBJECT

public:

#ifdef CREATE_OBJS_WITH_QICSTABLE
friend class QicsCell;
friend class QicsRow;
friend class QicsColumn;
friend class QicsMainGrid;
friend class QicsRowHeader;
friend class QicsColumnHeader;

#endif

    Q_ENUMS( QicsGridCellClipping )
    Q_ENUMS( QicsCurrentCellBorderStyle )

   


    
    Q_PROPERTY( int topRow READ topRow WRITE setTopRow )

    
    Q_PROPERTY( int bottomRow READ bottomRow )

    
    Q_PROPERTY( int leftColumn READ leftColumn WRITE setLeftColumn )

    
    Q_PROPERTY( int rightColumn READ rightColumn )

    
    Q_PROPERTY( int visibleRows READ visibleRows WRITE setVisibleRows )

    
    Q_PROPERTY( int visibleColumns READ visibleColumns WRITE setVisibleColumns )

    
    Q_PROPERTY( bool topHeaderVisible READ topHeaderVisible WRITE setTopHeaderVisible )
    
    Q_PROPERTY( bool bottomHeaderVisible READ bottomHeaderVisible WRITE setBottomHeaderVisible )
    
    Q_PROPERTY( bool leftHeaderVisible READ leftHeaderVisible WRITE setLeftHeaderVisible )
    
    Q_PROPERTY( bool rightHeaderVisible READ rightHeaderVisible WRITE setRightHeaderVisible )

    
    Q_PROPERTY( int tableMargin READ tableMargin WRITE setTableMargin )

    
    Q_PROPERTY( int tableSpacing READ tableSpacing WRITE setTableSpacing )

    
    Q_PROPERTY( int gridSpacing READ gridSpacing WRITE setGridSpacing )

    
    Q_PROPERTY( QicsSelectionPolicy  selectionPolicy  READ selectionPolicy WRITE setSelectionPolicy )

    
    Q_PROPERTY( bool columnHeaderUsesModel READ columnHeaderUsesModel WRITE setColumnHeaderUsesModel )

    
    Q_PROPERTY( bool rowHeaderUsesModel READ rowHeaderUsesModel WRITE setRowHeaderUsesModel )

    
    Q_PROPERTY( QicsRepaintBehavior repaintBehavior READ repaintBehavior WRITE setRepaintBehavior )

    
    Q_PROPERTY( QicsScrollBarMode hScrollBarMode READ hScrollBarMode WRITE setHScrollBarMode )
    
    Q_PROPERTY( QicsScrollBarMode vScrollBarMode READ vScrollBarMode WRITE setVScrollBarMode )



   


    
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

    
    Q_PROPERTY( bool dragEnabled READ dragEnabled WRITE setDragEnabled )



   


    
    Q_PROPERTY( QString label READ label WRITE setLabel )
    
    Q_PROPERTY( bool readOnly READ readOnly  WRITE setReadOnly )
    
    Q_PROPERTY( QColor selectedForegroundColor READ selectedForegroundColor WRITE setSelectedForegroundColor )
    
    Q_PROPERTY( QColor selectedBackgroundColor READ selectedBackgroundColor WRITE setSelectedBackgroundColor )
    
    Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap )
    
    Q_PROPERTY( int pixmapSpacing READ pixmapSpacing WRITE setPixmapSpacing )
    
    Q_PROPERTY( int alignment READ alignment WRITE setAlignment )
    
    Q_PROPERTY( int textFlags READ textFlags WRITE setTextFlags )
    
    Q_PROPERTY( int maxLength READ maxLength WRITE setMaxLength )
    
    Q_PROPERTY( int margin READ margin WRITE setMargin )
    
    Q_PROPERTY( int borderWidth READ borderWidth WRITE setBorderWidth )
    
    Q_PROPERTY( QicsLineStyle borderStyle READ borderStyle WRITE setBorderStyle )

    // REMOVE THIS if building with Qt 3.0.x 
    
    Q_PROPERTY( QPen borderPen READ borderPen WRITE setBorderPen )
    // END REMOVE THIS if building with Qt 3.0.x 




public:

    ////////////////// CONST/DEST 
    
    QicsTable(QicsDataModel *table = 0, QWidget *parent = 0, const char *name = 0);

    
    QicsTable(QicsDataModel::Foundry rhdmf,
	      QicsDataModel::Foundry chdmf,
	      QicsTableGrid::Foundry tf,
	      QicsHeaderGrid::Foundry hf,
	      QicsDataModel *model = 0,
	      QWidget *parent = 0, const char *name = 0);

    virtual ~QicsTable();

    ///////////////// Data Model

    
    QicsDataModel *dataModel(void) const
        { return myTableCommon->gridInfo().dataModel(); }

    
    virtual void setDataModel(QicsDataModel *dm);

    /////////////////// Flyweight class accessors

    
    QicsRow &rowRef(int rownum);
    
    const QicsRow &rowRef(int rownum) const;

    
    QicsRow *row(int rownum, bool follow_model = true);
    
    const QicsRow *row(int rownum, bool follow_model = true) const;

    
    QicsColumn &columnRef(int colnum);
    
    const QicsColumn &columnRef(int colnum) const;

    
    QicsColumn *column(int colnum, bool follow_model = true);
    
    const QicsColumn *column(int colnum, bool follow_model = true) const;

    
    QicsCell &cellRef(int rownum, int colnum);
    
    const QicsCell &cellRef(int rownum, int colnum) const;

    
    QicsCell *cell(int rownum, int colnum, bool follow_model = true);
    
    const QicsCell *cell(int rownum, int colnum, bool follow_model = true) const;

    
    QicsMainGrid &mainGridRef(void);
    
    const QicsMainGrid &mainGridRef(void) const;

    
    QicsMainGrid *mainGrid(void);
    
    const QicsMainGrid *mainGrid(void) const;

    
    QicsRowHeader &rowHeaderRef(void);
    
    const QicsRowHeader &rowHeaderRef(void) const;

    
    QicsRowHeader *rowHeader(void);
    
    const QicsRowHeader *rowHeader(void) const;

    
    QicsColumnHeader &columnHeaderRef(void);
    
    const QicsColumnHeader &columnHeaderRef(void) const;

    
    QicsColumnHeader *columnHeader(void);
    
    const QicsColumnHeader *columnHeader(void) const;

    /////////////////// Selection methods

    
    QicsSelectionList *selectionList(void) const;

    
    QicsSelectionList *selectionActionList(void) const;

    
    void setSelectionList(QicsSelectionList &sel_list);
    
    void clearSelectionList(void);
    
    void addSelection(QicsSelection &selection);
      
    /////////////////// PROPERTY GETS

    ////////// Cell Attribute Gets

    
    int margin(void) const;

    
    bool readOnly(void) const;
    
    
    QColor selectedForegroundColor(void) const;

    
    QColor selectedBackgroundColor(void) const;

    
    QPixmap pixmap(void) const;

    
    int pixmapSpacing(void) const;

    
    QicsCellDisplay *displayer(void) const;

    
    QicsDataItemFormatter *formatter(void) const;

    
    int alignment(void) const;

    
    int textFlags(void) const;

    
    QValidator *validator(void) const;

    
    QString label(void) const;

    
    int maxLength(void) const;

    
    int borderWidth(void) const;

    
    QicsLineStyle borderStyle(void) const;

    
    QPen borderPen(void) const;

    ////////// Grid Attribute Gets

    
    virtual QicsRegion viewport(void) const;

    
    virtual QicsRegion mainGridViewport(void) const;

    
    virtual QicsRegion currentViewport(void) const;

    
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

    
    bool dragEnabled(void) const;

    ////////// Table Common Gets

    
    bool topHeaderVisible(void) const;
    
    bool bottomHeaderVisible(void) const;
    
    bool leftHeaderVisible(void) const;
    
    bool rightHeaderVisible(void) const;

    
    int tableMargin(void) const;

    
    int tableSpacing(void) const;

    
    int gridSpacing(void) const;

    
    QicsSelectionPolicy selectionPolicy(void) const;
    
    void setSelectionPolicy(QicsSelectionPolicy policy);

    
    QicsRepaintBehavior repaintBehavior(void) const;

    
    QicsScrollBarMode hScrollBarMode(void) const;

    
    QicsScrollBarMode vScrollBarMode(void) const;

    
    void setHScrollBarMode(QicsScrollBarMode m);

    
    void setVScrollBarMode(QicsScrollBarMode m);

    
    void sortRows(int column,
		    QicsSortOrder order = Qics::Ascending,
		    int from = 0, int to = -1,
		    DataItemComparator func = 0);

    
    void sortColumns(int row,
		    QicsSortOrder order = Qics::Ascending,
		    int from = 0, int to = -1,
		    DataItemComparator func = 0);

    
    void moveRows(int target_row, const QMemArray<int> &rows);

    
    void moveColumns(int target_col, const QMemArray<int> &cols);

    
    bool rowHeaderUsesModel(void) const;

    
    bool columnHeaderUsesModel(void) const;

    /////////////////////////////////////////////////////////////////

    
    int topRow(void) const;
    
    int bottomRow(void) const;
    
    int leftColumn(void) const;
    
    int rightColumn(void) const;

    
    int visibleRows(void) const;
    
    int visibleColumns(void) const;

    
    QicsCell *currentCell(void);
    
     const QicsCell *currentCell(void) const;

    
    bool editCurrentCell(void);

    
    void uneditCurrentCell(void);

    
    bool editCell(int row, int col);

    
    QScrollBar *horizontalScrollBar(void) const;

    
    QScrollBar *verticalScrollBar(void) const;

    
    inline QWidget *topTitleWidget(void) const
	{ return myTopTitleWidget; }

    
    inline QWidget *bottomTitleWidget(void) const
	{ return myBottomTitleWidget; }

    
    inline QWidget *leftTitleWidget(void) const
	{ return myLeftTitleWidget; }

    
    inline QWidget *rightTitleWidget(void) const
	{ return myRightTitleWidget; }

    
    inline QWidget *topLeftCornerWidget(void) const
	{ return myTopLeftCornerWidget; }

    
    inline QWidget *topRightCornerWidget(void) const
	{ return myTopRightCornerWidget; }

    
    inline QWidget *bottomLeftCornerWidget(void) const
	{ return myBottomLeftCornerWidget; }

    
    inline QWidget *bottomRightCornerWidget(void) const
	{ return myBottomRightCornerWidget; }

public slots:
    /////// Cell Attribute Sets

    
    void setMargin(int margin);

    
    void setReadOnly(bool b);
    
    
    void setSelectedForegroundColor(const QColor &p);

    
    void setSelectedBackgroundColor(const QColor &p);

    
    void setPixmap(const QPixmap &p);

    
    void setPixmapSpacing(int sp);

    
    void setDisplayer(QicsCellDisplay *d);

    
    void setFormatter(QicsDataItemFormatter *d);

    
    void setAlignment(int flags);

    
    void setTextFlags(int flags);

    
    void setValidator(QValidator *v);

    
    void setLabel(const QString &label);

    
    void setMaxLength(int len);

    
    void setBorderWidth(int bw);

    
    void setBorderStyle(QicsLineStyle bs);

    
    void setBorderPen(const QPen &pen);

    /////// Grid Attribute Sets

    
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

    
    void setDragEnabled(bool b);

    /////// Table Attribute Sets

    // Headers
    
    void setTopHeaderVisible(bool);
    
    void setBottomHeaderVisible(bool);
    
    void setLeftHeaderVisible(bool);
    
    void setRightHeaderVisible(bool);

    
    void setTableMargin(int margin);

    
    void setTableSpacing(int spacing);

    
    void setGridSpacing(int spacing);

    
    void setRowHeaderUsesModel(bool b);

    
     void setColumnHeaderUsesModel(bool b);

    // Titles
    
    void setTopTitleWidget(QWidget *w);

    
    void setBottomTitleWidget(QWidget *w);

    
    void setLeftTitleWidget(QWidget *w);

    
    void setRightTitleWidget(QWidget *w);

    // Corners

    
    void setTopLeftCornerWidget(QWidget *w);

    
    void setTopRightCornerWidget(QWidget *w);

    
    void setBottomLeftCornerWidget(QWidget *w);

    
    void setBottomRightCornerWidget(QWidget *w);

    ////

    
    void setCurrentCell(int row, int col);

    
    void scroll(QicsScrollDirection dir, int num);

    
    void freezeTopRows(int num_rows);

    
    void freezeBottomRows(int num_rows);

    
    void unfreezeTopRows(void);

    
    void unfreezeBottomRows(void);

    
    void freezeLeftColumns(int num_cols);

    
    void freezeRightColumns(int num_cols);

    
    void unfreezeLeftColumns(void);

    
    void unfreezeRightColumns(void);

    
    void toggleRowHeightExpansion(int row);

    
    void toggleColumnWidthExpansion(int col);

    
    void setTopRow(int row);
    
    void setLeftColumn(int col);

    
    void setRepaintBehavior(QicsRepaintBehavior r);

    
    void setVisibleRows(int num);
    
    void setVisibleColumns(int num);

    
    bool traverseToCell(int row, int col);

    
    void traverseToBeginningOfTable(void);
    
    void traverseToEndOfTable(void);
    
    void traverseToBeginningOfRow(void);
    
    void traverseToEndOfRow(void);

    
    void traverseLeft(void);
    
    void traverseRight(void);
    
    void traverseUp(void);
    
    void traverseDown(void);

    
    void deleteColumn(int column);
    
    void addColumns(int howMany);
    
    void insertColumn(int column);
    
    void deleteRow(int row);
    
    void addRows(int rows);
    
    void insertRow(int row);



    
    virtual void cut(void);

    
    virtual void copy(void);

    
    virtual void paste(void);

signals:
    
    void pressed(int row, int col, int button, const QPoint &pos);

    
    void clicked(int row, int col, int button, const QPoint &pos);

    
    void doubleClicked(int row, int col, int button, const QPoint &pos);

    
    void currentCellChanged(int new_row, int new_col);

    
    void selectionListChanged(bool in_progress);

    
    void valueChanged(int row, int col);

protected slots:
    
    void revertRepaintBehavior(void);

    
    void handleHeaderDoubleClick(int idx, int button, QicsHeaderType type);

    
    void handleGridPress(int row, int col, int button, const QPoint &pos);

    
    void handleGridClick(int row, int col, int button, const QPoint &pos);

    
    void handleGridDoubleClick(int row, int col, int button, const QPoint &pos);

protected:
    void init(QicsDataModel *model,
	      QicsDataModel::Foundry rhdmf,
	      QicsDataModel::Foundry chdmf,
	      QicsTableGrid::Foundry tf,
	      QicsHeaderGrid::Foundry hf);

    virtual void initDataModels(QicsDataModel *dm);
    virtual void initObjects(void);
    virtual void initGridInfoObjects(void);

    inline QicsGridInfo &gridInfo(void) const
        { return myTableCommon->gridInfo(); }
    inline QicsGridInfo &rhGridInfo(void) const
        { return myTableCommon->rhGridInfo(); }
    inline QicsGridInfo &chGridInfo(void) const
        { return myTableCommon->chGridInfo(); }

    inline QicsStyleManager *styleManager(void) const
        { return gridInfo().styleManager(); }
    inline QicsStyleManager *rhStyleManager(void) const
        { return rhGridInfo().styleManager(); }
    inline QicsStyleManager *chStyleManager(void) const
        { return chGridInfo().styleManager(); }

    inline void setStyleManager(QicsStyleManager *sm)
        { gridInfo().setStyleManager(sm); }
    inline void setRHStyleManager(QicsStyleManager *sm)
        { rhGridInfo().setStyleManager(sm); }
    inline void setCHStyleManager(QicsStyleManager *sm)
        { chGridInfo().setStyleManager(sm); }

    inline QicsDimensionManager *dimensionManager(void) const
        { return gridInfo().dimensionManager(); }
    inline QicsDimensionManager *rhDimensionManager(void) const
        { return rhGridInfo().dimensionManager(); }
    inline QicsDimensionManager *chDimensionManager(void) const
        { return chGridInfo().dimensionManager(); }

    inline void setDimensionManager(QicsDimensionManager *sm)
        { gridInfo().setDimensionManager(sm); }
    inline void setRHDimensionManager(QicsDimensionManager *sm)
        { rhGridInfo().setDimensionManager(sm); }
    inline void setCHDimensionManager(QicsDimensionManager *sm)
        { chGridInfo().setDimensionManager(sm); }

    
    virtual void setMainGridViewport(const QicsRegion &vp);

    
    void initDisplay(void);

    
    QicsTableGrid *createGrid(int grid_row, int grid_col);

    
    QicsHeaderGrid *createHeader(QicsHeaderType type,
				 int grid_row, int grid_col,
				 int alignment = 0);

    
    void configureFrozen(void);

        QicsTableCommon *myTableCommon;

        QicsSelectionManager *mySelectionManager;

        QGridLayout *myGridLayout;
        QGridLayout *myMasterGrid;

        QicsScrollManager *myScrollManager;

        QicsScrollBarScroller *myRowScroller;
        QicsScrollBarScroller *myColumnScroller;

        QicsHeaderGrid::Foundry myHeaderGridFoundry;

        QicsTableGrid::Foundry myTableGridFoundry;

        QicsDataModel::Foundry myRowHeaderDMFoundry;

        QicsDataModel::Foundry myColumnHeaderDMFoundry;

    
    QicsTableGrid         *myGrids[3][3];
    
    QicsHeaderGrid       *myHHeaders[2][3];
    
    QicsHeaderGrid       *myVHeaders[3][2];

         int myNumTopFrozenRows;
         int myNumBottomFrozenRows;
         int myNumLeftFrozenColumns;
         int myNumRightFrozenColumns;

        QWidget *myTopTitleWidget;
        QWidget *myBottomTitleWidget;
        QWidget *myLeftTitleWidget;
        QWidget *myRightTitleWidget;

        QWidget *myTopLeftCornerWidget;
        QWidget *myTopRightCornerWidget;
        QWidget *myBottomLeftCornerWidget;
        QWidget *myBottomRightCornerWidget;

        QicsRegion myFullViewport;
};

#endif /* _QICSTABLE_H */
