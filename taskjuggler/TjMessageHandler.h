/*
 * TjMessageHandler.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _TjMessageHandler_h_
#define _TjMessageHandler_h_

#include <qstring.h>

/**
 * @short Stores an error or warning message.
 * @author Chris Schlaeger <cs@suse.de>
 */
class TjMessage
{
public:
	TjMessage(const QString& m, const QString& f, int l) :
		msg(m), file(f), line(l) { }
	~TjMessage() { }

	const QString& getMsg() const { return msg; }
	const QString& getFile() const { return file; }
	int getLine() const { return line; }

private:
	TjMessage() { }
	
	QString msg;
	QString file;
	int line;
} ;

/**
 * This class handles all error or warning messages that the library functions
 * can send out. Depending on the mode it either send the messages directory
 * to STDERR or buffers them. In the latter case they can be retrieved when
 * needed through arbritrator functions.
 *
 * @short Handles all error or warning messages.
 * @author Chris Schlaeger <cs@suse.de>
 */
class TjMessageHandler
{
public:
	TjMessageHandler(bool cm) : consoleMode(cm)
   	{
		msgBuffer.setAutoDelete(TRUE);
   	}
	~TjMessageHandler();

	errorMessage(const QString& msg, const QString& file, int line);

	TjMessage* firstMessage() { return msgBuffer.first(); }
	TjMessage* nextMessage() { return msgBuffer.next(); }

	clear() { msgBuffer.clear(); }

private:
	bool consoleMode;
	QPtrList<TjMessage> msgBuffer;
} ;

#endif

