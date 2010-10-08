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

#include "resourcebrowserwidget.h"
#include "pimoitemmodel.h"
#include "pimo.h"
#include "resourcepropertymodel.h"
#include "newclassdialog.h"

#include "resourcemodel.h"

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

#include <KDebug>
#include <krecursivefilterproxymodel.h>
#include <KActionCollection>

#include <Soprano/Vocabulary/RDFS>


ResourceBrowserWidget::ResourceBrowserWidget( KActionCollection* ac, QWidget* parent )
    : QWidget( parent ),
      m_actionCollection(ac)
{
    setupUi( this );

    m_pimoView->header()->hide();
    m_pimoView->setSelectionMode( QAbstractItemView::SingleSelection );

    m_pimoModel = new Nepomuk::PIMOItemModel( m_pimoView );
    m_pimoModel->setParentClass( Nepomuk::Vocabulary::PIMO::Thing() );
    m_pimoSortModel = new KRecursiveFilterProxyModel( m_pimoView );
    m_pimoSortModel->setSourceModel( m_pimoModel );
    m_pimoSortModel->setSortCaseSensitivity( Qt::CaseInsensitive );
    m_pimoSortModel->setDynamicSortFilter( true );
    m_pimoView->setModel( m_pimoSortModel );
    m_pimoView->setContextMenuPolicy( Qt::CustomContextMenu );
    m_pimoView->setSortingEnabled( true );
    m_pimoView->sortByColumn( 0, Qt::AscendingOrder );
    m_pimoView->setDragEnabled(true);
    m_pimoView->setAcceptDrops(true);
    m_pimoView->setDropIndicatorShown(true);

    m_classFilter->setProxy( m_pimoSortModel );

    m_baseClassCombo->addItem( i18n( "PIMO Classes" ), QVariant( Nepomuk::Vocabulary::PIMO::Thing() ) );
    m_baseClassCombo->addItem( i18n( "All Classes" ), QVariant( Soprano::Vocabulary::RDFS::Resource() ) );

    connect( m_pimoView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( slotPIMOViewContextMenu( const QPoint& ) ) );
    connect( m_resourceView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( slotResourceViewContextMenu( const QPoint& ) ) );
    connect( m_pimoView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
             this, SLOT( slotCurrentPIMOClassChanged( const QModelIndex&, const QModelIndex& ) ) );
    connect( m_resourceView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this, SLOT( slotCurrentResourceChanged( const QItemSelection&, const QItemSelection& ) ) );
    connect( m_baseClassCombo, SIGNAL( activated( int ) ),
             this, SLOT( slotBaseClassChanged( int ) ) );
    connect( m_resourceView, SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(slotResourceActivated(QModelIndex)) );
}


ResourceBrowserWidget::~ResourceBrowserWidget()
{
}


void ResourceBrowserWidget::slotPIMOViewContextMenu( const QPoint& pos )
{
    kDebug();
    QModelIndex index = m_pimoView->indexAt( pos );
    if ( index.isValid() ) {
        Nepomuk::Types::Class type = index.data( Nepomuk::PIMOItemModel::TypeRole ).value<Nepomuk::Types::Class>();

        QMenu::exec( QList<QAction*>()
                     << m_actionCollection->action( QLatin1String( "create_class" ) )
                     << m_actionCollection->action( QLatin1String( "create_property" ) )
                     << m_actionCollection->action( QLatin1String( "create_resource" ) ),
                     m_pimoView->viewport()->mapToGlobal( pos ) );
    }
}


void ResourceBrowserWidget::slotResourceViewContextMenu( const QPoint& pos )
{
    kDebug();
    QModelIndexList selection = m_resourceView->selectionModel()->selectedIndexes();

    if ( !selection.isEmpty() ) {
        QList<QAction*> actions;
        actions << m_actionCollection->action( QLatin1String( "resource_delete" ) );

        QMenu::exec( actions,
                     m_resourceView->viewport()->mapToGlobal( pos ) );
    }
}


void ResourceBrowserWidget::slotCurrentPIMOClassChanged( const QModelIndex& current, const QModelIndex& )
{
    if ( current.isValid() ) {
        Nepomuk::Types::Class type = current.data( Nepomuk::PIMOItemModel::TypeRole ).value<Nepomuk::Types::Class>();
        kDebug() << "Selection changed:" << type.label();
        m_resourceView->setType( type );
    }
}


void ResourceBrowserWidget::slotCurrentResourceChanged( const QItemSelection&, const QItemSelection& )
{
    kDebug();
    emit resourcesSelected( selectedResources() );
}


void ResourceBrowserWidget::slotBaseClassChanged( int index )
{
    m_pimoModel->setParentClass( m_baseClassCombo->itemData( index ).toUrl() );
}


void ResourceBrowserWidget::slotResourceActivated( const QModelIndex& index )
{
    QUrl res = index.data( Nepomuk::ResourceModel::ResourceUriRole ).value<QUrl>();
    kDebug() << res;
    emit resourceActivated( res );
}


void ResourceBrowserWidget::createClass()
{
    Nepomuk::Types::Class newClass = NewClassDialog::createClass( selectedClass(), this );
    if ( newClass.isValid() ) {
        // update the model
        m_pimoModel->updateClass( selectedClass() );
    }
}


void ResourceBrowserWidget::createProperty()
{
    // create a new resource
    if ( NewClassDialog::createProperty( selectedClass(), this ).isValid() ) {
        selectedClass().reset( false );
        // FIXME: udpate property model if necessary
    }
}


void ResourceBrowserWidget::createResource()
{
    // create a new resource
    Nepomuk::Resource res = NewClassDialog::createResource( selectedClass(), this );
    if ( res.isValid() ) {
        // update
        m_resourceView->addResource( res );
    }
}


QList<Nepomuk::Resource> ResourceBrowserWidget::selectedResources() const
{
    QList<Nepomuk::Resource> rl;
    QModelIndexList selection = m_resourceView->selectionModel()->selectedIndexes();
    Q_FOREACH( const QModelIndex& index, selection )
        rl << index.data( Nepomuk::ResourceModel::ResourceUriRole ).value<QUrl>();
    return rl;
}


Nepomuk::Types::Class ResourceBrowserWidget::selectedClass() const
{
    QModelIndex current = m_pimoView->selectionModel()->currentIndex();
    if ( current.isValid() )
        return current.data( Nepomuk::PIMOItemModel::TypeRole ).value<Nepomuk::Types::Class>();
    else
        return m_pimoModel->parentClass();
}

#include "resourcebrowserwidget.moc"
