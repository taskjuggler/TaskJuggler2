/*
 * ProjectFile.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ProjectFile_h_
#define _ProjectFile_h_

#include <stdio.h>
#include <time.h>

#include <qstring.h>
#include <qvaluelist.h>

#include "Project.h"
#include "Token.h"
#include "MacroTable.h"

class ProjectFile;
class Project;
class Operation;

class FileInfo
{
public:
	FileInfo(ProjectFile* p, const QString& file);
	~FileInfo() { }

	bool open();
	bool close();

	int getC(bool expandMacros = TRUE);
	void ungetC(int c);
	void expandMarco(QString& c);

	const QString& getFile() const { return file; }
	int getLine() const { return currLine; }

	TokenType nextToken(QString& buf);
	void returnToken(TokenType t, const QString& buf);

	bool readMacroCall();

	void fatalError(const QString& msg);

private:
	bool getDateFragment(QString& token, int& c);

	ProjectFile* pf;
	QString file;
	FILE* f;
	int currLine;
	QPtrList<Macro> macroStack;
	QString lineBuf;
	QValueList<int> ungetBuf;
	TokenType tokenTypeBuf;
	QString tokenBuf;
};

class ProjectFile
{
public:
	ProjectFile(Project* p);
	~ProjectFile() { }

	bool open(const QString& file);
	bool close();
	bool parse();
	TokenType nextToken(QString& token);
	void returnToken(TokenType t, const QString& buf)
	{
		openFiles.last()->returnToken(t, buf);
	}

	const QString& getFile() { return openFiles.last()->getFile(); }
	int getLine() { return openFiles.last()->getLine(); }

	void fatalError(const QString& msg);

	MacroTable& getMacros() { return macros; }

private:
	ProjectFile() {};	// don't use

	bool readTask(Task* parent);
	bool readResource(Resource* parent);
	bool readVacation(time_t& from, time_t& to, bool readName = FALSE,
					  QString* = 0);
	bool readAccount();
	bool readAllocate(Task* t);
	bool readPlanTimeFrame(Task* t, double& d);
	bool readTimeValue(ulong& value);
	bool readWorkingHours(int& dayOfWeek, QPtrList<Interval>* l);
	bool readPriority(int& priority);
	bool readHTMLReport(const QString& reportType);
	bool readXMLTaskReport();
	Operation* readLogicalExpression();
	bool readTaskSorting(Report* report);
	time_t date2time(const QString& date);
	time_t hhmm2time(const QString& hhmm);

	QString masterFile;
	Project* proj;
	QList<FileInfo> openFiles;
	MacroTable macros;
};

#endif
