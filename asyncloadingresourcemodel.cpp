/*
   Copyright (C) 2009 by Sebastian Trueg <trueg at kde.org>

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

#include "asyncloadingresourcemodel.h"
#include "nepomukshellsettings.h"

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Types/Class>

#include <Soprano/Model>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Util/AsyncQuery>

#include <KDebug>


namespace {
    const int s_queryLimit = 50;
}


class Nepomuk::AsyncLoadingResourceModel::Private
{
public:
    Private()
        : m_currentQuery( 0 ) {
    }

    AsyncLoadingResourceModel* q;
    Soprano::Util::AsyncQuery* m_currentQuery;

    int m_lastOffset;
    int m_lastCount;

    Types::Class m_type;

    void query();
    void _k_queryNextReady( Soprano::Util::AsyncQuery* );
    void _k_queryFinished( Soprano::Util::AsyncQuery* );
};


void Nepomuk::AsyncLoadingResourceModel::Private::query()
{
    m_lastCount = 0;
    QString query;
    if( Settings::self()->recursiveQuery() ) {
        query = QString::fromLatin1( "select distinct ?r where { "
                                     "?r a ?t . ?t %4 %1 . "
                                     "} OFFSET %2 LIMIT %3" )
                .arg( Soprano::Node::resourceToN3( m_type.uri() ) )
                .arg( m_lastOffset )
                .arg( s_queryLimit )
                .arg( Soprano::Node::resourceToN3( Soprano::Vocabulary::RDFS::subClassOf() ) );
    }
    else {
        query = QString::fromLatin1( "select distinct ?r where { "
                                     "?r a %1 . "
                                     "} OFFSET %2 LIMIT %3" )
                .arg( Soprano::Node::resourceToN3( m_type.uri() ) )
                .arg( m_lastOffset )
                .arg( s_queryLimit );
    }

    m_currentQuery = Soprano::Util::AsyncQuery::executeQuery( ResourceManager::instance()->mainModel(),
                                                              query,
                                                              Soprano::Query::QueryLanguageSparql );
    q->connect( m_currentQuery, SIGNAL( nextReady( Soprano::Util::AsyncQuery* ) ),
                SLOT( _k_queryNextReady( Soprano::Util::AsyncQuery* ) ) );
    q->connect( m_currentQuery, SIGNAL( finished( Soprano::Util::AsyncQuery* ) ),
                SLOT( _k_queryFinished( Soprano::Util::AsyncQuery* ) ) );
}


void Nepomuk::AsyncLoadingResourceModel::Private::_k_queryNextReady( Soprano::Util::AsyncQuery* query )
{
    ++m_lastCount;
    Soprano::Node r = query->binding( "r" );
    q->addResource( r.uri() );
    query->next();
}


void Nepomuk::AsyncLoadingResourceModel::Private::_k_queryFinished( Soprano::Util::AsyncQuery* )
{
    m_currentQuery = 0;
    if ( m_lastCount < s_queryLimit ) {
        emit q->finishedLoading();
    }
    else {
        m_lastOffset += s_queryLimit;
        query();
    }
}


Nepomuk::AsyncLoadingResourceModel::AsyncLoadingResourceModel( QObject* parent )
    : SimpleResourceModel( parent ),
      d( new Private() )
{
    d->q = this;
}


Nepomuk::AsyncLoadingResourceModel::~AsyncLoadingResourceModel()
{
    delete d->m_currentQuery;
    delete d;
}


void Nepomuk::AsyncLoadingResourceModel::loadResourcesOfType( const Nepomuk::Types::Class& type )
{
    delete d->m_currentQuery;
    d->m_currentQuery = 0;
    d->m_lastCount = 0;
    d->m_lastOffset = 0;
    d->m_type = type;

    clear();

    d->query();
}

#include "asyncloadingresourcemodel.moc"
