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

#ifndef _FILTER_DIALOG_
#define _FILTER_DIALOG_

class QListBox;

class KDialogBase;

class Resource;
class Project;
class FilterWidget;

/**
 * Dialog for defining filters
 *
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 * @short Filter dialog
 */
class FilterDialog: public KDialogBase
{
    Q_OBJECT
public:
    /**
     * Standard CTOR
     */
    FilterDialog( QWidget * parent = 0, const char * name = 0 );

    /**
     * Standard DTOR
     */
    ~FilterDialog();

    /**
     * Load a list of filters from the XML definition
     */
    void loadFilterList();

    /**
     * Save the filters to the XML file
     */
    void saveFilters();

private slots:
    /**
     * Add a line of conditions
     */
    void slotMore();

    /**
     * Remove a line of conditions
     */
    void slotFewer();

    /**
     * Clear all the conditions
     */
    void slotClear();

    /**
     * Create a new empty filter
     */
    void slotNewFilter();

    /**
     * Remove the currently selected filter
     */
    void slotDeleteFilter();

    /**
     * Rename the current filter
     */
    void slotRenameFilter();

    /**
     * React when the filter changes in the listbox
     * @param item The new filter's item
     */
    void slotFilterChanged( QListBoxItem * item );

private:
    /**
     * @return true if a filter with @p name already exists
     */
    bool filterExists( const QString & name ) const;

    /// the base widget
    FilterWidget * m_base;

    /// list of filter criteria conditions
    QStringList m_conditions;
};

#endif
