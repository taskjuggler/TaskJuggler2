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


#ifndef _QicsStyle_H
#define _QicsStyle_H

#include <QicsNamespace.h>

#include <qvaluevector.h>


class QicsStyle: public Qics
{
public:
    
    enum QicsStylePropertyType { QicsT_Int = 0, 
				 QicsT_Float, 
				 QicsT_QString, 
				 QicsT_QColor,
				 QicsT_Boolean,
				 QicsT_QWidget,
				 QicsT_QColorGroup,
				 QicsT_QPalette,
				 QicsT_QFont,
				 QicsT_QPixmap,
				 QicsT_QCursor,
				 QicsT_QicsCellDisplay,
                                 QicsT_QicsDataItemFormatter,
                                 QicsT_QValidator,
    				 QicsT_QPen,
                                 QicsT_QicsRegion,
                                 QicsT_Pointer };

        
    QicsStyle();
        
    
    virtual ~QicsStyle();
        
    
    inline void *getValue(int prop) const
	{ return myProperties[prop]; }
    
    void setValue(int prop, const void *val);

    
    void clear(int prop);
        
    
    bool isEmpty(void) const;

protected:
    void init(void);
       
        
    void **myProperties;

    int myNumProperties;

    /// counter keeping track of how many properties in the style have been set
    int mySetCount;

    
    const QicsStylePropertyType *myStyleTypeList;

private:
            
    
    QicsStyle& operator=(const QicsStyle&) { return *this;} // Copy operator
    
    QicsStyle(const QicsStyle&) {} // Copy Constructor
};

typedef QValueVector<QicsStyle *> QicsStylePV;
typedef QValueVector<QicsStylePV *> QicsStylePVPV;

#endif /*_QicsStyle_H --- Do not add anything past this line */
 
