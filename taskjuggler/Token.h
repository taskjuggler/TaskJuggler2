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

#ifndef _Token_h_
#define _Token_h_

#include <stdio.h>

#define EOM (EOF - 1)

typedef enum
{
	INVALID = 0, EndOfStatement, EndOfFile, MacroBody,
	ID, ABSOLUTE_ID, RELATIVE_ID,
	RCBRACE, LCBRACE, RBRACE, LBRACE, COMMA, TILDE, MINUS, AND, OR,
	DATE, HOUR, INTEGER, REAL, STRING
} TokenType;

class Token
{
public:

   Token(TokenType /* tt */, const QString& /* tx */) {}
	~Token() { }

private:
	Token() { } // don't use
	TokenType type;
	QString text;
} ;

#endif




