/*
 * ProjectFile.cpp - TaskJuggler
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
#include <stdio.h>
#include <stdlib.h>

#include <qregexp.h>

#include "ProjectFile.h"
#include "Project.h"
#include "Token.h"
#include "ExpressionTree.h"
#include "Allocation.h"

#include "kotrus.h"

// Dummy marco to mark all keywords of taskjuggler syntax
#define KW(a) a

#define READ_DATE(a, b) \
(token == a) \
{ \
	if ((tt = nextToken(token)) == DATE) \
		task->b(date2time(token)); \
	else \
	{ \
		fatalError("Date expected"); \
		return FALSE; \
	} \
}

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
		f = stdin;
	else
		if ((f = fopen(file, "r")) == 0)
			return FALSE;

	lineBuf = "";
	currLine = 1;
	return TRUE;
}

bool
FileInfo::close()
{
	if (f == stdin)
		return TRUE;

	if (fclose(f) == EOF)
		return FALSE;

	return TRUE;
}

int
FileInfo::getC(bool expandMacros)
{
 BEGIN:
	int c;
	if (ungetBuf.isEmpty())
	{
		c = getc(f);
	}
	else
	{
		c = ungetBuf.last();
		ungetBuf.remove(ungetBuf.fromLast());
		if (c == EOM)
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
			int d;
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
FileInfo::ungetC(int c)
{
	lineBuf = lineBuf.left(lineBuf.length() - 1);
	ungetBuf.append(c);
}

bool
FileInfo::getDateFragment(QString& token, int& c)
{
	token += c;
	c = getC();
	// c must be a digit
	if (!isdigit(c))
	{
		fatalError("Corrupted date");
		return FALSE;
	}
	token += c;
	// read other digits
	while ((c = getC()) != EOF && isdigit(c))
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
		int c = getC();
		switch (c)
		{
		case EOF:
			return EndOfFile;
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
						else if (c == EOF)
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
			while ((c = getC(FALSE)) != '\n' && c != EOF)
				;
			if (c == EOF)
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
		int c = getC();
		if (c == EOF)
		{
			fatalError("Unexpected end of file");
			return EndOfFile;
		}
		else if (isalpha(c) || (c == '_') || (c == '!'))
		{
			token += c;
			while ((c = getC()) != EOF &&
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
		else if (isdigit(c))
		{
			// read first number (maybe a year)
			token += c;
			while ((c = getC()) != EOF && isdigit(c))
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
					while ((c = getC()) != EOF &&
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
				while ((c = getC()) != EOF && isdigit(c))
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
					if ((c = getC()) != EOF && isdigit(c))
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
			while ((c = getC()) != EOF && c != '\'')
			{
				if (c == '\n')
					currLine++;
				token += c;
			}
			if (c == EOF)
			{
				fatalError("Non terminated string");
				return EndOfFile;
			}
			return STRING;
		}
		else if (c == '"')
		{
			// double quoted string
			while ((c = getC()) != EOF && c != '"')
			{
				if (c == '\n')
					currLine++;
				token += c;
			}
			if (c == EOF)
			{
				fatalError("Non terminated string");
				return EndOfFile;
			}
			return STRING;
		}
		else if (c == '[')
		{
			int nesting = 0;
			while ((c = getC(FALSE)) != EOF && (c != ']' || nesting > 0))
			{
				if (c == '[')
					nesting++;
				else if (c == ']')
					nesting--;
				if (c == '\n')
					currLine++;
				token += c;
			}
			if (c == EOF)
			{
				fatalError("Non terminated macro definition");
				return EndOfFile;
			}
			return MacroBody;
		}
		else
		{
			token += QChar(c);
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
				fatalError("Illegal character '%c'", c);
				return EndOfFile;
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
	ungetC(EOM);
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

ProjectFile::ProjectFile(Project* p)
{
	proj = p;

	openFiles.setAutoDelete(TRUE);
}

bool
ProjectFile::open(const QString& file, const QString& parentPath,
				  const QString& taskPrefix)
{
	QString absFileName = file;
	if (debugLevel > 2)
		qWarning("Requesting to open file %s", file.latin1());
	if (absFileName[0] != '/')
		absFileName = parentPath + absFileName;
	
	if (debugLevel > 2)
		qWarning("File name before compression: %s", absFileName.latin1());
	int end = 0;
	while ((end = absFileName.find("/../", end)) >= 0)
	{
		int start = absFileName.findRev('/', end - 1);
		if (start < 0)
			start = 0;
		else
			start++;	// move after '/'
		if (start < end && absFileName.mid(start, end - start) != "..")
			absFileName.remove(start, end + strlen("/../") - start);
		end = start - 1;
	}
	if (debugLevel > 2)
		qWarning("File name after compression: %s", absFileName.latin1());

	// Make sure that we include each file only once.
	if (includedFiles.findIndex(absFileName) != -1)
	{
		if (debugLevel > 2)
			qWarning("Ignoring already read file %s",
					 absFileName.latin1());
		return TRUE;
	}
		
	if (debugLevel > 2)
		qWarning("Reading %s", absFileName.latin1());

	FileInfo* fi = new FileInfo(this, absFileName, taskPrefix);

	if (!fi->open())
	{
		qFatal("Cannot open '%s'", absFileName.latin1());
		return FALSE;
	}

	openFiles.append(fi);
	includedFiles.append(absFileName);
	return TRUE;
}

bool
ProjectFile::close()
{
	bool error = FALSE;

	FileInfo* fi = openFiles.getLast();

	if (!fi->close())
		error = TRUE;
	if (debugLevel > 2)
		qWarning("Finished file %s", fi->getFile().latin1());
	openFiles.removeLast();

	return error;
}

bool
ProjectFile::parse()
{
	TokenType tt;
	QString token;

	for ( ; ; )
	{
		switch (tt = nextToken(token))
		{
		case EndOfFile:
			return TRUE;
		case ID:
			if (token == KW("task"))
			{
				if (!readTask(0))
					return FALSE;
				break;
			}
			else if (token == KW("account"))
			{
				if (!readAccount(0))
					return FALSE;
				break;
			}
			else if (token == KW("resource"))
			{
				if (!readResource(0))
					return FALSE;
				break;
			}
			else if (token == KW("shift"))
			{
				if (!readShift(0))
					return FALSE;
				break;	
			}
			else if (token == KW("vacation"))
			{
				time_t from, to;
				QString name;
				if (!readVacation(from, to, TRUE, &name))
					return FALSE;
				proj->addVacation(name, from, to);
				break;
			}
			else if (token == KW("priority"))
			{
				int priority;
				if (!readPriority(priority))
					return FALSE;
				proj->setPriority(priority);
				break;
			}
			else if (token == KW("now"))
			{
				fatalError("'now' is no longer a property. It's now an "
						   "optional project attribute. Please fix your "
						   "project file.");
				if (nextToken(token) != DATE)
				{
					fatalError("Date expected");
					return FALSE;
				}
				proj->setNow(date2time(token));
				break;
			}
			else if (token == KW("mineffort"))
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value exptected");
					return FALSE;
				}
				proj->setMinEffort(token.toDouble());
				break;
			}
			else if (token == KW("maxeffort"))
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value exptected");
					return FALSE;
				}
				proj->setMaxEffort(token.toDouble());
				break;
			}
			else if (token == KW("rate"))
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value exptected");
					return FALSE;
				}
				proj->setRate(token.toDouble());
				break;
			}
			else if (token == KW("currency"))
			{
				if (nextToken(token) != STRING)
				{
					fatalError("String expected");
					return FALSE;
				}
				proj->setCurrency(token);
				break;
			}
			else if (token == KW("currencydigits"))
			{
				if (nextToken(token) != INTEGER)
				{
					fatalError("Integer value expected");
					return FALSE;
				}
				proj->setCurrencyDigits(token.toInt());
				break;
			}
			else if (token == KW("timingresolution"))
			{
				fatalError("'timingresolution' is no longer a property. It's "
						   "now an optional project attribute. Please fix "
						   "your project file.");
				ulong resolution;
				if (!readTimeValue(resolution))
					return FALSE;
				if (proj->resourceCount() > 0)
				{
					fatalError("The timing resolution cannot be changed after "
							   "resources have been declared.");
					return FALSE;
				}
				if (resolution < 60 * 5)
				{
					fatalError("timing resolution must be at least 5 min");
					return FALSE;
				}
				proj->setScheduleGranularity(resolution);
				break;
			}
			else if (token == KW("workinghours"))
			{
				int dow;
				QPtrList<Interval>* l = new QPtrList<Interval>();
				if (!readWorkingHours(dow, l))
					return FALSE;

				proj->setWorkingHours(dow, l);
				break;
			}
			else if (token == KW("copyright"))
			{
				if (nextToken(token) != STRING)
				{
					fatalError("String expected");
					return FALSE;
				}
				proj->setCopyright(token);
				break;
			}
			else if (token == KW("include"))
			{
				if (!readInclude())
					return FALSE;
				break;
			}
			else if (token == KW("macro"))
			{
				QString id;
				if (nextToken(id) != ID)
				{
					fatalError("Macro ID expected");
					return FALSE;
				}
				QString file = openFiles.last()->getFile();
				uint line = openFiles.last()->getLine();
				if (nextToken(token) != MacroBody)
				{
					fatalError("Macro body expected");
					return FALSE;
				}
				Macro* macro = new Macro(id, token, file, line);
				if (!macros.addMacro(macro))
				{
					fatalError("Macro has been defined already");
					delete macro;
					return FALSE;
				}
				break;
			}
			else if (token == KW("flags"))
			{
				for ( ; ; )
				{
					QString flag;
					if (nextToken(flag) != ID)
					{
						fatalError("flag ID expected");
						return FALSE;
					}

					/* Flags can be declared multiple times, but we
					 * register a flag only once. */
					if (!proj->isAllowedFlag(flag))
						proj->addAllowedFlag(flag);

					if ((tt = nextToken(token)) != COMMA)
					{
						returnToken(tt, token);
						break;
					}
				}
				break;
			}
			else if (token == KW("project"))
			{
				if (!readProject())
					return FALSE;
				break;
			}
			else if (token == KW("projectid"))
			{
				for ( ; ; )
				{
					QString id;
					if (nextToken(id) != ID)
					{
						fatalError("Project ID expected");
						return FALSE;
					}

					if (!proj->addId(id))
					{
						fatalError("Project ID %s has already been registered",
								   id.latin1());
						return FALSE;
					}

					if ((tt = nextToken(token)) != COMMA)
					{
						returnToken(tt, token);
						break;
					}
				}
				break;
			}
			else if (token == KW("xmltaskreport"))
			{
			   if( !readXMLTaskReport())
			      return FALSE;
			   break;
			}
			else if (token == "icalreport" )
			{
#ifdef HAVE_ICAL
#ifdef HAVE_KDE
			   if( !readICalTaskReport())
#else
			      fatalError( "TaskJuggler was built without KDE support -> no ICal support!" );
#endif
			      return FALSE;
			   break;
#else
			   fatalError( "TaskJuggler was built without ICal-Support, sorry." );
			   break;
#endif
			}
			
			else if (token == KW("htmltaskreport") ||
					 token == KW("htmlresourcereport"))
			{
				if (!readHTMLReport(token))
					return FALSE;
				break;
			}
			else if (token == KW("htmlaccountreport"))
			{
				if (!readHTMLAccountReport())
					return FALSE;
				break;
			}
			else if (token == KW("export"))
			{
				if (!readExportReport())
					return FALSE;
				break;
			}
			else if (token == KW("kotrusmode"))
			{
				if (nextToken(token) != STRING ||
					(token != KW("db") && token != KW("xml") &&
					 token != KW("nokotrus")))
				{
					fatalError("Unknown kotrus mode");
					return FALSE;
				}
				if (token != KW("nokotrus"))
				{
					Kotrus* kotrus = new Kotrus();
					kotrus->setKotrusMode(token);
					proj->setKotrus(kotrus);
				}
				break;
			}
			else if (token == KW("supplement"))
			{
				if (nextToken(token) != ID ||
				   	(token != KW("task") && (token != KW("resource"))))
				{
					fatalError("'task' or 'resource' expected");
					return FALSE;
				}
				if ((token == "task" && !readTaskSupplement("")) ||
					(token == "resource" && !readResourceSupplement()))
					return FALSE;
				break;
			}	
			// break missing on purpose!
		default:
			fatalError("Syntax Error at '%s'!", token.latin1());
			return FALSE;
		}
	}

	return TRUE;
}

