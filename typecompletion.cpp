/*
   Copyright (C) 2008-2009 by Sebastian Trueg <trueg at kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "typecompletion.h"
#include "pimo.h"

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Types/Class>

#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/BindingSet>
#include <Soprano/RdfSchemaModel>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/RDFS>

#include <KDebug>
#include <KIcon>



class Nepomuk::TypeCompletion::Private
{
public:
    Private()
        : baseType( Vocabulary::PIMO::Thing() ) {
    }

    Types::Class baseType;
};


Nepomuk::TypeCompletion::TypeCompletion( QObject* parent )
    : KCompleter(),
      d( new Private() )
{
    setParent( parent );
}


Nepomuk::TypeCompletion::~TypeCompletion()
{
    delete d;
}


Nepomuk::Types::Class Nepomuk::TypeCompletion::baseType() const
{
    return d->baseType;
}


void Nepomuk::TypeCompletion::makeCompletion( const QString& string )
{
    kDebug() << string;

    if ( string.length() > 2 ) {
        Soprano::RdfSchemaModel model( ResourceManager::instance()->mainModel() );

        // FIXME: use the query API
        Soprano::QueryResultIterator it = model.executeQuery( string + "*",
                                                              Soprano::Query::QueryLanguageUser,
                                                              "lucene" );
        int cnt = 0;
        QString m;
        while ( it.next() ) {
            QUrl r = it.binding( 0 ).uri();
            if ( model.isClass( r ) ) {
                Types::Class type( r );
                if ( type.isSubClassOf( d->baseType ) ) {
                    kDebug() << "match:" << type.label() << type.uri();
                    addCompletion( KCompletionItem( type.label(),
                                                    type.label(),
                                                    type.comment(),
                                                    type.icon(),
                                                    type.uri() ) );

                    if ( ++cnt >= 10 ) {
                        kDebug() << "Stopping at" << cnt << "results";
                        return;
                    }
                }
            }
        }
    }
}


void Nepomuk::TypeCompletion::setBaseType( const Nepomuk::Types::Class& type )
{
    d->baseType = type;
}

#include "typecompletion.moc"
