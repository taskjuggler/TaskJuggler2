/*
 * ProjectFile.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <ctype.h>
#include <stdio.h>
#include <stream.h>

#include "ProjectFile.h"
#include "Project.h"
#include "Token.h"
#include "ExpressionTree.h"
#include "kotrus.h"

extern Kotrus *kotrus;

#define READ_DATE(a, b) \
(token == a && !hasSubTasks) \
{ \
	if ((tt = nextToken(token)) == DATE) \
		task->b(date2time(token)); \
	else \
	{ \
		fatalError("Date expected"); \
		return FALSE; \
	} \
	cantBeParent = TRUE; \
}

FileInfo::FileInfo(ProjectFile* p, const QString& file_)
	: pf(p)
{
	tokenTypeBuf = INVALID;
	file = file_;
}

bool
FileInfo::open()
{
	if ((f = fopen(file, "r")) == 0)
		return FALSE;

	lineBuf = "";
	currLine = 1;
	return TRUE;
}

bool
FileInfo::close()
{
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
			if ((d = getC()) == '{')
			{
				// remove $ from lineBuf;
				lineBuf = lineBuf.left(lineBuf.length() - 1);
				readMacroCall();
				goto BEGIN;
			}
			else
				ungetC(d);
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
			if (token.contains('!') || token.contains('.'))
				return RELATIVE_ID;
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
				// this must be a ISO date yyyy-mm-dd[-hh:mm]
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
		else if (c == '"')
		{
			// quoted string
			while ((c = getC()) != EOF && c != '"')
				token += c;
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
				fatalError(QString("Illegal character '") + c + "'");
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
	QString macro = pf->getMacros().expand(id);
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
FileInfo::fatalError(const QString& msg)
{
	if (macroStack.isEmpty())
	{
		qWarning("%s:%d:%s", file.latin1(), currLine, msg.latin1());
		qWarning("%s", lineBuf.latin1());
	}
	else
	{
		qWarning("Error in expanded macro");
		qWarning("%s:%d: %s",
				 macroStack.last()->getFile().latin1(),
				 macroStack.last()->getLine(), msg.latin1());
		qWarning("%s", lineBuf.latin1());
	}
}

ProjectFile::ProjectFile(Project* p)
{
	proj = p;
	openFiles.setAutoDelete(TRUE);
}

bool
ProjectFile::open(const QString& file)
{
	FileInfo* fi = new FileInfo(this, file);

	if (!fi->open())
	{
		qFatal("Cannot open '%s'", file.latin1());
		return FALSE;
	}

	openFiles.append(fi);
	return TRUE;
}

bool
ProjectFile::close()
{
	bool error = FALSE;

	FileInfo* fi = openFiles.getLast();

	if (!fi->close())
		error = TRUE;
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
			close();
			if (openFiles.isEmpty())
				return TRUE;
			break;
		case ID:
			if (token == "task")
			{
				if (!readTask(0))
					return FALSE;
				break;
			}
			if (token == "account")
			{
				if (!readAccount())
					return FALSE;
				break;
			}
			else if (token == "resource")
			{
				if (!readResource())
					return FALSE;
				break;
			}
			else if (token == "vacation")
			{
				time_t from, to;
				QString name;
				if (!readVacation(from, to, TRUE, &name))
					return FALSE;
				proj->addVacation(name, from, to);
				break;
			}
			else if (token == "priority")
			{
				int priority;
				if (!readPriority(priority))
					return FALSE;
				proj->setPriority(priority);
				break;
			}
			else if (token == "now")
			{
				if (nextToken(token) != DATE)
				{
					fatalError("Date expected");
					return FALSE;
				}
				proj->setNow(date2time(token));
				break;
			}
			else if (token == "minEffort")
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value exptected");
					return FALSE;
				}
				proj->setMinEffort(token.toDouble());
				break;
			}
			else if (token == "maxEffort")
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value exptected");
					return FALSE;
				}
				proj->setMaxEffort(token.toDouble());
				break;
			}
			else if (token == "rate")
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value exptected");
					return FALSE;
				}
				proj->setRate(token.toDouble());
				break;
			}
			else if (token == "timingResolution")
			{
				ulong resolution;
				if (!readTimeValue(resolution))
					return FALSE;
				if (resolution < 60 * 5)
				{
					fatalError("scheduleGranularity must be at least 5 min");
					return FALSE;
				}
				proj->setScheduleGranularity(resolution);
				break;
			}
			else if (token == "copyright")
			{
				if (nextToken(token) != STRING)
				{
					fatalError("String expected");
					return FALSE;
				}
				proj->setCopyright(token);
				break;
			}
			else if (token == "include")
			{
				if (nextToken(token) != STRING)
				{
					fatalError("File name expected");
					return FALSE;
				}
				if (!open(token))
					return FALSE;
				break;
			}
			else if (token == "macro")
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
			else if (token == "flags")
			{
				for ( ; ; )
				{
					QString flag;
					if (nextToken(flag) != ID)
					{
						fatalError("flag ID expected");
						return FALSE;
					}
					if (proj->isAllowedFlag(flag))
					{
						fatalError(QString("Flag ") + flag +
								   " can't be registered twice");
						return FALSE;
					}
					proj->addAllowedFlag(flag);
					if ((tt = nextToken(token)) != COMMA)
					{
						openFiles.last()->returnToken(tt, token);
						break;
					}
				}
				break;
			}
			else if (token == "project")
			{
				if (nextToken(token) != ID)
				{
					fatalError("Project ID expected");
					return FALSE;
				}
				proj->setId(token);
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
				break;
			}
			else if (token == "xmlTaskReport" )
			{
			   if( !readXMLTaskReport())
			      return FALSE;
			   break;
			}
			else if (token == "htmlTaskReport")
			{
				if (!readHTMLTaskReport())
					return FALSE;
				break;
			}
			else if (token == "htmlResourceReport")
			{
				if (!readHTMLResourceReport())
					return FALSE;
				break;
			}
			else if( token == "kotrusMode" )
			{
			   if( kotrus )
			   {
			      if( nextToken(token) != STRING )
			      {
				 kotrus->setKotrusMode( "NoKotrus" );
			      }
			      else
			      {
				 if( token == "DB" || token == "XML" || token == "NoKotrus" )
				    kotrus->setKotrusMode( token );
				 else
				 {
				    fatalError( "Unknown kotrus-mode");
				    return( false );
				 }
			      }
			   }
			   break;
			}
			// break missing on purpose!
		default:
			fatalError(QString("Syntax Error at '") + token + "'!");
			return FALSE;
		}
	}

	return TRUE;
}

TokenType
ProjectFile::nextToken(QString& buf)
{
	return openFiles.last()->nextToken(buf);
}

void
ProjectFile::fatalError(const QString& msg)
{
	openFiles.last()->fatalError(msg);
}

bool
ProjectFile::readTask(Task* parent)
{
	TokenType tt;
	QString token;

	if (proj->getStart() == 0)
	{
		fatalError("Project start date must be specified first");
		return FALSE;
	}

	QString id;
	if ((tt = nextToken(id)) != ID)
	{
		fatalError("ID expected");
		return FALSE;
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

	QString parentId;
	if (parent)
		parentId = parent->getId() + ".";
	Task* task = new Task(proj, parentId + id, name, parent,
						  getFile(), getLine());

	proj->addTask(task);
	if (parent)
		parent->addSubTask(task);

	for (bool done = false ; !done; )
	{
		bool hasSubTasks = FALSE;
		bool cantBeParent = FALSE;
		switch (tt = nextToken(token))
		{
		case ID:
			/* These attributes can be used in any type of task (normal,
			 * container, milestone. */
			if (token == "task" && !cantBeParent)
			{
				if (!readTask(task))
					return FALSE;
				hasSubTasks = TRUE;
			}
			else if (token == "note")
			{
				if ((tt = nextToken(token)) == STRING)
					task->setNote(token);
				else
				{
					fatalError("String expected");
					return FALSE;
				}
			}
			else if READ_DATE("start", setStart)
			else if READ_DATE("minStart", setMinStart)
			else if READ_DATE("maxStart", setMaxStart)
			else if READ_DATE("minEnd", setMinEnd)
			else if READ_DATE("maxEnd", setMaxEnd)
			else if READ_DATE("actualStart", setActualStart)
			else if READ_DATE("actualEnd", setActualEnd)
			else if (token == "length" && !hasSubTasks)
			{
				double d;
				if (!readTimeFrame(task, d))
					return FALSE;
				task->setLength(d);
				cantBeParent = TRUE;
			}
			else if (token == "effort" && !hasSubTasks)
			{
				double d;
				if (!readTimeFrame(task, d))
					return FALSE;
				task->setEffort(d);
				cantBeParent = TRUE;
			}
			else if (token == "duration" && !hasSubTasks)
			{
				double d;
				if (!readTimeFrame(task, d))
					return FALSE;
				task->setDuration(d);
				cantBeParent = TRUE;
			}
			else if (token == "complete" && !hasSubTasks)
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
				cantBeParent = TRUE;
				task->setComplete(complete);
			}
			else if (token == "allocate" && !hasSubTasks)
			{
				if (!readAllocate(task))
					return FALSE;
				cantBeParent = TRUE;
			}
			else if (token == "depends" && !hasSubTasks)
			{
				cantBeParent = TRUE;
				for ( ; ; )
				{
					QString id;
					if ((tt = nextToken(id)) != ID &&
						tt != RELATIVE_ID)
					{
						fatalError("Task ID expected");
						return FALSE;
					}
					task->addDependency(id);
					if ((tt = nextToken(token)) != COMMA)
					{
						openFiles.last()->returnToken(tt, token);
						break;
					}
				}
			}
			else if (token == "flags")
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
						openFiles.last()->returnToken(tt, token);
						break;
					}
				}
			}
			else if (token == "priority" && !hasSubTasks)
			{
				int priority;
				if (!readPriority(priority))
					return FALSE;
				task->setPriority(priority);
				cantBeParent = TRUE;
				break;
			}
			else if (token == "account")
			{
				QString account;
				if (nextToken(account) != ID ||
					proj->getAccount(account) == 0)
				{
					fatalError("Account ID expected");
					return FALSE;
				}
				task->setAccount(proj->getAccount(account));
				break;
			}
			else
			{
				fatalError(QString("Illegal task attribute '")
						   + token + "'");
				return FALSE;
			}
			break;
		case RCBRACE:
			done = true;
			break;
		default:
			fatalError(QString("Syntax Error at '") + token + "'");
			return FALSE;
		}
	}

	if (task->getName().isEmpty())
	{
		fatalError(QString("No name specified for task ") + id + "!");
		return FALSE;
	}

	return TRUE;
}

