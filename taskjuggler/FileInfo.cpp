/*
 * FileInfo.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <ctype.h>

#include "FileInfo.h"
#include "ProjectFile.h"

FileInfo::FileInfo(ProjectFile* p, const QString& file_, const QString& tp)
	: pf(p), taskPrefix(tp)
{
	tokenTypeBuf = INVALID;
	file = file_;
}

bool
FileInfo::open()
{
	if (file.right(2) == "/.")
	{
		f = new QTextStream(stdin, IO_ReadOnly);
		fh = stdin;
	}
	else
	{
		if ((fh = fopen(file, "r")) == 0)
			return FALSE;
		f = new QTextStream(fh, IO_ReadOnly);
	}

	lineBuf = QString::null;
	currLine = 1;
	return TRUE;
}

bool
FileInfo::close()
{
	delete f;
	if (fh == stdin)
		return TRUE;

	if (fclose(fh) == EOF)
		return FALSE;

	return TRUE;
}

QChar
FileInfo::getC(bool expandMacros)
{
 BEGIN:
	QChar c;
	if (ungetBuf.isEmpty())
	{
		*f >> c;
		if (feof(fh))
			c = QChar(EOFile);
	}
	else
	{
		c = ungetBuf.last();
		ungetBuf.remove(ungetBuf.fromLast());
		if (c.unicode() == EOMacro)
		{
			macroStack.removeLast();
			pf->getMacros().popArguments();
			goto BEGIN;
		}
	}
	lineBuf += c;

	if (expandMacros)
	{
		if (c == '$')
		{
			QChar d;
			if ((d = getC(FALSE)) == '{')
			{
				// remove $ from lineBuf;
				lineBuf = lineBuf.left(lineBuf.length() - 1);
				readMacroCall();
				goto BEGIN;
			}
			else
			{
				// $$ escapes $, so discard 2nd $
			   	if (d != '$')
					ungetC(d);
			}
		}
	}

	return c;
}

void
FileInfo::ungetC(QChar c)
{
	lineBuf = lineBuf.left(lineBuf.length() - 1);
	ungetBuf.append(c);
}

bool
FileInfo::getDateFragment(QString& token, QChar& c)
{
	token += c;
	c = getC();
	// c must be a digit
	if (!c.isDigit())
	{
		fatalError("Corrupted date");
		return FALSE;
	}
	token += c;
	// read other digits
	while ((c = getC()).unicode() != EOFile && c.isDigit())
		token += c;

	return TRUE;
}

QString
FileInfo::getPath() const
{
	if (file.find('/') >= 0)
		return file.left(file.findRev('/') + 1);
	else
		return "";
}

TokenType
FileInfo::nextToken(QString& token)
{
	if (tokenTypeBuf != INVALID)
	{
		token = tokenBuf;
		TokenType tt = tokenTypeBuf;
		tokenTypeBuf = INVALID;
		return tt;
	}

	token = "";

	// skip blanks and comments
	for ( ; ; )
	{
		QChar c = getC();
		if (c.unicode() == EOFile)
			return EndOfFile;
		switch (c)
		{
		case ' ':
		case '\t':
			break;
		case '/':
			/* This code skips c-style comments like the one you are just
			 * reading. */
			if ((c = getC(FALSE)) == '*')
			{
				do
				{
					while ((c = getC(FALSE)) != '*')
					{
						if (c == '\n')
							currLine++;
						else if (c.unicode() == EOFile)
						{
							fatalError("Unterminated comment");
							return EndOfFile;
						}
					}
				} while ((c = getC(FALSE)) != '/');
			}
			else
			{
				ungetC(c);
				ungetC('/');
				goto BLANKS_DONE;
			}
			break;
		case '#':	// Comments start with '#' and reach towards end of line
			while ((c = getC(FALSE)) != '\n' && c.unicode() != EOFile)
				;
			if (c.unicode() == EOFile)
				return EndOfFile;
			// break missing on purpose
		case '\n':
			// Increase line counter only when not replaying a macro.
			if (macroStack.isEmpty())
				currLine++;
			lineBuf = "";
			break;
		default:
			ungetC(c);
			goto BLANKS_DONE;
		}
	}
 BLANKS_DONE:

	// analyse non blank characters
	for ( ; ; )
	{
		QChar c = getC();
		if (c.unicode() == EOFile)
		{
			fatalError("Unexpected end of file");
			return EndOfFile;
		}
		else if (isalpha(c) || (c == '_') || (c == '!'))
		{
			token += c;
			while ((c = getC()).unicode() != EOFile &&
				   (isalnum(c) || (c == '_') || (c == '.') || (c == '!')))
				token += c;
			ungetC(c);
			if (token.contains('.'))
			{
				if (token[0] == '!')
					return RELATIVE_ID;
				else
					return ABSOLUTE_ID;
			}
			else
				return ID;
		}
		else if (c.isDigit())
		{
			// read first number (maybe a year)
			token += c;
			while ((c = getC()).unicode() != EOFile && c.isDigit())
				token += c;
			if (c == '-')
			{
				// this must be a ISO date yyyy-mm-dd[[-hh:mm:[ss]]-TZ]
				getDateFragment(token, c);
				if (c != '-')
				{
					fatalError("Corrupted date");
					return EndOfFile;
				}
				getDateFragment(token, c);
				if (c == '-')
				{
					getDateFragment(token, c);
					if (c != ':')
					{
						fatalError("Corrupted date");
						return EndOfFile;
					}
					getDateFragment(token, c);
					if (c == ':')
						getDateFragment(token, c);
				}
				int i = 0;
				if (c == '-')
				{
					/* Timezone can either be a name (ref.
					 * Utility::timezone2tz) or GMT[+-]hh:mm */
					token += c;
					while ((c = getC()).unicode() != EOFile &&
						   (isalnum(c) || c == '+' || c == '-' || c == ':')
						   && i++ < 9)
						token += c;
				}
				ungetC(c);
				return DATE;
			}
			else if (c == '.')
			{
				// must be a real number
				token += c;
				while ((c = getC()).unicode() != EOFile && c.isDigit())
					token += c;
				ungetC(c);
				return REAL;
			}
			else if (c == ':')
			{
				// must be a time (HH:MM)
				token += c;
				for (int i = 0; i < 2; i++)
				{
					if ((c = getC()).unicode() != EOFile && c.isDigit())
						token += c;
					else
					{
						fatalError("2 digits minutes expected");
						return EndOfFile;
					}
				}
				return HOUR;
			}
			else
			{
				ungetC(c);
				return INTEGER;
			}
		}
		else if (c == '\'')
		{
			// single quoted string
			while ((c = getC()).unicode() != EOFile && c != '\'')
			{
				if (c == '\n')
					currLine++;
				token += c;
			}
			if (c.unicode() == EOFile)
			{
				fatalError("Non terminated string");
				return EndOfFile;
			}
			return STRING;
		}
		else if (c == '"')
		{
			// double quoted string
			while ((c = getC()).unicode() != EOFile && c != '"')
			{
				if (c == '\n')
					currLine++;
				token += c;
			}
			if (c.unicode() == EOFile)
			{
				fatalError("Non terminated string");
				return EndOfFile;
			}
			return STRING;
		}
		else if (c == '[')
		{
			int nesting = 0;
			while ((c = getC(FALSE)).unicode() != EOFile &&
				   (c != ']' || nesting > 0))
			{
				if (c == '[')
					nesting++;
				else if (c == ']')
					nesting--;
				if (c == '\n')
					currLine++;
				token += c;
			}
			if (c.unicode() == EOFile)
			{
				fatalError("Non terminated macro definition");
				return EndOfFile;
			}
			return MacroBody;
		}
		else
		{
			token += c;
			switch (c)
			{
			case '{':
				return LCBRACE;
			case '}':
				return RCBRACE;
			case '(':
				return LBRACE;
			case ')':
				return RBRACE;
			case ',':
				return COMMA;
			case '~':
				return TILDE;
			case '-':
				return MINUS;
			case '&':
				return AND;
			case '|':
				return OR;
			default:
				fatalError("Illegal character '%c' (Unicode %d)", c.latin1(),
						   c.unicode());
				return INVALID;
			}
		}
	}
}

