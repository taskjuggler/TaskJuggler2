/*
 * ProjectFile.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ProjectFile_h_
#define _ProjectFile_h_

#include <time.h>

#include <qptrlist.h>

#include "Token.h"
#include "MacroTable.h"
#include "FileInfo.h"

class Project;
class Task;
class Resource;
class Account;
class Shift;
class Booking;
class Interval;
class Operation;
class Report;
class ReportHtml;

/**
 * @short File Parser for project files.
 * @author Chris Schlaeger <cs@suse.de>
 */
class ProjectFile
{
public:
	/**
	 * A ProjectFile cannot exist without a project. So the constructor needs
	 * to know what Project object to fill, when it parses the project files.
	 */
	ProjectFile(Project* p);
	~ProjectFile() { }

	/**
	 * The top-level project files needs to be opened before the parser can be
	 * started.
	 * @param file The file name of the file to start with.
	 * @param parentPath The path of the file that included this file. This
	 * feature is for internal use only. It's not part of the public API. 
	 * @param taskPrefix The ID prefix of the parent task. This is needed when
	 * the tasks of the project file should be read as a sub-task of an
	 * already existing task.
	 */
	bool open(const QString& file, const QString& parentPath,
			  const QString& taskPrefix);
	/**
	 * Close the just read input file.
	 */
	bool close();
	
	/**
	 * Calling the parse function will start the processing of the opened
	 * project file. It will automatically read all include files as well. The
	 * collected data is stored into the Project object.
	 */
	bool parse();

	/*
	 * The rest of the public methods are for use by FileInfo and are not part
	 * of the library public interface.
	 */
	
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

	void errorMessage(const char* msg, ...);

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
    bool readDate(time_t& val, time_t correction);
	bool readTimeValue(ulong& value);
	bool readPercent(double& value);
	bool readWorkingHours(int& dayOfWeek, QPtrList<Interval>* l);
	bool readPriority(int& priority);
	bool checkReportInterval(ReportHtml* report);
	bool readHTMLReport(const QString& reportType);
	bool readHTMLAccountReport();
	bool readExportReport();
	bool readXMLReport();
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
	QPtrList<FileInfo> openFiles;
	QStringList includedFiles;
	MacroTable macros;
};

#endif