bool
ProjectFile::readVacation(time_t& from, time_t& to, bool readName, QString* n)
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
		openFiles.last()->returnToken(tt, token);
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
ProjectFile::readResource()
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

	Resource* r = new Resource(proj, id, name, proj->getMinEffort(),
							   proj->getMaxEffort(), proj->getRate());
	TokenType tt;
	QString token;
	if ((tt = nextToken(token)) == LCBRACE)
	{
		// read optional attributes
		while ((tt = nextToken(token)) != RCBRACE)
		{
			if (tt != ID)
			{
				fatalError(QString("Unknown attribute '") + token + "'");
				return FALSE;
			}
			if (token == "minEffort")
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value exptected");
					return FALSE;
				}
				r->setMinEffort(token.toDouble());
			}
			else if (token == "maxEffort")
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value exptected");
					return FALSE;
				}
				r->setMaxEffort(token.toDouble());
			}
			else if (token == "efficiency")
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Read value expected");
					return FALSE;
				}
				r->setEfficiency(token.toDouble());
			}
			else if (token == "rate")
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value exptected");
					return FALSE;
				}
				r->setRate(token.toDouble());
			}
			else if (token == "kotrusId")
			{
				if (nextToken(token) != STRING)
				{
					fatalError("String expected");
					return FALSE;
				}
				r->setKotrusId(token);
			}
			else if (token == "vacation")
			{
				time_t from, to;
				if (!readVacation(from, to))
					return FALSE;
				r->addVacation(new Interval(from, to));
			}
			else if (token == "workingHours")
			{
				int dow;
				QPtrList<Interval>* l = new QPtrList<Interval>();
				if (!readWorkingHours(dow, l))
					return FALSE;
				
				r->setWorkingHours(dow, l);
			}
			else if (token == "flags")
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
						openFiles.last()->returnToken(tt, token);
						break;
					}
				}
			}
			else
			{
				fatalError(QString("Unknown attribute '") + token + "'");
				return FALSE;
			}
		}
	}
	else
		openFiles.last()->returnToken(tt, token);

	proj->addResource(r);

	return TRUE;
}

