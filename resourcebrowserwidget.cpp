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
#include "classmodel.h"
#include "resourcepropertymodel.h"
#include "newclassdialog.h"
#include "nepomukshellsettings.h"
#include "mainwindow.h"

// Migrated classes
#include "utils/resourcemodel.h"

#include <QtGui/QTreeView>
#include <QtGui/QListView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QMenu>
#include <QtGui/QHeaderView>
#include <QtGui/QComboBox>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QSortFilterProxyModel>

#include <nepomuk2/class.h>
#include <nepomuk2/property.h>
#include <nepomuk2/resourcemanager.h>
#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/ResourceTerm>
#include <Nepomuk2/Query/ResourceTypeTerm>
#include <Nepomuk2/Query/ComparisonTerm>
#include <Nepomuk2/Vocabulary/PIMO>

#include <KDebug>
#include <krecursivefilterproxymodel.h>
#include <KActionCollection>

#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/RDFS>


Q_DECLARE_METATYPE( Nepomuk2::Resource )
Q_DECLARE_METATYPE( Nepomuk2::Types::Class )


ResourceBrowserWidget::ResourceBrowserWidget( QWidget* parent )
    : QWidget( parent )
{
    setupUi( this );

    m_pimoView->header()->hide();
    m_pimoView->setSelectionMode( QAbstractItemView::SingleSelection );

    m_pimoModel = new Nepomuk2::Utils::ClassModel( m_pimoView );
    m_pimoModel->addRootClass( Nepomuk2::Vocabulary::PIMO::Thing() );
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

    m_baseClassCombo->addItem( i18nc( "@item:inlistbox Referring to all RDF classes in the Nepomuk PIMO ontology", "PIMO Classes" ), QVariant( Nepomuk2::Vocabulary::PIMO::Thing() ) );
    m_baseClassCombo->addItem( i18nc( "@item:inlistbox Referring to all RDF classes in the Nepomuk db", "All Classes" ), QVariant( Soprano::Vocabulary::RDFS::Resource() ) );

    connect( m_pimoView, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(slotPIMOViewContextMenu(QPoint)) );
    connect( m_pimoView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(slotCurrentPIMOClassChanged(QModelIndex,QModelIndex)) );
    connect( m_resourceView, SIGNAL(selectionChanged(QList<Nepomuk2::Resource>)),
             this, SIGNAL(resourcesSelected(QList<Nepomuk2::Resource>)) );
    connect( m_baseClassCombo, SIGNAL(activated(int)),
             this, SLOT(slotBaseClassChanged(int)) );
    connect( m_resourceView, SIGNAL(resourceActivated(Nepomuk2::Resource)),
             this, SIGNAL(resourceActivated(Nepomuk2::Resource)) );
    connect( m_resourceView, SIGNAL(resourceTypeActivated(Nepomuk2::Types::Class)),
             this, SLOT(setSelectedClass(Nepomuk2::Types::Class)) );
}


ResourceBrowserWidget::~ResourceBrowserWidget()
{
}


void ResourceBrowserWidget::slotPIMOViewContextMenu( const QPoint& pos )
{
    kDebug();
    QModelIndex index = m_pimoView->indexAt( pos );
    if ( index.isValid() ) {
        Nepomuk2::Types::Class type = index.data( Nepomuk2::Utils::ClassModel::TypeRole ).value<Nepomuk2::Types::Class>();

        QMenu::exec( QList<QAction*>()
                     << MainWindow::nepomukShellMain()->actionCollection()->action( QLatin1String( "create_class" ) )
                     << MainWindow::nepomukShellMain()->actionCollection()->action( QLatin1String( "create_property" ) )
                     << MainWindow::nepomukShellMain()->actionCollection()->action( QLatin1String( "create_resource" ) ),
                     m_pimoView->viewport()->mapToGlobal( pos ) );
    }
}


void ResourceBrowserWidget::slotCurrentPIMOClassChanged( const QModelIndex& current, const QModelIndex& )
{
    if ( current.isValid() ) {
        Nepomuk2::Types::Class type = current.data( Nepomuk2::Utils::ClassModel::TypeRole ).value<Nepomuk2::Types::Class>();
        kDebug() << "Selection changed:" << type.label();
        setSelectedClass(type);
    }
}


void ResourceBrowserWidget::slotBaseClassChanged( int index )
{
    m_pimoModel->setRootClass( m_baseClassCombo->itemData( index ).toUrl() );
}


void ResourceBrowserWidget::createClass()
{
    Nepomuk2::Types::Class newClass = NewClassDialog::createClass( selectedClass(), this );
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
    Nepomuk2::Resource res = NewClassDialog::createResource( selectedClass(), this );
    if ( res.isValid() ) {
        // update
        m_resourceView->addResource( res );
    }
}


QList<Nepomuk2::Resource> ResourceBrowserWidget::selectedResources() const
{
    return m_resourceView->selectedResources();
}


Nepomuk2::Types::Class ResourceBrowserWidget::selectedClass() const
{
    QModelIndex current = m_pimoView->selectionModel()->currentIndex();
    if ( current.isValid() )
        return current.data( Nepomuk2::Utils::ClassModel::TypeRole ).value<Nepomuk2::Types::Class>();
    else
        return m_pimoModel->rootClasses().first();
}


namespace {
void setIndexVisible( QTreeView* view, const QModelIndex& index ) {
    QModelIndex parent = view->model()->parent(index);
    if( parent.isValid() ) {
        setIndexVisible(view, parent);
        view->setExpanded(parent, true);
    }
}
}

void ResourceBrowserWidget::setSelectedClass( const Nepomuk2::Types::Class& type )
{
    kDebug() << type;

    QModelIndex index = m_pimoSortModel->mapFromSource(m_pimoModel->indexForClass(type));
    setIndexVisible( m_pimoView, index );
    kDebug() << index << index.data(Nepomuk2::Utils::ClassModel::TypeRole).value<Nepomuk2::Types::Class>();
    m_pimoView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);

    if( Settings::self()->recursiveQuery() ) {
        m_resourceView->setQuery( Nepomuk2::Query::Query( Nepomuk2::Query::ResourceTypeTerm( selectedClass() ) ) );
    }
    else {
        m_resourceView->setQuery( Nepomuk2::Query::Query( Nepomuk2::Query::ComparisonTerm( Soprano::Vocabulary::RDF::type(), Nepomuk2::Query::ResourceTerm( selectedClass().uri() ) ) ) );
    }
}

#include "resourcebrowserwidget.moc"
