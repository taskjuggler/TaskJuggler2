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


#include <QicsDataItem.h>
#include <qstring.h>
#include <qptrlist.h>
#include <assert.h>

////////////////////////////////////////////////////////////////////////

// A helper class to store data item class info

class QicsDataItemInfo {
public:
    QicsDataItemInfo(const QString &type, QicsDataItemParser parser,
		     QicsDataItemDecoder decoder)
	{ myType = type; myParser = parser; myDecoder = decoder; }
    QString type(void) const
	{ return myType; }
    QicsDataItemParser parser(void) const
	{ return myParser; }
    QicsDataItemDecoder decoder(void) const
	{ return myDecoder; }
private:
    QString myType;
    QicsDataItemParser myParser;
    QicsDataItemDecoder myDecoder;
};

static QPtrList<QicsDataItemInfo> myRegisteredTypes;


////////////////////////////////////////////////////////////////////////
/* default implementation */

QicsDataItem *
QicsDataItem::fromString(const QString &str)
{
    QicsDataItem *val = 0;

    // First check any of our registered types

    unsigned int i;
    for (i = 0; i < myRegisteredTypes.count(); ++i)
    {
	val = ((myRegisteredTypes.at(i))->parser())(str);
	if (val)
	    break;
    }
    if (val)
	return val;

    // The registered types couldn't handle it, so let's try the built-in types
    if (!(val = QicsDataInt::fromString(str)))
	if (!(val = QicsDataFloat::fromString(str)))
	    val = new QicsDataString(str);

    return val;
}

QicsDataItem *
QicsDataItem::decode(QDataStream &ds)
{
    QicsDataItem *val = 0;

    // get the type name of the next item
    QString type;
    ds >> type;

    // Check the built-in types
    if (type == QicsDataInt::typeName())
	return QicsDataInt::decode(ds);
    if (type == QicsDataLong::typeName())
	return QicsDataLong::decode(ds);
    if (type == QicsDataFloat::typeName())
	return QicsDataFloat::decode(ds);
    if (type == QicsDataDouble::typeName())
	return QicsDataDouble::decode(ds);
    if (type == QicsDataString::typeName())
	return QicsDataString::decode(ds);
    if (type == QicsDataDate::typeName())
	return QicsDataDate::decode(ds);
    if (type == QicsDataTime::typeName())
	return QicsDataTime::decode(ds);
    if (type == QicsDataDateTime::typeName())
	return QicsDataDateTime::decode(ds);

    // Now check any of our registered types
    unsigned int i;
    for (i = 0; i < myRegisteredTypes.count(); ++i)
    {
	if (type == (myRegisteredTypes.at(i))->type())
	{
	    val = ((myRegisteredTypes.at(i))->decoder())(ds);
	    break;
	}
    }

    return val;
}

void
QicsDataItem::registerType(const QString type_name,
			   QicsDataItemParser parser,
			   QicsDataItemDecoder decoder)
{
    QicsDataItemInfo *info = new QicsDataItemInfo(type_name, parser, decoder);
    myRegisteredTypes.append(info);
}

int
QicsDataItem::compareTo(const QicsDataItem &) const
{
    qDebug("Missing compareTo function for data type %d\n", this->type());
    return 0;
}

////////////////////////////////////////////////////////////////////////

QicsDataInt::QicsDataInt(const QicsDataInt &di) :
    QicsDataItem()
{
    myData = di.myData;
}

QicsDataItem *
QicsDataInt::fromString(const QString &str)
{
    bool ok;
    int val = str.toInt(&ok);

    if (ok)
	return new QicsDataInt(val);
    else
	return 0;
}

int
QicsDataInt::compareTo(const QicsDataItem &x) const
{
	assert(this->type() == x.type());

	const QicsDataInt *v = static_cast<const QicsDataInt *> (&x);
	return myData - v->myData;
}

////////////////////////////////////////////////////////////////////////

QicsDataLong::QicsDataLong(const QicsDataLong &dl) :
    QicsDataItem()
{
    myData = dl.myData;
}

