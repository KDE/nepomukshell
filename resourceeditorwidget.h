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

#include <Nepomuk/Resource>
#include <Nepomuk/Types/Class>

#include "ui_resourceeditorwidget.h"

namespace Nepomuk {
    class ResourcePropertyEditModel;
}

class ResourceEditorWidget : public QWidget, private Ui::ResourceEditorWidget
{
    Q_OBJECT

public:
    ResourceEditorWidget( QWidget* parent = 0 );
    ~ResourceEditorWidget();

    Nepomuk::Resource resource() const { return m_resource; }

public Q_SLOTS:
    void setResource( const Nepomuk::Resource& res );

private Q_SLOTS:
    void slotPropertyContextMenu( const QPoint& pos );

private:
    Nepomuk::ResourcePropertyEditModel* m_propertyModel;
    Nepomuk::Resource m_resource;
};

#endif
