/*
 * TjMessageHandler.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _TjMessageHandler_h_
#define _TjMessageHandler_h_

#include <qobject.h>
#include <qstring.h>

/**
 * This class handles all error or warning messages that the library functions
 * can send out. Depending on the mode it either send the messages to STDERR
 * or raises a Qt signal.
 *
 * @short Handles all error or warning messages.
 * @author Chris Schlaeger <cs@suse.de>
 */
class TjMessageHandler : public QObject
{
    Q_OBJECT
public:
    TjMessageHandler(bool cm) : consoleMode(cm) { }
    virtual ~TjMessageHandler() { }

    void warningMessage(const QString& msg, const QString& file = QString::null,
                   int linei = -1);
    void errorMessage(const QString& msg, const QString& file = QString::null, int
                 line = -1);
    void fatalMessage(const QString& msg, const QString& file = QString::null, int
                 line = -1);

signals:
    void printWarning(const QString& msg, const QString& file, int line);
    void printError(const QString& msg, const QString& file, int line);
    void printFatal(const QString& msg, const QString& file, int line);

private:
    TjMessageHandler() { }  // Don't use this.
    bool consoleMode;
} ;

extern TjMessageHandler TJMH;

#endif

