// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/***************************************************************************
 *   Copyright (C) 2004 by Lukas Tinkl                                     *
 *   lukas.tinkl@suse.cz                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _RU_FIND_DIALOG_H_
#define _RU_FIND_DIALOG_H_

class QSpacerItem;
class QButtonGroup;
class QLabel;
class QCheckBox;
class KLineEdit;
class QGridLayout;

#include <kdialogbase.h>

/**
 * This class provides a find dialog for the Resource Usage view (ResUsageView)
 *
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 * @short Find dialog for the Resource Usage view
 */
class ruFindDlg : public KDialogBase
{
    Q_OBJECT
public:
    /**
     * CTOR
     * @param data the stringlist to work on
     */
    ruFindDlg( QStringList data, QWidget* parent = 0, const char* name = 0 );
    ~ruFindDlg();

signals:
    /**
     * Signals that we found a matching entry, or -1 if none
     */
    void signalMatch( int );

protected slots:
    /**
     * Invoked when the 'Find' button is pressed, start the search
     */
    virtual void slotUser1();

private slots:
    /**
     * Enable/disable the 'Find' button depending on the text's length
     */
    void slotTextChanged( const QString & text );

private:
    /**
     * Start the search function
     */
    void startSearch();

    /**
     * Go to the next match (after first)
     */
    void findNext();

    QButtonGroup* grpResource;
    QLabel* lbName;
    QCheckBox* cbCaseSensitive;
    KLineEdit* leResource;
    QCheckBox* cbRegExp;
    QSpacerItem* spacer3;
    QGridLayout* grpResourceLayout;

    /// our data to search in
    QStringList m_data;
    /// the result list of row numbers
    QValueList<int> m_result;
    /// distinguish find/find next
    bool m_firstRun;
};

#endif
