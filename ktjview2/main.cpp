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

#include "ktjview2.h"
#include "version.h"

#include <kapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char description[] =
    I18N_NOOP("A KDE frontend to TaskJuggler");

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("ktjview2", I18N_NOOP("TaskJuggler IDE"), KTJIDE_VERSION, description,
                     KAboutData::License_GPL, "(C) 2004 SUSE AG", 0,
                     "http://www.taskjuggler.org/ide.php", "lukas.tinkl@suse.cz");
    about.addAuthor( "Lukáš Tinkl", I18N_NOOP( "Author and maintainer " ), "lukas.tinkl@suse.cz" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    // register ourselves as a dcop client
    app.dcopClient()->registerAs(app.name(), false);

    // see if we are starting with session management
    if (app.isRestored())
    {
        RESTORE(ktjview2);
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if (args->count() == 0)
        {
            ktjview2 *widget = new ktjview2;
            widget->show();
        }
        else
        {
            int i = 0;
            for (; i < args->count(); i++)
            {
                ktjview2 *widget = new ktjview2;
                widget->show();
                widget->load(args->url(i));
            }
        }
        args->clear();
    }

    return app.exec();
}
