/**************************************************************************
**
** Copyright (C) 2002-2003 Integrated Computer Solutions, Inc.
** All rights reserved.
**
** This file is part of the QicsTable Product.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.ics.com/qt/licenses/gpl/ for GPL licensing information.
**
** If you would like to use this software under a commericial license
** for the development of proprietary software, send email to sales@ics.com
** for details.
**
** Contact info@ics.com if any conditions of this licensing are
** not clear to you.
**
**************************************************************************/


#include <QicsDataModel.h>
#include <QicsDataItem.h>


#include <qstring.h>
#include <qfile.h>
#include <qstringlist.h>

QicsDataModel::QicsDataModel(int num_rows, int num_cols) :
	QObject(),
	myNumRows(num_rows),
	myNumColumns(num_cols),
	myEmitSignalsFlag(true)
{
    if (myNumColumns < 0)
	myNumColumns = 0;

    if (myNumRows < 0)
	myNumRows = 0;
}

QicsDataModel::~QicsDataModel()
{
}

QString QicsDataModel::itemString(int row, int col) const
{
    const QicsDataItem *itm = item(row, col);

    return (itm ? itm->string() : QString());
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// Adapted from code with the following copyright:

/* Copyright (C) 1999 Lucent Technologies */
/* Excerpted from 'The Practice of Programming' */
/* by Brian W. Kernighan and Rob Pike */

#include <qvaluevector.h>

class QicsCSVReader {
  public:
    QicsCSVReader(QTextStream &stream, const char separator) :
	myStream(stream), mySeparator(separator) { ; }

    int getLine(QString &str);
    QString getField(int n);
    int getNumFields() const { return myNumFields; }

  protected:
    QTextStream &myStream;
    QString myLine;
    QValueVector<QString> myFields;
    int myNumFields;
    char mySeparator;

    int split(void);
    int endOfLine(QChar c);
    int advPlain(const QString& line, QString& fld, int);
    int advQuoted(const QString& line, QString& fld, int);
};

// endofline: check for and consume \r, \n, \r\n, or EOF
int
QicsCSVReader::endOfLine(QChar c)
{
    int eol;

    eol = (c=='\r' || c=='\n');
    if (c == '\r')
    {
	if (!myStream.atEnd())
	{
	    myStream >> c;
	    if (c != '\n')
		myStream.device()->ungetch(c);	// read too far
	}
    }

    return eol;
}

// getline: get one line, grow as needed
int
QicsCSVReader::getLine(QString& str)
{
    QChar c;

    myLine = QString();

    while (!myStream.atEnd())
    {
	myStream >> c;

	if (!endOfLine(c))
	{
	    myLine.append(c);
	}
	else
	    break;
    }

    split();
    str = myLine;

    return (!myStream.atEnd());
}

// split: split line into fields
int
QicsCSVReader::split()
{
    QString fld;
    int i, j;

    myNumFields = 0;
    if (myLine.length() == 0)
	return 0;

    i = 0;
    do
    {
	if ((i < static_cast<int> (myLine.length())) && (myLine[i] == '"'))
	    j = advQuoted(myLine, fld, ++i);	// skip quote
	else
	    j = advPlain(myLine, fld, i);
	if (myNumFields >= static_cast<int> (myFields.size()))
	    myFields.push_back(fld);
	else
	    myFields[myNumFields] = fld;
	myNumFields++;
	i = j + 1;
    } while (j < static_cast<int> (myLine.length()));

    return myNumFields;
}

// advquoted: quoted field; return index of next separator
int
QicsCSVReader::advQuoted(const QString& str, QString& fld, int i)
{
    int j;

    fld = "";

    for (j = i; j < static_cast<int> (str.length()); j++)
    {
	if (str[j] == '"' && str[++j] != '"')
	{
	    int k = str.find(mySeparator, j);
	    if (k == -1)	// no separator found
		k = str.length();
	    for (k -= j; k-- > 0; )
		fld += str[j++];
	    break;
	}
	fld += str[j];
    }

    return j;
}

// advplain: unquoted field; return index of next separator
int
QicsCSVReader::advPlain(const QString& str, QString& fld, int i)
{
    int j;

    j = str.find(mySeparator, i); // look for separator
    if (j == -1)               // none found
	j = str.length();
    fld = str.mid(i, (j - i));

    return j;
}


// getfield: return n-th field
QString
QicsCSVReader::getField(int n)
{
    if (n < 0 || n >= myNumFields)
	return "";
    else
	return myFields[n];
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void
QicsDataModel::readASCII(QTextStream &stream, const char separator,
			 int start_row, int start_col, bool clear_model)
{
    bool old_emit = myEmitSignalsFlag;
    myEmitSignalsFlag = false;

    if (clear_model)
	clearModel();

    QicsCSVReader reader(stream, separator);

    QString line;
    int row = start_row;

    while (reader.getLine(line) != 0)
    {
	// do we need to add a row?
	if (row > lastRow())
	    addRows(row - lastRow());

	int nfields = reader.getNumFields();

	// do we need to add columns?
	if (((start_col + nfields - 1) > lastColumn()))
	    addColumns(start_col + nfields - 1 - lastColumn());

	int col = start_col;
	for (int i = 0; i < nfields; i++)
	{
	    QicsDataItem *itm = QicsDataItem::fromString(reader.getField(i));
	    setItem(row, col, *itm);
	    delete itm;

	    ++col;
	}

	++row;
    }

    myEmitSignalsFlag = old_emit;
    
    if (myEmitSignalsFlag) emit modelSizeChanged(numRows(), numColumns());
}

void
QicsDataModel::writeASCII(QTextStream &stream, const char separator,
			  int start_row, int start_col,
			  int nrows, int ncols)
{
    if (nrows == -1)
	nrows = QicsLAST_ROW;

    if (ncols == -1)
	ncols = QicsLAST_COLUMN;

    for (int i = start_row; i < (start_row + nrows - 1); ++i)
    {
	if (i > lastRow())
	    break;

	for (int j = start_col; j < (start_col + ncols - 1); ++j)
	{
	    if (j > lastColumn())
		break;

	    if (j != start_col)
		stream << separator;

	    const QicsDataItem *itm = item(i, j);
	    if (itm)
	    {
		stream << itm->string();
	    }
	}

	stream << "\n";
    }
}
