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

#ifndef _COMPLETION_MODEL_H_
#define _COMPLETION_MODEL_H_

#include <QtCore/QAbstractItemModel>

namespace Nepomuk {

    class Completion;
    class CompletionItem;

    /**
     * Internal class
     */
    class CompletionModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        CompletionModel( QObject* parent = 0 );
        ~CompletionModel();

        void clear();
        void addCompletionItem( const CompletionItem& item );

        QList<Nepomuk::CompletionItem> allItems() const;

        enum Roles {
            CompletionItemRole = 7777,
            UserDataRole = 7778
        };

        QVariant data( const QModelIndex& index, int role ) const;
        int columnCount( const QModelIndex& parent = QModelIndex() ) const;
        int rowCount( const QModelIndex& parent = QModelIndex() ) const;
        QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
        QModelIndex parent( const QModelIndex& index ) const;

    private:
        class Private;
        Private* const d;
    };
}

#endif
