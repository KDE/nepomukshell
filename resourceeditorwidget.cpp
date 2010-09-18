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
#include "statementeditor.h"

#include <QtGui/QTableView>
#include <QtGui/QMenu>

#include <KUrl>
#include <KDebug>
#include <KAction>


ResourceEditorWidget::ResourceEditorWidget( QWidget* parent )
    : QWidget( parent )
{
    setupUi( this );

    m_propertyModel = new Nepomuk::ResourcePropertyEditModel( m_propertyView );
    m_propertyView->setModel( m_propertyModel );
    m_propertyView->setContextMenuPolicy( Qt::CustomContextMenu );

    connect( m_propertyView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( slotPropertyContextMenu( const QPoint& ) ) );
    connect( m_propertyView, SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(slotNodeActivated(QModelIndex)) );
}


ResourceEditorWidget::~ResourceEditorWidget()
{
}


void ResourceEditorWidget::setResource( const Nepomuk::Resource& res )
{
    if( m_resource != res ) {
        m_resource = res;
        m_propertyModel->setResource( res );
        m_uriLabel->setText( KUrl(res.resourceUri()).url() );
        m_statementEditor->setResource( res );
    }
}


void ResourceEditorWidget::slotPropertyContextMenu( const QPoint& pos )
{
    kDebug();
    QModelIndex index = m_propertyView->indexAt( pos );
    if ( index.isValid() ) {
        QList<QAction*> actions;

        KAction actionDelete(this);
        actionDelete.setText( i18n("Delete statement") );
        actionDelete.setIcon( KIcon("edit-delete") );
        actions.append( &actionDelete );

        QAction* a = QMenu::exec( actions,
                                  m_propertyView->viewport()->mapToGlobal( pos ) );
        if( a == &actionDelete ) {
            m_propertyModel->removeRow( index.row() );
        }
    }

}


void ResourceEditorWidget::slotNodeActivated( const QModelIndex& index )
{
    Soprano::Node node = m_propertyModel->nodeForIndex( index );
    if ( node.isValid() && node.isResource() ) {
        Nepomuk::Resource res( node.uri() );
        if( res.exists() ) {
            emit resourceActivated( res );
        }
    }
}

#include "resourceeditorwidget.moc"
