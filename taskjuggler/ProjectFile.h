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

#include "Project.h"
#include "Token.h"

class Project;

class FileInfo
{
public:
	FileInfo(const QString& file);
	~FileInfo() { }

	bool open();
	bool close();

	const QString& getFile() const { return file; }
	int getLine() const { return currLine; }

	TokenType nextToken(QString& buf);
	void returnToken(TokenType t, const QString& buf);

	void fatalError(const QString& msg) const;

private:
	QString file;
	FILE* f;
	int currLine;
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

private:
	ProjectFile() {};	// don't use

	bool readTask(Task* parent);
	bool readResource();
	bool readVacation();
	bool readAllocate(Task* t);
	bool readLength(Task* t);
	bool readEffort(Task* t);
	time_t date2time(const QString& date);

	QString masterFile;
	Project* proj;
	QList<FileInfo> openFiles;
};

#endif
