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

#ifndef _NEPOMUK_RESOURCE_QUERY_WIDGET_H_
#define _NEPOMUK_RESOURCE_QUERY_WIDGET_H_

#include <QtGui/QWidget>

#include <Nepomuk/Resource>
#include <Nepomuk/Types/Class>

#include "ui_resourcequerywidget.h"

namespace Nepomuk {
    class QueryModel;
}

class ResourceQueryWidget : public QWidget, private Ui::ResourceQueryWidget
{
    Q_OBJECT

public:
    ResourceQueryWidget( QWidget* parent = 0 );
    ~ResourceQueryWidget();

Q_SIGNALS:
    void resourceActivated( const Nepomuk::Resource& res );

private Q_SLOTS:
    void slotQueryButtonClicked();
    void slotNodeActivated( const QModelIndex& index );

private:
    Nepomuk::QueryModel* m_queryModel;
};

#endif
