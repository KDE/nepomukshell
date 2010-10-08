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

#include "classmodel.h"
#include "pimo.h"

#include <nepomuk/ontology.h>
#include <nepomuk/resourcemanager.h>
#include <nepomuk/resource.h>
#include <nepomuk/class.h>

#include <kicon.h>
#include <kdebug.h>
#include <kurl.h>

#include <QtCore/QMimeData>

#include <Soprano/Statement>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/NRL>

#define USING_SOPRANO_NRLMODEL_UNSTABLE_API
#include <Soprano/NRLModel>


Q_DECLARE_METATYPE( Nepomuk::Types::Class )

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


class Nepomuk::ClassModel::Private
{
public:
    Private( ClassModel* parent )
        : baseClassNode( 0 ),
          q( parent ) {
    }

    bool createSubClassRelation( const Types::Class& theClass, const Types::Class& newParentClass, bool singleParent );

    ClassNode* findNode( const Types::Class& type, ClassNode* parent = 0 );

    ClassNode* baseClassNode;

private:
    ClassModel* q;
};



bool Nepomuk::ClassModel::Private::createSubClassRelation( const Types::Class& theClass, const Types::Class& newParentClass, bool singleParent )
{
    if ( theClass == newParentClass ) {
        kDebug() << "Cannot make a class sub class of itself";
        return false;
    }

    Soprano::Model* model = ResourceManager::instance()->mainModel();

    // the only way we have at the moment to distiguish between PIMO classes and user created ones is the
    // presence of nao:created
    if( theClass != Vocabulary::PIMO::Thing() &&
        !theClass.isSubClassOf( Vocabulary::PIMO::Thing() ) &&
        !model->containsAnyStatement( theClass.uri(), Soprano::Vocabulary::NAO::created(), Soprano::Node() ) ) {
        kDebug() << "Only pimo:Thing subclasses created by the user can be changed.";
        return false;
    }

    if( !newParentClass.isValid() ) {
        kDebug() << QString::fromLatin1( "Non-existing classes cannot be used as parents (%1)" ).arg( newParentClass.uri().toString() );
        return false;
    }

    if ( newParentClass.isSubClassOf( theClass ) ) {
        kDebug() << "Cannot create subclass relation loop.";
        return false;
    }

    if ( singleParent ) {
        model->removeAllStatements( theClass.uri(), Soprano::Vocabulary::RDFS::subClassOf(), Soprano::Node() );
    }

    Soprano::NRLModel nrlModel( model );
    QUrl graph = nrlModel.createGraph( Soprano::Vocabulary::NRL::Ontology() );
    return model->addStatement( theClass.uri(), Soprano::Vocabulary::RDFS::subClassOf(), newParentClass.uri(), graph ) == Soprano::Error::ErrorNone;
}


ClassNode* Nepomuk::ClassModel::Private::findNode( const Types::Class& type, ClassNode* parent )
{
    // FIXME: this should be improved somehow
    if ( !parent ) {
        parent = baseClassNode;
    }

    if ( type == parent->type ) {
        return parent;
    }

    for(int i=0; i<parent->children.size(); ++i) {
        if ( ClassNode* n = findNode( type, parent->children[i] ) ) {
            return n;
        }
    }

    return 0;
}



Nepomuk::ClassModel::ClassModel( QObject* parent )
    : QAbstractItemModel( parent ),
      d( new Private( this ) )
{
}


Nepomuk::ClassModel::~ClassModel()
{
    delete d;
}


void Nepomuk::ClassModel::setParentClass( const Types::Class& type )
{
    delete d->baseClassNode;
    d->baseClassNode = new ClassNode( type, 0 );
    reset();
}


Nepomuk::Types::Class Nepomuk::ClassModel::parentClass() const
{
    if ( d->baseClassNode )
        return d->baseClassNode->type;
    else
        return Types::Class();
}


int Nepomuk::ClassModel::columnCount( const QModelIndex& ) const
{
    return 1;
}


QVariant Nepomuk::ClassModel::data( const QModelIndex& index, int role ) const
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
            return QLatin1String( "<p>" ) + node->type.comment() + QLatin1String( "<br><i>" ) + node->type.uri().toString() + QLatin1String( "</i>" );

        case Qt::DecorationRole: {
            QIcon icon = node->type.icon();
            if ( icon.isNull() ) {
                icon = KIcon( QLatin1String( "nepomuk" ) );
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


QModelIndex Nepomuk::ClassModel::index( int row, int column, const QModelIndex& parent ) const
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


QModelIndex Nepomuk::ClassModel::parent( const QModelIndex& index ) const
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


int Nepomuk::ClassModel::rowCount( const QModelIndex& parent ) const
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


Qt::ItemFlags Nepomuk::ClassModel::flags( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        Qt::ItemFlags f = Qt::ItemIsSelectable|Qt::ItemIsDropEnabled|Qt::ItemIsEnabled;
        ClassNode* node = ( ClassNode* )index.internalPointer();
        Q_ASSERT( node );
        // user-created classes have a nao:created date.
        if ( ResourceManager::instance()->mainModel()->containsAnyStatement( node->type.uri(), Soprano::Vocabulary::NAO::created(), Soprano::Node() ) ) {
            f |= Qt::ItemIsDragEnabled|Qt::ItemIsEditable;
        }
        return f;
    }
    else {
        return QAbstractItemModel::flags( index );
    }
}


Qt::DropActions Nepomuk::ClassModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}


