/*
 * ProjectFile.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
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
class ReportHtml;

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
	QString getPath() const;

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
	void setDebugLevel(int l) { debugLevel = l; }
	TokenType nextToken(QString& token);
	void returnToken(TokenType t, const QString& buf)
	{
		if (!openFiles.isEmpty())
			openFiles.last()->returnToken(t, buf);
	}

	const QString& getFile()
   	{
		if (openFiles.isEmpty())
			return QString::null;
	   	return openFiles.last()->getFile(); 
	}
	int getLine()
   	{
		if (openFiles.isEmpty())
			return -1;
	   	return openFiles.last()->getLine(); 
	}

	bool moreFiles() { return !openFiles.isEmpty(); }

	void fatalError(const QString& msg);

	MacroTable& getMacros() { return macros; }

private:
	ProjectFile() {};	// don't use

	bool readInclude();
	bool readTask(Task* parent);
	bool readTaskSupplement();
	bool readTaskBody(Task* task);
	bool readResource(Resource* parent);
	bool readResourceSupplement();
	bool readResourceBody(Resource* r);
	bool readVacation(time_t& from, time_t& to, bool readName = FALSE,
					  QString* = 0);
	bool readAccount(Account* parent);
	bool readShift(Shift* parent);
	Booking* readBooking();
	bool readCredit(Account* a);
	bool readAllocate(Task* t);
	bool readPlanTimeFrame(Task* t, double& d);
	bool readTimeValue(ulong& value);
	bool readWorkingHours(int& dayOfWeek, QPtrList<Interval>* l);
	bool readPriority(int& priority);
	bool readHTMLReport(const QString& reportType);
	bool readHTMLAccountReport();
	bool readExportReport();
	bool readXMLTaskReport();
	bool readHtmlUrl(ReportHtml* report);
#ifdef HAVE_KDE
	bool readICalTaskReport();
#endif
	Operation* readLogicalExpression(int precedence = 0);
	Operation* readFunctionCall(const QString& name);
	bool readSorting(Report* report, int which);
	time_t date2time(const QString& date);
	time_t hhmm2time(const QString& hhmm);

	QString masterFile;
	Project* proj;
	QList<FileInfo> openFiles;
	QStringList includedFiles;
	MacroTable macros;
	int debugLevel;
};

#endif
