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

#ifndef _NEPOMUK_RESOURCE_BROWSER_WIDGET_H_
#define _NEPOMUK_RESOURCE_BROWSER_WIDGET_H_

#include <QtGui/QWidget>

#include <Nepomuk2/Resource>
#include <Nepomuk2/Types/Class>

#include "ui_resourcebrowserwidget.h"

class QSortFilterProxyModel;
namespace Nepomuk2 {
    namespace Utils {
        class ClassModel;
    }
}

class ResourceBrowserWidget : public QWidget, private Ui::ResourceBrowserWidget
{
    Q_OBJECT

public:
    ResourceBrowserWidget( QWidget* parent = 0 );
    ~ResourceBrowserWidget();

    QList<Nepomuk2::Resource> selectedResources() const;
    Nepomuk2::Types::Class selectedClass() const;

Q_SIGNALS:
    void resourcesSelected( const QList<Nepomuk2::Resource>& res );
    void resourceActivated( const Nepomuk2::Resource& res );

public Q_SLOTS:
    void setSelectedClass( const Nepomuk2::Types::Class& type );
    void createClass();
    void createProperty();
    void createResource();

private Q_SLOTS:
    void slotPIMOViewContextMenu( const QPoint& pos );
    void slotCurrentPIMOClassChanged( const QModelIndex& current, const QModelIndex& );
    void slotBaseClassChanged( int index );

private:
    void updateQuery( int offset );

    Nepomuk2::Utils::ClassModel* m_pimoModel;
    QSortFilterProxyModel* m_pimoSortModel;
};

#endif