bool
ProjectFile::readAccount()
{
	// Syntax: 'account id "name" { ... }
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

	Account* a = new Account(id, name);
	TokenType tt;
	QString token;
	if ((tt = nextToken(token)) == LCBRACE)
	{
		// read optional attributes
		while ((tt = nextToken(token)) != RCBRACE)
		{
			if (tt != ID)
			{
				fatalError(QString("Unknown attribute '") + token + "'");
				return FALSE;
			}
			if (token == "balance")
			{
				if (nextToken(token) != REAL)
				{
					fatalError("Real value exptected");
					return FALSE;
				}
				a->setOpeningBalance(token.toDouble());
			}
			else if (token == "kotrusId")
			{
				if (nextToken(token) != STRING)
				{
					fatalError("String expected");
					return FALSE;
				}
				a->setKotrusId(token);
			}
		}
	}
	else
		openFiles.last()->returnToken(tt, token);

	proj->addAccount(a);

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
				fatalError(QString("Unknown attribute '") + token + "'");
				return FALSE;
			}
			if (token == "load")
			{
				if (nextToken(token) != INTEGER)
				{
					fatalError("Integer expected");
					return FALSE;
				}
				int load = token.toInt();
				if (load < 1 || load > 100)
				{
					fatalError("Value must be in the range 1 - 100");
					return FALSE;
				}
				a->setLoad(token.toInt());
			}
			else if (token == "persistent")
			{
				a->setPersistent(TRUE);
			}
			else if (token == "alternative")
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
					a->addAlternative(r);
				} while ((tt = nextToken(token)) == COMMA);
				openFiles.last()->returnToken(tt, token);
			}
		}
	}
	else
		openFiles.last()->returnToken(tt, token);
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
	if (unit == "min")
		value = val.toULong() * 60;
	else if (unit == "h")
		value = val.toULong() * (60 * 60);
	else if (unit == "d")
		value = val.toULong() * (60 * 60 * 24);
	else if (unit == "w")
		value = val.toULong() * (60 * 60 * 24 * 7);
	else if (unit == "m")
		value = val.toULong() * (60 * 60 * 24 * 30);
	else if (unit == "y")
		value = val.toULong() * (60 * 60 * 24 * 356);
	else
	{
		fatalError("Unit expected");
		return FALSE;
	}
	return TRUE;
}

