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

#include "resourcepropertymodel.h"

#include <nepomuk/resource.h>
#include <nepomuk/class.h>
#include <nepomuk/property.h>
#include <nepomuk/variant.h>
#include <nepomuk/literal.h>

#include <KIcon>
#include <KDebug>
#include <KMessageBox>

#include <QtGui/QFont>

Q_DECLARE_METATYPE( Nepomuk::Types::Property )
Q_DECLARE_METATYPE( Nepomuk::Variant )

class Node
{
public:
    Node( int r )
        : row( r ) {
    }
    virtual ~Node() {
    }

    int row;
};


class TypeNode;

class PropertyNode : public Node
{
public:
    PropertyNode( const Nepomuk::Types::Property& p, TypeNode* parentNode, int row = 0 )
        : Node( row ),
          property( p ),
          typeNode( parentNode ) {
    }
    ~PropertyNode();

    Nepomuk::Types::Property property;
    Soprano::Node value;

    TypeNode* typeNode;
};


class TypeNode : public Node
{
public:
    TypeNode( const Nepomuk::Types::Class& t,  int row = 0 )
        : Node( row ),
          type( t ),
          initialized( false ) {
    }
    ~TypeNode() {
        qDeleteAll( propertyNodes );
    }

    Nepomuk::Types::Class type;

    QList<PropertyNode*> propertyNodes;

    bool initialized;

    void updateProperties();
};


void TypeNode::updateProperties()
{
    qDeleteAll( propertyNodes );
    propertyNodes.clear();
    int i = 0;
    foreach( Nepomuk::Types::Property p, type.domainOf() ) {
        propertyNodes.append( new PropertyNode( p, this, i++ ) );
    }
}

PropertyNode::~PropertyNode()
{
}



class Nepomuk::ResourcePropertyEditModel::Private
{
public:
    Resource resource;
    QList<Types::Class> types;
    QList<TypeNode*> typeNodes;

    void updateTypeNodes();
};


void Nepomuk::ResourcePropertyEditModel::Private::updateTypeNodes()
{
    qDeleteAll( typeNodes );
    typeNodes.clear();
    QSet<Types::Class> cl;
    foreach( Types::Class t, types ) {
        cl += t.allParentClasses().toSet();
        cl += t;
    }
    int i = 0;
    foreach( Types::Class t, cl ) {
        typeNodes.append( new TypeNode( t, i++ ) );
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
    return d->resource;
}


void Nepomuk::ResourcePropertyEditModel::setTypes( const QList<Types::Class>& types )
{
    d->types = types;
    d->updateTypeNodes();
    reset();
}


void Nepomuk::ResourcePropertyEditModel::setResource( const Nepomuk::Resource& resource )
{
    d->resource = resource;
    d->types.clear();
    foreach( QUrl type, resource.types() ) {
        d->types.append( type );
    }
    d->updateTypeNodes();
    reset();
}


void Nepomuk::ResourcePropertyEditModel::clear()
{
    d->resource = Resource();
    d->types.clear();
    d->updateTypeNodes();
    reset();
}


int Nepomuk::ResourcePropertyEditModel::columnCount( const QModelIndex& ) const
{
    return 2;
}


int Nepomuk::ResourcePropertyEditModel::rowCount( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        Node* node = static_cast<Node*>( index.internalPointer() );
        Q_ASSERT( node );

        if ( TypeNode* typeNode = dynamic_cast<TypeNode*>( node ) ) {
            return typeNode->type.domainOf().count();
        }
    }
    else {
        return d->typeNodes.count();
    }

    return 0;
}


QVariant Nepomuk::ResourcePropertyEditModel::data( const QModelIndex& index, int role ) const
{
    if ( index.isValid() ) {
        Node* node = static_cast<Node*>( index.internalPointer() );
        Q_ASSERT( node );

        if ( TypeNode* typeNode = dynamic_cast<TypeNode*>( node ) ) {
            if ( index.column() == 0 ) {
                switch( role ) {
                case Qt::DisplayRole:
                    return typeNode->type.label();

                case Qt::DecorationRole:
                    return typeNode->type.icon();

                case Qt::ToolTipRole:
                    return "<p>" + typeNode->type.comment() + "<br><i>" + typeNode->type.uri().toString() + "</i>";

                case Qt::FontRole: {
                    QFont fnt;
                    fnt.setBold( true );
                    return fnt;
                }
                }
            }
        }
        else {
            PropertyNode* propertyNode = static_cast<PropertyNode*>( node );
            switch( role ) {
            case Qt::DisplayRole:
                if ( index.column() == 0 ) {
                    return propertyNode->property.label();
                }
                else {
                    return d->resource.property( propertyNode->property.uri() ).toString();
                }

            case Qt::DecorationRole:
                return propertyNode->property.icon();

            case Qt::ToolTipRole:
                if ( index.column() == 0 ) {
                    return "<p>" + propertyNode->property.comment() + "<br><i>" + propertyNode->property.uri().toString() + "</i>";
                }

            case Qt::EditRole:
                if ( index.column() == 1 ) {
                    // FIXME: handle lists
                    Variant v = d->resource.property( propertyNode->property.uri() );
                    if ( v.isValid() ) {
                        // the actual data contains the data type
                        return v.variant();
                    }
                    else {
                        // just inform the delegate about the type we want to edit
                        // FIXME: handle URIs
                        return QVariant( propertyNode->property.literalRangeType().dataType() );
                    }
                }

            case PropertyRole:
                return QVariant::fromValue( propertyNode->property );

            case ValueRole:
                return QVariant::fromValue( d->resource.property( propertyNode->property.uri() ) );
            }
        }
    }

    return QVariant();
}