bool
ProjectFile::readProject()
{
	QString token;

	if (proj->accountCount() > 0 || proj->resourceCount() > 0 ||
		proj->shiftCount() > 0 || proj->taskCount() > 0)
	{
		fatalError("The project properties must be defined prior to any "
				   "account, shift, task or resource.");
		return FALSE;
	}
	
	if (nextToken(token) != ID)
	{
		fatalError("Project ID expected");
		return FALSE;
	}
	if (!proj->addId(token))
	{
		fatalError("Project ID %s has already been registered",
					token.latin1());
		return FALSE;
	}
	if (nextToken(token) != STRING)
	{
		fatalError("Project name expected");
		return FALSE;
	}
	proj->setName(token);
	if (nextToken(token) != STRING)
	{
		fatalError("Version string expected");
		return FALSE;
	}
	proj->setVersion(token);
	time_t start, end;
	if (nextToken(token) != DATE)
	{
		fatalError("Start date expected");
		return FALSE;
	}
	start = date2time(token);
	if (nextToken(token) != DATE)
	{
		fatalError("End date expected");
		return FALSE;
	}
	end = date2time(token);
	if (end <= start)
	{
		fatalError("End date must be larger then start date");
		return FALSE;
	}
	proj->setStart(start);
	proj->setEnd(end);

	TokenType tt;
	if ((tt = nextToken(token)) == LCBRACE)
	{
		for ( ; ; )
		{
			if ((tt = nextToken(token)) != ID && tt != RCBRACE)
			{
				fatalError("Attribute ID expected");
				return FALSE;
			}
			if (tt == RCBRACE)
				break;
			if (token == KW("dailyworkinghours"))
			{
				if ((tt = nextToken(token)) != REAL && tt != INTEGER)
				{
					fatalError("Real number expected");
					return FALSE;
				}
				proj->setDailyWorkingHours(token.toDouble());
			}
			else if (token == KW("yearlyworkingdays"))
			{
				if ((tt = nextToken(token)) != REAL && tt != INTEGER)
				{
					fatalError("Real number expected");
					return FALSE;
				}
				proj->setYearlyWorkingDays(token.toDouble());
			}
			else if (token == KW("now"))
			{
				if (nextToken(token) != DATE)
				{
					fatalError("Date expected");
					return FALSE;
				}
				proj->setNow(date2time(token));
			}
			else if (token == KW("timingresolution"))
			{
				ulong resolution;
				if (!readTimeValue(resolution))
					return FALSE;
				if (resolution < 60 * 5)
				{
					fatalError("timing resolution must be at least 5 min");
					return FALSE;
				}
				proj->setScheduleGranularity(resolution);
			}
			else if (token == KW("timezone"))
			{
				if (nextToken(token) != STRING)
				{
					fatalError("Timezone name expected");
					return FALSE;
				}
				if (setenv("TZ", token, 1) < 0)
					qFatal("Ran out of space in environment section while "
						   "setting timezone.");
			}
			else
			{
				fatalError("Unknown attribute %s", token.latin1());
				return FALSE;
			}
		}
	}
	else
		returnToken(tt, token);
	
	return TRUE;
}

TokenType
ProjectFile::nextToken(QString& buf)
{
	if (openFiles.isEmpty())
		return EndOfFile;

	TokenType tt;
	while ((tt = openFiles.last()->nextToken(buf)) == EndOfFile)
	{
		close();
		if (openFiles.isEmpty())
			return EndOfFile;
	}

	return tt;
}

