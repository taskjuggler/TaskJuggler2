/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * As a special exception, you have permission to link this program with
 * the Qt3 library and distribute executables, as long as you follow the
 * requirements of the GNU GPL version 2 in regard to all of the software
 * in the executable aside from Qt3.
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

static const char version[] = VERSION;

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
                     "(c) 2002, 2003, 2004, 2005, 2006 Chris Schlaeger",
                     I18N_NOOP("TaskJuggler is a project management tool for "
                               "Linux and UNIX-like operating systems. Its \n"
                               "new approach to project planning and tracking "
                               "is superior to the commonly used Gantt chart \n"
                               "editing tools. It has already been "
                               "successfully used in many projects and \n"
                               "scales easily to projects with hundreds of "
                               "resources and thousands of tasks."),
                     "http://www.taskjuggler.org",
                     "bugs@taskjuggler.org");
    about.addAuthor("Chris Schlaeger", 0, "cs@kde.org" );

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    // register ourselves as a dcop client
    app.dcopClient()->registerAs(app.name(), false);

    // see if we are starting with session management
    if (app.isRestored())
    {
        for (int i = 1; KMainWindow::canBeRestored(i); ++i)
        {
            TaskJuggler* widget = new TaskJuggler;
            widget->restore(i);
            if (i == 1)
                widget->loadLastProject();
        }
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
        else
            widget->loadLastProject();

        args->clear();
    }

    return app.exec();
}