bool
FileInfo::readMacroCall()
{
	QString id;
	TokenType tt;
	if ((tt = nextToken(id)) != ID && tt != INTEGER)
	{
		fatalError("Macro ID expected");
		return FALSE;
	}
	QString token;
	// Store all arguments in a newly created string list.
	QStringList* sl = new QStringList;
	while ((tt = nextToken(token)) == STRING)
		sl->append(token);
	if (tt != RCBRACE)
	{
		fatalError("'}' expected");
		return FALSE;
	}

	// push string list to global argument stack
	pf->getMacros().pushArguments(sl);

	// expand the macro
	pf->getMacros().setLocation(file, currLine);
	QString macro = pf->getMacros().resolve(id);
	if (macro.isNull())
	{
		fatalError(QString("Unknown macro ") + id);
		return FALSE;
	}

	// Push pointer to macro on stack. Needed for error handling.
	macroStack.append(pf->getMacros().getMacro(id));

	// mark end of macro
	ungetC(QChar(EOMacro));
	// push expanded macro reverse into ungetC buffer.
	for (int i = macro.length() - 1; i >= 0; --i)
		ungetC(macro[i].latin1());
	return TRUE;
}

void
FileInfo::returnToken(TokenType tt, const QString& buf)
{
	if (tokenTypeBuf != INVALID)
	{
		qFatal("Internal Error: Token buffer overflow!");
		return;
	}
	tokenTypeBuf = tt;
	tokenBuf = buf;
}

void
FileInfo::fatalError(const char* msg, ...)
{
	va_list ap;
	char buf[1024];
	va_start(ap, msg);
	vsnprintf(buf, 1024, msg, ap);
	va_end(ap);
	
	if (macroStack.isEmpty())
	{
		qWarning("%s:%d:%s", file.latin1(), currLine, buf);
		qWarning("%s", lineBuf.latin1());
	}
	else
	{
		qWarning("Error in expanded macro");
		qWarning("%s:%d: %s",
				 macroStack.last()->getFile().latin1(),
				 macroStack.last()->getLine(), buf);
		qWarning("%s", lineBuf.latin1());
	}
}

void
FileInfo::fatalErrorVA(const char* msg, va_list ap)
{
	char buf[1024];
	vsnprintf(buf, 1024, msg, ap);
	
	if (macroStack.isEmpty())
	{
		qWarning("%s:%d:%s", file.latin1(), currLine, buf);
		qWarning("%s", lineBuf.latin1());
	}
	else
	{
		qWarning("Error in expanded macro");
		qWarning("%s:%d: %s",
				 macroStack.last()->getFile().latin1(),
				 macroStack.last()->getLine(), buf);
		qWarning("%s", lineBuf.latin1());
	}
}


