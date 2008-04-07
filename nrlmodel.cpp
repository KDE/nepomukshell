/*
 * This file is part of Soprano Project.
 *
 * Copyright (C) 2007 Sebastian Trueg <trueg@kde.org>
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

#include "nrlmodel.h"
#include "knx.h"

#include <soprano/error.h>
#include <soprano/nrl.h>
#include <soprano/nao.h>
#include <soprano/rdf.h>
#include <soprano/rdfs.h>
#include <soprano/queryresultiterator.h>
#include <soprano/statementiterator.h>
#include <soprano/node.h>
#include <soprano/statement.h>

#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QRegExp>

#include <KRandom>


class Soprano::NRLModel::Private
{
public:
    Private( NRLModel* parent )
        : ignoreContext( true ),
          baseNamespace( "http://soprano.sourceforge.net/data#" ),
          q( parent ) {
    }

    bool ignoreContext;
    QUrl baseNamespace;

    QUrl createUniqueUri( QString name = QString() );

private:
    NRLModel* q;
};


QUrl Soprano::NRLModel::Private::createUniqueUri( QString name )
{
    QUrl s( baseNamespace );
    if ( !name.isEmpty() ) {
        s.setFragment( name.replace( QRegExp( "[^\\w\\.\\-_:]" ), "") );
    }
    else {
        s.setFragment( KRandom::randomString( 20 ) );
    }
    while( 1 ) {
        if( !q->containsContext( s ) &&
            !q->containsAnyStatement( s, Soprano::Node(), Soprano::Node() ) &&
            !q->containsAnyStatement( Soprano::Node(), s, Soprano::Node() ) &&
            !q->containsAnyStatement( Soprano::Node(), Soprano::Node(), s ) )
            return s;
        s.setFragment( name + KRandom::randomString( 20 ) );
    }
}


Soprano::NRLModel::NRLModel( Model* parent )
    : RdfSchemaModel( parent ),
      d( new Private(this) )
{
}


Soprano::NRLModel::~NRLModel()
{
    delete d;
}


void Soprano::NRLModel::setIgnoreContext( bool b )
{
    d->ignoreContext = b;
}


bool Soprano::NRLModel::ignoreContext() const
{
    return d->ignoreContext;
}


// Soprano::Error::ErrorCode Soprano::NRLModel::addStatement( const Statement& statement )
// {
//     // 1. check if any cardinality restrictions are defined for s.predicate()
//     // 2. if so -> enforce
//     // 3. if not -> check if some for superproperties are defined (optional advanced feature)

//     QString query = QString( "select ?min ?max ?c where { { <%1> <%2> ?min } UNION { <%1> <%3> ?max } UNION { <%1> <%4> ?c } }" )
//                     .arg( statement.predicate().toString() )
//                     .arg( Vocabulary::NRL::minCardinality().toString() )
//                     .arg( Vocabulary::NRL::maxCardinality().toString() )
//                     .arg( Vocabulary::NRL::cardinality().toString() );

//     QueryResultIterator it = FilterModel::executeQuery( query, Query::QueryLanguageSparql );
//     if ( !it.isValid() ) {
//         setError( QString( "Query failed: '%1'" ).arg( query ) );
//         return Error::ErrorUnknown;
//     }

//     int min = -1;
//     int max = -1;
//     int c = -1;

//     while ( it.next() ) {
//         if ( it.binding( "min" ).isLiteral() ) {
//             min = it.binding( "min" ).literal().toInt();
//         }
//         else if ( it.binding( "max" ).isLiteral() ) {
//             max = it.binding( "max" ).literal().toInt();
//         }
//         if ( it.binding( "c" ).isLiteral() ) {
//             c = it.binding( "c" ).literal().toInt();
//         }
//     }

//     if ( min >= 0 || max >= 0 || c >= 0 ) {
//         qDebug() << "Predicate " << statement.predicate() << " has cardinalities: " << min << "; " << max << "; " << c;

//         // the simple case (and also the most frequently used I suppose)
//         if ( c == 1 || max == 1 ) {
//             if ( ignoreContext() ) {
//                 FilterModel::removeAllStatements( Statement( statement.subject(), statement.predicate(), Node() ) );
//             }
//             else {
//                 FilterModel::removeAllStatements( Statement( statement.subject(), statement.predicate(), Node(), statement.context() ) );
//             }
//             return FilterModel::addStatement( statement );
//         }

//         // the general case
//         else if ( max > 1 || c > 1 ) {
//             if ( c > 1 )
//                 max = c;

//             StatementIterator sit;
//             if ( ignoreContext() ) {
//                 sit = FilterModel::listStatements( Statement( statement.subject(), statement.predicate(), Node() ) );
//             }
//             else {
//                 sit = FilterModel::listStatements( Statement( statement.subject(), statement.predicate(), statement.context() ) );
//             }

//             QList<Statement> matchingStatements;
//             while ( sit.next() ) {
//                 Statement s = *sit;
//                 if ( ignoreContext() || s.context() == statement.context() ) {
//                     matchingStatements.append( s );
//                 }
//             }

//             if ( matchingStatements.count() >= max ) {
//                 qDebug() << "Found " << matchingStatements.count() << " statements that define " << statement.predicate() << " for " << statement.subject() << endl
//                          << "  -> need to remove " << ( matchingStatements.count()-max+1 ) << " before adding the new statement.";
//                 if ( matchingStatements.count() == 1 ) {
//                     qDebug() << "Removing one statement is easy...";
//                     FilterModel::removeStatement( matchingStatements[0] );
//                 }
//                 else {
//                     qDebug() << "FIXME: which statements to remove? Random? Best would be to remove the oldest...";
//                     setError( "Max cardinality for predicate reached" );
//                     return Error::ErrorUnknown;
//                 }
//             }

//             return FilterModel::addStatement( statement );
//         }
//         else {
//             qDebug()  << "Predicate " << statement.predicate() << " has unusable cardinality restrictions";
//             return FilterModel::addStatement( statement );
//         }
//     }
//     else {
//         qDebug() << "Predicate " << statement.predicate() << " has no cardinality restrictions";
//         return FilterModel::addStatement( statement );
//     }

//     // make gcc shut up
//     return Error::ErrorNone;
// }


QUrl Soprano::NRLModel::addStatements( const QList<Statement>& statements, const QUrl& graphRole )
{
    if ( statements.isEmpty() ) {
        setError( "No statements to add." );
        return QUrl();
    }
    if ( !graphRole.isValid() ) {
        setError( "No valid graph role set." );
        return QUrl();
    }

    // create a new graph
    QUrl graph = newGraphUri();
    if ( static_cast<FilterModel*>( this )->addStatement( graph,
                                                          Vocabulary::RDF::type(),
                                                          graphRole ) != Error::ErrorNone ||
         static_cast<FilterModel*>( this )->addStatement( graph,
                                                          Vocabulary::NAO::created(),
                                                          LiteralValue( QDateTime::currentDateTime() ) ) != Error::ErrorNone ) {
        return QUrl();
    }

    // add all data
    foreach( Statement s, statements ) {
        s.setContext( graph );
        if ( addStatement( s ) != Error::ErrorNone ) {
            return QUrl();
        }
    }

    return graph;
}


QUrl Soprano::NRLModel::baseNamespace() const
{
    return d->baseNamespace;
}


void Soprano::NRLModel::setBaseNamespace( const QUrl& uri )
{
    d->baseNamespace = uri;
}


QUrl Soprano::NRLModel::newGraphUri()
{
    return d->createUniqueUri();
}


QUrl Soprano::NRLModel::newClassUri( const QString& name )
{
    return d->createUniqueUri( name );
}


QUrl Soprano::NRLModel::newPropertyUri( const QString& name )
{
    return d->createUniqueUri( name );
}


QUrl Soprano::NRLModel::newResourceUri( const QString& name )
{
    return d->createUniqueUri( name );
}


QUrl Soprano::NRLModel::createClass( const QUrl& parentClass,
                                     const QString label,
                                     const QString& comment,
                                     const QString& icon )
{
    if ( !parentClass.isValid() ) {
        setError( "Invalid parent class" );
        return QUrl();
    }
    if ( label.isEmpty() ) {
        setError( "No label for new class set." );
        return QUrl();
    }
    if ( !isClass( parentClass ) ) {
        setError( QString( "Not a class: %1. Only classes can be used as parents." ).arg( parentClass.toString() ) );
        return QUrl();
    }

    QUrl classUri = newClassUri( label );

    QList<Statement> sl;
    sl << Statement( classUri, Vocabulary::RDF::type(), Vocabulary::RDFS::Class() )
       << Statement( classUri, Vocabulary::RDFS::subClassOf(), parentClass )
       << Statement( classUri, Vocabulary::RDFS::label(), LiteralValue( label ) );
    if ( !comment.isEmpty() ) {
        sl << Statement( classUri, Vocabulary::RDFS::comment(), LiteralValue( comment ) );
    }
    if ( !icon.isEmpty() ) {
        sl << Statement( classUri, Nepomuk::Vocabulary::KNX::hasIcon(), LiteralValue( icon ) );
    }

    QUrl graph = addStatements( sl, Vocabulary::NRL::Ontology() );
    if ( graph.isValid() ) {
        return classUri;
    }
    else {
        return QUrl();
    }
}


QUrl Soprano::NRLModel::createProperty ( const QUrl& domain,
                                         const QUrl& range,
                                         const QString label,
                                         const QString& comment,
                                         const QString& icon )
{
    if ( !domain.isValid() ) {
        setError( "Invalid domain" );
        return QUrl();
    }
    if ( !range.isValid() ) {
        setError( "Invalid range" );
        return QUrl();
    }
    if ( label.isEmpty() ) {
        setError( "No label for new class set." );
        return QUrl();
    }
    if ( !isClass( domain ) ) {
        setError( QString( "Not a class: %1. Only classes can be used as parents." ).arg( domain.toString() ) );
        return QUrl();
    }

    QUrl propertyUri = newPropertyUri( label );

    QList<Statement> sl;
    sl << Statement( propertyUri, Vocabulary::RDF::type(), Vocabulary::RDF::Property() )
       << Statement( propertyUri, Vocabulary::RDFS::domain(), domain )
       << Statement( propertyUri, Vocabulary::RDFS::range(), range )
       << Statement( propertyUri, Vocabulary::RDFS::label(), LiteralValue( label ) );
    if ( !comment.isEmpty() ) {
        sl << Statement( propertyUri, Vocabulary::RDFS::comment(), LiteralValue( comment ) );
    }
    if ( !icon.isEmpty() ) {
        sl << Statement( propertyUri, Nepomuk::Vocabulary::KNX::hasIcon(), LiteralValue( icon ) );
    }

    QUrl graph = addStatements( sl, Vocabulary::NRL::Ontology() );
    if ( graph.isValid() ) {
        return propertyUri;
    }
    else {
        return QUrl();
    }
}


QUrl Soprano::NRLModel::createResource( const QUrl& type,
                                        const QString& label,
                                        const QString& comment,
                                        const QString& icon )
{
    if ( !type.isValid() ) {
        setError( "Invalid type" );
        return QUrl();
    }
    if ( label.isEmpty() ) {
        setError( "No label for new resource set." );
        return QUrl();
    }
    if ( !isClass( type ) ) {
        setError( QString( "Not a class: %1. Only classes can be used as types." ).arg( type.toString() ) );
        return QUrl();
    }

    QUrl resourceUri = newResourceUri( label );

    QList<Statement> sl;
    sl << Statement( resourceUri, Vocabulary::RDF::type(), type )
       << Statement( resourceUri, Vocabulary::RDFS::label(), LiteralValue( label ) );
    if ( !comment.isEmpty() ) {
        sl << Statement( resourceUri, Vocabulary::RDFS::comment(), LiteralValue( comment ) );
    }
    if ( !icon.isEmpty() ) {
        sl << Statement( resourceUri, Nepomuk::Vocabulary::KNX::hasIcon(), LiteralValue( icon ) );
    }

    QUrl graph = addStatements( sl, Vocabulary::NRL::InstanceBase() );
    if ( graph.isValid() ) {
        return resourceUri;
    }
    else {
        return QUrl();
    }
}