const QString&
ProjectFile::getTaskPrefix()
{
	if (openFiles.isEmpty())
		return QString::null;

	return openFiles.last()->getTaskPrefix();
}

void
ProjectFile::fatalError(const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
 	
	if (openFiles.isEmpty())
		qWarning("Unexpected end of file found. Probably a missing '}'.");
	else
		openFiles.last()->fatalErrorVA(msg, ap);
	va_end(ap);
}

bool
ProjectFile::readInclude()
{
	QString fileName;

	if (nextToken(fileName) != STRING)
	{
		fatalError("File name expected");
		return FALSE;
	}
	QString token;
	TokenType tt;

	QString taskPrefix = getTaskPrefix();
	/* The nextToken() call may yield an EndOfFile and shift file scope to
	 * parent file. So we have to save the path of the current file to pass it
	 * later to open(). */
	QString parentPath = openFiles.last()->getPath();
	
	if ((tt = nextToken(token)) == LCBRACE)
	{
		while ((tt = nextToken(token)) != RCBRACE)
		{
			if (tt == ID && token == KW("taskprefix"))
			{
				if (nextToken(token) != ID || tt == ABSOLUTE_ID)
				{
					fatalError("String expected");
					return FALSE;
				}
				if (!proj->getTask(getTaskPrefix() + token))
				{
					fatalError("Task prefix must be a known task");
					return FALSE;
				}
				taskPrefix = getTaskPrefix() + token + ".";
			}
			else
			{
				fatalError("Invalid optional attribute \'%s\'", token.latin1());
				return FALSE;
			}
		}
	}
	else
		returnToken(tt, token);
	
	if (!open(fileName, parentPath, taskPrefix))
		return FALSE;
	
	return TRUE;
}

bool
ProjectFile::readTask(Task* parent)
{
	TokenType tt;
	QString token;

	QString id;
	if ((tt = nextToken(id)) != ID &&
		(tt != ABSOLUTE_ID) && (tt != RELATIVE_ID))
	{
		fatalError("ID expected");
		return FALSE;
	}

	if (tt == RELATIVE_ID)
	{
		/* If a relative ID has been specified the task is declared out of
		 * it's actual scope. So we have to set 'parent' to point to the
		 * correct parent task. */
		do
		{
			if (id[0] == '!')
			{
				if (parent != 0)
					parent = parent->getParent();
				else
				{
					fatalError("Invalid relative task ID '%s'", id.latin1());
					return FALSE;
				}
				id = id.right(id.length() - 1);
			}
			else if (id.find('.') >= 0)
			{
				QString tn = (parent ? parent->getId() + "." : QString())
					+ id.left(id.find('.'));
				TaskList tl;
				if (parent)
					parent->getSubTaskList(tl);
				else
					tl = proj->getTaskList();
				bool found = FALSE;
				for (Task* t = tl.first(); t != 0; t = tl.next())
					if (t->getId() == tn)
					{
						parent = t;
						id = id.right(id.length() - id.find('.') - 1);
						found = TRUE;
						break;
					}
				if (!found)
				{
					fatalError("Task '%s' unknown", tn.latin1());
					return FALSE;
				}
			}
		} while (id[0] == '!' || id.find('.') >= 0);
	}
	else if (tt == ABSOLUTE_ID)
	{
		QString path = getTaskPrefix() + id.left(id.findRev('.', -1));
		if ((parent = proj->getTask(path)) == 0)
		{
			fatalError("Task '%s' has not been defined", path.latin1());
			return FALSE;
		}
		id = id.right(id.length() - id.findRev('.', -1) - 1);
	}

	QString name;
	if ((tt = nextToken(name)) != STRING)
	{
		fatalError("String expected");
		return FALSE;
	}

	if ((tt = nextToken(token)) != LCBRACE)
	{
		fatalError("{ expected");
		return FALSE;
	}

	/*
	 * If a pointer to a parent task was given, the id of the parent task is
	 * used as a prefix to the ID of the task. Toplevel task may be prefixed
	 * by a task prefix as specified by when including a project file.
	 */
	if (parent)
		id = parent->getId() + "." + id;
	else
	{
		QString tp = getTaskPrefix();
		if (!tp.isEmpty())
		{
			// strip trailing '.'
			tp = tp.left(tp.length() - 1);
			parent = proj->getTask(tp);
			id = tp + "." + id;
		}
	}
	
	// We need to check that the task id has not been declared before.
	TaskList tl = proj->getTaskList();
	for (Task* t = tl.first(); t != 0; t = tl.next())
		if (t->getId() == id)
		{
			fatalError("Task %s has already been declared", id.latin1());
			return FALSE;
		}

	Task* task = new Task(proj, id, name, parent, getFile(), getLine());

	proj->addTask(task);
	if (parent)
		parent->addSub(task);


	if (!readTaskBody(task))
		return FALSE;
	
	if (task->getName().isEmpty())
	{
		fatalError("No name specified for task '%s'!", id.latin1());
		return FALSE;
	}

	return TRUE;
}

bool
ProjectFile::readTaskSupplement(QString prefix)
{
	QString token;
	TokenType tt;
	Task* task;

	/* When supplement is used within a task declaration, the prefix is the id
	 * of the parent task. If it's empty, then we need to use the prefix for
	 * the current file. The parent task id has no trailing dot, so we have to
	 * append it. */
	if (prefix.isEmpty())
		prefix = getTaskPrefix();
	else
		prefix += ".";
	
	if (((tt = nextToken(token)) != ID && tt != ABSOLUTE_ID) ||
		((task = proj->getTask(prefix == "" ?
							   token : prefix + token)) == 0))
	{
		fatalError("Task '%s' has not been defined yet",
				   (prefix == "" ? token : prefix + token).latin1());
		return FALSE;
	}
	if (nextToken(token) != LCBRACE)
	{
		fatalError("'}' expected");
		return FALSE;
	}
	return readTaskBody(task);
}

