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


#ifndef _QICSDIMENSIONMANAGER_H
#define _QICSDIMENSIONMANAGER_H

#include <QicsNamespace.h>
#include <QicsStyleManager.h>

#include <qobject.h>
#include <qvaluevector.h>
#include <qvaluelist.h>

///////////////////////////////////////////////////////////////////////////



class QicsDimensionManager : public QObject, public Qics
{
    Q_OBJECT
public:

    
    QicsDimensionManager(QicsStyleManager &sm);

    QicsDimensionManager(const QicsDimensionManager &dm, QicsStyleManager &sm);

    
    ~QicsDimensionManager();

    
    void setControllingRowDimensionManager(QicsDimensionManager *dm);
    
    void setControllingColumnDimensionManager(QicsDimensionManager *dm);

    
    void setStyleManager(QicsStyleManager &sm);

    
    void setDefaultFont(const QFont &fnt);

    
    void setRowFont(QicsGridType grid_type, int row, const QFont &fnt);
    
    void unsetRowFont(QicsGridType grid_type, int row);

    
    void setColumnFont(QicsGridType grid_type, int col, const QFont &fnt);
    
    void unsetColumnFont(QicsGridType grid_type, int col);

    
    void setCellFont(QicsGridType grid_type, int row, int col, const QFont &fnt);
    
    void unsetCellFont(QicsGridType grid_type, int row, int col);

    
    void setRowHeightInPixels(int row, int height);
    
    void setRowHeightInChars(int row, int height);

    
    void setColumnWidthInPixels(int col, int width);
    
    void setColumnWidthInChars(int col, int width);

    
    int rowHeight(int row) const;
    
    int columnWidth(int col) const;

    
    void setRowMinHeightInPixels(int row, int height);
    
    void setRowMinHeightInChars(int row, int height);

    
    void setColumnMinWidthInPixels(int col, int width);
    
    void setColumnMinWidthInChars(int col, int width);

    
    int rowMinHeight(int row) const;
    
    int columnMinWidth(int col) const;

    
    void setDefaultMargin(int margin);
    
    void setRowMargin(QicsGridType grid_type, int row, int margin);
    
    void setColumnMargin(QicsGridType grid_type, int col, int margin);
    
    void setCellMargin(QicsGridType grid_type, int row, int col, int margin);

    
    void setDefaultBorderWidth(int bw);
    
    void setRowBorderWidth(QicsGridType grid_type, int row, int bw);
    
    void setColumnBorderWidth(QicsGridType grid_type, int col, int bw);
    
    void setCellBorderWidth(QicsGridType grid_type, int row, int col, int bw);

    
    int regionHeight(const QicsRegion &region) const;
    
    int regionWidth(const QicsRegion &region) const;

    
    void overrideRowHeight(int row, int height);

    
    void overrideColumnWidth(int col, int width);

    
    void resetRowHeight(int row);

    
    void resetColumnWidth(int col);

    
    int overriddenRowHeight(int row) const;

    
    int overriddenColumnWidth(int col) const;

    void dumpWidths(void) const;
    void dumpHeights(void) const ;

public slots:
    
    void insertRows(int num, int start_position);
    
    void insertColumns(int num, int start_position);
    
    void deleteRows(int num, int start_position);
    
    void deleteColumns(int num, int start_position);

signals:
    
    void dimensionChanged();

// Should be protected, but the Sun Workshop compiler complains
public:
    enum QicsGridRole { QicsTableGridRole,
			QicsRowHeaderGridRole,
			QicsColumnHeaderGridRole };

    enum QicsDimensionMode { QicsDimensionUnset,
			     QicsDimensionPixel,
			     QicsDimensionChar };

protected:
    /////////////////////////////////////////////////////////////////////
    ////////////////  A bunch of helper structs/classes  ////////////////
    /////////////////////////////////////////////////////////////////////

    
    class QicsCellSetting
    {
    public:
	/// constructor
	QicsCellSetting();
	/// is anything set?
	bool isEmpty(void) const;

	/// role of this grid (table or header)
	QicsGridRole role;
	/// row index
	int row;
	/// column index
	int col;
	/// height of the font that was set in this cell
	int font_height;
	/// width of the font that was set in this cell
	int font_width;
	/// cell margin that was set in this cell
	int cell_margin;
	/// border width that was set in this cell
	int border_width;
    };
    /// vector of QicsCellSetting objects
    typedef QValueVector<QicsCellSetting> QicsCellSettingV;
    /// vector of pointers to QicsCellSetting objects
    typedef QValueVector<QicsCellSetting *> QicsCellSettingPV;

    
    class QicsRowSetting
    {
    public:
	/// constructor
	QicsRowSetting();
	/// is anything set?
	bool isEmpty(void) const;

	/// role of this grid (table or header)
	QicsGridRole role;
	/// row index
	int row;
	/// height of the font that was set for this row
	int font_height;
	/// width of the font that was set for this row
	int font_width;
	/// cell margin that was set for this row
	int cell_margin;
	/// border width that was set for this row
	int border_width;
    };
    /// vector of QicsRowSetting objects
    typedef QValueVector<QicsRowSetting> QicsRowSettingV;
    /// vector of pointers to QicsRowSetting objects
    typedef QValueVector<QicsRowSetting *> QicsRowSettingPV;

    
    class QicsColumnSetting
    {
    public:
	/// constructor
	QicsColumnSetting();
	/// is anything set?
	bool isEmpty(void) const;

