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

#ifndef _NEPOMUK_RESOURCE_PROPERTY_DELEGATE_H_
#define _NEPOMUK_RESOURCE_PROPERTY_DELEGATE_H_

#include <QtGui/QStyledItemDelegate>

namespace Nepomuk {
    class ResourcePropertyDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        ResourcePropertyDelegate( QObject* parent = 0 );
        ~ResourcePropertyDelegate();

        QWidget * createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
        void setEditorData( QWidget* editor, const QModelIndex& index ) const;
        void setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const;
    };
}

#endif
