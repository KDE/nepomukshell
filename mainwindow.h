/*
   Copyright (C) 2008 by Sebastian Trueg <trueg at kde.org>

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

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <KMainWindow>
#include "ui_mainwindow.h"

class QTreeView;
class QListView;
class QSortFilterProxyModel;
namespace Nepomuk {
    class PIMOItemModel;
    class SimpleResourceModel;
    class ResourcePropertyEditModel;
}

class MainWindow : public KMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private Q_SLOTS:
    void slotPIMOViewContextMenu( const QPoint& pos );
    void slotCurrentPIMOClassChanged( const QModelIndex& current, const QModelIndex& );
    void slotCurrentResourceChanged( const QItemSelection& current, const QItemSelection& );
    void slotBaseClassChanged( int index );

private:
    Nepomuk::PIMOItemModel* m_pimoModel;
    QSortFilterProxyModel* m_pimoSortModel;
    Nepomuk::SimpleResourceModel* m_resourceModel;
    Nepomuk::ResourcePropertyEditModel* m_propertyModel;
};

#endif