bool
ProjectFile::readTaskBody(Task* task)
{
	QString token;
	TokenType tt;
	
	for (bool done = false ; !done; )
	{
		switch (tt = nextToken(token))
		{
		case ID:
			if (token == KW("task"))
			{
				if (!readTask(task))
					return FALSE;
			}
			else if (token == KW("note"))
			{
				if ((tt = nextToken(token)) == STRING)
					task->setNote(token);
				else
				{
					fatalError("String expected");
					return FALSE;
				}
			}
			else if (token == KW("milestone"))
			{
				task->setMilestone();
			}
			else if READ_DATE(KW("start"), setPlanStart)
			else if READ_DATE(KW("end"), setPlanEnd)
			else if READ_DATE(KW("minstart"), setMinStart)
			else if READ_DATE(KW("maxstart"), setMaxStart)
			else if READ_DATE(KW("minend"), setMinEnd)
			else if READ_DATE(KW("maxend"), setMaxEnd)
			else if READ_DATE(KW("actualstart"), setActualStart)
			else if READ_DATE(KW("actualend"), setActualEnd)
			else if (token == KW("length"))
			{
				double d;
				if (!readPlanTimeFrame(d, TRUE))
					return FALSE;
				task->setPlanLength(d);
			}
			else if (token == KW("effort"))
			{
				double d;
				if (!readPlanTimeFrame(d, TRUE))
					return FALSE;
				task->setPlanEffort(d);
			}
			else if (token == KW("duration"))
			{
				double d;
				if (!readPlanTimeFrame(d, FALSE))
					return FALSE;
				task->setPlanDuration(d);
			}
			else if (token == KW("actuallength"))
			{
				double d;
				if (!readPlanTimeFrame(d, TRUE))
					return FALSE;
				task->setActualLength(d);
			}
			else if (token == KW("actualeffort"))
			{
				double d;
				if (!readPlanTimeFrame(d, TRUE))
					return FALSE;
				task->setActualEffort(d);
			}
			else if (token == KW("actualduration"))
			{
				double d;
				if (!readPlanTimeFrame(d, FALSE))
					return FALSE;
				task->setActualDuration(d);
			}
			else if (token == KW("complete"))
			{
				if (nextToken(token) != INTEGER)
				{
					fatalError("Integer value expected");
					return FALSE;
				}
				int complete = token.toInt();
				if (complete < 0 || complete > 100)
				{
					fatalError("Value of complete must be between 0 and 100");
					return FALSE;
				}
				task->setComplete(complete);
			}
			else if (token == KW("startbuffer"))
			{
				double value;
				if (!readPercent(value))
					return FALSE;
				task->setStartBuffer(value);
			}
			else if (token == KW("endbuffer"))
			{
				double value;
				if (!readPercent(value))
					return FALSE;
				task->setEndBuffer(value);
			}
			else if (token == KW("responsible"))
			{
				Resource* r;
				if (nextToken(token) != ID ||
					(r = proj->getResource(token)) == 0)
				{
					fatalError("Resource ID expected");
					return FALSE;
				}
				task->setResponsible(r);
			}
			else if (token == KW("shift"))
			{
				time_t from, to;
				from = proj->getStart();
				to = proj->getEnd();
				Shift* s;
				if ((s = readShiftSelection(from, to)) == 0)
					return FALSE;
				if (!task->addShift(Interval(from, to), s))
				{
					fatalError("Shift intervals may not overlap");
					return FALSE;
				}
			}
			else if (token == KW("allocate"))
			{
				if (!readAllocate(task))
					return FALSE;
			}
			else if (token == KW("depends"))
			{
				for ( ; ; )
				{
					QString id;
					if ((tt = nextToken(id)) != ID &&
						tt != RELATIVE_ID && tt != ABSOLUTE_ID)
					{
						fatalError("Task ID expected");
						return FALSE;
					}
					if (tt == ABSOLUTE_ID)
						id = getTaskPrefix() + id;
					task->addDependency(id);
					task->setScheduling(Task::ASAP);
					if ((tt = nextToken(token)) != COMMA)
					{
						returnToken(tt, token);
						break;
					}
				}
			}
			else if (token == KW("preceeds"))
			{
				for ( ; ; )
				{
					QString id;
					if ((tt = nextToken(id)) != ID &&
						tt != RELATIVE_ID && tt != ABSOLUTE_ID)
					{
						fatalError("Task ID expected");
						return FALSE;
					}
					if (tt == ABSOLUTE_ID)
						id = getTaskPrefix() + id;
					task->addPreceeds(id);
					task->setScheduling(Task::ALAP);
					if ((tt = nextToken(token)) != COMMA)
					{
						returnToken(tt, token);
						break;
					}
				}
			}
			else if (token == KW("scheduling"))
			{
				nextToken(token);
				if (token == KW("asap"))
					task->setScheduling(Task::ASAP);
				else if (token == KW("alap"))
					task->setScheduling(Task::ALAP);
				else
				{
					fatalError("Unknown scheduling policy");
					return FALSE;
				}
			}
			else if (token == KW("flags"))
			{
				for ( ; ; )
				{
					QString flag;
					if (nextToken(flag) != ID || !proj->isAllowedFlag(flag))
					{
						fatalError("Flag unknown");
						return FALSE;
					}
					task->addFlag(flag);
					if ((tt = nextToken(token)) != COMMA)
					{
						returnToken(tt, token);
						break;
					}
				}
			}
			else if (token == KW("priority"))
			{
				int priority;
				if (!readPriority(priority))
					return FALSE;
				task->setPriority(priority);
			}
			else if (token == KW("account"))
			{
				QString account;
				if (nextToken(account) != ID ||
					proj->getAccount(account) == 0)
				{
					fatalError("Account ID expected");
					return FALSE;
				}
				task->setAccount(proj->getAccount(account));
			}
			else if (token == KW("startcredit"))
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value expected");
					return FALSE;
				}
				task->setStartCredit(token.toDouble());
			}
			else if (token == KW("endcredit"))
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value expected");
					return FALSE;
				}
				task->setEndCredit(token.toDouble());
			}
			else if (token == KW("projectid"))
			{
				if (nextToken(token) != ID ||
					!proj->isValidId(token))
				{
					fatalError("Project ID expected");
					return FALSE;
				}
				task->setProjectId(token);
			}
			else if (token == KW("supplement"))
			{
				if (nextToken(token) != ID || (token != KW("task")))
				{
					fatalError("'task' expected");
					return FALSE;
				}
				if ((token == "task" && 
					 !readTaskSupplement(task->getId())))
					return FALSE;
				break;
			}	
			else if (token == KW("include"))
			{
				if (!readInclude())
					return FALSE;
			}
			else
			{
				fatalError("Illegal task attribute '%s'", token.latin1());
				return FALSE;
			}
			break;
		case RCBRACE:
			done = true;
			break;
		default:
			fatalError("Syntax Error at '%s'", token.latin1());
			return FALSE;
		}
	}

	return TRUE;
}

bool
ProjectFile::readVacation(time_t& from, time_t& to, bool readName,
						  QString* n)
{
	TokenType tt;
	if (readName)
	{
		if ((tt = nextToken(*n)) != STRING)
		{
			fatalError("String expected");
			return FALSE;
		}
	}
	QString start;
	if ((tt = nextToken(start)) != DATE)
	{
		fatalError("Date expected");
		return FALSE;
	}
	QString token;
	if ((tt = nextToken(token)) != MINUS)
	{
		// vacation e. g. 2001-11-28
		returnToken(tt, token);
		from = date2time(start);
		to = sameTimeNextDay(date2time(start)) - 1;
	}
	else
	{
		// vacation e. g. 2001-11-28 - 2001-11-30
		QString end;
		if ((tt = nextToken(end)) != DATE)
		{
			fatalError("Date expected");
			return FALSE;
		}
		from = date2time(start);
		if (date2time(start) > date2time(end))
		{
			fatalError("Vacation must start before end");
			return FALSE;
		}
		to = date2time(end) - 1;
	}
	return TRUE;
}

bool
ProjectFile::readPercent(double& value)
{
	QString token;
	TokenType tt;
	
	if ((tt = nextToken(token)) != INTEGER && tt != REAL)
	{
		fatalError("Number expected");
		return FALSE;
	}
	value = token.toDouble();
	if (value < 0.0 || value > 100.0)
	{
		fatalError("Value must be between 0 and 100");
		return FALSE;
	}
	return TRUE;
}

bool
ProjectFile::readResource(Resource* parent)
{
	// Syntax: 'resource id "name" { ... }
	QString id;
	if (nextToken(id) != ID)
	{
		fatalError("ID expected");
		return FALSE;
	}
	QString name;
	if (nextToken(name) != STRING)
	{
		fatalError("String expected");
		return FALSE;
	}

	if (proj->getResource(id))
	{
		fatalError("Resource %s has already been defined", id.latin1());
		return FALSE;
	}

	Resource* r = new Resource(proj, id, name, parent);

	TokenType tt;
	QString token;
	if ((tt = nextToken(token)) == LCBRACE)
	{
		// read optional attributes
		if (!readResourceBody(r))
			return FALSE;
	}
	else
		returnToken(tt, token);

	proj->addResource(r);

	return TRUE;
}

bool
ProjectFile::readResourceSupplement()
{
	QString token;
	Resource* r;
	if (nextToken(token) != ID || (r = proj->getResource(token)) == 0)
	{
		fatalError("Already defined resource ID expected");
		return FALSE;
	}
	if (nextToken(token) != LCBRACE)
	{
		fatalError("'{' expected");
		return FALSE;
	}
	return readResourceBody(r);
}

