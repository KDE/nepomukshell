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

#ifndef _NEPOMUK_RESOURCE_EDITOR_WIDGET_H_
#define _NEPOMUK_RESOURCE_EDITOR_WIDGET_H_

#include <QtGui/QWidget>
#include <QtCore/QStack>

#include <Nepomuk2/Resource>
#include <Nepomuk2/Types/Class>

#include "ui_resourceeditorwidget.h"

namespace Nepomuk2 {
    class ResourcePropertyEditModel;
}

class ResourceEditorWidget : public QWidget, private Ui::ResourceEditorWidget
{
    Q_OBJECT

public:
    ResourceEditorWidget( QWidget* parent = 0 );
    ~ResourceEditorWidget();

    Nepomuk2::Resource resource() const { return m_resource; }

Q_SIGNALS:
    void resourceActivated( const Nepomuk2::Resource& res );

public Q_SLOTS:
    void setResource( const Nepomuk2::Resource& res );

private Q_SLOTS:
    void slotPropertyContextMenu( const QPoint& pos );
    void slotNodeActivated( const QModelIndex& index );
    void slotResourceHistoryBack();
    void slotResourceHistoryForward();

private:
    void setResourceInternal( const Nepomuk2::Resource& res );
    void updateResourceHistoryButtonStates();

    Nepomuk2::ResourcePropertyEditModel* m_propertyModel;
    Nepomuk2::ResourcePropertyEditModel* m_backlinksModel;
    Nepomuk2::Resource m_resource;

    QStack<QUrl> m_backStack;
    QStack<QUrl> m_forwardStack;
};

#endif
