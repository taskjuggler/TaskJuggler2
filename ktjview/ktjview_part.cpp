#include "ktjview_part.h"
#include <kshortcut.h>
#include <kiconloader.h>
#include <kaccel.h>
#include <kaction.h>
#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kparts/genericfactory.h>
#include <kdebug.h>
#include <qsplitter.h>
#include <kdatewidget.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>

#include "ktvtasktable.h"
#include "ktjgantt.h"
#include "ktvtaskcanvasview.h"

#include "TjMessageHandler.h"
#include <kshortcut.h>

/* Handler for TaskJuggler error messages. */
TjMessageHandler TJMH(TRUE);

typedef KParts::GenericFactory<KTjviewPart> KTjviewPartFactory;
K_EXPORT_COMPONENT_FACTORY( libktjviewpart, KTjviewPartFactory );

KTjviewPart::KTjviewPart( QWidget *parentWidget, const char *,
			  QObject *parent, const char *name,
			  const QStringList & /*args*/ )
    : KParts::ReadOnlyPart(parent, name)
{
    // we need an instance
    setInstance( KTjviewPartFactory::instance() );

    // this should be your custom internal widget
    m_gantt = new KTJGantt( parentWidget, "GANTT" );
    connect( m_gantt, SIGNAL( statusBarChange( const QString& )),
	     this, SLOT(  slChangeStatusBar( const QString& )));

    // notify the part that this is our internal widget
    setWidget(m_gantt);

    // create our actions
    setupActions();
    // set our XML-UI resource file
    setXMLFile("ktjview_part.rc");

}

void KTjviewPart::setupActions()
{
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    (void) new KAction( i18n("&Reload"), "reload",
			KShortcut( "Ctrl+R" ), this,
			SLOT(slReload()), actionCollection(), "reload");

    (void) new KAction(i18n("Zoom In"), "viewmag+", CTRL+Key_I,
		       m_gantt, SLOT(slZoomIn()),
		       actionCollection(), "zoomIn" );

    (void) new KAction(i18n("Zoom Out"), "viewmag-", CTRL+Key_O,
		       m_gantt, SLOT(slZoomOut()),
		       actionCollection(), "zoomOut" );

    (void) new KAction(i18n("Zoom 1:1"), "viewmag1", CTRL+Key_S,
		       m_gantt, SLOT(slZoomO riginal()),
		       actionCollection(), "zoomOriginal" );

    (void) new KAction(i18n("Timeframe"), "history", CTRL+Key_T,
		       m_gantt, SLOT(slTimeFrame()),
		       actionCollection(), "timeFrame" );


}


KTjviewPart::~KTjviewPart()
{
}


KAboutData *KTjviewPart::createAboutData()
{
    // the non-i18n name here must be the same as the directory in
    // which the part's rc file is installed ('partrcdir' in the
    // Makefile)
    KAboutData *aboutData = new KAboutData("ktjviewpart", I18N_NOOP("KTjviewPart"), "0.1");
    aboutData->addAuthor("Klaas Freitag", 0, "freitag@kde.org");
    return aboutData;
}

bool KTjviewPart::openFile()
{
    // m_file is always local so we can use QFile on it
    QFile file(m_file);
    if (file.open(IO_ReadOnly) == false)
        return false;

    m_project = new Project();
    // p->setDebugLevel(3);
    m_project->loadFromXML( m_file );

    m_gantt->showProject(m_project);

    // just for fun, set the status bar
    emit setStatusBarText( m_url.prettyURL() );

    return true;
}

void KTjviewPart::slReload()
{
    slChangeStatusBar( i18n("Reverting file"));

    delete m_project;
    openFile();

}


void KTjviewPart::slChangeStatusBar( const QString& str )
{
   emit setStatusBarText( str );
}


void KTjviewPart::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QString file_name = KFileDialog::getOpenFileName(QString::null,
						     "*.xml|TaskJuggler xml\n*.tjx|TaskJuggler output");

    if (file_name.isEmpty() == false)
        openURL(file_name);
}


#include "ktjview_part.moc"