bool
ProjectFile::readTimeFrame(Task* task, double& value)
{
	if (task->getEffort() > 0.0 ||
		task->getLength() > 0.0 ||
		task->getDuration() > 0.0)
	{
		fatalError(
			"You can specify either a length, a duration or an effort.");
		return FALSE;
	}
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
	if (unit == "min")
		value = val.toDouble() / (8 * 60);
	else if (unit == "h")
		value = val.toDouble() / 8;
	else if (unit == "d")
		value = val.toDouble();
	else if (unit == "w")
		value = val.toDouble() * 5;
	else if (unit == "m")
		value = val.toDouble() * 20;
	else if (unit == "y")
		value = val.toDouble() * 240;
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
	const char* days[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
	for (dayOfWeek = 0; dayOfWeek < 7; dayOfWeek++)
		if (days[dayOfWeek] == day)
			break;
	if (dayOfWeek == 7)
	{
		fatalError("Weekday expected");
		return FALSE;
	}
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
		l->append(new Interval(hhmm2time(start), hhmm2time(end)));
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

bool
ProjectFile::readXMLTaskReport()
{
   QString token;
   if (nextToken(token) != STRING)
   {
      fatalError("File name expected");
      return FALSE;
   }
   ReportXML *rep = new ReportXML( proj, token, proj->getStart(), proj->getEnd());
   proj->addXMLReport( rep );

   return( true );
}


bool
ProjectFile::readHTMLTaskReport()
{
	QString token;
	if (nextToken(token) != STRING)
	{
		fatalError("File name expected");
		return FALSE;
	}
	HTMLTaskReport* report = new HTMLTaskReport(proj, token, proj->getStart(),
												proj->getEnd());
	TokenType tt;
	if (nextToken(token) != LCBRACE)
	{
		openFiles.last()->returnToken(tt, token);
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
		if (token == "columns")
		{
			for ( ; ; )
			{
				QString col;
				if ((tt = nextToken(col)) != ID)
				{
					fatalError("Column ID expected");
					return FALSE;
				}
				report->addColumn(col);
				if ((tt = nextToken(token)) != COMMA)
				{
					openFiles.last()->returnToken(tt, token);
					break;
				}
			}
		}
		else if (token == "start")
		{
			if (nextToken(token) != DATE)
			{
				fatalError("Date expected");
				return FALSE;
			}
			report->setStart(date2time(token));
		}
		else if (token == "end")
		{
			if (nextToken(token) != DATE)
			{
				fatalError("Date expected");
				return FALSE;
			}
			report->setEnd(date2time(token));
		}
		else if (token == "showActual")
		{
			report->setShowActual(TRUE);
		}
		else if (token == "hidetask")
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setHideTask(et);
		}
		else if (token == "rolluptask")
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setRollUpTask(et);
		}
		else if (token == "sort")
		{
			nextToken(token);
			if (token == "tasktree")
				report->setSorting(TaskList::TaskTree);
			else if (token == "startup")
				report->setSorting(TaskList::StartUp);
			else if (token == "startdown")
				report->setSorting(TaskList::StartDown);
			else if (token == "endup")
				report->setSorting(TaskList::EndUp);
			else if (token == "enddown")
				report->setSorting(TaskList::EndDown);
			else if (token == "priorityup")
				report->setSorting(TaskList::PrioUp);
			else if (token == "prioritydown")
				report->setSorting(TaskList::PrioDown);
			else
			{
				fatalError("Sorting criteria expected");
				return FALSE;
			}
		}
		else
		{
			fatalError("Illegal attribute");
			return FALSE;
		}
	}
	proj->addHTMLTaskReport(report);
	return TRUE;
}

