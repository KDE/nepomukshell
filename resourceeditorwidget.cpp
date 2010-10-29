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

#include "resourceeditorwidget.h"
#include "resourcepropertymodel.h"

#include <QtGui/QTableView>
#include <QtGui/QMenu>

#include <KUrl>
#include <KDebug>
#include <KAction>
#include <KIcon>


ResourceEditorWidget::ResourceEditorWidget( QWidget* parent )
    : QWidget( parent )
{
    setupUi( this );

    m_buttonBack->setIcon( KIcon( QLatin1String( "go-previous" ) ) );
    m_buttonForward->setIcon( KIcon( QLatin1String( "go-next" ) ) );

    m_propertyModel = new Nepomuk::ResourcePropertyEditModel( m_propertyView );
    m_propertyView->setModel( m_propertyModel );
    m_propertyView->setContextMenuPolicy( Qt::CustomContextMenu );

    m_backlinksModel = new Nepomuk::ResourcePropertyEditModel( m_backlinksView );
    m_backlinksModel->setMode( Nepomuk::ResourcePropertyEditModel::BacklinksMode );
    m_backlinksView->setModel( m_backlinksModel );
    m_backlinksView->setContextMenuPolicy( Qt::CustomContextMenu );

    connect( m_propertyView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( slotPropertyContextMenu( const QPoint& ) ) );
    connect( m_backlinksView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( slotPropertyContextMenu( const QPoint& ) ) );
    connect( m_propertyView, SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(slotNodeActivated(QModelIndex)) );
    connect( m_backlinksView, SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(slotNodeActivated(QModelIndex)) );
    connect(m_buttonBack, SIGNAL(clicked()),
            this, SLOT(slotResourceHistoryBack()) );
    connect(m_buttonForward, SIGNAL(clicked()),
            this, SLOT(slotResourceHistoryForward()) );

    updateResourceHistoryButtonStates();
}


ResourceEditorWidget::~ResourceEditorWidget()
{
}


void ResourceEditorWidget::setResource( const Nepomuk::Resource& res )
{
    if( resource().isValid() )
        m_backStack.push( resource().resourceUri() );
    m_forwardStack.clear();
    setResourceInternal( res );
    updateResourceHistoryButtonStates();
}


void ResourceEditorWidget::slotPropertyContextMenu( const QPoint& pos )
{
    kDebug();
    QTableView* view = qobject_cast<QTableView*>( sender() );
    QModelIndex index = view->indexAt( pos );
    if ( index.isValid() ) {
        QList<QAction*> actions;

        KAction actionDelete(this);
        actionDelete.setText( i18n("Delete statement") );
        actionDelete.setIcon( KIcon( QLatin1String( "edit-delete" ) ) );
        actions.append( &actionDelete );

        QAction* a = QMenu::exec( actions,
                                  view->viewport()->mapToGlobal( pos ) );
        if( a == &actionDelete ) {
            view->model()->removeRow( index.row() );
        }
    }

}


void ResourceEditorWidget::slotNodeActivated( const QModelIndex& index )
{
    Soprano::Node node = qobject_cast<const Nepomuk::ResourcePropertyEditModel*>( index.model() )->nodeForIndex( index );
    if ( node.isValid() && node.isResource() ) {
        Nepomuk::Resource res = Nepomuk::Resource::fromResourceUri( node.uri() );
        if( res.exists() ) {
            emit resourceActivated( res );
        }
    }
}


void ResourceEditorWidget::slotResourceHistoryBack()
{
    if( !m_backStack.isEmpty() ) {
        m_forwardStack.push( resource().resourceUri() );
        setResourceInternal( m_backStack.pop() );
        updateResourceHistoryButtonStates();
    }
}


void ResourceEditorWidget::slotResourceHistoryForward()
{
    if( !m_forwardStack.isEmpty() ) {
        m_backStack.push( resource().resourceUri() );
        setResourceInternal( m_forwardStack.pop() );
        updateResourceHistoryButtonStates();
    }
}


void ResourceEditorWidget::setResourceInternal( const Nepomuk::Resource& res )
{
    if( m_resource != res ) {
        m_resource = res;
        m_propertyModel->setResource( res );
        m_backlinksModel->setResource( res );
        m_uriLabel->setText( KUrl(res.resourceUri()).url() );
    }
}


void ResourceEditorWidget::updateResourceHistoryButtonStates()
{
    m_buttonBack->setEnabled( !m_backStack.isEmpty() );
    m_buttonForward->setEnabled( !m_forwardStack.isEmpty() );
}

#include "resourceeditorwidget.moc"
