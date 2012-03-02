/*
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "pimomodel.h"

#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/XMLSchema>
#include <Soprano/Node>
#include <Soprano/Statement>
#include <Soprano/LiteralValue>
#include <Soprano/QueryResultIterator>

#include <Nepomuk/Types/Class>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Thing>
#include <Nepomuk/Vocabulary/PIMO>

#include <krandom.h>

#include <QtCore/QDateTime>


// FIXME: Do not create only one pimo graph. Actually why not do the same thing we do with resources?

using namespace Soprano;

class Nepomuk::PimoModel::Private
{
public:
    Private( PimoModel* parent )
        : q( parent ) {
    }

    QUrl pimoContext;

    QUrl createUniqueUri( QString name = QString() );

private:
    PimoModel* q;
};


QUrl Nepomuk::PimoModel::Private::createUniqueUri( QString name )
{
    if ( !name.isEmpty() ) {
        QString normalizedName = name.replace( QRegExp( QLatin1String("[^\\w\\.\\-_:]") ), QString() );
        QUrl s = QString(QLatin1String("nepomuk:/") + normalizedName);
        while( 1 ) {
            if ( !q->executeQuery( QString::fromLatin1("ask where { { <%1> ?p1 ?o1 . } UNION { ?r2 <%1> ?o2 . } UNION { ?r3 ?p3 <%1> . } }")
                                   .arg( QString::fromAscii( s.toEncoded() ) ), Soprano::Query::QueryLanguageSparql ).boolValue() ) {
                return s;
            }
            s = QLatin1String("nepomuk:/") + normalizedName + QLatin1String("_") +  KRandom::randomString( 20 );
        }
    }
    else {
        return ResourceManager::instance()->generateUniqueUri(QString());
    }
}


Nepomuk::PimoModel::PimoModel( Soprano::Model* parentModel )
    : RdfSchemaModel( parentModel ),
      d( new Private(this) )
{
}


Nepomuk::PimoModel::~PimoModel()
{
    delete d;
}


QUrl Nepomuk::PimoModel::pimoContext()
{
    //
    // TODO: so far we have only one local PIMO, thus there is no real need
    // to relate any user information to it. At some point, however, this
    // might become important, for example once we share data.
    //
    if ( !d->pimoContext.isValid() ) {
        Soprano::QueryResultIterator it
            = executeQuery( QString::fromLatin1("select ?c ?onto where { "
                                                "?c a <%1> . "
                                                "OPTIONAL { "
                                                "?c a ?onto . "
                                                "FILTER(?onto = <%2>) . } }")
                            .arg( Vocabulary::PIMO::PersonalInformationModel().toString() )
                            .arg( Soprano::Vocabulary::NRL::Ontology().toString() ),
                            Soprano::Query::QueryLanguageSparql );
        if ( it.next() ) {
            d->pimoContext = it.binding(0).uri();
            if ( !it.binding( 1 ).isValid() ) {
                it.close();
                addStatement( d->pimoContext,
                              Soprano::Vocabulary::RDF::type(),
                              Soprano::Vocabulary::NRL::Ontology(),
                              d->pimoContext );
            }
        }
        else {
            it.close();
            d->pimoContext = ResourceManager::instance()->generateUniqueUri(QString());
            addStatement( d->pimoContext,
                          Soprano::Vocabulary::RDF::type(),
                          Vocabulary::PIMO::PersonalInformationModel(),
                          d->pimoContext );
            addStatement( d->pimoContext,
                          Soprano::Vocabulary::RDF::type(),
                          Soprano::Vocabulary::NRL::Ontology(),
                          d->pimoContext );
        }
    }

    return d->pimoContext;
}


QUrl Nepomuk::PimoModel::newClassUri( const QString& name )
{
    return d->createUniqueUri( name );
}


QUrl Nepomuk::PimoModel::newPropertyUri( const QString& name )
{
    return d->createUniqueUri( name );
}


Soprano::Error::ErrorCode Nepomuk::PimoModel::addPimoStatements( const QList<Soprano::Statement>& sl )
{
    QUrl c = pimoContext();
    QList<Soprano::Statement> newSl;
    foreach( Soprano::Statement s, sl ) {
        s.setContext( c );
        newSl << s;
    }
    return addStatements( newSl );
}


QUrl Nepomuk::PimoModel::createClass( const QUrl& parentClassUri,
                                      const QString& label,
                                      const QString& comment,
                                      const QString& icon )
{
    if ( label.isEmpty() ) {
        setError( QLatin1String("No label for new class set.") );
        return QUrl();
    }

    Types::Class parentClass( parentClassUri );
    if( parentClassUri != Vocabulary::PIMO::Thing() &&
        !parentClass.isSubClassOf( Vocabulary::PIMO::Thing() ) ) {
        setError( QLatin1String("New PIMO class needs to be subclass of pimo:Thing.") );
        return QUrl();
    }

    QUrl classUri = newClassUri( label );

    QList<Soprano::Statement> sl;
    sl << Soprano::Statement( classUri, Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::RDFS::Class() )
       << Soprano::Statement( classUri, Soprano::Vocabulary::RDFS::subClassOf(), parentClassUri )
       << Soprano::Statement( classUri, Soprano::Vocabulary::RDFS::label(), Soprano::LiteralValue( label ) )
       << Soprano::Statement( classUri, Soprano::Vocabulary::NAO::created(), Soprano::LiteralValue( QDateTime::currentDateTime() ) );
    if ( !comment.isEmpty() ) {
        sl << Soprano::Statement( classUri, Soprano::Vocabulary::RDFS::comment(), Soprano::LiteralValue( comment ) );
    }
    if ( !icon.isEmpty() ) {
        // FIXME: create a proper Symbol object, if possible maybe a subclass DesktopIcon if its a standard icon
        sl << Soprano::Statement( classUri, Soprano::Vocabulary::NAO::hasSymbol(), Soprano::LiteralValue( icon ) );
    }

    if( addPimoStatements( sl ) == Soprano::Error::ErrorNone ) {
        return classUri;
    }
    else {
        return QUrl();
    }
}


bool Nepomuk::PimoModel::createSubClassRelation( const QUrl& classUri, const QUrl& newParentClassUri, bool singleParent )
{
    if ( classUri == newParentClassUri ) {
        setError( QLatin1String("Cannot make a class sub class of itself") );
        return false;
    }

    // the only way we have at the moment to distiguish between PIMO classes and user created ones is the
    // presence of nao:created
    Types::Class type( classUri );
    if( classUri != Vocabulary::PIMO::Thing() &&
        !type.isSubClassOf( Vocabulary::PIMO::Thing() ) &&
        !containsAnyStatement( classUri, Soprano::Vocabulary::NAO::created(), Soprano::Node() ) ) {
        setError( QLatin1String("Only pimo:Thing subclasses created by the user can be changed.") );
        return false;
    }

    Types::Class parentType( newParentClassUri );
    if( !parentType.isValid() ) {
        setError( QString::fromLatin1("Non-existing classes cannot be used as parents (%1)" ).arg( newParentClassUri.toString() ) );
        return false;
    }

    if ( parentType.isSubClassOf( type ) ) {
        setError( QLatin1String("Cannot create subclass relation loop.") );
        return false;
    }

    if ( singleParent ) {
        removeAllStatements( classUri, Soprano::Vocabulary::RDFS::subClassOf(), Soprano::Node() );
    }

    return addPimoStatements( QList<Statement>() << Statement( classUri, Soprano::Vocabulary::RDFS::subClassOf(), newParentClassUri ) ) == Error::ErrorNone;
}


QUrl Nepomuk::PimoModel::createProperty ( const QUrl& domainUri,
                                          const QUrl& range,
                                          const QString label,
                                          const QString& comment,
                                          const QString& icon )
{
    if ( !range.isValid() ) {
        setError( QLatin1String("Invalid range") );
        return QUrl();
    }
    if ( label.isEmpty() ) {
        setError( QLatin1String("No label for new class set.") );
        return QUrl();
    }

    Types::Class domain( domainUri );
    if( domainUri != Vocabulary::PIMO::Thing() &&
        !domain.isSubClassOf( Vocabulary::PIMO::Thing() ) ) {
        setError( QLatin1String("New PIMO properties need to have a pimo:Thing related domain.") );
        return QUrl();
    }

    QUrl propertyUri = newPropertyUri( label );

    QList<Soprano::Statement> sl;
    sl << Soprano::Statement( propertyUri, Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::RDF::Property() )
       << Soprano::Statement( propertyUri, Soprano::Vocabulary::RDFS::subPropertyOf(), Vocabulary::PIMO::isRelated() )
       << Soprano::Statement( propertyUri, Soprano::Vocabulary::RDFS::domain(), domainUri )
       << Soprano::Statement( propertyUri, Soprano::Vocabulary::RDFS::range(), range )
       << Soprano::Statement( propertyUri, Soprano::Vocabulary::RDFS::label(), Soprano::LiteralValue( label ) )
       << Soprano::Statement( propertyUri, Soprano::Vocabulary::NAO::created(), Soprano::LiteralValue( QDateTime::currentDateTime() ) );
    if ( !comment.isEmpty() ) {
        sl << Soprano::Statement( propertyUri, Soprano::Vocabulary::RDFS::comment(), Soprano::LiteralValue( comment ) );
    }
    if ( !icon.isEmpty() ) {
        // FIXME: create a proper Symbol object, if possible maybe a subclass DesktopIcon if its a standard icon
        sl << Soprano::Statement( propertyUri, Soprano::Vocabulary::NAO::hasSymbol(), Soprano::LiteralValue( icon ) );
    }

    if( addPimoStatements( sl ) == Soprano::Error::ErrorNone ) {
        return propertyUri;
    }
    else {
        return QUrl();
    }
}


QUrl Nepomuk::PimoModel::createThing( const QUrl& typeUri,
                                      const QString& label,
                                      const QString& comment,
                                      const QString& icon )
{
    Types::Class type( typeUri );
    if( typeUri != Vocabulary::PIMO::Thing() &&
        !type.isSubClassOf( Vocabulary::PIMO::Thing() ) ) {
        setError( QLatin1String("New PIMO resources need to have a pimo:Thing related type.") );
        return QUrl();
    }

    if ( label.isEmpty() ) {
        setError( QLatin1String("No label for new resource set.") );
        return QUrl();
    }

    Nepomuk::Thing thing( QUrl(), typeUri );
    thing.setLabel( label );
    if ( !comment.isEmpty() ) {
        thing.setDescription( comment );
    }
    if ( !icon.isEmpty() ) {
        // FIXME: create a proper Symbol object, if possible maybe a subclass DesktopIcon if its a standard icon
        thing.addSymbol( icon );
    }

    return thing.resourceUri();
}

// void Nepomuk::PimoModel::findMatches( const QString& text )
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
