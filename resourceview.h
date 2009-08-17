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

#ifndef _RESOURCE_VIEW_H_
#define _RESOURCE_VIEW_H_

#include <QtGui/QListView>

class QDropEvent;
class QDragEnterEvent;
class QDragMoveEvent;
namespace Nepomuk {
    namespace Types {
        class Class;
    }
    class Resource;
    class AsyncLoadingResourceModel;
}


class ResourceView : public QListView
{
    Q_OBJECT

public:
    ResourceView( QWidget* parent = 0 );
    ~ResourceView();

public Q_SLOTS:
    /**
     * Load all resources of type \a type async.
     */
    void setType( const Nepomuk::Types::Class& type );

    /**
     * Convenience method to quickly update the list
     * after creation of a resource without having to
     * reload everything
     */
    void addResource( const Nepomuk::Resource& res );

private:
    void dropEvent( QDropEvent* e );
    void dragEnterEvent( QDragEnterEvent* event );
    void dragMoveEvent( QDragMoveEvent* event );

    Nepomuk::AsyncLoadingResourceModel* m_resourceModel;
};

#endif