bool
ProjectFile::readResourceBody(Resource* r)
{
	QString token;
	TokenType tt;

	while ((tt = nextToken(token)) != RCBRACE)
	{
		if (tt != ID)
		{
			fatalError("Unknown attribute '%s'", token.latin1());
			return FALSE;
		}
		if (token == KW("resource"))
		{
			if (!readResource(r))
				return FALSE;
		}
		else if (token == KW("mineffort"))
		{
			if (nextToken(token) != REAL)
			{
				fatalError("Real value exptected");
				return FALSE;
			}
			r->setMinEffort(token.toDouble());
		}
		else if (token == KW("maxeffort"))
		{
			if (nextToken(token) != REAL)
			{
				fatalError("Real value exptected");
				return FALSE;
			}
			r->setMaxEffort(token.toDouble());
		}
		else if (token == KW("efficiency"))
		{
			if (nextToken(token) != REAL)
			{
				fatalError("Read value expected");
				return FALSE;
			}
			r->setEfficiency(token.toDouble());
		}
		else if (token == KW("rate"))
		{
			if (nextToken(token) != REAL)
			{
				fatalError("Real value exptected");
				return FALSE;
			}
			r->setRate(token.toDouble());
		}
		else if (token == KW("kotrusid"))
		{
			if (nextToken(token) != STRING)
			{
				fatalError("String expected");
				return FALSE;
			}
			r->setKotrusId(token);
		}
		else if (token == KW("vacation"))
		{
			time_t from, to;
			if (!readVacation(from, to))
				return FALSE;
			r->addVacation(new Interval(from, to));
		}
		else if (token == KW("workinghours"))
		{
			int dow;
			QPtrList<Interval>* l = new QPtrList<Interval>();
			if (!readWorkingHours(dow, l))
				return FALSE;

			r->setWorkingHours(dow, l);
		}
		else if (token == KW("shift"))
		{
			time_t from, to;
			from = proj->getStart();
			to = proj->getEnd();
			Shift* s;
			if ((s = readShiftSelection(from, to)) == 0)
				return FALSE;
			if (!r->addShift(Interval(from, to), s))
			{
				fatalError("Shift interval overlaps with other");
				return FALSE;
			}
		}
		else if (token == KW("planbooking"))
		{
			Booking* b;
			if ((b = readBooking()) == 0)
				return FALSE;
			if (!r->addPlanBooking(b))
			{
				fatalError("Resource is already booked during this period");
				return FALSE;
			}
		}
		else if (token == KW("actualbooking"))
		{
			Booking* b;
			if ((b = readBooking()) == 0)
				return FALSE;
			if (!r->addActualBooking(b))
			{
				fatalError("Resource is already booked during this period");
				return FALSE;
			}
		}
		else if (token == KW("flags"))
		{
			for ( ; ; )
			{
				QString flag;
				if (nextToken(flag) != ID || !proj->isAllowedFlag(flag))
				{
					fatalError("flag unknown");
					return FALSE;
				}
				r->addFlag(flag);
				if ((tt = nextToken(token)) != COMMA)
				{
					returnToken(tt, token);
					break;
				}
			}
		}
		else if (token == KW("include"))
		{
			if (!readInclude())
				return FALSE;
			break;
		}
		else
		{
			fatalError("Unknown attribute '%s'", token.latin1());
			return FALSE;
		}
	}

	return TRUE;
}

bool
ProjectFile::readShift(Shift* parent)
{
	// Syntax: 'shift id "name" { ... }
	QString id;
	if (nextToken(id) != ID)
	{
		fatalError("ID expected");
		return FALSE;
	}
	QString name;
	if (nextToken(name) != STRING)
	{
		fatalError("String expected");
		return FALSE;
	}

	if (proj->getShift(id))
	{
		fatalError("Shift %s has already been defined", id.latin1());
		return FALSE;
	}

	Shift* s = new Shift(proj, id, name, parent);

	TokenType tt;
	QString token;
	if ((tt = nextToken(token)) == LCBRACE)
	{
		// read optional attributes
		while ((tt = nextToken(token)) != RCBRACE)
		{
			if (tt != ID)
			{
				fatalError("Unknown attribute '%s'", token.latin1());
				return FALSE;
			}
			if (token == KW("shift"))
			{
				if (!readShift(s))
					return FALSE;
			}
			else if (token == KW("workinghours"))
			{
				int dow;
				QPtrList<Interval>* l = new QPtrList<Interval>();
				if (!readWorkingHours(dow, l))
					return FALSE;
				
				s->setWorkingHours(dow, l);
			}
			else if (token == KW("include"))
			{
				if (!readInclude())
					return FALSE;
				break;
			}
			else
			{
				fatalError("Unknown attribute '%s'", token.latin1());
				return FALSE;
			}
		}
	}
	else
		returnToken(tt, token);

	proj->addShift(s);

	return TRUE;
}

Shift*
ProjectFile::readShiftSelection(time_t& from, time_t& to)
{
	// Syntax: ID [from [- to]]
	QString id;
	if (nextToken(id) != ID)
	{
		fatalError("Shift ID expected");
		return 0;
	}
	Shift* s = 0;
	if ((s = proj->getShift(id)) == 0)
	{
		fatalError("Unknown shift");
		return 0;
	}
	QString token;
	TokenType tt;
	// Clumsy look-ahead
	tt = nextToken(token);
	returnToken(tt, token);
	if (tt == DATE)
		if (!readVacation(from, to))
			return 0;

	return s;
}

Booking*
ProjectFile::readBooking()
{
	QString token;

	if (nextToken(token) != DATE)
	{
		fatalError("Start date expected");
		return 0;
	}
	time_t start = date2time(token);
	if (start < proj->getStart() || start >= proj->getEnd())
	{
		fatalError("Start date must be within the project timeframe");
		return 0;
	}
	
	if (nextToken(token) != DATE)
	{
		fatalError("End date expected");
		return 0;
	}
	time_t end = date2time(token);
	if (end <= proj->getStart() || end > proj->getEnd())
	{
		fatalError("End date must be within the project timeframe");
		return 0;
	}
	if (start >= end)
	{
		fatalError("End date must be after start date");
		return 0;
	}

	Task* task;
	TokenType tt;
	if (((tt = nextToken(token)) != ID && tt != ABSOLUTE_ID) ||
		(task = proj->getTask(getTaskPrefix() + token)) == 0)
	{
		fatalError("Task ID expected");
		return 0;
	}
	return new Booking(Interval(start, end), task, "", task->getProjectId());
}

bool
ProjectFile::readAccount(Account* parent)
{
	// Syntax: 'account id "name" { ... }
	QString id;
	if (nextToken(id) != ID)
	{
		fatalError("ID expected");
		return FALSE;
	}

	if (proj->getAccount(id))
	{
		fatalError("Account %s has already been defined", id.latin1());
		return FALSE;
	}

	QString name;
	if (nextToken(name) != STRING)
	{
		fatalError("String expected");
		return FALSE;
	}
	Account::AccountType acctType;
	if (parent == 0)
	{
		/* Only accounts with no parent can have a type specifier. All
		 * sub accounts inherit the type of the parent. */
		QString at;
		if (nextToken(at) != ID && (at != KW("cost") ||
								   	at != KW("revenue")))
		{
			fatalError("Account type 'cost' or 'revenue' expected");
			return FALSE;
		}
		acctType = at == KW("cost") ? Account::Cost : Account::Revenue;
	}
	else
		acctType = parent->getAcctType();

	Account* a = new Account(proj, id, name, parent, acctType);
	if (parent)
		parent->addSub(a);

	TokenType tt;
	QString token;
	if ((tt = nextToken(token)) == LCBRACE)
	{
		bool hasSubAccounts = FALSE;
		bool cantBeParent = FALSE;
		// read optional attributes
		while ((tt = nextToken(token)) != RCBRACE)
		{
			if (tt != ID)
			{
				fatalError("Unknown attribute '%s'", token.latin1());
				return FALSE;
			}
			if (token == KW("account") && !cantBeParent)
			{
				if (!readAccount(a))
					return FALSE;
				hasSubAccounts = TRUE;
			}
			else if (token == KW("credit"))
			{
				if (!readCredit(a))
					return FALSE;
			}
			else if (token == KW("kotrusid") && !hasSubAccounts)
			{
				if (nextToken(token) != STRING)
				{
					fatalError("String expected");
					return FALSE;
				}
				a->setKotrusId(token);
				cantBeParent = TRUE;
			}
			else if (token == KW("include"))
			{
				if (!readInclude())
					return FALSE;
			}
			else
			{
				fatalError("Illegal attribute");
				return FALSE;
			}
		}
	}
	else
		returnToken(tt, token);

	proj->addAccount(a);

	return TRUE;
}

