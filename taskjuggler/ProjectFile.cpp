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
	macroLevel = 0;
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
			macroLevel--;
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
	if (c == EOF - 1)
		macroLevel++;
	lineBuf = lineBuf.left(lineBuf.length() - 1);
	ungetBuf.append(c);
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
			if (macroLevel > 0)
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
				// this must be a ISO date yyyy-mm-dd
				token += c;
				c = getC();
				// c must be a digit
				if (!isdigit(c))
				{
					fatalError("Corrupted date");
					return EndOfFile;
				}
				token += c;
				// read rest of month
				while ((c = getC()) != EOF && isdigit(c))
					token += c;
				if (c != '-')
				{
					fatalError("Corrupted date");
					return EndOfFile;
				}
				token += c;
				c = getC();
				// c must be a digit
				if (!isdigit(c))
				{
					fatalError("Corrupted date");
					return EndOfFile;
				}
				token += c;
				// read rest of day
				while ((c = getC()) != EOF && isdigit(c))
					token += c;
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
			if (c == '{')
				return LBRACKET;
			else if (c == '}')
				return RBRACKET;
			else if (c == ',')
				return COMMA;
			else if (c == '-')
				return MINUS;
			else
			{
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
	if (tt != RBRACKET)
	{
		fatalError("'}' expected");
		return FALSE;
	}
	QString macro = pf->getMacros().expand(id);

	// push string list to global argument stack
	pf->getMacros().pushArguments(sl);
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
		cerr << "Internal Error: Token buffer overflow!" << endl;
		return;
	}
	tokenTypeBuf = tt;
	tokenBuf = buf;
}

void
FileInfo::fatalError(const QString& msg) const
{
	cerr << file << ":" << currLine << ":" << msg << endl;
	cerr << lineBuf << endl;
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
		cerr << "Cannot open " << file << "!" << endl;
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
			else if (token == "start")
			{
				if ((tt = nextToken(token)) != DATE)
				{
					fatalError("Date expected");
					return FALSE;
				}
				proj->setStart(date2time(token));
				break;
			}
			else if (token == "end")
			{
				if ((tt = nextToken(token)) != DATE)
				{
					fatalError("Date expected");
					return FALSE;
				}
				proj->setEnd(date2time(token));
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
				if (!readVacation())
					return FALSE;
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
			{				if (nextToken(token) != REAL)
				{
					fatalError("Real value exptected");
					return FALSE;
				}
				proj->setRate(token.toDouble());
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
				if (nextToken(token) != MacroBody)
				{
					fatalError("Macro body expected");
					return FALSE;
				}
				Macro* macro = new Macro(id, token);
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
			else if (token == "htmlTaskReport")
			{
				if (nextToken(token) != STRING)
				{
					fatalError("File name expected");
					return FALSE;
				}
				proj->setHtmlTaskReport(token);
				for ( ; ; )
				{
					QString col;
					if ((tt = nextToken(col)) != ID)
					{
						fatalError("Column ID expected");
						return FALSE;
					}
					proj->addHtmlTaskReportColumn(col);
					if ((tt = nextToken(token)) != COMMA)
					{
						openFiles.last()->returnToken(tt, token);
						break;
					}
				}
				break;
			}
			else if (token == "htmlResourceReport")
			{
				if (nextToken(token) != STRING)
				{
					fatalError("File name expected");
					return FALSE;
				}
				proj->setHtmlResourceReport(token);
				if (nextToken(token) != DATE)
				{
					fatalError("Start date expected");
					return FALSE;
				}
				proj->setHtmlResourceReportStart(date2time(token));
				if (nextToken(token) != DATE)
				{
					fatalError("End date expected");
					return FALSE;
				}
				proj->setHtmlResourceReportEnd(date2time(token));
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

	if ((tt = nextToken(token)) != LBRACKET)
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
				if (!readLength(task))
					return FALSE;
				cantBeParent = TRUE;
			}
			else if (token == "effort" && !hasSubTasks)
			{
				if (!readEffort(task))
					return FALSE;
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
						fatalError("flag unknown");
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
		case RBRACKET:
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
ProjectFile::readVacation()
{
	TokenType tt;
	QString name;
	if ((tt = nextToken(name)) != STRING)
	{
		fatalError("String expected");
		return FALSE;
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
		// vacation 2001-11-28
		openFiles.last()->returnToken(tt, token);
		proj->addVacation(name, date2time(start),
						  date2time(start) + (60 * 60 * 24 - 1));
	}
	else
	{
		// vacation 2001-11-28 - 2001-11-30
		QString end;
		if ((tt = nextToken(end)) != DATE)
		{
			fatalError("Date expected");
			return FALSE;
		}
		if (date2time(start) >= date2time(end))
		{
			fatalError("Vacation must start before end");
			return FALSE;
		}
		proj->addVacation(name, date2time(start),
						  date2time(end) + (60 * 60 * 24 - 1));
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

	Resource* r = new Resource(id, name, proj->getMinEffort(),
							   proj->getMaxEffort(), proj->getRate());
	TokenType tt;
	QString token;
	if ((tt = nextToken(token)) == LBRACKET)
	{
		// read optional attributes
		while ((tt = nextToken(token)) != RBRACKET)
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
	if ((tt = nextToken(token)) == LBRACKET)
	{
		// read optional attributes
		while ((tt = nextToken(token)) != RBRACKET)
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
	if ((tt = nextToken(token)) == LBRACKET)
	{
		while ((tt = nextToken(token)) != RBRACKET)
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
ProjectFile::readLength(Task* task)
{
	if (task->getEffort() > 0.0)
	{
		fatalError("You can specify either a length or an effort.");
		return FALSE;
	}
	QString len;
	if (nextToken(len) != INTEGER)
	{
		fatalError("Integer expected");
		return FALSE;
	}
	QString unit;
	if (nextToken(unit) != ID)
	{
		fatalError("Unit expected");
		return FALSE;
	}
	if (unit == "d")
		task->setLength(len.toInt());
	else if (unit == "w")
		task->setLength(len.toInt() * 5);
	else if (unit == "m")
		task->setLength(len.toInt() * 20);
	else
	{
		fatalError("Unit expected");
		return FALSE;
	}

	return TRUE;
}

bool
ProjectFile::readEffort(Task* task)
{
	if (task->getLength() > 0)
	{
		fatalError("You can specify either a length or an effort.");
		return FALSE;
	}
	QString effort;
	TokenType tt;
	if ((tt = nextToken(effort)) != INTEGER &&
		tt != REAL)
	{
		fatalError("Real number expected");
		return FALSE;
	}
	QString unit;
	if (nextToken(unit) != ID)
	{
		fatalError("Unit expected");
		return FALSE;
	}
	if (unit == "md")
		task->setEffort(effort.toDouble());
	else if (unit == "mw")
		task->setEffort(effort.toDouble() * 5);
	else if (unit == "mm")
		task->setEffort(effort.toDouble() * 20);
	else
	{
		fatalError("Unit expected");
		return FALSE;
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
	if (priority < 1 || priority > 100)
	{
		fatalError("Priority value must be between 1 and 100");
		return FALSE;
	}
	return TRUE;
}

time_t
ProjectFile::date2time(const QString& date)
{
	int y, m, d;
	sscanf(date, "%d-%d-%d", &y, &m, &d);
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

	struct tm t = { 0, 0, 0, d, m - 1, y - 1900, 0, 0, 0 };
	return mktime(&t);
}
