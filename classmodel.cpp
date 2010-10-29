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

class Nepomuk::Utils::ClassModel::ClassNode
{
public:
    ClassNode( ClassModel* model, const Nepomuk::Types::Class& t, int _row, ClassNode* parentNode = 0 )
        : parent( parentNode ),
          updating( false ),
          row( _row ),
          type( t ),
          m_model( model ) {
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

    ClassNode* findNode( const Nepomuk::Types::Class& type, bool autoUpdate = false ) {
        if ( type == this->type ) {
            return this;
        }

        if ( autoUpdate /*&& !updating */) {
            updateChildren();
        }

        for( int i = 0; i < children.size(); ++i ) {
            if ( ClassNode* n = children[i]->findNode( type, autoUpdate ) ) {
                return n;
            }
        }

        return 0;
    }

    void updateChildren() {
        // 1. remove the nodes that are no longer valid
        Q_FOREACH( ClassModel::ClassNode* node, children ) {
            if ( !type.subClasses().contains( node->type ) ) {
                m_model->beginRemoveRows( m_model->createIndex( row, 0, this ), node->row, node->row );
                delete children.takeAt( node->row );
                m_model->endRemoveRows();
            }
        }

        // 2. add the nodes that are not there yet (in one batch)
        QList<Nepomuk::Types::Class> classesToAdd;
        Q_FOREACH( const Nepomuk::Types::Class& subClass, type.subClasses() ) {
            bool haveSubClass = false;
            // check if we already have a node for that subclass
            Q_FOREACH( ClassModel::ClassNode* node, children ) {
                if ( node->type == subClass ) {
                    haveSubClass = true;
                }
            }
            if ( !haveSubClass )
                classesToAdd << subClass;
        }

        if ( !classesToAdd.isEmpty() ) {
            m_model->beginInsertRows( m_model->createIndex( row, 0, this ), children.count(), type.subClasses().count()-1 );
            int i = children.count();
            foreach( const Nepomuk::Types::Class& subType, classesToAdd ) {
                children << new ClassNode( m_model, subType, i++, this );
            }
            m_model->endInsertRows();
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

private:
    ClassModel* m_model;
};


class Nepomuk::Utils::ClassModel::Private
{
public:
    Private( ClassModel* parent )
        : q( parent ) {
    }

    bool createSubClassRelation( const Types::Class& theClass, const Types::Class& newParentClass, bool singleParent );

    ClassModel::ClassNode* createRootNode( const Types::Class& type );
    ClassModel::ClassNode* findNode( const Types::Class& type, bool autoUpdate = false );

    QList<ClassModel::ClassNode*> baseClassNodes;

private:
    ClassModel* q;
};



bool Nepomuk::Utils::ClassModel::Private::createSubClassRelation( const Types::Class& theClass, const Types::Class& newParentClass, bool singleParent )
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


Nepomuk::Utils::ClassModel::ClassNode* Nepomuk::Utils::ClassModel::Private::findNode( const Types::Class& type, bool autoUpdate )
{
    Q_FOREACH( ClassModel::ClassNode* root, baseClassNodes ) {
        if ( ClassModel::ClassNode* n = root->findNode( type, autoUpdate ) ) {
            return n;
        }
    }
    return 0;
}


Nepomuk::Utils::ClassModel::ClassModel( QObject* parent )
    : QAbstractItemModel( parent ),
      d( new Private( this ) )
{
}


Nepomuk::Utils::ClassModel::~ClassModel()
{
    delete d;
}


void Nepomuk::Utils::ClassModel::addRootClass( const Types::Class& type )
{
    beginInsertRows( QModelIndex(), d->baseClassNodes.count(), d->baseClassNodes.count()+1 );
    d->baseClassNodes.append( new ClassNode( this, type, 0 ) );
    endInsertRows();
}


void Nepomuk::Utils::ClassModel::setRootClass( const Types::Class& type )
{
    setRootClasses( QList<Types::Class>() << type );
}


void Nepomuk::Utils::ClassModel::setRootClasses( const QList<Types::Class>& types )
{
    qDeleteAll( d->baseClassNodes );
    d->baseClassNodes.clear();
    Q_FOREACH( const Types::Class& type, types ) {
        d->baseClassNodes.append( new ClassNode( this, type, 0 ) );
    }
    reset();
}


void Nepomuk::Utils::ClassModel::clear()
{
    setRootClasses( QList<Types::Class>() );
}


QList<Nepomuk::Types::Class> Nepomuk::Utils::ClassModel::rootClasses() const
{
    QList<Nepomuk::Types::Class> types;
    Q_FOREACH( ClassNode* n, d->baseClassNodes )
        types.append( n->type );
    return types;
}


int Nepomuk::Utils::ClassModel::columnCount( const QModelIndex& ) const
{
    return 1;
}


QVariant Nepomuk::Utils::ClassModel::data( const QModelIndex& index, int role ) const
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


QModelIndex Nepomuk::Utils::ClassModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( parent.isValid() ) {
        ClassNode* parentNode = ( ClassNode* )parent.internalPointer();
        Q_ASSERT( parentNode );
        if ( row < parentNode->children.count() ) {
            return createIndex( row, column, parentNode->children[row] );
        }
        else {
            return QModelIndex();
        }
    }
    else if ( row < d->baseClassNodes.count() ) {
        return createIndex( row, column, d->baseClassNodes[row] );
    }

    return QModelIndex();
}


QModelIndex Nepomuk::Utils::ClassModel::parent( const QModelIndex& index ) const
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


bool Nepomuk::Utils::ClassModel::hasChildren( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) {
        ClassNode* parentNode = ( ClassNode* )parent.internalPointer();
        Q_ASSERT( parentNode );
        return !parentNode->type.subClasses().isEmpty();
    }
    else {
        return !d->baseClassNodes.isEmpty();
    }
}


int Nepomuk::Utils::ClassModel::rowCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) {
        ClassNode* parentNode = ( ClassNode* )parent.internalPointer();
        Q_ASSERT( parentNode );
        return parentNode->children.count();
    }
    else {
        return d->baseClassNodes.count();
    }
}


