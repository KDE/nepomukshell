/*
   Copyright (C) 2009 by Sebastian Trueg <trueg at kde.org>

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

#include "resourceview.h"
#include "asyncloadingresourcemodel.h"

#include "resourcedelegate.h"
#include "kpixmapsequenceoverlaypainter.h"
#include "pimo.h"

#include <Nepomuk/Thing>
#include <Nepomuk/Variant>
#include <Nepomuk/Types/Class>

#include <KDialog>
#include <KUrl>
#include <KDebug>

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QDropEvent>
#include <QtCore/QMimeData>


ResourceView::ResourceView( QWidget* parent )
    : QListView( parent )
{
    m_resourceModel = new Nepomuk::AsyncLoadingResourceModel( this );
    setModel( m_resourceModel );
    setContextMenuPolicy( Qt::CustomContextMenu );
    Nepomuk::ResourceDelegate* delegate = new Nepomuk::ResourceDelegate( this );
    delegate->setDisplayMode( Nepomuk::AbstractResourceGuiItem::DisplayFull );
    setItemDelegateForColumn( 0, delegate );
    setIconSize( QSize( 48, 48 ) );
    setSpacing( KDialog::spacingHint() );
    setDragEnabled(true);
    setAcceptDrops(true);
}


ResourceView::~ResourceView()
{
}


void ResourceView::setType( const Nepomuk::Types::Class& type )
{
    KPixmapSequenceOverlayPainter* op = new KPixmapSequenceOverlayPainter( this );
    op->setWidget( viewport() );
    connect( m_resourceModel, SIGNAL( finishedLoading() ), op, SLOT( deleteLater() ) );
    op->start();

    m_resourceModel->loadResourcesOfType( type );
}


void ResourceView::addResource( const Nepomuk::Resource& res )
{
    m_resourceModel->addResource( res );
}


void ResourceView::dragEnterEvent( QDragEnterEvent* event )
{
    event->setAccepted( event->mimeData()->hasFormat( "text/uri-list" ) );
}


void ResourceView::dragMoveEvent( QDragMoveEvent* event )
{
    event->setAccepted( event->mimeData()->hasFormat( "text/uri-list" ) &&
                        indexAt( event->pos() ).isValid() );
}


void ResourceView::dropEvent( QDropEvent* e )
{
    KUrl::List uris = KUrl::List::fromMimeData( e->mimeData(), KUrl::List::PreferLocalUrls );

    if ( uris.count() == 1 ) {
        QModelIndex index = indexAt( e->pos() );
        if ( index.isValid() ) {
            Nepomuk::Resource selectedRes( index.data( Nepomuk::ResourceModel::ResourceUriRole ).value<QUrl>() );
            Nepomuk::Resource droppedRes( uris.first() );

            QAction actionRelate( KIcon(), i18n( "Relate to '%1'", droppedRes.genericLabel() ), this );
            QAction* a = QMenu::exec( QList<QAction*>() << &actionRelate,
                                      viewport()->mapToGlobal( e->pos() ) );
            if ( a == &actionRelate ) {
                selectedRes.addProperty( Nepomuk::Vocabulary::PIMO::isRelated(), droppedRes.pimoThing() );
            }
        }
    }
}

#include "resourceview.moc"
