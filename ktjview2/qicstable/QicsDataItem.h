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


#ifndef _QicsDataItem_H
#define _QicsDataItem_H

#include <QicsNamespace.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qdatastream.h>

// limited introspection of data items
typedef unsigned int QicsDataItemType;
const QicsDataItemType QicsDataItem_UserDefined = 0;
const QicsDataItemType QicsDataItem_Int		= 1;
const QicsDataItemType QicsDataItem_Long	= 2;
const QicsDataItemType QicsDataItem_Float	= 3;
const QicsDataItemType QicsDataItem_Double	= 4;
const QicsDataItemType QicsDataItem_String	= 5;
const QicsDataItemType QicsDataItem_Date	= 6;
const QicsDataItemType QicsDataItem_Time	= 7;
const QicsDataItemType QicsDataItem_DateTime	= 8;

////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////

// This is used in QicsDataItem::registerType() and must be declared
// first.

class QicsDataItem;

typedef QicsDataItem * (*QicsDataItemParser)(const QString &);

typedef QicsDataItem * (*QicsDataItemDecoder)(QDataStream &);

////////////////////////////////////////////////////////////////////////


class QicsDataItem: public Qics {
    
public:

    
    virtual QicsDataItem *create(void) const = 0;

    
    virtual QicsDataItem *clone(void) const = 0;

    virtual ~QicsDataItem() {}
    
    virtual bool setString(const QString &qs) = 0;

    
    virtual const QString string(void) const = 0;
        
    
    virtual QicsDataItemType type(void) const = 0;

    
    virtual QString typeString(void) const = 0;

    
    inline virtual QString format(const char *) const
	{ return string(); }

    
    virtual int compareTo(const QicsDataItem &x) const;

    
    virtual void encode(QDataStream &ds) const = 0;

    
    static QicsDataItem *fromString(const QString &str);

    
    static QicsDataItem *decode(QDataStream &ds);

    
    static void registerType(const QString type_name,
			     QicsDataItemParser parser,
			     QicsDataItemDecoder decoder);

};

typedef int (*DataItemComparator)(const QicsDataItem *, const QicsDataItem *); 

////////////////////////////////////////////////////////////////////////


class QicsDataInt: public QicsDataItem {
public:
            
    
    QicsDataInt() { myData = 0; }

    
    QicsDataInt(int i) { myData = i;}

    
    QicsDataInt(const QicsDataInt &di);

    inline QicsDataItem *create(void) const { return new QicsDataInt(); }
    inline QicsDataItem *clone(void) const { return new QicsDataInt(*this); }

    
    inline virtual QicsDataItemType type(void) const
	{ return (QicsDataItem_Int); }

    
    inline static QString typeName(void)
	{ return QString("int"); }

    
    inline virtual QString typeString(void) const
	{ return QicsDataInt::typeName(); }

    
    inline virtual QicsDataInt& operator=(const int& i)
	{ myData = i; return *this;}
        
    inline virtual const QString string(void) const { return QString("%1").arg(myData); }
    inline virtual bool setString(const QString &qs) 
	{bool ok; myData = qs.toInt(&ok); return ok;}

    
    inline int data(void) const { return myData; }

    
    inline void setData(int i) { myData = i;}
        
    inline virtual QString format(const char *fmt_string) const
	{ QString str; return str.sprintf(fmt_string, myData); }

    int compareTo(const QicsDataItem &x) const;

    inline void encode(QDataStream &ds) const 
	{ ds << typeString(); ds << myData; }

    
    static QicsDataItem *fromString(const QString &str);

    
    static QicsDataItem *decode(QDataStream &ds)
	{ int val; ds >> val; return new QicsDataInt(val); }

protected:
        int myData;     
};

////////////////////////////////////////////////////////////////////////


class QicsDataLong: public QicsDataItem {
public:
            
    
    QicsDataLong() { myData = 0; }

    
    QicsDataLong(long i) { myData = i;}

    
    QicsDataLong(const QicsDataLong &dl);

