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


#ifndef _QICSSELECTIONMANAGER_H
#define _QICSSELECTIONMANAGER_H

#include <QicsGrid.h>
#include <QicsStyleManager.h>
#include <QicsDataModel.h>
#include <QicsSelection.h>

#include <qvaluevector.h>
#include <qmemarray.h>

//#define DEBUG_SELECTION 1

class QicsHeaderGrid;


class QicsSelectionManager: public QObject, public Qics
{
    Q_OBJECT

public:
    
    QicsSelectionManager();
    
    virtual ~QicsSelectionManager(void);

    
    QicsDataModel *dataModel(void) const;

    
    void setDataModel(QicsDataModel *sm);

    
    QicsStyleManager *styleManager(void) const;

    
    void setStyleManager(QicsStyleManager *sm);

    
    void setGridInfo(QicsGridInfo *);

    
    void addHeader(QicsHeaderGrid *hdr);

    
    void removeHeader(QObject *hdr);

    
    QicsSelectionList *selectionList(void) const;
    
    void setSelectionList(QicsSelectionList &sel_list);
    
    void clearSelectionList(void);

    
    QicsSelectionList *selectionActionList(void) const;

    
    void addSelection(QicsSelection &selection);

    
    inline QicsSelectionPolicy selectionPolicy(void) const
	{ return mySelectionPolicy; }
    
    void setSelectionPolicy(QicsSelectionPolicy policy);

    
    void processSelectionEvent(QicsSelectionType stype,
			       int begin_row, int begin_col,
			       int end_row, int end_col);

signals:
    
    void selectionListChanged(bool in_progress);

    
    void selectionCellsChanged(QicsRegion);

protected:

    // List of widgets
    typedef QValueVector<QicsGrid *> QicsGridPV;
    typedef QValueVector<QicsHeaderGrid *> QicsHeaderGridPV;

    
    enum QicsSelectState { QicsSelectFalse, QicsSelectTrue,
			   QicsSelectFalseRevert, QicsSelectTrueRevert };

    
    void beginSelection(int begin_row, int begin_col, int end_row, int end_col);
    
    void dragSelection(int begin_row, int begin_col, int end_row, int end_col);
    
    void extendSelection(int begin_row, int begin_col, int end_row, int end_col);
    
    void addSelection(int begin_row, int begin_col, int end_row, int end_col);
    
    void endSelection(int begin_row, int begin_col, int end_row, int end_col);
    
    void deleteSelection(void);
    
    void invalidateSelection(int begin_row, int begin_col,
			     int end_row, int end_col);

    
    void setSelectionProperty(const QicsSelection &selection,
			      QicsSelectionManager::QicsSelectState sel);
    
    void setSelectionProperty(int begin_row, int begin_col,
			      int end_row, int end_col,
			      QicsSelectionManager::QicsSelectState sel);

    
    void setRowSelectionProperty(QicsGridInfo &, int row,
				 QicsSelectionManager::QicsSelectState sel,
				 QicsStyleManager *sm);

    
    void setColumnSelectionProperty(QicsGridInfo &, int col,
				    QicsSelectionManager::QicsSelectState sel,
				    QicsStyleManager *sm);

    
    void addToSelectionList(QicsSelection &sel);

    
    QicsSelection &findSelectionBlock(int anchor_row, int anchor_col);

    
    void announceChanges(bool in_progress);

    void handleInsertRows(QicsSelectionList *slist, int num, int pos);
    void handleInsertColumns(QicsSelectionList *slist, int num, int pos);
    void handleDeleteRows(QicsSelectionList *slist, int num, int pos);
    void handleDeleteColumns(QicsSelectionList *slist, int num, int pos);

        QicsGridInfo *myGridInfo;
        QicsStyleManager *myStyleManager;
        QicsDataModel *myDataModel;

        QicsSelection myCurrentSelection;

        QicsSelectState myCurrentDragAction;

        QicsSelectionList *mySelectionList;

        QicsSelectionList *mySelectionActionList;

        QicsHeaderGridPV myHeaderList;

        /// has changed since the last grid redraw.
    QicsRegion myAffectedRegion;

        QicsSelectionPolicy mySelectionPolicy;

        bool mySelectionChangedFlag;

public slots:
    
    void orderChanged(QicsIndexType, QMemArray<int>);

protected slots:
    
    void insertRows(int num, int start_position);
    
    void insertColumns(int num, int start_position);
    
    void deleteRows(int num, int start_position);
    
    void deleteColumns(int num, int start_position);

private:
};

#endif /* _QICSSELECTIONMANAGER_H */