bool
ProjectFile::readCredit(Account* a)
{
	QString token;

	if (nextToken(token) != DATE)
	{
		fatalError("Date expected");
		return FALSE;
	}
	time_t date = date2time(token);

	QString description;
	if (nextToken(description) != STRING)
	{
		fatalError("String expected");
		return FALSE;
	}

	if (nextToken(token) != REAL)
	{
		fatalError("Real value expected");
		return FALSE;
	}
	Transaction* t = new Transaction(date, token.toDouble(), description);
	a->credit(t);

	return TRUE;
}

bool
ProjectFile::readAllocate(Task* t)
{
	QString id;
	Resource* r;
	if (nextToken(id) != ID || (r = proj->getResource(id)) == 0)
	{
		fatalError("Resource ID expected");
		return FALSE;
	}
	Allocation* a = new Allocation(r);
	QString token;
	TokenType tt;
	if ((tt = nextToken(token)) == LCBRACE)
	{
		while ((tt = nextToken(token)) != RCBRACE)
		{
			if (tt != ID)
			{
				fatalError("Unknown attribute '%s'", token.latin1());
				return FALSE;
			}
			if (token == KW("load"))
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value expected");
					return FALSE;
				}
				double load = token.toDouble();
				if (load < 0.01)
				{
					fatalError("Value must be at least 0.01");
					return FALSE;
				}
				a->setLoad((int) (100 * load));
			}
			else if (token == KW("shift"))
			{
				time_t from = proj->getStart();
				time_t to = proj->getEnd();
				Shift* s;
				if ((s = readShiftSelection(from, to)) == 0)
					return FALSE;
				if (!a->addShift(Interval(from, to), s))
				{
					fatalError("Shift intervals may not overlap");
					return FALSE;
				}
			}
			else if (token == KW("persistent"))
			{
				a->setPersistent(TRUE);
			}
			else if (token == KW("alternative"))
			{
				do
				{
					Resource* r;
					if ((tt = nextToken(token)) != ID ||
						(r = proj->getResource(token)) == 0)
					{
						fatalError("Resource ID expected");
						return FALSE;
					}
					a->addCandidate(r);
				} while ((tt = nextToken(token)) == COMMA);
				returnToken(tt, token);
			}
			else if (token == KW("select"))
			{
				if (nextToken(token) != ID || !a->setSelectionMode(token))
				{
					fatalError("Invalid selction mode");
					return FALSE;
				}
			}
			else
			{
				fatalError("Unknown attribute '%s'", token.latin1());
				return FALSE;
			}
		}
	}
	else
		returnToken(tt, token);
	t->addAllocation(a);

	return TRUE;
}

bool
ProjectFile::readTimeValue(ulong& value)
{
	QString val;
	if (nextToken(val) != INTEGER)
	{
		fatalError("Integer value expected");
		return FALSE;
	}
	QString unit;
	if (nextToken(unit) != ID)
	{
		fatalError("Unit expected");
		return FALSE;
	}
	if (unit == KW("min"))
		value = val.toULong() * 60;
	else if (unit == KW("h"))
		value = val.toULong() * (60 * 60);
	else if (unit == KW("d"))
		value = val.toULong() * (60 * 60 * 24);
	else if (unit == KW("w"))
		value = val.toULong() * (60 * 60 * 24 * 7);
	else if (unit == KW("m"))
		value = val.toULong() * (60 * 60 * 24 * 30);
	else if (unit == KW("y"))
		value = val.toULong() * (60 * 60 * 24 * 356);
	else
	{
		fatalError("Unit expected");
		return FALSE;
	}
	return TRUE;
}

bool
ProjectFile::readPlanTimeFrame(double& value, bool workingDays)
{
	QString val;
	TokenType tt;
	if ((tt = nextToken(val)) != REAL && tt != INTEGER)
	{
		fatalError("Real value expected");
		return FALSE;
	}
	QString unit;
	if (nextToken(unit) != ID)
	{
		fatalError("Unit expected");
		return FALSE;
	}
	if (unit == KW("min"))
		value = val.toDouble() / (proj->getDailyWorkingHours() * 60);
	else if (unit == KW("h"))
		value = val.toDouble() / proj->getDailyWorkingHours();
	else if (unit == KW("d"))
		value = val.toDouble();
	else if (unit == KW("w"))
		value = val.toDouble() * 
			(workingDays ? proj->getWeeklyWorkingDays() : 7);
	else if (unit == KW("m"))
		value = val.toDouble() * 
			(workingDays ? proj->getMonthlyWorkingDays() : 30.416);
	else if (unit == KW("y"))
		value = val.toDouble() * 
			(workingDays ? proj->getYearlyWorkingDays() : 365);
	else
	{
		fatalError("Unit expected");
		return FALSE;
	}

	return TRUE;
}

bool
ProjectFile::readWorkingHours(int& dayOfWeek, QPtrList<Interval>* l)
{
	l->setAutoDelete(TRUE);
	QString day;
	if (nextToken(day) != ID)
	{
		fatalError("Weekday expected");
		return FALSE;
	}
	const char* days[] = { KW("sun"), KW("mon"), KW("tue"), KW("wed"),
	   	KW("thu"), KW("fri"), KW("sat") };
	for (dayOfWeek = 0; dayOfWeek < 7; dayOfWeek++)
		if (days[dayOfWeek] == day)
			break;
	if (dayOfWeek == 7)
	{
		fatalError("Weekday expected");
		return FALSE;
	}

	QString token;
	TokenType tt;
	if ((tt = nextToken(token)) == ID && token == KW("off"))
		return TRUE;
	else
		returnToken(tt, token);

	for ( ; ; )
	{
		QString start;
		if (nextToken(start) != HOUR)
		{
			fatalError("Start time as HH:MM expected");
			return FALSE;
		}
		QString token;
		if (nextToken(token) != MINUS)
		{
			fatalError("'-' expected");
			return FALSE;
		}
		QString end;
		if (nextToken(end) != HOUR)
		{
			fatalError("End time as HH:MM expected");
			return FALSE;
		}
		time_t st, et;
		if ((st = hhmm2time(start)) < 0)
			return FALSE;
		if ((et = hhmm2time(end)) < 0)
			return FALSE;
		if (et <= st)
		{
			fatalError("End time must be larger than start time");
			return FALSE;
		}
		Interval* iv = new Interval(st, et - 1); 
		for (Interval* i = l->first(); i != 0; i = l->next())
			if (iv->overlaps(*i))
			{
				fatalError("Working hour intervals may not overlap");
				return FALSE;
			}
		l->append(iv);
		TokenType tt;
		if ((tt = nextToken(token)) != COMMA)
		{
			returnToken(tt, token);
			break;
		}
	}
	return TRUE;
}

bool
ProjectFile::readPriority(int& priority)
{
	QString token;

	if (nextToken(token) != INTEGER)
	{
		fatalError("Integer value expected");
		return FALSE;
	}
	priority = token.toInt();
	if (priority < 1 || priority > 1000)
	{
		fatalError("Priority value must be between 1 and 1000");
		return FALSE;
	}
	return TRUE;
}

#ifdef HAVE_ICAL
#ifdef HAVE_KDE
bool
ProjectFile::readICalTaskReport()
{
   QString token;
   if (nextToken(token) != STRING)
   {
      fatalError("File name expected");
      return FALSE;
   }
   ReportICal *rep = new ReportICal( proj, token, proj->getStart(), proj->getEnd());
   proj->addICalReport( rep );

   return( true );
}
#endif
#endif

bool
ProjectFile::readXMLTaskReport()
{
   QString token;
   if (nextToken(token) != STRING)
   {
      fatalError("File name expected");
      return FALSE;
   }
   ReportXML *rep = new ReportXML(proj, token, proj->getStart(),
								  proj->getEnd(), getFile(), getLine());
   proj->addXMLReport( rep );

   return( true );
}


