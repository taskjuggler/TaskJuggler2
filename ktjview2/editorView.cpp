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

    connect( doc(), SIGNAL( textChanged() ), this, SIGNAL( textChanged() ) );  // FIXME check this
    connect( m_view, SIGNAL( cursorPositionChanged() ), this, SIGNAL( cursorChanged() ) );
    connect( doc(), SIGNAL( selectionChanged() ), this, SIGNAL( selectionChanged() ) );

    return true;
}

bool EditorView::loadDocument( const KURL & url )
{
    if ( !doc() )
        return false;

    kdDebug() << "Opening URL " << url << endl;

    doc()->setReadWrite( true );
    return doc()->openURL( url ); // also calls closeURL()
}

void EditorView::closeDocument()
{
    doc()->closeURL();
    doc()->setReadWrite( false );
}

#include "editorView.moc"
