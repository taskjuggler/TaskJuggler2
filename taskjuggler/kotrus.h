/*
 * Kotrus.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *                       Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ResourceList.h"

class QSqlDatabase;

enum kotrusMode{ NoKotrus = 0, DB, XML};

class Kotrus
{
public:
   Kotrus( );

   ~Kotrus( );

   /**
    * @returns the database person id on a given name
    */
   int personID( const QString& );

   /**
    * @returns the database-'Kostenträger'-id on a given name
    */
   int ktID( const QString& );

   /**
    * @returns a list of bookings for the person identified by the kotrusID
    * for all projects except the one given in skipProjectID. If skipProjectID
    * is 0, all projects are in the return list.
    * The member task remains emtpy in the bookings.
    * @param kotrusID is the email address of the user for whom the projects should be loaded
    * @param skipProjectID points to a project which should be excluded. Pass zero for getting
    *        all projects.
    * @param user is the user ID of the person locking the bookings.
    *
    */
   BookingList loadBookings( const QString& kotrusID, const QString& skipProjectID, int user=0 );

   int         saveBookings( const QString& kotrusID , const QString& projectID,
			     const BookingList&, int lockedFor  );

   int 	       getKotrusAccountId( const QString& acc );
   void        unlockBookings( const QString& kotrusID );
   int         lockBookings( int personID, int lockID );

   void        setKotrusMode( const QString& newKotrusMode = "NoKotrus" );
       
   kotrusMode  getKotrusMode() const { return mode; }
   
private:
   BookingList loadBookingsDB( const QString& kotrusID, const QString& skipProjectID, int user=0 );
   BookingList loadBookingsXML( const QString& kotrusID, const QString& skipProjectID, int user=0 );

   int         saveBookingsXML( const QString& kotrusID , const QString& projectID,
				const BookingList&, int lockedFor  );
   
   
   void 		connect();
   QString 		Param( QString key ) const;
   
   QSqlDatabase 	*kotrusDB;
   kotrusMode           mode;
   
   
};
