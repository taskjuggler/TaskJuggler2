/*
 * kotrus.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *                       Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdio.h>


#include <qsettings.h> 
#include <qstringlist.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qsqlrecord.h>
#include <qfileinfo.h>
#include <qdir.h>

#include "ResourceList.h"
#include "Task.h"
#include "Project.h"
#include "kotrus.h"
#include "Interval.h"
/*

CREATE TABLE ktBookings(
  bookID mediumint(9) NOT NULL autoincrement,
  userID mediumint(9) NOT NULL,
  ktNo mediumint(9) NOT NULL,
  LockTime TimeStamp,               <= locked since.
  lockedBy mediumint(9) default 0,  <= Locking person, 0 means no lock.
  startTS TimeStamp,
  endTS TimeStamp,
  projectID mediumint(9) default 0,
  
  PRIMARY KEY  (bookID)
) ;


 */

Kotrus::Kotrus()
   : kotrusDB( 0L ),
     mode( NoKotrus )
{
   QSettings settings;

   QFileInfo fi ( QDir::home().path() + QString("/.qt/taskjugglerrc") );
   if( ! fi.exists())
   {
      QSettings settings;
      settings.writeEntry( "/taskjuggler/general/datasource", "xml|database" );
      settings.writeEntry( "/taskjuggler/general/manager", "yourKotrusUser eg. phm@suse.de" );
      
      settings.writeEntry( "/taskjuggler/DB/password", "setpwd" );
      settings.writeEntry( "/taskjuggler/DB/user", "root" );
      settings.writeEntry( "/taskjuggler/DB/host", "localhost" );
      settings.writeEntry( "/taskjuggler/DB/database", "kotrus" );
      settings.writeEntry( "/taskjuggler/general/defaultmailpostfix", "@suse.de" );

   }
}

Kotrus::~Kotrus()
{
}

QString Kotrus::Param( QString key ) const
{
   QString ret;
   
   QSettings settings;
   bool ok;
      
   ret = settings.readEntry( key, QString(), &ok );

   return ret;
}

int Kotrus::personID( const QString& resourcename )
{
   connect();
   QSqlCursor cur( "persons");
   cur.select( "login_name='" + resourcename + "'" );
   int id = 0;
   
   while( cur.next())
   {
      id = cur.value( "PersonID").toInt();
      // qDebug( "Found UserID : %d", id );
   }

   return id;
}

/* returns the internal kotrus No for a given Kostenträger identified
 * by its name
 * Note that has nothing to do with _projects_ !
 */
int Kotrus::ktID( const QString& ktName )
{
   connect();
   QSqlCursor cur( "kt");
   // qDebug( "Selecting for " + resourcename );
   cur.select( "name='" + ktName + "'" );

   int id = 0;
   
   while( cur.next()) {
      id = cur.value( "ktNo").toInt();
      qDebug( "Found KotrusID: %d", id );
   }

   return id;
}


/* Locks all entries for the resource identified by personID for the
 * project manager identified by lockID
 */
int Kotrus::lockBookings( int pid, int lockID )
{
   if( mode != DB ) return( 0 );
   connect();
   
   /* Check if the lockID > 0. If not, read it from the parameter file */
   if( lockID == 0 )
   {
      const QString user = Param( "/taskjuggler/general/manager" );
      lockID = personID( user );

      if( lockID == 0 )
      {
	 qFatal( "Unknown user <" + user + ">, please edit $HOME/.qt/taskjugglerrc");
	 return( -1 );
      }
   }
   else if( lockID == -1 )
   {
      /* Unlock bookings */
      lockID=0;
   }
      

   /* This statement locks ALL bookings of the person. Not only for the
    * project */
   QString sql( "UPDATE ktBookings SET lockedBy=");
   sql += QString::number(lockID) + QString( ", LockTime=");

   /* Set lock-Time to Zero in case it is an unlock-request with personID=0 */
   if( lockID> 0 )
      sql += QString( "NOW()");
   else
      sql += QString( "0000-00-00 00:00:00" );
   
   sql += QString( " WHERE userID=" ) + QString::number(pid);
   sql += QString( " AND lockedBy=0" ); /* Only lock unlocked ! */
   int rows = 0;
   
   QSqlQuery query( sql );
   if( query.isActive() ) rows = query.numRowsAffected();

   return( rows );
}