QicsDataItem *
QicsDataLong::fromString(const QString &str)
{
    bool ok;
    long val = str.toLong(&ok);

    if (ok)
	return new QicsDataLong(val);
    else
	return 0;
}

int
QicsDataLong::compareTo(const QicsDataItem &x) const
{
	assert(this->type() == x.type());

	const QicsDataLong *v = static_cast<const QicsDataLong *> (&x);
	return myData - v->myData;
}

////////////////////////////////////////////////////////////////////////

QicsDataFloat::QicsDataFloat(const QicsDataFloat &df) :
    QicsDataItem()
{
    myData = df.myData;
}

QicsDataItem *
QicsDataFloat::fromString(const QString &str)
{
    bool ok;
    float val = str.toFloat(&ok);

    if (ok)
	return new QicsDataFloat(val);
    else
	return 0;
}

int
QicsDataFloat::compareTo(const QicsDataItem &x) const
{
	assert(this->type() == x.type());

	const QicsDataFloat *v = static_cast<const QicsDataFloat *> (&x);
	if(myData < v->myData) return -1;
	if(myData == v->myData) return 0;
	return 1;
}

////////////////////////////////////////////////////////////////////////

QicsDataDouble::QicsDataDouble(const QicsDataDouble &dd) :
    QicsDataItem()
{
    myData = dd.myData;
}

QicsDataItem *
QicsDataDouble::fromString(const QString &str)
{
    bool ok;
    double val = str.toDouble(&ok);

    if (ok)
	return new QicsDataDouble(val);
    else
	return 0;
}

int
QicsDataDouble::compareTo(const QicsDataItem &x) const
{
	assert(this->type() == x.type());

	const QicsDataDouble *v = static_cast<const QicsDataDouble *> (&x);
	if(myData < v->myData) return -1;
	if(myData == v->myData) return 0;
	return 1;
}

////////////////////////////////////////////////////////////////////////

QicsDataString::QicsDataString(const QicsDataString &ds) :
    QicsDataItem()
{
    myData = ds.myData;
}

QicsDataString::QicsDataString(const QString &qs) 
{ 
    myData = qs;
}

QicsDataString & 
QicsDataString::operator=(const QicsDataString& s)
{
    if (this != &s)
    {
        myData = s.string(); 
    }
    return *this;
}

int
QicsDataString::compareTo(const QicsDataItem &x) const
{
	assert(this->type() == x.type());

	const QicsDataString *v = static_cast<const QicsDataString *> (&x);
	return myData.compare(v->myData);
}


////////////////////////////////////////////////////////////////////////

QicsDataDate::QicsDataDate(Qt::DateFormat format) :
    QicsDataItem(),
    myDefaultFormat(format)
{
}

QicsDataDate::QicsDataDate(const QicsDataDate &dd) :
    QicsDataItem(),
    myData(dd.myData),
    myDefaultFormat(dd.myDefaultFormat)
{
}

QicsDataDate::QicsDataDate(const QDate &date, Qt::DateFormat format) :
    QicsDataItem(),
    myData(date),
    myDefaultFormat(format)
{
}

QicsDataDate::QicsDataDate(int y, int m, int d, Qt::DateFormat format) :
    QicsDataItem(),
    myDefaultFormat(format)
{
    myData = QDate(y, m, d);
}

QicsDataDate &
QicsDataDate::operator=(const QDate &date)
{
    setData(date);

    return *this;
}
        
bool
QicsDataDate::setString(const QString &qs)
{
    QDate date = QDate::fromString(qs, myDefaultFormat);

    if (date.isValid())
    {
	this->setData(date);
	return true;
    }
    else
	return false;
}

QString
QicsDataDate::format(const char *fmt_string) const
{
    return (myData.toString(fmt_string));
}

QicsDataItem *
QicsDataDate::fromString(const QString &str)
{
    if (str.isEmpty())
	return 0;

    QDate val = QDate::fromString(str);

    if (val.isValid())
	return new QicsDataDate(val);
    else
	return 0;
}