Qt::ItemFlags Nepomuk::Utils::ClassModel::flags( const QModelIndex& index ) const
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


Qt::DropActions Nepomuk::Utils::ClassModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}


QStringList Nepomuk::Utils::ClassModel::mimeTypes() const
{
    return( QStringList()
            << QLatin1String( "application/x-nepomuk-class-uri" )
            << KUrl::List::mimeDataTypes() );
}


QMimeData* Nepomuk::Utils::ClassModel::mimeData( const QModelIndexList& indexes ) const
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


bool Nepomuk::Utils::ClassModel::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int, const QModelIndex& parent )
{
    ClassNode* parentNode = 0;
    if ( parent.isValid() ) {
        parentNode = static_cast<ClassNode*>( parent.internalPointer() );
    }
    else {
        kDebug() << "Dropped into thin air";
        return false;
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


Nepomuk::Types::Class Nepomuk::Utils::ClassModel::classForIndex( const QModelIndex& index ) const
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


QModelIndex Nepomuk::Utils::ClassModel::indexForClass( const Nepomuk::Types::Class& cls ) const
{
    if ( ClassNode* node = d->findNode( cls, true ) ) {
        return createIndex( node->row, 0, node );
    }
    else {
        return QModelIndex();
    }
}


bool Nepomuk::Utils::ClassModel::canFetchMore( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) {
        ClassNode* parentNode = static_cast<ClassNode*>( parent.internalPointer() );
        return parentNode->type.subClasses().count() != parentNode->children.count();
    }
    else {
        return false;
    }
}


void Nepomuk::Utils::ClassModel::fetchMore( const QModelIndex& parent )
{
    if ( parent.isValid() ) {
        ClassNode* parentNode = static_cast<ClassNode*>( parent.internalPointer() );
        if ( parentNode->type.subClasses().count() != parentNode->children.count() ) {
            parentNode->updateChildren();
        }
    }
}


void Nepomuk::Utils::ClassModel::updateClass( const Types::Class& type )
{
    if ( ClassModel::ClassNode* node = d->findNode( type ) ) {
//         node->updating = true;

        // reset the class
        Types::Class tmp( type );
        tmp.reset();

        // add the missing children
        if ( type.subClasses().count() ) {
            node->updateChildren();
        }

//         node->updating = false;
    }
}

#include "classmodel.moc"
