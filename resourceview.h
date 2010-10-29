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

#include <QtGui/QWidget>
#include "ui_resourceview.h"

#include <Nepomuk/Query/Query>

class QItemSelection;
class QModelIndex;
namespace Nepomuk {
    namespace Types {
        class Class;
    }
    namespace Utils {
        class SimpleResourceModel;
    }
    namespace Query {
        class QueryServiceClient;
    }
    class Resource;
}

/**
 * Displays a list of resources in pages. Use pageForward() and
 * pageBack() to browse the pages.
 */
class ResourceView : public QWidget, public Ui::ResourceView
{
    Q_OBJECT

public:
    ResourceView( QWidget* parent = 0 );
    ~ResourceView();

    QList<Nepomuk::Resource> selectedResources() const;

public Q_SLOTS:
    void setQuery( const Nepomuk::Query::Query& query );

    /**
     * Convenience method to quickly update the list
     * after creation of a resource without having to
     * reload everything
     */
    void addResource( const Nepomuk::Resource& res );

Q_SIGNALS:
    void selectionChanged( const QList<Nepomuk::Resource>& );
    void resourceActivated( const Nepomuk::Resource& );
    void resourceTypeActivated( const Nepomuk::Types::Class& );

private Q_SLOTS:
    void pageBack();
    void pageForward();
    void updatePageButtons();
    void slotCurrentResourceChanged( const QItemSelection&, const QItemSelection& );
    void slotIndexActivated( const QModelIndex& index );
    void slotResourceViewContextMenu( const QPoint& pos );
    void slotTotalResultCount( int );

private:
    void listQuery();
    bool atStart() const;
    bool atEnd() const;

    Nepomuk::Query::Query m_currentQuery;
    Nepomuk::Query::QueryServiceClient* m_queryClient;
    Nepomuk::Utils::SimpleResourceModel* m_resourceModel;
    int m_queryCount;
    int m_queryPage;
};

#endif