void Kotrus::unlockBookings( const QString& kotrusID )
{
   int pid = personID( kotrusID );

   lockBookings( pid, -1 );
}



			   
BookingList Kotrus::loadBookings( const QString& kotrusID,
				  const QStringList& skipProjectIDs, int user )
{
   
   connect();
   if( mode == DB ) return( loadBookingsDB( kotrusID, skipProjectIDs, user ));
   if( mode == XML ) return( loadBookingsXML( kotrusID, skipProjectIDs, user ));

   BookingList blist;
   return( blist );
}

BookingList Kotrus::loadBookingsXML( const QString& kotrusID,
				     const QStringList& skipProjectIDs,
				     int user )
{
   BookingList blist;
   /* TODO */
   
   return( blist );
}

BookingList Kotrus::loadBookingsDB( const QString& kotrusID,
				    const QStringList& skipProjectIDs,
				    int user )
{
   QSqlCursor cur( "ktBookings" );
   BookingList blist;
   
   int pid = personID( kotrusID );

   if( lockBookings( pid, user ) == -1 )
   {
      qFatal( "ERR: Could not lock bookings!" );
      return blist;
   }
   
   if( pid > 0 )
   {
      QString sql( "select kt.name, UNIX_TIMESTAMP(b.startTS), UNIX_TIMESTAMP(b.endTS),"
		   "b.projectID, b.LockTime, b.lockedBy "
		   "from kt, ktBookings b where b.ktNo=kt.ktNo AND b.userID =" + QString::number(pid));

      int anz = skipProjectIDs.count();
      qDebug( "count in list: %d", anz );

      if( anz > 0 )
      {

	 QString allSkips = "";
	 bool needOr = false;
	 int validSkips = 0;
	 
	 for ( QStringList::ConstIterator it=skipProjectIDs.begin(); it != skipProjectIDs.end(); ++it )
	 {
	    QString skippy( *it );
	    if( !skippy.isEmpty())
	    {
	       if( needOr ) {
		  allSkips += " OR ";
	       }
	       allSkips += "b .projectID='" + skippy +"'";
	       needOr = true;
	       validSkips++;
	    }
	 }

	 if( validSkips > 0 )
	    sql += " AND NOT (" + allSkips + ")"; 
      }
      sql += " ORDER BY b.startTS, b.projectID";
      
      qDebug( "SQL: " + sql );
      QSqlQuery query( sql );

      
      while( query.next() )
      {
	 bool ok;
	 bool generalOk = true;
	 
	 time_t start = query.value(1).toUInt(&ok);
	 generalOk = generalOk && ok;
	 time_t end = query.value(2).toUInt(&ok);
	 generalOk = generalOk && ok;

	 const Interval interval( start, end );

	 if( generalOk )
	 {
	    QString account = query.value(0).toString(); /* account => kt.name */
	    QString project = query.value(3).toString(); /* project => projectID */
	    QString ltime = query.value(4).toString();   /* LockTime */
	    QString locker = query.value(5).toString();  /* email of locker */

	    qDebug("Loaded booking for project " + project );
	    
	    Booking *nbook = new Booking( interval, 0L,
					  account,
					  project );
	    nbook->setLockTS( ltime );
	    nbook->setLockerId( locker );
	    
	    blist.append( nbook );
	 }
	 else
	 {
	    qFatal( "ERR: Could not convert timestamps!" );
	 }
      }
   }
   else
   {
      if( kotrusID.isEmpty() )
	 qDebug( "WRN: Can not load bookings for empty user!"); 
      else
	 qDebug( "WRN: Could not resolve user " + kotrusID );
   }
   return( blist );
}





