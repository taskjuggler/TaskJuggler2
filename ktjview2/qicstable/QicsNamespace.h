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


#ifndef _QICSNAMESPACE_H
#define _QICSNAMESPACE_H

#include <qglobal.h>

#define QICSTABLE_VERSION "qicstable_version: 1.0.1 (13-Feb-04)"

#include <limits.h>

#if defined(Q_WS_X11)
#include <assert.h>
#endif










class Qics {
public:


#if defined(Q_WS_X11)
    static const int QicsLAST_ROW;
    static const int QicsLAST_COLUMN;
#endif

    
    enum QicsCellOverflowBehavior { Clip = 0,
				    Overflow,
				    ToolTip };

    
    enum QicsCurrentCellStyle { Spreadsheet = 0,
				NormalSelected };

    
    enum QicsGridCellClipping { AllowPartial = 0,
				NoDisplayOnPartial,
				UseClippedSymbol };

    
    enum QicsGridType { TableGrid = 0,
			RowHeaderGrid,
			ColumnHeaderGrid };

    
    enum QicsHeaderType { RowHeader = 0,
			  ColumnHeader };


    
    enum QicsIndexType { RowIndex,
			 ColumnIndex };
    
    
    enum QicsLineStyle { None = 0,
			 Plain,
			 Raised,
			 Sunken };

    
    enum QicsRepaintBehavior { RepaintOff = 0,
			       RepaintOn = 1,
			       RepaintCalcOnly = 2 };

    
    enum QicsScrollBarMode { Auto = 0,
			     AlwaysOff,
			     AlwaysOn };

    
    enum QicsScrollDirection { ScrollNone = 0,
			       ScrollUp,
			       ScrollDown,
			       ScrollLeft,
			       ScrollRight };

    
    enum QicsSelectionPolicy { SelectNone = 0,
			       SelectSingle,
			       SelectMultiple,
			       SelectSingleRow,
			       SelectMultipleRow };

    
    enum QicsSelectionType { SelectionNone = 0,
			     SelectionBegin,
			     SelectionDrag,
			     SelectionEnd,
			     SelectionExtend,
			     SelectionAdd };


    
    enum QicsSortOrder { Ascending = 0,
	    		 Descending };

    
    enum QicsTableDisplayOption { DisplayNever = 0,
				  DisplayAlways,
				  DisplayFirstPage };
};

#endif /* _QICSNAMESPACE_H */
