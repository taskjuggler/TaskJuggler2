#include "ktjview_part.h"

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kparts/genericfactory.h>
#include <kdebug.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>

#include "ktvtasktable.h"

typedef KParts::GenericFactory<KTjviewPart> KTjviewPartFactory;
K_EXPORT_COMPONENT_FACTORY( libktjviewpart, KTjviewPartFactory );

KTjviewPart::KTjviewPart( QWidget *parentWidget, const char *widgetName,
			  QObject *parent, const char *name,
			  const QStringList & /*args*/ )
    : KParts::ReadOnlyPart(parent, name)
{
    // we need an instance
    setInstance( KTjviewPartFactory::instance() );

    // this should be your custom internal widget
    m_widget = new KTVTaskTable( parentWidget, widgetName );

    // notify the part that this is our internal widget
    setWidget(m_widget);

    // create our actions
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());

    // set our XML-UI resource file
    setXMLFile("ktjview_part.rc");

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

    Project *p = new Project();
    p->loadFromXML( m_file );

    m_widget->showProject(p);
#if 0
    // our example widget is text-based, so we use QTextStream instead
    // of a raw QDataStream
    QTextStream stream(&file);
    QString str;
    while (!stream.eof())
    {
       QString goof = stream.readLine();
       kdDebug() << "# " << goof << endl;
       str += stream.readLine() + "\n";
    }

    file.close();
#endif
    // just for fun, set the status bar
    emit setStatusBarText( m_url.prettyURL() );

    return true;
}

void KTjviewPart::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QString file_name = KFileDialog::getOpenFileName();

    if (file_name.isEmpty() == false)
        openURL(file_name);
}

#include "ktjview_part.moc"