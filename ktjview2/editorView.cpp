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

// local includes
#include "editorView.h"

// Qt includes
#include <qstring.h>
#include <qlayout.h>
#include <qfile.h>

// KDE includes
#include <kmessagebox.h>
#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>

#include <ktexteditor/editorchooser.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/markinterface.h>

EditorView::EditorView( QWidget * parent, const char * name )
    : QWidget( parent, name ), m_view( 0 )
{
    m_layout = new QVBoxLayout( this );
    init();
}

EditorView::~EditorView()
{
    delete m_view->document();
}

KURL EditorView::url() const
{
    if ( doc() )
        return doc()->url();
    else
        return KURL();
}

bool EditorView::init()
{
    KTextEditor::Document *document;
    if ( !(document = KTextEditor::EditorChooser::createDocument( this, "KTextEditor::Document" ) ) )
    {
        KMessageBox::error( this, i18n( "A KDE text-editor component could not be found;\n"
                                        "please check your KDE installation." ) );
        return false;
    }

    m_view = document->createView( this, "text_view" );
    doc()->setReadWrite( false );
    doc()->setModified( false );

    m_layout->addWidget( m_view, 1 );
    m_view->show();

    return true;
}

bool EditorView::loadDocument( const KURL & url )
{
    if ( !doc() )
        return false;

    doc()->setReadWrite( true );

    if ( url.url().endsWith( ".tjx" ) ) // XML file, perhaps compressed
    {
        KTextEditor::editInterface( doc() )->setText( i18n( "This file is compressed XML. Editting has been disabled." ) );
        doc()->setModified( false );
        doc()->setReadWrite( false );
        return false;
    }

    //kdDebug() << "Opening URL " << url << endl;

    KTextEditor::markInterface( doc() )->clearMarks();
    return doc()->openURL( url ); // also calls closeURL()
}

void EditorView::closeDocument()
{
    doc()->closeURL();
    doc()->setReadWrite( false );
}

void EditorView::gotoLine( int number )
{
    KTextEditor::viewCursorInterface( m_view )->setCursorPosition( number, 0 );
}

#include "editorView.moc"
