/*
 * This file is part of Soprano Project.
 *
 * Copyright (C) 2008 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "pimomodel.h"
#include "pimo.h"

#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/XMLSchema>


class Nepomuk::PIMOModel::Private
{
public:
};


Nepomuk::PIMOModel::PIMOModel( Soprano::Model* parentModel )
    : NRLModel( parentModel ),
      d( new Private() )
{
}


Nepomuk::PIMOModel::~PIMOModel()
{
    delete d;
}


// void Nepomuk::PIMOModel::findMatches( const QString& text )
// {
//     // Find a direct match Things
//     // FIXME: This does only work with inference!
//     QueryResultIteartor it = executeQuery( QString("select ?r where { ?r a <%1> . ?r <%2> \"%3\"^^<%4> . }")
//                                            .arg( Vocabulary::PIMO::Thing().toString() )
//                                            .arg( Vocabulary::RDFS::label().toString() )
//                                            .arg( QString( text ).replace( "\"", "\\\"" ) )
//                                            .arg( Vocabulary::XMLSchema::string().toString() ),
//                                            Query::QueryLanguageSparql );
//     while( it.next() ) {
//         emit match( it.binding("r").uri(), Vocabulary::RDF::type(), 1.0 );
//     }


// }

#include "pimomodel.moc"