    inline QicsDataItem *create(void) const { return new QicsDataLong(); }
    inline QicsDataItem *clone(void) const { return new QicsDataLong(*this); }

    
    inline virtual QicsDataItemType type(void) const
	{ return (QicsDataItem_Long); }

    
    inline static QString typeName(void)
	{ return QString("long"); }

    
    inline virtual QString typeString(void) const
	{ return QicsDataLong::typeName(); }

    
    inline virtual QicsDataLong& operator=(const long& i)
	{ myData = i; return *this;}
        
    inline virtual const QString string(void) const { return QString("%1").arg(myData); }
    inline virtual bool setString(const QString &qs) 
	{bool ok; myData = qs.toLong(&ok); return ok;}

    
    inline long data(void) const { return myData; }

    
    inline void setData(long i) { myData = i;}
        
    inline virtual QString format(const char *fmt_string) const
	{ QString str; return str.sprintf(fmt_string, myData); }

    int compareTo(const QicsDataItem &x) const;

    inline void encode(QDataStream &ds) const 
	{ ds << typeString(); ds << myData; }

    
    static QicsDataItem *fromString(const QString &str);

    
    static QicsDataItem *decode(QDataStream &ds)
	{ long val; ds >> val; return new QicsDataLong(val); }

protected:
        long myData;
};

////////////////////////////////////////////////////////////////////////


class QicsDataFloat: public QicsDataItem {
public:
            
    
    QicsDataFloat() { myData = 0.0; }

    
    QicsDataFloat(float f) { myData = f;}

    
    QicsDataFloat(const QicsDataFloat &df);

    inline QicsDataItem *create(void) const { return new QicsDataFloat(); }
    inline QicsDataItem *clone(void) const { return new QicsDataFloat(*this); }

    
    inline virtual QicsDataItemType type(void) const
	{ return (QicsDataItem_Float); }

    
    inline static QString typeName(void)
	{ return QString("float"); }

    
    inline virtual QString typeString(void) const
	{ return QicsDataFloat::typeName(); }

    
    inline virtual QicsDataFloat& operator=(const float& f)
	{ myData = f; return *this;}
        
    inline virtual const QString string(void) const { return QString("%1").arg(myData); }
    inline virtual bool setString(const QString &qs) 
	{bool ok; myData = qs.toFloat(&ok); return ok;}

    
    inline float data(void) const { return myData; }

    
    inline void setData(float f) { myData = f;}
        
    inline virtual QString format(const char *fmt_string) const
	{ QString str; return str.sprintf(fmt_string, myData); }

    int compareTo(const QicsDataItem &x) const;

    inline void encode(QDataStream &ds) const 
	{ ds << typeString(); ds << myData; }

    
    static QicsDataItem *fromString(const QString &str);

    
    static QicsDataItem *decode(QDataStream &ds)
	{ float val; ds >> val; return new QicsDataFloat(val); }

protected:
        float myData;     
};

////////////////////////////////////////////////////////////////////////


class QicsDataDouble: public QicsDataItem {
public:
            
    
    QicsDataDouble() { myData = 0.0; }

    
    QicsDataDouble(double d) { myData = d;}

    
    QicsDataDouble(const QicsDataDouble &dd);

    inline QicsDataItem *create(void) const { return new QicsDataDouble(); }
    inline QicsDataItem *clone(void) const { return new QicsDataDouble(*this); }

    
    inline static QString typeName(void)
	{ return QString("double"); }

    
    inline virtual QString typeString(void) const
	{ return QicsDataDouble::typeName(); }

    
    inline virtual QicsDataItemType type(void) const
	{ return (QicsDataItem_Double); }

    
    inline virtual QicsDataDouble& operator=(const double& d)
	{ myData = d; return *this;}
        
    inline virtual const QString string(void) const { return QString("%1").arg(myData); }
    inline virtual bool setString(const QString &qs) 
	{bool ok; myData = qs.toDouble(&ok); return ok;}

    
    inline double data(void) const { return myData; }

    
    inline void setData(double d) { myData = d;}
        
