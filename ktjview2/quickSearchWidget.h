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

#ifndef _QUICK_SEARCH_WIDGET_H_
#define _QUICK_SEARCH_WIDGET_H_

#include <qwidget.h>

class QLabel;
class KListView;
class KListViewSearchLine;
class KToolBarButton;

/**
 * This widget provides a quick search component, suitable
 * for plugging into toolbars using KWidgetAction.
 * It contains a linedit (KListViewSearchLine) for filtering
 * a listview and a tool button to clear the search.
 *
 * @short Quick Search component
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class QuickSearchWidget: public QWidget
{
    Q_OBJECT
public:
    QuickSearchWidget( QWidget * parent, const char * name = 0 );
    ~QuickSearchWidget() { };

    /**
     * Tell the internal listview to operate on @p view
     */
    void setListView( KListView * view );

public slots:
    /**
     * Clear the contents of the search lineedit
     */
    void reset();

private:
    KToolBarButton * m_clearButton;
    QLabel * m_searchLabel;
    KListViewSearchLine * m_searchLine;
};

#endif
