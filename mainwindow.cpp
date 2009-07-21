/*
   Copyright (C) 2008-2009 by Sebastian Trueg <trueg at kde.org>

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

#include "mainwindow.h"
#include "pimoitemmodel.h"
#include "pimo.h"
#include "newclassdialog.h"
#include "resourcepropertymodel.h"
#include "resourcepropertydelegate.h"
#include "asyncloadingresourcemodel.h"
#include "resourcedelegate.h"
#include "kpixmapsequenceoverlaypainter.h"
#include "nepomukcontext.h"

#include <QtGui/QTreeView>
#include <QtGui/QListView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QMenu>
#include <QtGui/QHeaderView>
#include <QtGui/QComboBox>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QSortFilterProxyModel>

#include <nepomuk/class.h>
#include <nepomuk/property.h>
#include <nepomuk/resourcemanager.h>

#include <KIcon>
#include <KDebug>
#include <KAction>
#include <KStandardAction>
#include <KActionCollection>
#include <KApplication>

#include <Soprano/Vocabulary/RDFS>


MainWindow::MainWindow()
    : KXmlGuiWindow( 0 )
{
    QWidget* w = new QWidget( this );
    setCentralWidget( w );
    setupUi( w );

    setupActions();

    m_pimoView->header()->hide();
    m_pimoView->setSelectionMode( QAbstractItemView::SingleSelection );

    m_pimoModel = new Nepomuk::PIMOItemModel( m_pimoView );
    m_pimoModel->setParentClass( Nepomuk::Vocabulary::PIMO::Thing() );
    m_pimoSortModel = new QSortFilterProxyModel( m_pimoView );
    m_pimoSortModel->setSourceModel( m_pimoModel );
    m_pimoSortModel->setSortCaseSensitivity( Qt::CaseInsensitive );
    m_pimoView->setModel( m_pimoSortModel );
    m_pimoView->setContextMenuPolicy( Qt::CustomContextMenu );
    m_pimoView->setSortingEnabled( true );
    m_pimoView->setDragEnabled(true);
    m_pimoView->setAcceptDrops(true);
    m_pimoView->setDropIndicatorShown(true);

    m_resourceModel = new Nepomuk::AsyncLoadingResourceModel( m_resourceView );
    m_resourceView->setModel( m_resourceModel );
    m_resourceView->setContextMenuPolicy( Qt::CustomContextMenu );
    Nepomuk::ResourceDelegate* delegate = new Nepomuk::ResourceDelegate( this );
    delegate->setDisplayMode( Nepomuk::AbstractResourceGuiItem::DisplayFull );
    m_resourceView->setItemDelegateForColumn( 0, delegate );
    m_resourceView->setIconSize( QSize( 48, 48 ) );
    m_resourceView->setSpacing( KDialog::spacingHint() );

    m_propertyModel = new Nepomuk::ResourcePropertyEditModel( m_propertyView );
    m_propertyView->setModel( m_propertyModel );
    m_propertyView->header()->hide();
    m_propertyView->header()->setResizeMode( QHeaderView::ResizeToContents );
    m_propertyView->setItemDelegateForColumn( 1, new Nepomuk::ResourcePropertyDelegate( m_propertyView ) );

    m_baseClassCombo->addItem( "PIMO Classes", QVariant( Nepomuk::Vocabulary::PIMO::Thing() ) );
    m_baseClassCombo->addItem( "All Classes", QVariant( Soprano::Vocabulary::RDFS::Resource() ) );

    connect( m_pimoView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( slotPIMOViewContextMenu( const QPoint& ) ) );
    connect( m_resourceView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( slotResourceViewContextMenu( const QPoint& ) ) );
    connect( m_pimoView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
             this, SLOT( slotCurrentPIMOClassChanged( const QModelIndex&, const QModelIndex& ) ) );
    connect( m_resourceView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this, SLOT( slotCurrentResourceChanged( const QItemSelection&, const QItemSelection& ) ) );
    connect( m_resourceModel, SIGNAL( modelReset() ),
             m_propertyModel, SLOT( clear() ) );
    connect( m_baseClassCombo, SIGNAL( activated( int ) ),
             this, SLOT( slotBaseClassChanged( int ) ) );
}


MainWindow::~MainWindow()
{
}


void MainWindow::setupActions()
{
    m_actionNewSubClass = new KAction( KIcon( "document-new" ), i18n( "Create Subclass" ), actionCollection() );
    m_actionNewProperty = new KAction( KIcon( "document-new" ), i18n( "Create Property" ), actionCollection() );
    m_actionNewResource = new KAction( KIcon( "document-new" ), i18n( "Create Resource" ), actionCollection() );

    connect( m_actionNewSubClass, SIGNAL(triggered(bool)), this, SLOT(slotCreateClass()) );
    connect( m_actionNewProperty, SIGNAL(triggered(bool)), this, SLOT(slotCreateProperty()) );
    connect( m_actionNewResource, SIGNAL(triggered(bool)), this, SLOT(slotCreateResource()) );

    actionCollection()->addAction( "create_class", m_actionNewSubClass );
    actionCollection()->addAction( "create_property", m_actionNewProperty );
    actionCollection()->addAction( "create_resource", m_actionNewResource );

    KStandardAction::quit( kapp, SLOT( quit() ), actionCollection() );

    setupGUI();
}


void MainWindow::slotPIMOViewContextMenu( const QPoint& pos )
{
    QModelIndex index = m_pimoView->indexAt( pos );
    if ( index.isValid() ) {
        Nepomuk::Types::Class type = index.data( Nepomuk::PIMOItemModel::TypeRole ).value<Nepomuk::Types::Class>();

        QMenu::exec( QList<QAction*>()
                     << m_actionNewSubClass
                     << m_actionNewProperty
                     << m_actionNewResource,
                     m_pimoView->viewport()->mapToGlobal( pos ) );
    }
}


void MainWindow::slotResourceViewContextMenu( const QPoint& pos )
{
    kDebug();
    QModelIndex index = m_resourceView->indexAt( pos );
    if ( index.isValid() ) {
        Nepomuk::Resource res( index.data( Nepomuk::ResourceModel::ResourceUriRole ).value<QUrl>() );
        kDebug() << "Have valid resource" << res.resourceUri();

        QList<QAction*> actions;

        QAction sep( this );
        sep.setSeparator( true );
        QAction actionSetContext( KIcon( "nepomuk" ), i18n( "Set as current context" ), this );

        if ( Nepomuk::ContextServiceInterface::isAvailable() ) {
            actions << &sep << &actionSetContext;
        }

        QAction actionRemove( KIcon( "edit-delete" ), i18n( "Remove resource '%1'", res.genericLabel() ), this );

        QAction* a = QMenu::exec( actions
                                  << &actionRemove,
                                  m_resourceView->viewport()->mapToGlobal( pos ) );
        if( a == &actionRemove ) {
            res.remove();
        }
        else if ( a == &actionSetContext ) {
            Nepomuk::ContextServiceInterface::instance()->setCurrentContext( res.resourceUri() );
        }
    }
 }


void MainWindow::slotCurrentPIMOClassChanged( const QModelIndex& current, const QModelIndex& )
{
    if ( current.isValid() ) {
        Nepomuk::Types::Class type = current.data( Nepomuk::PIMOItemModel::TypeRole ).value<Nepomuk::Types::Class>();
        kDebug() << "Selection changed:" << type.label();

        KPixmapSequenceOverlayPainter* op = new KPixmapSequenceOverlayPainter( this );
        op->setWidget( m_resourceView->viewport() );
        op->setPosition( Qt::AlignCenter );
        connect( m_resourceModel, SIGNAL( finishedLoading() ), op, SLOT( deleteLater() ) );
        op->start();

        m_resourceModel->loadResourcesOfType( type );
    }
}


void MainWindow::slotCurrentResourceChanged( const QItemSelection& current, const QItemSelection& )
{
    kDebug();
    if ( !current.indexes().isEmpty() ) {
        m_propertyModel->setResource( current.indexes().first().data( Nepomuk::ResourceModel::ResourceUriRole ).value<QUrl>() );
    }
    else {
        m_propertyModel->clear();
    }
}


void MainWindow::slotBaseClassChanged( int index )
{
    m_pimoModel->setParentClass( m_baseClassCombo->itemData( index ).toUrl() );
}


Nepomuk::Types::Class MainWindow::selectedClass() const
{
    QModelIndex current = m_pimoView->selectionModel()->currentIndex();
    if ( current.isValid() )
        return current.data( Nepomuk::PIMOItemModel::TypeRole ).value<Nepomuk::Types::Class>();
    else
        return m_pimoModel->parentClass();
}


void MainWindow::slotCreateClass()
{
    Nepomuk::Types::Class newClass = NewClassDialog::createClass( selectedClass(), this );
    if ( newClass.isValid() ) {
        // update the model
        m_pimoModel->updateClass( selectedClass() );
    }
}


void MainWindow::slotCreateProperty()
{
    // create a new resource
    if ( NewClassDialog::createProperty( selectedClass(), this ).isValid() ) {
        selectedClass().reset( false );
        // FIXME: udpate property model if necessary
    }
}


void MainWindow::slotCreateResource()
{
    // create a new resource
    Nepomuk::Resource res = NewClassDialog::createResource( selectedClass(), this );
    if ( res.isValid() ) {
        // update
        m_resourceModel->addResource( res );
    }
}

#include "mainwindow.moc"