int
QicsDataDate::compareTo(const QicsDataItem &x) const
{
	assert(this->type() == x.type());

	const QicsDataDate &v = static_cast<const QicsDataDate &> (x);

	if (data() == v.data()) return 0;
	if (data() < v.data()) return -1;
	return 1;
}

////////////////////////////////////////////////////////////////////////

QicsDataTime::QicsDataTime(Qt::DateFormat format) :
    QicsDataItem(),
    myDefaultFormat(format)
{
}

QicsDataTime::QicsDataTime(const QicsDataTime &dt) :
    QicsDataItem(),
    myData(dt.myData),
    myDefaultFormat(dt.myDefaultFormat)
{
}

QicsDataTime::QicsDataTime(const QTime &time, Qt::DateFormat format) :
    QicsDataItem(),
    myData(time),
    myDefaultFormat(format)
{
}

QicsDataTime::QicsDataTime(int h, int m, int s, int msec,
			   Qt::DateFormat format) :
    QicsDataItem(),
    myDefaultFormat(format)
{
    myData = QTime(h, m, s, msec);
}

QicsDataTime &
QicsDataTime::operator=(const QTime &time)
{
    setData(time);

    return *this;
}
        
bool
QicsDataTime::setString(const QString &qs)
{
    QTime time = QTime::fromString(qs, myDefaultFormat);

    if (time.isValid())
    {
	this->setData(time);
	return true;
    }
    else
	return false;
}

void
QicsDataTime::setData(const QTime &time)
{
    myData = time;
}

QString
QicsDataTime::format(const char *fmt_string) const
{
    return (myData.toString(fmt_string));
}

QicsDataItem *
QicsDataTime::fromString(const QString &str)
{
    if (str.isEmpty())
	return 0;

    QTime val = QTime::fromString(str);

    if (val.isValid())
	return new QicsDataTime(val);
    else
	return 0;
}

int
QicsDataTime::compareTo(const QicsDataItem &x) const
{
	assert(this->type() == x.type());

	const QicsDataTime &v = static_cast<const QicsDataTime &> (x);

	if (data() == v.data()) return 0;
	if (data() < v.data()) return -1;
	return 1;
}


////////////////////////////////////////////////////////////////////////

QicsDataDateTime::QicsDataDateTime(Qt::DateFormat format) :
    QicsDataItem(),
    myDefaultFormat(format)
{
}

QicsDataDateTime::QicsDataDateTime(const QicsDataDateTime &ddt) :
    QicsDataItem(),
    myData(ddt.myData),
    myDefaultFormat(ddt.myDefaultFormat)
{
}

QicsDataDateTime::QicsDataDateTime(const QDateTime &dt, Qt::DateFormat format) :
    QicsDataItem(),
    myData(dt),
    myDefaultFormat(format)
{
}

QicsDataDateTime::QicsDataDateTime(const QDate &date, const QTime &time,
				   Qt::DateFormat format) :
    QicsDataItem(),
    myDefaultFormat(format)
{
    myData = QDateTime(date, time);
}

QicsDataDateTime &
QicsDataDateTime::operator=(const QDateTime &dt)
{
    setData(dt);

    return *this;
}
        
bool
QicsDataDateTime::setString(const QString &qs)
{
    QDateTime dt = QDateTime::fromString(qs, myDefaultFormat);

    if (dt.isValid())
    {
	setData(dt);
	return true;
    }
    else
	return false;
}

void
QicsDataDateTime::setData(const QDateTime &dt)
{
    myData = dt;
}

QString
QicsDataDateTime::format(const char *fmt_string) const
{
    return (myData.toString(fmt_string));
}

QicsDataItem *
QicsDataDateTime::fromString(const QString &str)
{
    if (str.isEmpty())
	return 0;

    QDateTime val = QDateTime::fromString(str);

    if (val.isValid())
	return new QicsDataDateTime(val);
    else
	return 0;
}

int
QicsDataDateTime::compareTo(const QicsDataItem &x) const
{
	assert(this->type() == x.type());

	const QicsDataDateTime &v = static_cast<const QicsDataDateTime &> (x);

	if (data() == v.data()) return 0;
	if (data() < v.data()) return -1;
	return 1;
}
