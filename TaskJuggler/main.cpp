/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */


#include "taskjuggler.h"
#include <kapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char description[] =
    I18N_NOOP("A Project Management Software for Linux");

static const char version[] = "0.1";

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP("Project (TJP File) to open"), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("taskjuggler", I18N_NOOP("TaskJuggler"), version,
                     description,
                     KAboutData::License_GPL,
                     "(c) 2002, 2003, 2004, 2005 Chris Schlaeger",
                     0, 0, "cs@suse.de");
    about.addAuthor( "Chris Schlaeger", 0, "cs@suse.de" );

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    // register ourselves as a dcop client
    app.dcopClient()->registerAs(app.name(), false);

    // see if we are starting with session management
    if (app.isRestored())
    {
        RESTORE(TaskJuggler);
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        TaskJuggler* widget = new TaskJuggler;
        app.setMainWidget(widget);
        widget->show();
        if (args->count() > 0)
            widget->load(args->url(0));
        args->clear();
    }

    return app.exec();
}