bool Nepomuk::ResourcePropertyEditModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    kDebug() << index << value << role;

    // FIXME: handle lists

    if ( d->resource.isValid() ) {
        if ( index.isValid() ) {
            Node* node = static_cast<Node*>( index.internalPointer() );
            Q_ASSERT( node );

            if ( PropertyNode* propertyNode = dynamic_cast<PropertyNode*>( node ) ) {

                // Do we have a class range
                if ( propertyNode->property.range().isValid() ) {
                    kDebug() << propertyNode->property.uri() << "is resource property with range" << propertyNode->property.range().uri();

                    QUrl uri = value.toUrl();
                    if ( !uri.isValid() ) {
                        // fallback for simple string editors
                        uri = value.toString();
                    }

                    if ( uri.isValid() ) {
                        // ensure that the resource has the proper type
                        Resource res( uri );
                        if ( res.hasType( propertyNode->property.range().uri() ) ) {
                            if ( propertyNode->property.maxCardinality() == 1 ||
                                 !d->resource.hasProperty( propertyNode->property.uri() ) ||
                                 KMessageBox::questionYesNo( 0,
                                                             i18n( "Property %1 has a cardinality greater than 1. "
                                                                   "Should the value be set or added to the list?", propertyNode->property.label() ),
                                                             i18n( "Property change" ),
                                                             KGuiItem( i18n( "Overwrite" ) ),
                                                             KGuiItem( i18n( "Add value" ) ) ) == KMessageBox::Yes ) {
                                kDebug() << "Setting resource value for property" << propertyNode->property.uri() << "to" << uri;
                                d->resource.setProperty( propertyNode->property.uri(), res );
                            }
                            else {
                                kDebug() << "Adding resource value for property" << propertyNode->property.uri() << ":" << uri;
                                Variant v = d->resource.property( propertyNode->property.uri() );
                                v.append( res );
                                d->resource.setProperty( propertyNode->property.uri(), v );
                            }
                            emit dataChanged( index, index );
                            return true;
                        }
                        else {
                            kDebug() << uri << "has invalid type for property" << propertyNode->property;
                            return false;
                        }
                    }
                    else if ( value.type() == QVariant::Url || value.type() == QVariant::Invalid ) {
                        kDebug() << "Removing property" << propertyNode->property.uri();
                        // clear property
                        d->resource.removeProperty( propertyNode->property.uri() );
                        emit dataChanged( index, index );
                        return true;
                    }
                }

                // looks like a literal range
                else {
                    kDebug() << "Setting literal value for property" << propertyNode->property.uri() << "to" << value;
                    // FIXME: make sure we have the proper type
                    d->resource.setProperty( propertyNode->property.uri(), Variant( value ) );
                    emit dataChanged( index, index );
                    return true;
                }
            }
            else {
                // this should never happen since type nodes are not editable
                return false;
            }
        }
        else {
            kDebug() << "Cannot edit invalid index.";
            return false;
        }
    }
    else {
        kDebug() << "Cannot change data with an invalid resource.";
        return false;
    }

    // shut up GCC
    return false;
}


QModelIndex Nepomuk::ResourcePropertyEditModel::index( int row, int column, const QModelIndex& parent ) const
{
    // parent elements: the types as a more fancy header
    // then the properties assigned to the types as children
    if ( parent.isValid() ) {
        TypeNode* parentNode = static_cast<TypeNode*>( parent.internalPointer() );
        Q_ASSERT( parentNode );

        if ( !parentNode->initialized ) {
            parentNode->updateProperties();
            parentNode->initialized = true;
        }

        if ( row >= 0 && row < parentNode->propertyNodes.count() ) {
            return createIndex( row, column, parentNode->propertyNodes[row] );
        }
    }
    else if ( row >= 0 && row < d->typeNodes.count() ) {
        return createIndex( row, column, d->typeNodes[row] );
    }

    return QModelIndex();
}


QModelIndex Nepomuk::ResourcePropertyEditModel::parent( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        Node* node = static_cast<Node*>( index.internalPointer() );
        Q_ASSERT( node );

        if ( PropertyNode* propertyNode = dynamic_cast<PropertyNode*>( node ) ) {
            return createIndex( propertyNode->typeNode->row, 0, propertyNode->typeNode );
        }
    }

    return QModelIndex();
}


Qt::ItemFlags Nepomuk::ResourcePropertyEditModel::flags( const QModelIndex& index ) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags( index );

    if ( index.isValid() ) {
        Node* node = static_cast<Node*>( index.internalPointer() );
        Q_ASSERT( node );

        if ( dynamic_cast<PropertyNode*>( node ) && index.column() == 1 ) {
            flags |= Qt::ItemIsEditable;
        }
    }

    return flags;
}

#include "resourcepropertymodel.moc"