QStringList Nepomuk::ClassModel::mimeTypes() const
{
    return( QStringList()
            << QLatin1String( "application/x-nepomuk-class-uri" )
            << KUrl::List::mimeDataTypes() );
}


QMimeData* Nepomuk::ClassModel::mimeData( const QModelIndexList& indexes ) const
{
    KUrl::List classUris;
    foreach ( const QModelIndex& index, indexes ) {
        if (index.isValid()) {
            classUris << classForIndex( index ).uri();
        }
    }

    QMimeData* mimeData = new QMimeData();
    classUris.populateMimeData( mimeData );

    QByteArray data;
    QDataStream s( &data, QIODevice::WriteOnly );
    s << classUris;
    mimeData->setData( mimeTypes().first(), data );

    return mimeData;
}


bool Nepomuk::ClassModel::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int, const QModelIndex& parent )
{
    ClassNode* parentNode = 0;
    if ( parent.isValid() ) {
        parentNode = ( ClassNode* )parent.internalPointer();
    }
    else {
        parentNode = d->baseClassNode;
    }

    if ( !parentNode )
        return false;
    if ( parentNode->children.count() <= row )
        return false;

    if ( row >= 0 )
        parentNode = parentNode->children[row];

    // FIXME: add methods for handling mimedata to Resource and Entity (compare the KUrl::List methods)
    if ( data->hasFormat( QLatin1String( "application/x-nepomuk-class-uri" ) ) ) {
        KUrl::List classUris = KUrl::List::fromMimeData( data );
        foreach( const KUrl& uri, classUris ) {
            if ( !d->createSubClassRelation( uri, parentNode->type.uri(), action == Qt::MoveAction ) )
                return false;
            updateClass( uri );
        }
        updateClass( parentNode->type );
        return true;
    }
    else if ( data->hasFormat( QLatin1String( "application/x-nepomuk-resource-uri" ) ) ) {
        KUrl::List uris = KUrl::List::fromMimeData( data );
        foreach( const KUrl& uri, uris ) {
            Resource res( uri );
            res.addType( parentNode->type.uri() );
        }
        return true;
    }
    else {
        return false;
    }
}


void Nepomuk::ClassModel::updateClass( Types::Class type )
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


Nepomuk::Types::Class Nepomuk::ClassModel::classForIndex( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        ClassNode* node = ( ClassNode* )index.internalPointer();
        Q_ASSERT( node );
        return node->type;
    }
    else {
        return Types::Class();
    }
}

QModelIndex Nepomuk::ClassModel::indexForClass( const Nepomuk::Types::Class& cls ) const
{
    bool found;
    ClassNode* node = d->findNode(cls);
    ClassNode* parent = d->baseClassNode;
    while (!node || node->type != cls) {
        found=false;
        if (!parent->updating && parent->type.subClasses().count() != parent->children.count()) {
            parent->updateChildren();
        }
        node = d->findNode(cls);
        if (!node || node->type != cls) {
            for (int i = 0; i < parent->children.size() && !found; ++i) {
                if (cls.isSubClassOf(parent->children[i]->type)) {
                    parent=parent->children[i];
                    found=true;
                }
            }
        }
    }
    // should we check the node again?
    if (node) {
        QModelIndex idx = createIndex(node->row, 0, node);
        return idx;
    }

    return QModelIndex();

}

QModelIndexList Nepomuk::ClassModel::parentIndexList( const Types::Class& cls )
{
    bool found;
    ClassNode* parent=d->baseClassNode;
    QModelIndexList list;
    list << createIndex(d->baseClassNode->row, 0, d->baseClassNode);
    while ( parent->type != cls ) {
        found=false;
        if (!parent->updating && parent->type.subClasses().count() != parent->children.count()) {
            parent->updateChildren();
        }
        for (int i = 0; i < parent->children.size() && !found; ++i) {
            if (parent->children[i]->type == cls || cls.isSubClassOf(parent->children[i]->type)) {
                list << createIndex(parent->children[i]->row, 0, parent->children[i]);
                parent = parent->children[i];
                found=true;
            }
        }
        if (!found) {
            // bad: we have not found any child node to be a parent of our type! Return an empty list...
            list.clear();
            return list;
        }
    }
    // delete the node containing the parameter: a node is not a parent of itself
    list.removeLast();

    return list;
}

#include "classmodel.moc"