    inline virtual QString format(const char *fmt_string) const
	{ QString str; return str.sprintf(fmt_string, myData); }

    int compareTo(const QicsDataItem &x) const;

    inline void encode(QDataStream &ds) const 
	{ ds << typeString(); ds << myData; }

    
    static QicsDataItem *fromString(const QString &str);

    
    static QicsDataItem *decode(QDataStream &ds)
	{ double val; ds >> val; return new QicsDataDouble(val); }

protected:
        double myData;     
};

////////////////////////////////////////////////////////////////////////


class QicsDataString: public QicsDataItem {
public:

    
    QicsDataString() { myData = QString(); }

      
    QicsDataString(const QString &qs);

    virtual ~QicsDataString() {}

    
    QicsDataString(const QicsDataString &ds);

    inline QicsDataItem *create(void) const { return new QicsDataString(); }
    inline QicsDataItem *clone(void) const { return new QicsDataString(*this); }

    
    inline static QString typeName(void)
	{ return QString("qstring"); }

    
    inline virtual QString typeString(void) const
	{ return QicsDataString::typeName(); }

    
    inline virtual QicsDataItemType type(void) const
	{ return (QicsDataItem_String); }

    
    QicsDataString& operator=(const QicsDataString& qs); 
        
    inline virtual bool setString(const QString &qs) 
	{ myData = qs; return true;}

    inline bool setData(const char *s)
	{ myData = s; return true;}

    
    inline bool setData(const QString &qs)
	{ myData = qs; return true;}

    
    inline QString data(void) const { return myData;} 
        
    inline virtual const QString string(void) const { return myData; } 

    inline virtual QString format(const char *fmt_string) const
	{ QString str; return str.sprintf(fmt_string, (const char *) myData); }

    int compareTo(const QicsDataItem &x) const;

    inline void encode(QDataStream &ds) const 
	{ ds << typeString(); ds << myData; }

    
    static QicsDataItem *decode(QDataStream &ds)
	{ QString val; ds >> val; return new QicsDataString(val); }

protected: 
        QString myData;
};

////////////////////////////////////////////////////////////////////////



class QicsDataDate: public QicsDataItem
{
public:

      
    QicsDataDate(Qt::DateFormat format = Qt::TextDate);
      
    QicsDataDate(const QDate &date, Qt::DateFormat format = Qt::TextDate);
      
    QicsDataDate(int y, int m, int d, Qt::DateFormat format = Qt::TextDate);

    
    QicsDataDate(const QicsDataDate &dd);

    inline QicsDataItem *create(void) const { return new QicsDataDate(); }
    inline QicsDataItem *clone(void) const { return new QicsDataDate(*this); }

    
    inline static QString typeName(void)
	{ return QString("qdate"); }

    
    inline virtual QString typeString(void) const
	{ return QicsDataDate::typeName(); }

    
    inline virtual QicsDataItemType type(void) const
	{ return (QicsDataItem_Date); }

    
    virtual QicsDataDate& operator=(const QDate &d);
        
    inline virtual const QString string(void) const
	{ return myData.toString(); }
    virtual bool setString(const QString &qs);

    
    inline QDate data(void) const { return myData; }

    
    inline void setData(const QDate &date)
	{ myData = date; }

    virtual QString format(const char *fmt_string) const;

    
    inline virtual Qt::DateFormat defaultDateFormat(void) const
	{ return myDefaultFormat;}

    
    inline virtual void setDefaultDateFormat(Qt::DateFormat f)
	{ myDefaultFormat = f; }

    int compareTo(const QicsDataItem &x) const;

    inline void encode(QDataStream &ds) const 
	{ ds << typeString(); ds << myData; }

    
    static QicsDataItem *fromString(const QString &str);

    
    static QicsDataItem *decode(QDataStream &ds)
	{ QDate val; ds >> val; return new QicsDataDate(val); }

protected:
        QDate myData;
        Qt::DateFormat myDefaultFormat;
};

