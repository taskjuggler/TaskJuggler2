/*
 * TjMessageHandler.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007
 *               by Chris Schlaeger <cs@kde.org>
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
 * @author Chris Schlaeger <cs@kde.org>
 */
class TjMessageHandler : public QObject
{
    Q_OBJECT
public:
    TjMessageHandler(bool cm = true) :
        QObject(),
        consoleMode(cm),
        warnings(0),
        errors(0)
    { }
    virtual ~TjMessageHandler() { }

    void warningMessage(const QString& msg, const QString& file = QString::null,
                        int line = -1);
    void errorMessage(const QString& msg, const QString& file = QString::null,
                      int line = -1);
    void fatalMessage(const QString& msg, const QString& file = QString::null,
                      int line = -1);

    void setConsoleMode(bool cm) { consoleMode = cm; }

    void resetCounters() { warnings = errors = 0; }

    int getWarnings() const { return warnings; }
    int getErrors() const { return errors; }

signals:
    void printWarning(const QString& msg, const QString& file, int line);
    void printError(const QString& msg, const QString& file, int line);
    void printFatal(const QString& msg, const QString& file, int line);

private:
    bool consoleMode;
    int warnings;
    int errors;
} ;

extern TjMessageHandler TJMH;

#endif