bool
ProjectFile::readHTMLReport(const QString& reportType)
{
	QString token;
	if (nextToken(token) != STRING)
	{
		fatalError("File name expected");
		return FALSE;
	}
	
	ReportHtml* report;
	if (reportType == KW("htmltaskreport"))
		report = new HTMLTaskReport(proj, token, proj->getStart(),
									proj->getEnd(), getFile(), getLine());
	else if (reportType == KW("htmlresourcereport"))
		report = new HTMLResourceReport(proj, token, proj->getStart(),
										proj->getEnd(), getFile(), getLine());
	else
	{
		qFatal("readHTMLReport: bad report type");
		return FALSE;	// Just to please the compiler.
	}
		
	TokenType tt;
	if ((tt = nextToken(token)) != LCBRACE)
	{
		returnToken(tt, token);
		return TRUE;
	}

	for ( ; ; )
	{
		if ((tt = nextToken(token)) == RCBRACE)
			break;
		else if (tt != ID)
		{
			fatalError("Attribute ID or '}' expected");
			return FALSE;
		}
		if (token == KW("columns"))
		{
			report->clearColumns();
			for ( ; ; )
			{
				QString col;
				if ((tt = nextToken(col)) != ID)
				{
					fatalError("Column ID expected");
					return FALSE;
				}
				report->addReportColumn(col);
				if ((tt = nextToken(token)) != COMMA)
				{
					returnToken(tt, token);
					break;
				}
			}
		}
		else if (token == KW("start"))
		{
			if (nextToken(token) != DATE)
			{
				fatalError("Date expected");
				return FALSE;
			}
			report->setStart(date2time(token));
		}
		else if (token == KW("end"))
		{
			if (nextToken(token) != DATE)
			{
				fatalError("Date expected");
				return FALSE;
			}
			report->setEnd(date2time(token));
		}
		else if (token == KW("headline"))
		{
			if (nextToken(token) != STRING)
			{
				fatalError("String exptected");
				return FALSE;
			}
			report->setHeadline(token);
		}
		else if (token == KW("caption"))
		{
			if (nextToken(token) != STRING)
			{
				fatalError("String exptected");
				return FALSE;
			}
			report->setCaption(token);
		}
		else if (token == KW("rawhead"))
		{
			if (nextToken(token) != STRING)
			{
				fatalError("String expected");
				return FALSE;
			}
			report->setRawHead(token);
		}
		else if (token == KW("rawtail"))
		{
			if (nextToken(token) != STRING)
			{
				fatalError("String expected");
				return FALSE;
			}
			report->setRawTail(token);
		}
		else if (token == KW("rawstylesheet"))
		{
			if (nextToken(token) != STRING)
			{
				fatalError("String expected");
				return FALSE;
			}
			report->setRawStyleSheet(token);
		}
		else if (token == KW("showactual"))
		{
			report->setShowActual(TRUE);
		}
		else if (token == KW("showprojectids"))
		{
			report->setShowPIDs(TRUE);
		}
		else if (token == KW("hidetask"))
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setHideTask(et);
		}
		else if (token == KW("rolluptask"))
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setRollUpTask(et);
		}
		else if (token == KW("sorttasks"))
		{
			if (!readSorting(report, 0))
				return FALSE;
		}
		else if (token == KW("hideresource"))
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setHideResource(et);
		}
		else if (token == KW("rollupresource"))
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setRollUpResource(et);
		}
		else if (token == KW("sortresources"))
		{
			if (!readSorting(report, 1))
				return FALSE;
		}
		else if (token == KW("url"))
		{
			if (!readHtmlUrl(report))
				return FALSE;
		}
		else if (token == KW("loadunit"))
		{
			if (nextToken(token) != ID || !report->setLoadUnit(token))
			{
				fatalError("Illegal load unit");
				return FALSE;
			}
		}
		else
		{
			fatalError("Illegal attribute");
			return FALSE;
		}
	}

	if (reportType == KW("htmltaskreport"))
		proj->addHTMLTaskReport((HTMLTaskReport*) report);
	else
		proj->addHTMLResourceReport((HTMLResourceReport*) report);

	return TRUE;
}

bool
ProjectFile::readHTMLAccountReport()
{
	QString token;
	if (nextToken(token) != STRING)
	{
		fatalError("File name expected");
		return FALSE;
	}
	
	HTMLAccountReport* report;
	report = new HTMLAccountReport(proj, token, proj->getStart(),
								   proj->getEnd(), getFile(), getLine());
		
	TokenType tt;
	if ((tt = nextToken(token)) != LCBRACE)
	{
		returnToken(tt, token);
		return TRUE;
	}

	for ( ; ; )
	{
		if ((tt = nextToken(token)) == RCBRACE)
			break;
		else if (tt != ID)
		{
			fatalError("Attribute ID or '}' expected");
			return FALSE;
		}
		if (token == KW("columns"))
		{
			report->clearColumns();
			for ( ; ; )
			{
				QString col;
				if ((tt = nextToken(col)) != ID)
				{
					fatalError("Column ID expected");
					return FALSE;
				}
				report->addReportColumn(col);
				if ((tt = nextToken(token)) != COMMA)
				{
					returnToken(tt, token);
					break;
				}
			}
		}
		else if (token == KW("start"))
		{
			if (nextToken(token) != DATE)
			{
				fatalError("Date expected");
				return FALSE;
			}
			report->setStart(date2time(token));
		}
		else if (token == KW("end"))
		{
			if (nextToken(token) != DATE)
			{
				fatalError("Date expected");
				return FALSE;
			}
			report->setEnd(date2time(token));
		}
		else if (token == KW("headline"))
		{
			if (nextToken(token) != STRING)
			{
				fatalError("String exptected");
				return FALSE;
			}
			report->setHeadline(token);
		}
		else if (token == KW("caption"))
		{
			if (nextToken(token) != STRING)
			{
				fatalError("String exptected");
				return FALSE;
			}
			report->setCaption(token);
		}
		else if (token == KW("hideplan"))
		{
			report->setHidePlan(TRUE);
		}
		else if (token == KW("showactual"))
		{
			report->setShowActual(TRUE);
		}
		else if (token == KW("accumulate"))
		{
			report->setAccumulate(TRUE);
		}
		else if (token == KW("hideaccount"))
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setHideAccount(et);
		}
		else if (token == KW("rollupaccount"))
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setRollUpAccount(et);
		}
		else if (token == KW("sortaccounts"))
		{
			if (!readSorting(report, 2))
				return FALSE;
		}
		else
		{
			fatalError("Illegal attribute");
			return FALSE;
		}
	}

	proj->addHTMLAccountReport(report);

	return TRUE;
}

bool
ProjectFile::readExportReport()
{
	QString token;
	if (nextToken(token) != STRING)
	{
		fatalError("File name expected");
		return FALSE;
	}
	
	ExportReport* report;
	report = new ExportReport(proj, token, getFile(), getLine());
		
	TokenType tt;
	if ((tt = nextToken(token)) != LCBRACE)
	{
		returnToken(tt, token);
		return TRUE;
	}

	for ( ; ; )
	{
		if ((tt = nextToken(token)) == RCBRACE)
			break;
		else if (tt != ID)
		{
			fatalError("Attribute ID or '}' expected");
			return FALSE;
		}
		
		if (token == KW("hidetask"))
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setHideTask(et);
		}
		else if (token == KW("rolluptask"))
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setRollUpTask(et);
		}
		else if (token == KW("hideresource"))
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setHideResource(et);
		}
		else if (token == KW("rollupresource"))
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setRollUpResource(et);
		}
		else if (token == KW("taskattributes"))
		{
			for ( ; ; )
			{
				QString ta;
				if (nextToken(ta) != ID ||
					!report->addTaskAttribute(ta))
				{
					fatalError("task attribute expected");
					return FALSE;
				}

				if ((tt = nextToken(token)) != COMMA)
				{
					returnToken(tt, token);
					break;
				}
			}
		}
		else
		{
			fatalError("Illegal attribute");
			return FALSE;
		}
	}

	proj->addExportReport(report);

	return TRUE;
}

