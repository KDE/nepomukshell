/*
   Copyright (c) 2010 Sebastian Trueg <trueg@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "querymodel.h"

#include <Nepomuk/ResourceManager>

#include <Soprano/Model>
#include <Soprano/Node>
#include <Soprano/QueryResultIterator>

#include <KDebug>

// FIXME: load not everything in one go using fetchMore and friends

class Nepomuk::QueryModel::Private
{
public:
    QString m_query;
    QList<Soprano::BindingSet> m_bindings;

    void updateQuery();
};


void Nepomuk::QueryModel::Private::updateQuery()
{
    m_bindings.clear();

    if( !m_query.isEmpty() ) {
        Soprano::QueryResultIterator it = ResourceManager::instance()->mainModel()->executeQuery( m_query, Soprano::Query::QueryLanguageSparql );
        if ( it.isBool() ) {
            Soprano::BindingSet set;
            set.insert( QLatin1String( "result" ), Soprano::LiteralValue( it.boolValue() ) );
            m_bindings.append( set );
        }
        else if ( it.isGraph() ) {
            while ( it.next() ) {
                const Soprano::Statement s = it.currentStatement();
                Soprano::BindingSet set;
                set.insert( QLatin1String( "subject" ), s.subject() );
                set.insert( QLatin1String( "predicate" ), s.predicate() );
                set.insert( QLatin1String( "object" ), s.object() );
                set.insert( QLatin1String( "context" ), s.context() );
                m_bindings << set;
            }
        }
        else {
            m_bindings = it.allBindings();
        }
    }
}


Nepomuk::QueryModel::QueryModel( QObject* parent )
    : QAbstractTableModel( parent ),
      d(new Private())
{
}


Nepomuk::QueryModel::~QueryModel()
{
    delete d;
}


int Nepomuk::QueryModel::columnCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent);
    if( d->m_bindings.isEmpty() )
        return 0;
    else
        return d->m_bindings.first().count();
}


int Nepomuk::QueryModel::rowCount( const QModelIndex& parent ) const
{
    if( !parent.isValid() )
        return d->m_bindings.count();
    else
        return 0;
}


QVariant Nepomuk::QueryModel::data( const QModelIndex& index, int role ) const
{
    if( index.isValid() &&
        index.row() < d->m_bindings.count() &&
        role == Qt::DisplayRole ) {
        return d->m_bindings[index.row()][index.column()].toString();
    }

    return QVariant();
}


QModelIndex Nepomuk::QueryModel::parent( const QModelIndex& index ) const
{
    Q_UNUSED(index);
    return QModelIndex();
}


Qt::ItemFlags Nepomuk::QueryModel::flags( const QModelIndex& index ) const
{
    return QAbstractTableModel::flags( index );
}


QVariant Nepomuk::QueryModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( !d->m_bindings.isEmpty() &&
        section < d->m_bindings.first().count() &&
        orientation == Qt::Horizontal &&
        role == Qt::DisplayRole ) {
        return d->m_bindings.first().bindingNames()[section];
    }
    else {
        return QAbstractTableModel::headerData( section, orientation, role );
    }
}


void Nepomuk::QueryModel::setQuery( const QString& query )
{
    d->m_query = query;
    d->updateQuery();
    reset();
}


Soprano::Node Nepomuk::QueryModel::nodeForIndex( const QModelIndex& index ) const
{
    if( index.isValid() ) {
        return d->m_bindings[index.row()][index.column()];
    }
    return Soprano::Node();
}

#include "querymodel.moc"
