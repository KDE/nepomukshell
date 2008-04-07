/*
   Copyright (C) 2008 by Sebastian Trueg <trueg at kde.org>

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

#include "resourcecompletion.h"

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Resource>
#include <Nepomuk/Types/Class>

#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/BindingSet>
#include <Soprano/Vocabulary/RDFS>

#include <KDebug>
#include <KIcon>



class Nepomuk::ResourceCompletion::Private
{
public:
    Private()
        : type( Soprano::Vocabulary::RDFS::Resource() ) {
    }

    Types::Class type;
};


Nepomuk::ResourceCompletion::ResourceCompletion( QObject* parent )
    : Completion(),
      d( new Private() )
{
    setParent( parent );
}


Nepomuk::ResourceCompletion::~ResourceCompletion()
{
    delete d;
}


Nepomuk::Types::Class Nepomuk::ResourceCompletion::type() const
{
    return d->type;
}


void Nepomuk::ResourceCompletion::makeCompletion( const QString& string )
{
    kDebug() << string;

    if ( string.length() > 2 ) {
        Soprano::QueryResultIterator it
            = ResourceManager::instance()->mainModel()->executeQuery( string + "*",
                                                                      Soprano::Query::QueryLanguageUser,
                                                                      "lucene" );
        int cnt = 0;
        QString m;
        while ( it.next() ) {
            // FIXME: this is awfully slow
            // but we have no inference and no combined lucene + sparql queries
            Resource res( it.binding( 0 ).uri() );
            kDebug() << "possible match for input" << string << res.uri();
            if ( res.hasType( d->type ) ) {
                kDebug() << "Match for input" << string << res.uri();
                if ( m.isEmpty() ) {
                    m = res.resourceUri().toString();
                }
                addCompletion( CompletionItem( res.genericLabel(),
                                               QString( "%1 (%2)" ).arg( res.genericLabel() ).arg( Types::Class( res.type() ).label() ),
                                               res.genericDescription(),
                                               KIcon( res.genericIcon() ),
                                               res.resourceUri() ) );

                if ( ++cnt >= 10 ) {
                    kDebug() << "Stopping at" << cnt << "results";
                    return;
                }
            }
        }
    }
}


void Nepomuk::ResourceCompletion::setType( const Nepomuk::Types::Class& type )
{
    d->type = type;
}

#include "resourcecompletion.moc"
