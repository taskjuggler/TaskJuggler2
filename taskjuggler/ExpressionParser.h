/*
 * ExpressionParser.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ExpressionParser_h_
#define _ExpressionParser_h_

#include <qstring.h>

class Project;
class Operation;
class Tokenizer;
class ExpressionTree;

/**
 * @short This class can be used to generate an @ref ExpressionTree from a
 * file stream or QString.
 * @author Chris Schlaeger <cs@kde.org>
 */
class ExpressionParser
{
public:
    ExpressionParser();
    ~ExpressionParser();

    Operation* parse(const QString& text, const Project* proj);

private:
    Operation* parseLogicalExpression(int precedence, const Project* proj);
    Operation* parseFunctionCall(const QString& token, const Project* proj);

    void errorMessage(const char* msg, ...);

    Tokenizer* tokenizer;
};

#endif