	/// role of this grid (table or header)
	QicsGridRole role;
	/// column index
	int col;
	/// height of the font that was set for this column
	int font_height;
	/// width of the font that was set for this column
	int font_width;
	/// cell margin that was set for this column
	int cell_margin;
	/// border width that was set for this column
	int border_width;
    };
    ///  \internal vector of QicsColumnSetting objects
    typedef QValueVector<QicsColumnSetting> QicsColumnSettingV;
    ///  \internal vector of pointers to QicsColumnSetting objects
    typedef QValueVector<QicsColumnSetting *> QicsColumnSettingPV;

    
    class QicsDefaultDimensionSetting {
    public:
	
	QicsDefaultDimensionSetting();
	
	QicsDefaultDimensionSetting(const QicsDefaultDimensionSetting &);
	
	void setFont(const QFont &fnt);
	
	void compute(void);

	/// the default dimension mode
	QicsDimensionMode mode;
	/// default height
	int height;
	/// default width
	int width;
	/// default height in character units
	int height_chars;
	/// default width in character units
	int width_chars;
	/// height of default font
	int font_height;
	/// height of default font
	int font_width;
	/// default cell_margin
	int cell_margin;
	/// default border width
	int border_width;
    };

    
    class QicsRowHeight {
    public:
	
	QicsRowHeight();
	
	QicsRowHeight(const QicsRowHeight &rh);

	/// the mode of this height setting
	QicsDimensionMode mode;
	/// height of this row in pixels
	int height;
	/// height of this row in character units
	int chars;
    };
    /// vector of QicsRowHeight objects
    typedef QValueVector<QicsRowHeight> QicsRowHeightV;
    /// vector of pointers to QicsRowHeight objects
    typedef QValueVector<QicsRowHeight *> QicsRowHeightPV;

    
    class QicsColumnWidth {
    public:
	
	QicsColumnWidth();
	
	QicsColumnWidth(const QicsColumnWidth &cw);

	/// the mode of this width setting
	QicsDimensionMode mode;
	/// width of this column in pixels
	int width;
	/// width of this column in character units
	int chars;
    };
    ///  \internal vector of QicsColumnWidth objects
    typedef QValueVector<QicsColumnWidth> QicsColumnWidthV;
    ///  \internal vector of pointers to QicsColumnWidth objects
    typedef QValueVector<QicsColumnWidth *> QicsColumnWidthPV;

    class QicsOverrideSetting {
    public:
	inline QicsOverrideSetting()
	    { myIndex = -1; myVal = -1; }
	inline QicsOverrideSetting(int idx, int val)
	    { myIndex = idx; myVal = val; }

	inline QicsOverrideSetting(const QicsOverrideSetting &os)
	    { myIndex = os.myIndex; myVal = os.myVal; }

	inline int index(void) const
	    { return myIndex; }

	inline void setIndex(int idx)
	    { myIndex = idx; }

	inline int value(void) const
	    { return myVal; }

	inline void setValue(int val)
	    { myVal = val;; }

    protected:
	int myIndex;
	int myVal;
    };
    typedef QValueList<QicsOverrideSetting> QicsOverrideSettingL;

    ////////////////////////////////////////////////////////////////////////

    
    QicsGridRole gridRole(QicsGridType grid_type) const;

    
    bool computeRowHeight(int row, QicsRowHeightPV &row_heights,
			  QicsDefaultDimensionSetting &default_dims);

    
    bool computeColumnWidth(int col, QicsColumnWidthPV &col_widths,
			    QicsDefaultDimensionSetting &default_dims);


    
    bool computeDefaultRowFontHeight(void);
    
    bool computeDefaultColumnFontWidth(void);

    
    bool computeAllRowHeights(void);
    
    bool computeAllColumnWidths(void);

        QicsStyleManager &myStyleManager;

        QicsDimensionManager *myRowDM;
        QicsDimensionManager *myColumnDM;

        QicsCellSettingV mySetCells;
        QicsRowSettingV mySetRows;
        QicsColumnSettingV mySetColumns;

        QicsRowHeightPV myRowHeights;
        QicsColumnWidthPV myColumnWidths;

        QicsRowHeightPV myRowMinHeights;
        QicsColumnWidthPV myColumnMinWidths;

        QicsDefaultDimensionSetting myOrigDefaultDimensions;
        QicsDefaultDimensionSetting myCurrentDefaultDimensions;

        QicsDefaultDimensionSetting myOrigDefaultMinDimensions;
        QicsDefaultDimensionSetting myCurrentDefaultMinDimensions;

    QicsOverrideSettingL myRowOverrides;
    QicsOverrideSettingL myColumnOverrides;

    friend class QicsDimensionManager::QicsRowHeight;
    friend class QicsDimensionManager::QicsColumnWidth;
    friend class QicsDimensionManager::QicsDefaultDimensionSetting;
};

#include <QicsMappedDimensionManager.h>
#endif /* _QICSDIMENSIONMANAGER_H */