int Kotrus::saveBookings( const QString& kotrusID,  /* the user id */
			  const QString& projectID, 
			  const BookingList& blist,
			  int   lockedFor )
{
   if( mode == NoKotrus ) return( 0 );
   if( mode == XML ) return( saveBookingsXML( kotrusID, projectID, blist, lockedFor ));

   connect();
   
   int pid = personID( kotrusID );
   int cnt_bookings = 0;

   /* It is only possible to save bookings if the users bookings are locked by the
    * user starting to save the bookings (=lockedFor). 
    */

   /* In case there are Bookings for the userID (=kotrusID) locked for somebody else,
    * stop throwing an fatal error
    */
   QString sql( "SELECT Count(bookID) FROM ktBookings WHERE userID=" );
   sql += QString::number(pid);
   sql += QString( " AND lockedBy !=%1" ).arg( lockedFor );

   QSqlQuery query( sql );
   if( query.isActive() )
   {
      int foreignLocks = query.value(0).toInt();
      qDebug("Have %d foreign Locks!", foreignLocks );

      if( foreignLocks > 0 )
      {
	 qWarning( "Have foreign Locks on User " + kotrusID );
	 return 0;
      }
   }
   else
   {
      qWarning( "Can not count Locks, query is not active!" );
      return 0;
   }

   /* First delete all bookings for the person */
   sql = QString("DELETE FROM ktBookings WHERE userID=%1 ").arg( pid );
   sql += "AND projectID=" + projectID;

   int rows = 0;
   query.exec( sql );
   if ( query.isActive() ) rows = query.numRowsAffected() ;
   qDebug("deleted %d booking rows for resource", rows );

   /* And now insert again */
   QSqlCursor cur( "ktBookings" );
   
   BookingListIterator it( blist );
   for( ; it.current(); ++it )
   {
      int kotrusNumId = getKotrusAccountId( (*it)->getAccount());
      QSqlRecord *buffer = cur.primeInsert();
      buffer->setValue( "userID", pid );
      buffer->setValue( "ktNo", kotrusNumId );
      buffer->setValue( "startTS", QString("FROM_UNIXTIME(%1)").arg((*it)->getStart()));
      buffer->setValue( "endTS", QString("FROM_UNIXTIME(%1)").arg((*it)->getEnd()));
      buffer->setValue( "projectID", projectID );

      cnt_bookings += cur.insert();
   }
   
   return( cnt_bookings );
}

int Kotrus::saveBookingsXML( const QString& kotrusID,  /* the user id */
			  const QString& projectID, 
			  const BookingList& blist,
			  int   lockedFor )
{
   /* TODO */
   return( 0 );
}

int Kotrus::getKotrusAccountId( const QString& acc )
{
   return(0);
}

void Kotrus::connect()
{
   if( mode != DB || kotrusDB ) return;

   kotrusDB = QSqlDatabase::addDatabase( "QMYSQL3" );

   QStringList drivers = QSqlDatabase::drivers();
   QSqlError err = kotrusDB->lastError();

   if( err.type() != QSqlError::None )
      qDebug( "An Error!" );
   
   for ( QStringList::Iterator it = drivers.begin(); it != drivers.end(); ++it ) {
      qDebug( "Driver available: " + *it + "\n");
    }
   
   if ( kotrusDB )
   {
      
      QString db = Param(  "/taskjuggler/DB/database" );
      QString pwd = Param( "/taskjuggler/DB/password" );
      QString usr = Param( "/taskjuggler/DB/user" );
      QString host = Param("/taskjuggler/DB/host" );

      
      kotrusDB->setDatabaseName( db );
      kotrusDB->setUserName( usr );
      kotrusDB->setPassword( pwd );
      kotrusDB->setHostName( host );
      
      if ( kotrusDB->open() )
      {
	 // Database successfully opened; we can now issue SQL commands.
	 qDebug( "Opening database " + db + ": Success" );
      }
      else
      {
	 qDebug( "Opening database " + db + ": Failed" );

      }
   }
   else
   {
      qDebug( "Failed to connect :(\n" );
   }
}

void Kotrus::setKotrusMode( const QString& newKotrusMode )
{
   
   if( newKotrusMode.upper() == "XML" )
   {
      mode = XML;
   }
   else if( newKotrusMode.upper() == "DB" )
   {
      mode = DB;
   }
   else
   {
      mode = NoKotrus;
   }
  
}
