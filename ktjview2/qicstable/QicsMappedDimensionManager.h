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


#ifndef _QICSMAPPEDDIMENSIONMANAGER_H
#define _QICSMAPPEDDIMENSIONMANAGER_H

#include <qobject.h>

///////////////////////////////////////////////////////////////////////////

class QicsDimensonManager;
class QicsGrid;
class QicsGridInfo;




class QicsMappedDimensionManager /* : public QObject, public Qics */
{
    /* Q_OBJECT */
public:

    
    QicsMappedDimensionManager(QicsDimensionManager *, QicsGridInfo *);

    
    ~QicsMappedDimensionManager();

    
    void setDefaultFont(const QFont &fnt);

    
    void setRowFont(Qics::QicsGridType grid_type, int row, const QFont &fnt);
    
    void unsetRowFont(Qics::QicsGridType grid_type, int row);

    
    void setColumnFont(Qics::QicsGridType grid_type, int col, const QFont &fnt);
    
    void unsetColumnFont(Qics::QicsGridType grid_type, int col);

    
    void setCellFont(Qics::QicsGridType grid_type, int row, int col, const QFont &fnt);
    
    void unsetCellFont(Qics::QicsGridType grid_type, int row, int col);

    
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
    
    void setRowMargin(Qics::QicsGridType grid_type, int row, int margin);
    
    void setColumnMargin(Qics::QicsGridType grid_type, int col, int margin);
    
    void setCellMargin(Qics::QicsGridType grid_type, int row, int col, int margin);

    
    void setDefaultBorderWidth(int bw);
    
    void setRowBorderWidth(Qics::QicsGridType grid_type, int row, int bw);
    
    void setColumnBorderWidth(Qics::QicsGridType grid_type, int col, int bw);
    
    void setCellBorderWidth(Qics::QicsGridType grid_type, int row, int col, int bw);

    
    int regionHeight(const QicsRegion &region) const;
    
    int regionWidth(const QicsRegion &region) const;

private:
    QicsDimensionManager *myDM;
    QicsGridInfo *myInfo;
};

#endif /* _QICSMAPPEDDIMENSIONMANAGER_H */