bool
ProjectFile::readHtmlUrl(ReportHtml* report)
{
	QString key;
	QString url;

	if (nextToken(key) != ID)
	{
		fatalError("URL ID expected");
		return FALSE;
	}
	if (nextToken(url) != STRING)
	{
		fatalError("String expected");
		return FALSE;
	}
	if (!report->setUrl(key, url))
	{
		fatalError("Unknown URL ID");
		return FALSE;
	}
	return TRUE;
}

Operation*
ProjectFile::readLogicalExpression(int precedence)
{
	Operation* op;
	QString token;
	TokenType tt;

	if ((tt = nextToken(token)) == ID || tt == ABSOLUTE_ID)
	{
		if (proj->isAllowedFlag(token) ||
			proj->getTask(token) ||
			proj->getResource(token) ||
			proj->getAccount(token) ||
			proj->isValidId(token))
		{
			op = new Operation(Operation::Id, token);
		}
		else if (ExpressionTree::isFunction(token))
		{
			if ((op = readFunctionCall(token)) == 0)
				return 0;
		}
		else
		{
			fatalError("Flag, function or ID '%s' is unknown.", token.latin1());
			return 0;
		}
	}
	else if (tt == DATE)
	{
		time_t date;
		if ((date = date2time(token)) == 0)
			fatalError("%s", getUtilityError().latin1());
		else
			op = new Operation(Operation::Date, date);
	}
	else if (tt == INTEGER)
	{
		op = new Operation(token.toLong());
	}
	else if (tt == TILDE)
	{
		if ((op = readLogicalExpression(1)) == 0)
		{
			return 0;
		}
		op = new Operation(op, Operation::Not);
	}
	else if (tt == LBRACE)
	{
		if ((op = readLogicalExpression()) == 0)
		{
			return 0;
		}
		if ((tt = nextToken(token)) != RBRACE)
		{
			fatalError("')' expected");
			return 0;
		}
	}
	else
	{
		fatalError("Logical expression expected");
		return 0;
	}
	
	if (precedence < 1)
	{
		if ((tt = nextToken(token)) != AND && tt != OR)
		{
			returnToken(tt, token);
		}
		else if (tt == AND)
		{
			Operation* op2 = readLogicalExpression();
			op = new Operation(op, Operation::And, op2);
		}
		else if (tt == OR)
		{
			Operation* op2 = readLogicalExpression();
			op = new Operation(op, Operation::Or, op2);
		}
	}

	return op;
}

Operation*
ProjectFile::readFunctionCall(const QString& name)
{
	QString token;
	TokenType tt;
	
	if ((tt = nextToken(token)) != LBRACE)
	{
		fatalError("'(' expected");
		return 0;
	}
	QPtrList<Operation> args;
	for (int i = 0; i < ExpressionTree::arguments(name); i++)
	{
		Operation* op;
		if ((op = readLogicalExpression()) == 0)
			return 0;
		args.append(op);
		if ((i < ExpressionTree::arguments(name) - 1) &&
			nextToken(token) != COMMA)
		{
			fatalError("Comma expected");
			return 0;
		}
	}
	if ((tt = nextToken(token)) != RBRACE)
	{
		fatalError("')' expected");
		return 0;
	}
	return new Operation(name, args);
}

bool
ProjectFile::readSorting(Report* report, int which)
{
	TokenType tt;
	QString token;

	int i = 0;
	do
	{
		nextToken(token);
		CoreAttributesList::SortCriteria sorting;
		if (token == KW("tree"))
			sorting = CoreAttributesList::TreeMode;
		else if (token == KW("indexup"))
			sorting = CoreAttributesList::IndexUp;
		else if (token == KW("indexdown"))
			sorting = CoreAttributesList::IndexDown;
		else if (token == KW("idup"))
			sorting = CoreAttributesList::IdUp;
		else if (token == KW("iddown"))
			sorting = CoreAttributesList::IdDown;
		else if (token == KW("fullnameup"))
			sorting = CoreAttributesList::FullNameUp;
		else if (token == KW("fullnamedown"))
			sorting = CoreAttributesList::FullNameDown;
		else if (token == KW("nameup"))
			sorting = CoreAttributesList::NameUp;
		else if (token == KW("namedown"))
			sorting = CoreAttributesList::NameDown;
		else if (token == KW("startup") || token == KW("planstartup"))
			sorting = CoreAttributesList::PlanStartUp;
		else if (token == KW("startdown") || token == KW("planstartdown"))
			sorting = CoreAttributesList::PlanStartDown;
		else if (token == KW("endup") || token == KW("planendup"))
			sorting = CoreAttributesList::PlanEndUp;
		else if (token == KW("enddown") || token == KW("planenddown"))
			sorting = CoreAttributesList::PlanEndDown;
		else if (token == KW("actualstartup"))
			sorting = CoreAttributesList::ActualStartUp;
		else if (token == KW("actualstartdown"))
			sorting = CoreAttributesList::ActualStartDown;
		else if (token == KW("actualendup"))
			sorting = CoreAttributesList::ActualEndUp;
		else if (token == KW("actualenddown"))
			sorting = CoreAttributesList::ActualEndDown;
		else if (token == KW("priorityup"))
			sorting = CoreAttributesList::PrioUp;
		else if (token == KW("prioritydown"))
			sorting = CoreAttributesList::PrioDown;
		else if (token == KW("responsibleup"))
			sorting = CoreAttributesList::ResponsibleUp;
		else if (token == KW("responsibledown"))
			sorting = CoreAttributesList::ResponsibleDown;
		else if (token == KW("mineffortup"))
			sorting = CoreAttributesList::MinEffortUp;
		else if (token == KW("mineffortdown"))
			sorting = CoreAttributesList::MinEffortDown;
		else if (token == KW("maxeffortup"))
			sorting = CoreAttributesList::MaxEffortUp;
		else if (token == KW("maxeffortdown"))
			sorting = CoreAttributesList::MaxEffortDown;
		else if (token == KW("rateup"))
			sorting = CoreAttributesList::RateUp;
		else if (token == KW("ratedown"))
			sorting = CoreAttributesList::RateDown;
		else if (token == KW("kotrusidup"))
			sorting = CoreAttributesList::KotrusIdUp;
		else if (token == KW("kotrusiddown"))
			sorting = CoreAttributesList::KotrusIdDown;
		else
		{
			fatalError("Sorting criteria expected");
			return FALSE;
		}

		bool ok = TRUE;
		switch (which)
		{
			case 0:
				ok = report->setTaskSorting(sorting, i);
				break;
			case 1:
				ok = report->setResourceSorting(sorting, i);
				break;
			case 2:
				ok = report->setAccountSorting(sorting, i);
				break;
			default:
				qFatal("readSorting: Unknown sorting attribute");
				return FALSE;
		}
		if (!ok)
		{
			fatalError("This sorting criteria is not supported for the list "
					   "or it is used at the wrong position.");
			return FALSE;
		}
		tt = nextToken(token);
	} while (++i < CoreAttributesList::maxSortingLevel && tt == COMMA);

	returnToken(tt, token);

	return TRUE;
}

time_t
ProjectFile::date2time(const QString& date)
{
	time_t res;
	if ((res = ::date2time(date)) == 0)
		fatalError(getUtilityError());
	return res;
}

int
ProjectFile::hhmm2time(const QString& hhmm)
{
	int hour = hhmm.left(hhmm.find(':')).toInt();
	if (hour > 24)
	{
		fatalError("Hour must be in the range of 0 - 24");
		return -1;
	}
	int min = hhmm.mid(hhmm.find(':') + 1).toInt();
	if (min > 59)
	{
		fatalError("Minutes must be in the range of 0 - 59");
		return -1;
	}
	if (hour == 24 && min != 0)
	{
		fatalError("Maximum time is 24:00");
		return -1;
	}
	return hour * 60 * 60 + min * 60;
}
