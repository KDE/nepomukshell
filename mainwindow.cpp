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

#include "mainwindow.h"
#include "pimoitemmodel.h"
#include "pimo.h"
#include "newclassdialog.h"
#include "resourcepropertymodel.h"
#include "resourcepropertydelegate.h"
#include "simpleresourcemodel.h"
#include "resourcedelegate.h"

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

#include <Soprano/Vocabulary/RDFS>


MainWindow::MainWindow()
    : KMainWindow( 0 )
{
    QWidget* w = new QWidget( this );
    setCentralWidget( w );
    setupUi( w );

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

    m_resourceModel = new Nepomuk::SimpleResourceModel( m_resourceView );
    m_resourceView->setModel( m_resourceModel );
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


void MainWindow::slotPIMOViewContextMenu( const QPoint& pos )
{
    QModelIndex index = m_pimoView->indexAt( pos );
    if ( index.isValid() ) {
        Nepomuk::Types::Class type = index.data( Nepomuk::PIMOItemModel::TypeRole ).value<Nepomuk::Types::Class>();

        QAction actionNewSubClass( KIcon( "document-new" ), i18n( "Create Subclass" ), this );
        QAction actionNewProperty( KIcon( "document-new" ), i18n( "Create Property" ), this );
        QAction actionNewResource( KIcon( "document-new" ), i18n( "Create Resource" ), this );

        QAction* a = QMenu::exec( QList<QAction*>()
                                  << &actionNewSubClass
                                  << &actionNewProperty
                                  << &actionNewResource,
                                  m_pimoView->viewport()->mapToGlobal( pos ) );

        if ( a == &actionNewSubClass ) {
            Nepomuk::Types::Class newClass = NewClassDialog::createClass( type, this );
            if ( newClass.isValid() ) {
                // update the model
                m_pimoModel->updateClass( type );
            }
        }
        else if ( a == &actionNewResource ) {
            // create a new resource
            if ( NewClassDialog::createResource( type, this ).isValid() ) {
                // update
                m_resourceModel->setResources( Nepomuk::ResourceManager::instance()->allResourcesOfType( type.uri() ) );
            }
        }
        else if ( a == &actionNewProperty ) {
            // create a new resource
            if ( NewClassDialog::createProperty( type, this ).isValid() ) {
                type.reset( false );
                // FIXME: udpate property model if necessary
            }
        }
    }
}


void MainWindow::slotCurrentPIMOClassChanged( const QModelIndex& current, const QModelIndex& )
{
    if ( current.isValid() ) {
        Nepomuk::Types::Class type = current.data( Nepomuk::PIMOItemModel::TypeRole ).value<Nepomuk::Types::Class>();
        kDebug() << "Selection changed:" << type.label();
        m_resourceModel->setResources( Nepomuk::ResourceManager::instance()->allResourcesOfType( type.uri() ) );
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

#include "mainwindow.moc"
