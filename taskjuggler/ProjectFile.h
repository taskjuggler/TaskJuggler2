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

	void fatalError(const QString& msg) const;

private:
	ProjectFile* pf;
	QString file;
	FILE* f;
	int currLine;
	int macroLevel;
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

	const QString& getFile() { return openFiles.last()->getFile(); }
	int getLine() { return openFiles.last()->getLine(); }

	void fatalError(const QString& msg);

	MacroTable& getMacros() { return macros; }

private:
	ProjectFile() {};	// don't use

	bool readTask(Task* parent);
	bool readResource();
	bool readVacation();
	bool readAccount();
	bool readAllocate(Task* t);
	bool readLength(Task* t);
	bool readEffort(Task* t);
	bool readPriority(int& priority);
	time_t date2time(const QString& date);

	QString masterFile;
	Project* proj;
	QList<FileInfo> openFiles;
	MacroTable macros;
};

#endif
