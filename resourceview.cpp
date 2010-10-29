/*
   Copyright (C) 2009-2010 by Sebastian Trueg <trueg at kde.org>

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

#include "resourceview.h"
#include "nepomukshellsettings.h"
#include "mainwindow.h"

#include "resourcedelegate.h"
#include "pimo.h"

#include <Nepomuk/Thing>
#include <Nepomuk/Variant>
#include <Nepomuk/Types/Class>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Utils/SimpleResourceModel>

#include <kpixmapsequenceoverlaypainter.h>
#include <KDialog>
#include <KUrl>
#include <KDebug>
#include <KIcon>
#include <KActionCollection>

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QDropEvent>
#include <QtCore/QMimeData>

Q_DECLARE_METATYPE(Nepomuk::Types::Class)


ResourceView::ResourceView( QWidget* parent )
    : QWidget( parent ),
      m_queryCount(-1),
      m_queryPage(1)
{
    setupUi(this);

    m_resourceModel = new Nepomuk::Utils::SimpleResourceModel( this );
    m_resourceView->setModel( m_resourceModel );
    //m_resourceView->setSpacing( KDialog::spacingHint() );

    connect( m_resourceView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this, SLOT( slotCurrentResourceChanged( const QItemSelection&, const QItemSelection& ) ) );
    connect( m_resourceView, SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(slotIndexActivated(QModelIndex)) );
    connect( m_resourceView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( slotResourceViewContextMenu( const QPoint& ) ) );

    m_queryClient = new Nepomuk::Query::QueryServiceClient( this );
    connect( m_queryClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
             m_resourceModel, SLOT(addResults(QList<Nepomuk::Query::Result>)) );
    connect( m_queryClient, SIGNAL(finishedListing()),
             this, SLOT(updatePageButtons()) );
    connect( m_queryClient, SIGNAL(totalResultCount(int)),
             this, SLOT(slotTotalResultCount(int)) );

    m_pageBackButton->setIcon( KIcon( QLatin1String("go-previous") ) );
    m_pageForwardButton->setIcon( KIcon( QLatin1String("go-next") ) );
    connect( m_pageBackButton, SIGNAL(clicked()),
             this, SLOT(pageBack()) );
    connect( m_pageForwardButton, SIGNAL(clicked()),
             this, SLOT(pageForward()) );

    updatePageButtons();
}


ResourceView::~ResourceView()
{
}


bool ResourceView::atStart() const
{
    return m_currentQuery.offset() == 0;
}


bool ResourceView::atEnd() const
{
    if( m_queryCount >= 0 ) {
        return m_queryPage*Settings::self()->queryLimit() >= m_queryCount;
    }
    else {
        return m_queryClient->isListingFinished() && m_resourceModel->rowCount() < Settings::self()->queryLimit();
    }
}


void ResourceView::setQuery( const Nepomuk::Query::Query& query )
{
    // reset
    m_queryCount = -1;
    m_queryPage = 1;
    updatePageButtons();

    // prepare query
    m_currentQuery = query;
    m_currentQuery.setOffset(0);
    m_currentQuery.setLimit( Settings::self()->queryLimit() );

    // run query
    listQuery();
}


void ResourceView::addResource( const Nepomuk::Resource& res )
{
    m_resourceModel->addResource( res );
}


void ResourceView::pageBack()
{
    if( !atStart() ) {
        --m_queryPage;
        m_currentQuery.setOffset( qMax( 0, m_currentQuery.offset() - Settings::self()->queryLimit() ) );
        listQuery();
    }
}


void ResourceView::pageForward()
{
    if( !atEnd() ) {
        ++m_queryPage;
        m_currentQuery.setOffset( m_currentQuery.offset() + Settings::self()->queryLimit() );
        listQuery();
    }
}


void ResourceView::updatePageButtons()
{
    m_pageBackButton->setEnabled(!atStart());
    m_pageForwardButton->setEnabled(!atEnd());
    if( m_queryCount >= 0 ) {
        const int numPages = (m_queryCount+Settings::self()->queryLimit())/Settings::self()->queryLimit();
        m_pagesLabel->setText( i18np("%1 result", "%1 results", m_queryCount) + QLatin1String(" - ") + i18np("Page %2 of %1", "Page %2 of %1", numPages, m_queryPage) );
    }
    else {
        m_pagesLabel->setText( i18n("Page %1", m_queryPage) );
    }
}


void ResourceView::slotCurrentResourceChanged( const QItemSelection&, const QItemSelection& )
{
    kDebug();
    emit selectionChanged( selectedResources() );
}


QList<Nepomuk::Resource> ResourceView::selectedResources() const
{
    QList<Nepomuk::Resource> rl;
    QModelIndexList selection = m_resourceView->selectionModel()->selectedIndexes();
    Q_FOREACH( const QModelIndex& index, selection )
        rl << index.data( Nepomuk::Utils::ResourceModel::ResourceRole ).value<Nepomuk::Resource>();
    return rl;
}


void ResourceView::slotIndexActivated( const QModelIndex& index )
{
    if( index.column() == Nepomuk::Utils::ResourceModel::ResourceTypeColumn ) {
        emit resourceTypeActivated( index.data( Nepomuk::Utils::ResourceModel::ResourceTypeRole ).value<Nepomuk::Types::Class>() );
    }
    else {
        emit resourceActivated( index.data( Nepomuk::Utils::ResourceModel::ResourceRole ).value<Nepomuk::Resource>() );
    }
}


void ResourceView::slotResourceViewContextMenu( const QPoint& pos )
{
    kDebug();

    if ( !selectedResources().isEmpty() ) {
        QList<QAction*> actions;
        actions << MainWindow::nepomukShellMain()->actionCollection()->action( QLatin1String( "resource_delete" ) );

        QMenu::exec( actions,
                     m_resourceView->viewport()->mapToGlobal( pos ) );
    }
}


void ResourceView::slotTotalResultCount(int count)
{
    kDebug() << count;
    m_queryCount = count;
    updatePageButtons();
}


void ResourceView::listQuery()
{
    // show busy thingi
    KPixmapSequenceOverlayPainter* op = new KPixmapSequenceOverlayPainter( this );
    op->setWidget( m_resourceView->viewport() );
    connect( m_queryClient, SIGNAL(finishedListing()), op, SLOT( deleteLater() ) );
    op->start();

    // start the query
    m_resourceModel->clear();
    m_queryClient->query(m_currentQuery);
    kDebug() << m_currentQuery;
}

#include "resourceview.moc"
