/*
 * TjMessageHandler.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "TjMessageHandler.h"

TjMessageHandler TJMH(true);

void
TjMessageHandler::warningMessage(const QString& msg, const QString& file, int
                                 line)
{
    if (consoleMode)
    {
        if (file.isEmpty())
            qWarning("%s", msg.latin1());
        else
            qWarning("%s:%d: %s", file.latin1(), line, msg.latin1());
    }
    else
        printWarning(msg, file, line);
}

void
TjMessageHandler::errorMessage(const QString& msg, const QString& file, int
                               line)
{
    if (consoleMode)
    {
        if (file.isEmpty())
            qWarning("%s", msg.latin1());
        else
            qWarning("%s:%d: %s", file.latin1(), line, msg.latin1());
    }
    else
        printError(msg, file, line);
}

void
TjMessageHandler::fatalMessage(const QString& msg, const QString& file, int
                               line)
{
    if (consoleMode)
    {
        if (file.isEmpty())
            qFatal("%s", msg.latin1());
        else
            qFatal("%s:%d: %s", file.latin1(), line, msg.latin1());
    }
    else
        printFatal(msg, file, line);
}

#include "TjMessageHandler.moc"
