// local includes
#include "editorView.h"

// Qt includes
#include <qstring.h>
#include <qlayout.h>
#include <qfile.h>

// KDE includes
#include <klibloader.h>
#include <ktrader.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <ktexteditor/editinterface.h>

EditorView::EditorView( QWidget * parent, const char * name )
    : QWidget( parent, name ), m_view( 0 ), m_doc( 0 )
{
    m_layout = new QVBoxLayout( this );
    init();
}

EditorView::~EditorView()
{
    m_view = 0;
    m_doc = 0;
}

QString EditorView::documentPath() const
{
    return m_path;
}

bool EditorView::init()
{
    KTrader::OfferList offers = KTrader::self()->query( "KTextEditor/Document" );
    if( offers.isEmpty() )
    {
        KMessageBox::error( this, i18n( "Cannot start a text editor component.\n"
                                        "Please check your KDE installation." ) );
        m_doc = 0;
        m_view = 0;
        return false;
    }

    KService::Ptr service = *offers.begin();
    KLibFactory *factory = KLibLoader::self()->factory( service->library().latin1() );
    if( !factory )
    {
        KMessageBox::error( this, i18n( "Cannot start a text editor component.\n"
                                        "Please check your KDE installation." ) );
        m_doc = 0;
        m_view = 0;
        return false;
    }

    m_doc = static_cast<KTextEditor::Document *>( factory->create( this, 0, "KTextEditor::Document" ) );

    if( !m_doc )
    {
        KMessageBox::error( this, i18n( "Cannot start a text editor component.\n"
                                        "Please check your KDE installation." ) );
        m_doc = 0;
        m_view = 0;
        return false;
    }
    m_view = m_doc->createView( this, 0 );
    m_layout->addWidget( static_cast<QWidget *>( m_view ), 1 );
    static_cast<QWidget *>( m_view )->show();

    return true;
}

bool EditorView::loadDocument( const QString & path )
{
    if ( !m_doc )
        return false;

    bool result = false;

    QFile file( path );
    if ( file.open( IO_ReadOnly ) )
    {
        QTextStream stream( &file );
        QString text = stream.read();

        dynamic_cast<KTextEditor::EditInterface *>( m_doc )->setText( text );
        m_doc->setReadWrite( false ); // TODO
        m_doc->setModified( false );
        result = true;
    }
    else
        result = false;

    file.close();
    return result;
}

void EditorView::closeDocument()
{
    dynamic_cast<KTextEditor::EditInterface *>( m_doc )->clear();
    m_path = QString();
}

#include "editorView.moc"
