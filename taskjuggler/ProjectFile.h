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
#include <stdarg.h>

#include <qstring.h>
#include <qvaluelist.h>

#include "Project.h"
#include "Token.h"
#include "MacroTable.h"

class QTextStream;
class ProjectFile;
class Project;
class Operation;
class ReportHtml;

class FileInfo
{
public:
	FileInfo(ProjectFile* p, const QString& file, const QString& tp);
	~FileInfo() { }

	bool open();
	bool close();

	QChar getC(bool expandMacros = TRUE);
	void ungetC(QChar c);
	void expandMarco(QString& c);

	const QString& getFile() const { return file; }
	QString getPath() const;

	int getLine() const { return currLine; }

	TokenType nextToken(QString& buf);
	void returnToken(TokenType t, const QString& buf);

	bool readMacroCall();

	const QString& getTaskPrefix() { return taskPrefix; }

	void fatalErrorVA(const char* msg, va_list ap);
	void fatalError(const char* msg, ...);

private:
	bool getDateFragment(QString& token, QChar& c);

	/**
	 * A pointer to the ProjectFile class that stores all read-in
	 * data.
	 */
	ProjectFile* pf;
	
	// The name of the file.
	QString file;

	// The file handle of the file to read.
	FILE* fh;

	// The stream used to read the file.
	QTextStream* f;

	// The number of the line currently being read.
	int currLine;

	/**
	 * Macros have file scope. So we keep a stack of macros for each file that
	 * we read.
	 */
	QPtrList<Macro> macroStack;

	/**
	 * A buffer for the part of the line that has been parsed already. This is
	 * primarily used for error reporting.
	 */
	QString lineBuf;

	/**
	 * A buffer for characters that have been pushed back again. This
	 * simplifies file parsing in some situations.
	 */
	QValueList<QChar> ungetBuf;

	/**
     * Besides read in characters we can also push back a token. Contrary to
	 * characters we can push back only 1 token. This is stored as type and
	 * a string buffer.
	 */	 
	TokenType tokenTypeBuf;
	QString tokenBuf;

	/**
	 * Task trees of include files can not only be added at global scope but
	 * also as sub-trees. This strings stores the prefix that has to be
	 * specified at include times.
	 */
	QString taskPrefix;
};

class ProjectFile
{
public:
	ProjectFile(Project* p);
	~ProjectFile() { }

	bool open(const QString& file, const QString& parentPath,
			  const QString& taskPrefix);
	bool close();
	bool parse();
	void setDebugLevel(int l) { debugLevel = l; }
	void setDebugMode(int m) { debugMode = m; }
	
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

	const QString& getTaskPrefix();

	void fatalError(const char* msg, ...);

	MacroTable& getMacros() { return macros; }

private:
	ProjectFile() {};	// don't use

	bool readProject();
	bool readInclude();
	bool readTask(Task* parent);
	bool readTaskSupplement(QString prefix);
	bool readTaskBody(Task* task);
	bool readResource(Resource* parent);
	bool readResourceSupplement();
	bool readResourceBody(Resource* r);
	bool readVacation(time_t& from, time_t& to, bool readName = FALSE,
					  QString* = 0);
	bool readAccount(Account* parent);
	bool readShift(Shift* parent);
	Shift* readShiftSelection(time_t& from, time_t& to);
	Booking* readBooking();
	bool readCredit(Account* a);
	bool readAllocate(Task* t);
	bool readPlanTimeFrame(double& d, bool workingDays);
	bool readTimeValue(ulong& value);
	bool readPercent(double& value);
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
	int hhmm2time(const QString& hhmm);

	QString masterFile;
	Project* proj;
	QList<FileInfo> openFiles;
	QStringList includedFiles;
	MacroTable macros;
	static int debugLevel;
	static int debugMode;
};

#endif