////////////////////////////////////////////////////////////////////////



class QicsDataTime: public QicsDataItem
{
public:

      
    QicsDataTime(Qt::DateFormat format = Qt::TextDate);
      
    QicsDataTime(const QTime &time, Qt::DateFormat format = Qt::TextDate);
      
    QicsDataTime(int hour, int minute, int second, int msec,
		 Qt::DateFormat format);

    
    QicsDataTime(const QicsDataTime &dt);

    inline QicsDataItem *create(void) const { return new QicsDataTime(); }
    inline QicsDataItem *clone(void) const { return new QicsDataTime(*this); }

    
    inline static QString typeName(void)
	{ return QString("qtime"); }

    
    inline virtual QString typeString(void) const
	{ return QicsDataTime::typeName(); }

    
    inline virtual QicsDataItemType type(void) const
	{ return (QicsDataItem_Time); }

    
    virtual QicsDataTime& operator=(const QTime &t);
        
    inline virtual const QString string(void) const { return myData.toString(); }
    virtual bool setString(const QString &qs);

    
    inline QTime data(void) const { return myData; }

    
    void setData(const QTime &time);

    virtual QString format(const char *fmt_string) const;

    
    inline virtual Qt::DateFormat defaultDateFormat(void) const
	{ return myDefaultFormat;}

    
    inline virtual void setDefaultDateFormat(Qt::DateFormat f)
	{ myDefaultFormat = f; }

    int compareTo(const QicsDataItem &x) const;

    inline void encode(QDataStream &ds) const 
	{ ds << typeString(); ds << myData; }

    
    static QicsDataItem *fromString(const QString &str);

    
    static QicsDataItem *decode(QDataStream &ds)
	{ QTime val; ds >> val; return new QicsDataTime(val); }

protected:
        QTime myData;
        Qt::DateFormat myDefaultFormat;
};

////////////////////////////////////////////////////////////////////////



class QicsDataDateTime: public QicsDataItem
{
public:

      
    QicsDataDateTime(Qt::DateFormat format = Qt::TextDate);
      
    QicsDataDateTime(const QDateTime &dt, Qt::DateFormat format = Qt::TextDate);
      
    QicsDataDateTime(const QDate &date, const QTime &time,
		     Qt::DateFormat format = Qt::TextDate);

    
    QicsDataDateTime(const QicsDataDateTime &ddt);

    inline QicsDataItem *create(void) const { return new QicsDataDateTime(); }
    inline QicsDataItem *clone(void) const { return new QicsDataDateTime(*this); }

    inline virtual QicsDataItemType type(void) const
	{ return (QicsDataItem_DateTime); }

    
    inline static QString typeName(void)
	{ return QString("qdatetime"); }

    
    inline virtual QString typeString(void) const
	{ return QicsDataDateTime::typeName(); }

    
    virtual QicsDataDateTime& operator=(const QDateTime &dt);
        
    inline virtual const QString string(void) const { return myData.toString(); }
    virtual bool setString(const QString &qs);

    
    inline QDateTime data(void) const { return myData; }

    
    void setData(const QDateTime &dt);

    virtual QString format(const char *fmt_string) const;

    
    inline virtual Qt::DateFormat defaultDateFormat(void) const
	{ return myDefaultFormat;}

    
    inline virtual void setDefaultDateFormat(Qt::DateFormat f)
	{ myDefaultFormat = f; }

    int compareTo(const QicsDataItem &x) const;

    inline void encode(QDataStream &ds) const 
	{ ds << typeString(); ds << myData; }

    
    static QicsDataItem *fromString(const QString &str);

    
    static QicsDataItem *decode(QDataStream &ds)
	{ QDateTime val; ds >> val; return new QicsDataDateTime(val); }

protected:
        QDateTime myData;
        Qt::DateFormat myDefaultFormat;
};

#endif /*_QicsDataItem_H --- Do not add anything past this line */
 
