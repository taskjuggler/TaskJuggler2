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


#ifndef _QICSCELLCOMMON_H
#define _QICSCELLCOMMON_H

#include <QicsGridInfo.h>
#include <QicsCellStyle.h>
#include <QicsCellDisplay.h>
#include <QicsDataItemFormatter.h>

#include <qobject.h>
#include <qfont.h>
#include <qvalidator.h>

////////////////////////////////////////////////////////////////////////////////




class QicsCellCommon: public QObject, public Qics {
Q_OBJECT
public:

   


    
    Q_PROPERTY( QString label READ label WRITE setLabel )
    
    Q_PROPERTY( bool enabled READ enabled WRITE setEnabled )
    
    Q_PROPERTY( bool readOnly READ readOnly  WRITE setReadOnly )
    
    Q_PROPERTY( bool selected READ selected )
    
    Q_PROPERTY( QFont font READ font WRITE setFont )
    
    Q_PROPERTY( QPalette palette READ palette WRITE setPalette )
    
    Q_PROPERTY( QColor foregroundColor READ foregroundColor WRITE setForegroundColor )
    
    Q_PROPERTY( QColor backgroundColor READ backgroundColor WRITE setBackgroundColor )
    
    Q_PROPERTY( QColor selectedForegroundColor READ selectedForegroundColor WRITE setSelectedForegroundColor )
    
    Q_PROPERTY( QColor selectedBackgroundColor READ selectedBackgroundColor WRITE setSelectedBackgroundColor )
    
    Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap )
    
    Q_PROPERTY( int pixmapSpacing READ pixmapSpacing WRITE setPixmapSpacing )
    
    Q_PROPERTY( QCursor cursor READ cursor WRITE setCursor )
    
    Q_PROPERTY( int alignment READ alignment WRITE setAlignment )
    
    Q_PROPERTY( int textFlags READ textFlags WRITE setTextFlags )
    
    Q_PROPERTY( int maxLength READ maxLength WRITE setMaxLength )
    
    Q_PROPERTY( int margin READ margin WRITE setMargin )
    
    Q_PROPERTY( int borderWidth READ borderWidth WRITE setBorderWidth )
    
    Q_PROPERTY( QicsLineStyle borderStyle READ borderStyle WRITE setBorderStyle )
    // REMOVE THIS if building with Qt 3.0.x 
    
    Q_PROPERTY( QPen borderPen READ borderPen WRITE setBorderPen )
    // END REMOVE THIS if building with Qt 3.0.x 


public:

    
    QicsCellCommon(QObject *parent = 0);

    
    QicsCellCommon(QicsGridInfo *info, QObject *parent = 0);

    
    virtual ~QicsCellCommon();

    
    inline virtual void setInfo(QicsGridInfo *info) { myInfo = info; }


    
    inline QicsDataModel *dataModel(void) const
	{ return (myInfo->dataModel()); }

    
    int margin(void) const;

    
    bool readOnly(void) const;
    
    
    QFont font(void) const;

    
    QPalette palette(void) const;

    
    QColor foregroundColor(void) const;

    
    QColor backgroundColor(void) const;

    
    QColor selectedForegroundColor(void) const;

    
    QColor selectedBackgroundColor(void) const;

    
    QPixmap pixmap(void) const;

    
    int pixmapSpacing(void) const;

    
    QicsCellDisplay *displayer(void) const;

    
    QicsDataItemFormatter *formatter(void) const;

    
    int alignment(void) const;

    
    bool enabled(void) const;

    
    bool selected(void) const;

    
    int textFlags(void) const;

    
    QValidator *validator(void) const;

    
    QString label(void) const;

    
    int maxLength(void) const;

    
    const QCursor &cursor(void) const;

    
    int borderWidth(void) const;

    
    QicsLineStyle borderStyle(void) const;

    
    QPen borderPen(void) const;

    
    void *userData(void) const;

public slots:
    
    void setMargin(int margin);

    
    void setReadOnly(bool b);
    
    
    void setFont(const QFont &font);

    
    void setPalette(const QPalette &pal);

    
    void setForegroundColor(const QColor &p);

    
    void setBackgroundColor(const QColor &p);

    
    void setSelectedForegroundColor(const QColor &p);

    
    void setSelectedBackgroundColor(const QColor &p);

    
    void setPixmap(const QPixmap &p);

    
    void setPixmapSpacing(int sp);

    
    void setDisplayer(QicsCellDisplay *d);

    
    void setFormatter(QicsDataItemFormatter *d);

    
    void setAlignment(int flags);

    
    void setEnabled(bool b);

    
    void setTextFlags(int flags);

    
    void setValidator(QValidator *v);

    
    void setLabel(const QString &label);

    
    void setMaxLength(int len);

    
    void setCursor(const QCursor &c);
    
    void unsetCursor(void);

    
    void setBorderWidth(int bw);

    
    void setBorderStyle(QicsLineStyle bs);

    
    void setBorderPen(const QPen &pen);

    
    void setUserData(void *data);

protected:
    
    virtual void setAttr(QicsCellStyle::QicsCellStyleProperty attr,
			 const void *val) = 0;
    
    virtual void *getAttr(QicsCellStyle::QicsCellStyleProperty attr) const = 0;
    
    virtual void clearAttr(QicsCellStyle::QicsCellStyleProperty attr) = 0;

    
    virtual void setDMMargin(int margin) = 0;
    
    virtual void setDMFont(const QFont &font) = 0;

    
    inline QicsStyleManager &styleManager(void) const
	{ return (*(myInfo->styleManager())); }

    
    inline QicsDimensionManager &dimensionManager(void) const
	{ return (*(myInfo->dimensionManager())); }

    
    QicsGridInfo *myInfo;

private:
#ifdef Q_DISABLE_COPY
    QicsCellCommon(const QicsCellCommon &cc);
    QicsCellCommon &operator=(const QicsCellCommon &cc);
#endif
};

#endif /* _QICSTABLECOMMON_H */
