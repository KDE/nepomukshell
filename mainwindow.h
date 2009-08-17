/*
   Copyright (C) 2008-2009 by Sebastian Trueg <trueg at kde.org>

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

#include <KXmlGuiWindow>
#include "ui_mainwindow.h"

#include <Nepomuk/Types/Class>

class QTreeView;
class QListView;
class QSortFilterProxyModel;
class KAction;
namespace Nepomuk {
    class PIMOItemModel;
    class ResourcePropertyEditModel;
}

class MainWindow : public KXmlGuiWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    Nepomuk::Types::Class selectedClass() const;

private Q_SLOTS:
    void slotPIMOViewContextMenu( const QPoint& pos );
    void slotResourceViewContextMenu( const QPoint& pos );
    void slotCurrentPIMOClassChanged( const QModelIndex& current, const QModelIndex& );
    void slotCurrentResourceChanged( const QItemSelection& current, const QItemSelection& );
    void slotBaseClassChanged( int index );

    void slotCreateClass();
    void slotCreateProperty();
    void slotCreateResource();

private:
    void setupActions();

    Nepomuk::PIMOItemModel* m_pimoModel;
    QSortFilterProxyModel* m_pimoSortModel;
    Nepomuk::ResourcePropertyEditModel* m_propertyModel;

    KAction* m_actionNewSubClass;
    KAction* m_actionNewProperty;
    KAction* m_actionNewResource;
};

#endif
