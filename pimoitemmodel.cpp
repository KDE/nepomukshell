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

#include "pimoitemmodel.h"
#include "pimo.h"

#include <nepomuk/ontology.h>

#include <KIcon>
#include <KDebug>


class ClassNode
{
public:
    ClassNode( const Nepomuk::Types::Class& t, int _row, ClassNode* parentNode = 0 )
        : parent( parentNode ),
          updating( false ),
          row( _row ),
          type( t ) {
    }

    ~ClassNode() {
        qDeleteAll( children );
    }

    ClassNode* getChild( int row ) {
        if ( row < children.count() ) {
            return children[row];
        }
        else {
            return 0;
        }
    }

    void updateChildren() {
        qDeleteAll( children );
        children.clear();
        int i = 0;
        foreach( Nepomuk::Types::Class subType, type.subClasses() ) {
            children << new ClassNode( subType, i++, this );
        }
    }

    // although classes can have multiple parents in this tree they always have one
    // but one type can occure in different nodes then
    ClassNode* parent;

    // if true updateClass is currently running on this node
    bool updating;

    int row;
    Nepomuk::Types::Class type;
    QList<ClassNode*> children;
};


class Nepomuk::PIMOItemModel::Private
{
public:
    Private( PIMOItemModel* parent )
        : baseClassNode( 0 ),
          q( parent ) {
    }

    ClassNode* findNode( const Types::Class& type, ClassNode* parent = 0 );

    ClassNode* baseClassNode;

private:
    PIMOItemModel* q;
};


ClassNode* Nepomuk::PIMOItemModel::Private::findNode( const Types::Class& type, ClassNode* parent )
{
    // FIXME: this should be improved somehow
    if ( !parent ) {
        parent = baseClassNode;
    }

    if ( type == parent->type ) {
        return parent;
    }

    foreach( ClassNode* node, parent->children ) {
        if ( ClassNode* n = findNode( type, node ) ) {
            return n;
        }
    }

    return 0;
}



Nepomuk::PIMOItemModel::PIMOItemModel( QObject* parent )
    : QAbstractItemModel( parent ),
      d( new Private( this ) )
{
}


Nepomuk::PIMOItemModel::~PIMOItemModel()
{
    delete d;
}


void Nepomuk::PIMOItemModel::setParentClass( const Types::Class& type )
{
    delete d->baseClassNode;
    d->baseClassNode = new ClassNode( type, 0 );
    reset();
}


int Nepomuk::PIMOItemModel::columnCount( const QModelIndex& parent ) const
{
    return 1;
}


QVariant Nepomuk::PIMOItemModel::data( const QModelIndex& index, int role ) const
{
    if ( index.isValid() ) {
        ClassNode* node = ( ClassNode* )index.internalPointer();
        Q_ASSERT( node );

        switch( role ) {
        case Qt::DisplayRole:
            if ( index.column() == 0 ) {
                return node->type.label();
            }
            else {
                return node->type.comment();
            }

        case Qt::ToolTipRole:
            return "<p>" + node->type.comment() + "<br><i>" + node->type.uri().toString() + "</i>";

        case Qt::DecorationRole: {
            QIcon icon = node->type.icon();
            if ( icon.isNull() ) {
                icon = KIcon( "FIXME" );
            }
            return icon;
        }

        case TypeRole:
            return qVariantFromValue( node->type );

        default:
            return QVariant();
        }
    }
    else {
        return QVariant();
    }
}


QModelIndex Nepomuk::PIMOItemModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( d->baseClassNode ) {
        if ( parent.isValid() ) {
            ClassNode* parentNode = ( ClassNode* )parent.internalPointer();
            Q_ASSERT( parentNode );
            if ( !parentNode->updating &&
                 parentNode->type.subClasses().count() != parentNode->children.count() ) {
                parentNode->updateChildren();
            }

            if ( row < parentNode->children.count() ) {
                return createIndex( row, column, parentNode->children[row] );
            }
            else {
                return QModelIndex();
            }
        }
        else if ( row == 0 ) {
            return createIndex( row, column, d->baseClassNode );
        }
    }

    return QModelIndex();
}


QModelIndex Nepomuk::PIMOItemModel::parent( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        ClassNode* node = ( ClassNode* )index.internalPointer();
        Q_ASSERT( node );
        if ( node->parent ) {
            return createIndex( node->parent->row, index.column(), node->parent );
        }
    }

    return QModelIndex();
}


int Nepomuk::PIMOItemModel::rowCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) {
        ClassNode* parentNode = ( ClassNode* )parent.internalPointer();
        Q_ASSERT( parentNode );
        return parentNode->type.subClasses().count();
    }
    else {
        return d->baseClassNode ? 1 : 0;
    }
}


Qt::ItemFlags Nepomuk::PIMOItemModel::flags( const QModelIndex& index ) const
{
    // FIXME: we can only decide based on the graph the classes are stored in whether they are fixed or not. :(
    return QAbstractItemModel::flags( index );
}


void Nepomuk::PIMOItemModel::updateClass( Types::Class type )
{
    if ( ClassNode* node = d->findNode( type ) ) {
        node->updating = true;

        // remove all children
        if ( node->children.count() ) {
            beginRemoveRows( createIndex( node->row, 0, node ), 0, node->children.count()-1 );
            node->children.clear();
            endRemoveRows();
        }

        // reset the class
        type.reset();

        // re-add all children
        if ( type.subClasses().count() ) {
            beginInsertRows( createIndex( node->row, 0, node ), 0, type.subClasses().count()-1 );
            node->updateChildren();
            endInsertRows();
        }

        node->updating = false;
    }
}

#include "pimoitemmodel.moc"
