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

#ifndef _EDITOR_VIEW_H_
#define _EDITOR_VIEW_H_

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

class KURL;
class QVBoxLayout;

/**
 * Widget for editing the source code of the project
 * (uses the KTextEditor interfaces).
 * @short Editor widget
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class EditorView: public QWidget
{
    Q_OBJECT
public:
    /**
     * Standard constructor
     */
    EditorView( QWidget * parent = 0, const char * name = 0 );

    virtual ~EditorView();

    /**
     * @return the URL of the document being displayed
     */
    KURL url() const;

    /**
     * Load a document (project definition) from @p url
     *
     * @return true on success
     */
    bool loadDocument( const KURL & url );

    /**
     * Close the current document, clear the view
     */
    void closeDocument();

    /**
     * @return KTextEditor::Document bound to #view()
     */
    KTextEditor::Document * doc() const
        { return m_view->document(); }

    /**
     * @return KTextEditor::View bound to this component
     */
     KTextEditor::View * view() const
        { return m_view; }

private:
    /**
     * Initialize the KTextEditor components (Document and View)
     */
    bool init();

    /// text view
    KTextEditor::View * m_view;
    /// main layout
    QVBoxLayout * m_layout;
};

#endif