bool
ProjectFile::readHTMLResourceReport()
{
	QString token;

	if (nextToken(token) != STRING)
	{
		fatalError("File name expected");
		return FALSE;
	}

	HTMLResourceReport* report = new HTMLResourceReport(
		proj, token, proj->getStart(), proj->getEnd());

	TokenType tt;
	if (nextToken(token) != LCBRACE)
	{
		openFiles.last()->returnToken(tt, token);
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
		if (token == "columns")
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
				report->addColumn(col);
				if ((tt = nextToken(token)) != COMMA)
				{
					openFiles.last()->returnToken(tt, token);
					break;
				}
			}
		}
		else if (token == "start")
		{
			if (nextToken(token) != DATE)
			{
				fatalError("Date expected");
				return FALSE;
			}
			report->setStart(date2time(token));
		}
		else if (token == "end")
		{
			if (nextToken(token) != DATE)
			{
				fatalError("Date expected");
				return FALSE;
			}
			report->setEnd(date2time(token));
		}
		else if (token == "hidetask")
		{
			Operation* op;
			if ((op = readLogicalExpression()) == 0)
				return FALSE;
			ExpressionTree* et = new ExpressionTree(op);
			report->setHideTask(et);
		}
		else
		{
			fatalError("Illegal attribute");
			return FALSE;
		}
	}
	proj->addHTMLResourceReport(report);
	return TRUE;
}

Operation*
ProjectFile::readLogicalExpression()
{
	Operation* op;
	QString token;
	TokenType tt;

	if ((tt = nextToken(token)) == ID || tt == INTEGER)
	{
		if (tt == ID)
		{
			if (!proj->isAllowedFlag(token))
			{
				fatalError(QString("Flag ") + token + " is unknown.");
				return 0;
			}
			op = new Operation(token);
		}
		else
			op = new Operation(token.toLong());
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
	else if (tt == TILDE)
	{
		if ((op = readLogicalExpression()) == 0)
			return 0;
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
	
	return op;
}

time_t
ProjectFile::date2time(const QString& date)
{
	int y, m, d, hour, min;
	if (date.find(':') == -1)
	{
		sscanf(date, "%d-%d-%d", &y, &m, &d);
		hour = min = 0;
	}
	else
		sscanf(date, "%d-%d-%d-%d:%d", &y, &m, &d, &hour, &min);

	if (y < 1970)
	{
		fatalError("Year must be larger than 1969");
		y = 1970;
	}
	if (m < 1 || m > 12)
	{
		fatalError("Month must be between 1 and 12");
		m = 1;
	}
	if (d < 1 || d > 31)
	{
		fatalError("Day must be between 1 and 31");
		d = 1;
	}

	struct tm t = { 0, min, hour, d, m - 1, y - 1900, 0, 0, -1, 0, 0 };
	time_t localTime = mktime(&t);
#ifndef UTCTIME
	return localTime;
#else
	// The code is needed after we switch internal time handling to UTC.
	struct tm* tms = localtime(&localTime);
	time_t gmTime = localTime + timezone + (tms->tm_isdst == 1 ? -3600 : 0);
	return gmTime;
#endif
}

time_t
ProjectFile::hhmm2time(const QString& hhmm)
{
	int hour, min;
	sscanf(hhmm, "%d:%d", &hour, &min);
	return hour * 60 * 60 + min * 60;
}
