/*
 * FileToken.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: $
 */
#ifndef _FileToken_h_
#define _FileToken_h_

#include <memory>

#include <qtextstream.h>
#include <qptrlist.h>

#include "MacroTable.h"
#include "Token.h"
#include <memory>
/**
 * @short Common base of FileInfo and Tokenizer.
 * @author Andreas Scherer <andreas_hacker@freenet.de>
 */
class FileToken
{
public:
    FileToken( const QString& file, const QString& tp );
    FileToken();
    virtual ~FileToken() {}

    const QString& getFile() const { return m_file; }
    QString getPath() const;

    int getLine() const { return m_currLine; }

    virtual TokenType nextToken(QString& buf) = 0;
    void returnToken(TokenType t, const QString& buf);

    const QString& getTaskPrefix() const { return m_taskPrefix; }

    virtual void errorMessage(const QString& msg) = 0;

protected:
    virtual QChar getC(bool expandMacros = true ) = 0;

    virtual void  setLocation(const QString& df, int dl) = 0;
    virtual QString resolve(const QStringList* argList) = 0;
    virtual Macro* getMacro(const QString& name) const = 0;
    bool readMacroCall();

    bool getDateFragment(QString& token, QChar& c);
    bool readEnvironment();

    QString cleanupLine(const QString& line);

    // The name of the file.
    QString m_file;

    // The file handle of the file to read.
    FILE* m_fh;

    // The stream used to read the file.
    std::auto_ptr<QTextStream> m_f;

    // The number of the line currently being read.
    int m_currLine;

    /**
     * Macros have file scope. So we keep a stack of macros for each file that
     * we read.
     */
    QPtrList<Macro> m_macroStack;

    /**
     * A buffer for the part of the line that has been parsed already. This is
     * primarily used for error reporting.
     */
    QString m_lineBuf;

    /**
     * A buffer for characters that have been pushed back again. This
     * simplifies file parsing in some situations.
     */
    QValueList<QChar> m_ungetBuf;

    /**
     * Besides read in characters we can also push back a token. Contrary to
     * characters we can push back only 1 token. This is stored as type and
     * a string buffer.
     */
    TokenType m_tokenTypeBuf;
    QString m_tokenBuf;

    /**
     * Task trees of include files can not only be added at global scope but
     * also as sub-trees. This strings stores the prefix that has to be
     * specified at include times.
     */
    QString m_taskPrefix;
};

#endif
