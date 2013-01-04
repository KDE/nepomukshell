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

#include <nepomuk2/resource.h>
#include <nepomuk2/resourcemanager.h>
#include <nepomuk2/class.h>
#include <nepomuk2/property.h>
#include <nepomuk2/variant.h>
#include <nepomuk2/literal.h>
#include <nepomuk2/nie.h>

#include <KDebug>

#include <QtGui/QFont>
#include <QtGui/QIcon>
#include <QtCore/QPair>
#include <QtCore/QList>

#include <Soprano/Statement>
#include <Soprano/Node>
#include <Soprano/LiteralValue>
#include <Soprano/Model>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/QueryResultIterator>


Q_DECLARE_METATYPE( Nepomuk2::Types::Property )
Q_DECLARE_METATYPE( Nepomuk2::Variant )


class Nepomuk2::ResourcePropertyEditModel::Private
{
public:
    Resource m_resource;
    ResourcePropertyEditModel::Mode m_mode;

    QList<QPair<Soprano::Statement, QDateTime> > m_properties;

    void rebuild();
};


void Nepomuk2::ResourcePropertyEditModel::Private::rebuild()
{
    m_properties.clear();

    // we get all statements + their creation date
    QString query;
    if ( m_mode == ResourcePropertyEditModel::PropertiesMode ) {
        query = QString::fromLatin1("select ?p ?o ?g ?d where { graph ?g { %1 ?p ?o } . OPTIONAL { ?g %2 ?d . } }")
                .arg(Soprano::Node::resourceToN3(m_resource.uri()),
                     Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::created()));
    }
    else {
        query = QString::fromLatin1("select ?s ?p ?g ?d where { graph ?g { ?s ?p %1 } . OPTIONAL { ?g %2 ?d . } }")
                .arg(Soprano::Node::resourceToN3(m_resource.uri()),
                     Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::created()));
    }

    Soprano::QueryResultIterator it = ResourceManager::instance()->mainModel()->executeQuery( query, Soprano::Query::QueryLanguageSparql );
    while( it.next() ) {
        Soprano::Statement s;
        if ( m_mode == ResourcePropertyEditModel::PropertiesMode )
            s = Soprano::Statement( m_resource.uri(), it[QLatin1String("p")], it[QLatin1String("o")], it[QLatin1String("g")] );
        else
            s = Soprano::Statement( it[QLatin1String("s")], it[QLatin1String("p")], m_resource.uri(), it[QLatin1String("g")] );
        
        // Make sure the string is not too long
        if( s.object().literal().isString() ) {
            QString string = s.object().literal().variant().toString().mid(0, 50);
            s.setObject( Soprano::LiteralValue( string ) );
        }
        m_properties.append( qMakePair( s, it[QLatin1String("d")].literal().toDateTime() ) );
    }
}


Nepomuk2::ResourcePropertyEditModel::ResourcePropertyEditModel( QObject* parent )
    : QAbstractTableModel( parent ),
      d( new Private() )
{
    d->m_mode = PropertiesMode;
}


Nepomuk2::ResourcePropertyEditModel::~ResourcePropertyEditModel()
{
    delete d;
}


void Nepomuk2::ResourcePropertyEditModel::setMode( Mode mode )
{
    if ( d->m_mode != mode ) {
        d->m_mode = mode;
        d->rebuild();
        reset();
    }
}


Nepomuk2::ResourcePropertyEditModel::Mode Nepomuk2::ResourcePropertyEditModel::mode() const
{
    return d->m_mode;
}


Nepomuk2::Resource Nepomuk2::ResourcePropertyEditModel::resource() const
{
    return d->m_resource;
}


void Nepomuk2::ResourcePropertyEditModel::setResource( const Nepomuk2::Resource& resource )
{
    d->m_resource = resource;
    d->rebuild();
    reset();
}


int Nepomuk2::ResourcePropertyEditModel::columnCount( const QModelIndex& ) const
{
    return 3;
}


int Nepomuk2::ResourcePropertyEditModel::rowCount( const QModelIndex& index ) const
{
    Q_UNUSED(index);
    return d->m_properties.count();
}


QVariant Nepomuk2::ResourcePropertyEditModel::data( const QModelIndex& index, int role ) const
{
    if ( index.isValid() and index.row() < d->m_properties.count() ) {
        const Nepomuk2::Types::Property property = d->m_properties[index.row()].first.predicate().uri();
        const Soprano::Node value = nodeForIndex( index );
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
                return QString(QLatin1String( "<p>" ) + property.comment() + QLatin1String( "<br><i>" ) + property.uri().toString() + QLatin1String( "</i>" ));
            }

        case 1:
            switch( role ) {
            case Qt::DisplayRole:
                if( value.isResource() ) {
                    Resource res = Resource::fromResourceUri( value.uri() );
                    if ( property == Nepomuk2::Vocabulary::NIE::url() ) {
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
                    return QVariant::fromValue(Nepomuk2::Resource(value.uri()));
                else
                    return value.literal().variant();

            case Qt::DecorationRole:
                if( value.isResource() )
                    return Nepomuk2::Resource(value.uri()).genericIcon();
                break;

            case Qt::ToolTipRole:
                return value.toString();
            }

        case 2:
            if( role == Qt::DisplayRole ) {
                if( date.isValid() )
                    return date;
                else
                    return i18nc("@item refers to an unknown date", "Unknown");
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


QVariant Nepomuk2::ResourcePropertyEditModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( orientation == Qt::Horizontal &&
        role == Qt::DisplayRole ) {
        switch( section ) {
        case 0:
            return i18nc("@title:column The RDF property of a triple", "Property");
        case 1:
            return i18nc("@title:column The object value of an RDF triple", "Value");
        case 2:
            return i18nc("@title:column The creation date of an RDF triple", "Creation Date");
        }
    }

    return QAbstractTableModel::headerData( section, orientation, role );
}


bool Nepomuk2::ResourcePropertyEditModel::removeRows( int row, int count, const QModelIndex& parent )
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


bool Nepomuk2::ResourcePropertyEditModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    kDebug() << index << value << role;

    if ( d->m_resource.isValid() ) {
        if ( index.isValid() ) {
            const Nepomuk2::Types::Property property = d->m_properties[index.row()].first.predicate().uri();
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


Soprano::Node Nepomuk2::ResourcePropertyEditModel::nodeForIndex( const QModelIndex& index ) const
{
    if( index.isValid() &&
        index.row() < d->m_properties.count() ) {
        switch( index.column() ) {
        case 0:
            return d->m_properties[index.row()].first.predicate();
        case 1:
            if ( d->m_mode == PropertiesMode )
                return d->m_properties[index.row()].first.object();
            else
                return d->m_properties[index.row()].first.subject();
        case 2:
            return Soprano::LiteralValue( d->m_properties[index.row()].second );
        }
    }

    return Soprano::Node();
}


QModelIndex Nepomuk2::ResourcePropertyEditModel::parent( const QModelIndex& index ) const
{
    Q_UNUSED(index);
    return QModelIndex();
}


Qt::ItemFlags Nepomuk2::ResourcePropertyEditModel::flags( const QModelIndex& index ) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags( index );
    // FIXME: implement resource range editing
    if( index.column() == 1 && d->m_properties[index.row()].first.object().isLiteral() )
        flags |= Qt::ItemIsEditable;
    return flags;
}

#include "resourcepropertymodel.moc"
