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

#ifndef _Token_h_
#define _Token_h_

#include <stdio.h>

#define EOM (EOF - 1)

typedef enum
{
	INVALID = 0, EndOfStatement, EndOfFile, MacroBody,
	ID, RELATIVE_ID,
	RBRACKET, LBRACKET, COMMA, MINUS,
	DATE, INTEGER, REAL, STRING
} TokenType;

class Token
{
public:

	Token(TokenType tt, const QString& tx) {}
	~Token() { }

private:
	Token() { } // don't use
	TokenType type;
	QString text;
} ;

#endif




