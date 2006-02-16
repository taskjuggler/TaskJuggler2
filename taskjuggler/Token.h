/*
 * ProjectFile.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006
 * Chris Schlaeger <cs@kde.org>
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

#include <qstring.h>

#define EOFile 0xFFFF
#define EOMacro 0xFFFE

typedef enum
{
    INVALID = 0, EndOfStatement, EndOfFile, MacroBody,
    ID, ABSOLUTE_ID, RELATIVE_ID, PERCENT,
    RBRACE, LBRACE, RBRACKET, LBRACKET, COMMA, COLON, TILDE,
    QUESTIONMARK, PLUS, MINUS, AND, OR,
    GREATER, SMALLER, EQUAL, GREATEROREQUAL, SMALLEROREQUAL,
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




