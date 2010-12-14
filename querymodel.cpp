/*
   Copyright (c) 2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2011 Vishesh Handa <handa.vish@gmail.com>

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

#include <QtCore/QTime>

#include <Nepomuk/ResourceManager>

#include <Soprano/Model>
#include <Soprano/Node>
#include <Soprano/QueryResultIterator>
#include <Soprano/Util/AsyncQuery>

#define USING_SOPRANO_NRLMODEL_UNSTABLE_API
#include <Soprano/NRLModel>

#include <KDebug>


namespace {
void splitUri( const QUrl& uri, QUrl& ns, QString& name )
{
    const QString uriStr = uri.toString();
    const int i = uriStr.lastIndexOf( QRegExp(QLatin1String("[/#]")) ) + 1;
    ns = uriStr.left( i );
    name = uriStr.mid( i );
}
}

class Nepomuk::QueryModel::Private
{
public:
    Private( Nepomuk::QueryModel * parent );
    
    Nepomuk::QueryModel * q;
    
    QString m_query;
    QList<Soprano::BindingSet> m_bindings;
    int m_queryTime;
    QTime m_queryTimer;

    QHash<QUrl, QString> m_bnames;

    void updateQuery();
    QString resourceToString( const QUrl& uri ) const;
};


Nepomuk::QueryModel::Private::Private(Nepomuk::QueryModel* parent)
    : q( parent )
{
}


void Nepomuk::QueryModel::Private::updateQuery()
{
    m_bindings.clear();

    if( !m_query.isEmpty() ) {
        Soprano::Model* model = ResourceManager::instance()->mainModel();
        m_queryTimer.start();
        Soprano::Util::AsyncQuery* asynQuery = Soprano::Util::AsyncQuery::executeQuery( model, m_query, Soprano::Query::QueryLanguageSparql );
        connect( asynQuery, SIGNAL(nextReady(Soprano::Util::AsyncQuery*)),
                 q, SLOT(slotNextResultReady(Soprano::Util::AsyncQuery*)) );
        connect( asynQuery, SIGNAL(finished(Soprano::Util::AsyncQuery*)),
                 q, SLOT(slotQueryFinished(Soprano::Util::AsyncQuery*)) );
    }
    
    return;
}


QString Nepomuk::QueryModel::Private::resourceToString(const QUrl &uri) const
{
    QUrl ns;
    QString name;
    splitUri( uri, ns, name );
    QHash<QUrl, QString>::const_iterator it = m_bnames.constFind(ns);
    if( it != m_bnames.constEnd() ) {
        return QString::fromLatin1("%1:%2").arg( it.value(), name );
    }
    else {
        return uri.toString();
    }
}


Nepomuk::QueryModel::QueryModel( QObject* parent )
    : QAbstractTableModel( parent ),
      d(new Private( this ))
{
    Soprano::NRLModel nrlModel( ResourceManager::instance()->mainModel() );
    nrlModel.setEnableQueryPrefixExpansion( true );
    QHash<QString, QUrl> queryPrefixes = nrlModel.queryPrefixes();
    for( QHash<QString, QUrl>::const_iterator it = queryPrefixes.constBegin();
         it != queryPrefixes.constEnd(); ++it ) {
        d->m_bnames.insert( it.value(), it.key() );
    }
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
        index.row() < d->m_bindings.count() ) {
        const Soprano::Node node = d->m_bindings[index.row()][index.column()];
            switch( role ) {
            case Qt::DisplayRole:
                if( node.isResource() ) {
                    return d->resourceToString(node.uri());
                }
                else {
                    return node.toString();
                }

            case Qt::ToolTipRole:
                return node.toString();
            }
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

void Nepomuk::QueryModel::slotNextResultReady(Soprano::Util::AsyncQuery* query)
{
    //FIXME: emit the rowsAboutToBeInserted() signal
    emit layoutAboutToBeChanged();
    if( query->isBool() ) {
        Soprano::BindingSet set;
        set.insert( QLatin1String( "result" ), Soprano::LiteralValue( query->boolValue() ) );
        d->m_bindings.append( set );
    }
    else if ( query->isGraph() ) {
        query->next();
        
        const Soprano::Statement s = query->currentStatement();
        Soprano::BindingSet set;
        set.insert( QLatin1String( "subject" ), s.subject() );
        set.insert( QLatin1String( "predicate" ), s.predicate() );
        set.insert( QLatin1String( "object" ), s.object() );
        set.insert( QLatin1String( "context" ), s.context() );
        d->m_bindings << set;
    }
    else {
        query->next();
        d->m_bindings << query->currentBindings();
    }
    emit layoutChanged();
}

void Nepomuk::QueryModel::slotQueryFinished(Soprano::Util::AsyncQuery* query)
{
    //FIXME: This doesn't work!
    Soprano::Model* model = ResourceManager::instance()->mainModel();
    kDebug() << model->lastError().message();
    kDebug() << query->lastError().message();
    if( query->lastError() != Soprano::Error::ErrorNone )
        emit queryError( query->lastError() );
    
    d->m_queryTime = d->m_queryTimer.elapsed();
    emit queryFinished();
}

int Nepomuk::QueryModel::queryTime() const
{
    return d->m_queryTime;
}


#include "querymodel.moc"
