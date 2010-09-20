/*
   Copyright (C) 2008-2010 by Sebastian Trueg <trueg at kde.org>

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

#include "resourcepropertymodel.h"
#include "nie.h"

#include <nepomuk/resource.h>
#include <nepomuk/resourcemanager.h>
#include <nepomuk/class.h>
#include <nepomuk/property.h>
#include <nepomuk/variant.h>
#include <nepomuk/literal.h>

#include <KIcon>
#include <KDebug>
#include <KMessageBox>

#include <QtGui/QFont>
#include <QtCore/QPair>
#include <QtCore/QList>

#include <Soprano/Statement>
#include <Soprano/Node>
#include <Soprano/LiteralValue>
#include <Soprano/Model>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/QueryResultIterator>


Q_DECLARE_METATYPE( Nepomuk::Types::Property )
Q_DECLARE_METATYPE( Nepomuk::Variant )


class Nepomuk::ResourcePropertyEditModel::Private
{
public:
    Resource m_resource;
    QList<QPair<Soprano::Statement, QDateTime> > m_properties;

    void rebuild();
};


void Nepomuk::ResourcePropertyEditModel::Private::rebuild()
{
    m_properties.clear();

    // we get all statements + their creation date
    const QString query = QString::fromLatin1("select ?p ?o ?g ?d where { graph ?g { %1 ?p ?o } . OPTIONAL { ?g %2 ?d . } }")
                          .arg(Soprano::Node::resourceToN3(m_resource.resourceUri()),
                               Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::created()));
    Soprano::QueryResultIterator it = ResourceManager::instance()->mainModel()->executeQuery( query, Soprano::Query::QueryLanguageSparql );
    while( it.next() ) {
        m_properties.append( qMakePair( Soprano::Statement( m_resource.resourceUri(), it["p"], it["o"], it["g"] ),
                                        it["d"].literal().toDateTime() ) );
    }
}


Nepomuk::ResourcePropertyEditModel::ResourcePropertyEditModel( QObject* parent )
    : QAbstractTableModel( parent ),
      d( new Private() )
{
}


Nepomuk::ResourcePropertyEditModel::~ResourcePropertyEditModel()
{
    delete d;
}


Nepomuk::Resource Nepomuk::ResourcePropertyEditModel::resource() const
{
    return d->m_resource;
}


void Nepomuk::ResourcePropertyEditModel::setResource( const Nepomuk::Resource& resource )
{
    d->m_resource = resource;
    d->rebuild();
    reset();
}


int Nepomuk::ResourcePropertyEditModel::columnCount( const QModelIndex& ) const
{
    return 3;
}


int Nepomuk::ResourcePropertyEditModel::rowCount( const QModelIndex& index ) const
{
    return d->m_properties.count();
}


QVariant Nepomuk::ResourcePropertyEditModel::data( const QModelIndex& index, int role ) const
{
    if ( index.isValid() and index.row() < d->m_properties.count() ) {
        const Nepomuk::Types::Property property = d->m_properties[index.row()].first.predicate().uri();
        const Soprano::Node value = d->m_properties[index.row()].first.object();
        const QDateTime date = d->m_properties[index.row()].second;

        if( role == PropertyRole ) {
            return QVariant::fromValue( property );
        }

        switch( index.column() ) {
        case 0:
            switch( role ) {
            case Qt::DisplayRole:
                return property.label();

            case Qt::DecorationRole:
                return property.icon();

            case Qt::ToolTipRole:
                return "<p>" + property.comment() + "<br><i>" + property.uri().toString() + "</i>";
            }

        case 1:
            switch( role ) {
            case Qt::DisplayRole:
                if( value.isResource() ) {
                    Resource res = Resource::fromResourceUri( value.uri() );
                    if ( property == Nepomuk::Vocabulary::NIE::url() ) {
                        return KUrl( value.uri() ).prettyUrl();
                    }
                    else {
                        return res.genericLabel();
                    }
                }
                else
                    return value.literal().variant();

            case Qt::EditRole:
                if( value.isResource() )
                    return QVariant::fromValue(Nepomuk::Resource(value.uri()));
                else
                    return value.literal().variant();

            case Qt::DecorationRole:
                if( value.isResource() )
                    return Nepomuk::Resource(value.uri()).genericIcon();
                break;

            case Qt::ToolTipRole:
                return value.toString();
            }

        case 2:
            if( role == Qt::DisplayRole ) {
                if( date.isValid() )
                    return date;
                else
                    return i18n("Unknown");
            }
            else if( role == Qt::ToolTipRole ) {
                if( date.isValid() )
                    return Soprano::LiteralValue( date ).toString();
                else
                    return i18n("An invalid creation date means invalid Nepomuk data!");
            }
        }
    }

    return QVariant();
}


QVariant Nepomuk::ResourcePropertyEditModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( orientation == Qt::Horizontal &&
        role == Qt::DisplayRole ) {
        switch( section ) {
        case 0:
            return i18n("Property");
        case 1:
            return i18n("Value");
        case 2:
            return i18n("Creation Date");
        }
    }

    return QAbstractTableModel::headerData( section, orientation, role );
}


bool Nepomuk::ResourcePropertyEditModel::removeRows( int row, int count, const QModelIndex& parent )
{
    kDebug() << row << count;
    if( !parent.isValid() && row+count <= d->m_properties.count() ) {
        for( int i = row; i < row+count; ++i ) {
            kDebug() << "Deleting row" << row;
            // we do NOT use i since we are removing the row
            Soprano::Statement statement = d->m_properties[row].first;
            ResourceManager::instance()->mainModel()->removeStatement( statement );
            d->m_properties.removeAt( row );
        }
        return true;
    }

    return false;
}


bool Nepomuk::ResourcePropertyEditModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    kDebug() << index << value << role;

    if ( d->m_resource.isValid() ) {
        if ( index.isValid() ) {
            const Nepomuk::Types::Property property = d->m_properties[index.row()].first.predicate().uri();
            Soprano::Node newObjectNode;
            if( property.range().isValid() ) {
                newObjectNode = value.toUrl();
            }
            else {
                newObjectNode = Soprano::LiteralValue( value );
            }

            if( newObjectNode.isValid() ) {
                Soprano::Statement statement = d->m_properties[index.row()].first;
                ResourceManager::instance()->mainModel()->removeStatement( statement );
                statement.setObject( newObjectNode );
                ResourceManager::instance()->mainModel()->addStatement( statement );
                d->rebuild();
                emit dataChanged( index, index );
                return true;
            }
        }
    }

    return false;
}


Soprano::Node Nepomuk::ResourcePropertyEditModel::nodeForIndex( const QModelIndex& index ) const
{
    if( index.isValid() &&
        index.row() < d->m_properties.count() ) {
        switch( index.column() ) {
        case 0:
            return d->m_properties[index.row()].first.predicate();
        case 1:
            return d->m_properties[index.row()].first.object();
        case 2:
            return Soprano::LiteralValue( d->m_properties[index.row()].second );
        }
    }

    return Soprano::Node();
}


QModelIndex Nepomuk::ResourcePropertyEditModel::parent( const QModelIndex& index ) const
{
    return QModelIndex();
}


Qt::ItemFlags Nepomuk::ResourcePropertyEditModel::flags( const QModelIndex& index ) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags( index );
    // FIXME: implement resource range editing
    if( index.column() == 1 && d->m_properties[index.row()].first.object().isLiteral() )
        flags |= Qt::ItemIsEditable;
    return flags;
}

#include "resourcepropertymodel.moc"
