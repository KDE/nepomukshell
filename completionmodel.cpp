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

#include "completionmodel.h"
#include "completion.h"

#include <QtGui/QIcon>

#include <KDebug>


Q_DECLARE_METATYPE( Nepomuk::CompletionItem )

class Nepomuk::CompletionModel::Private
{
public:
    Private( CompletionModel* parent )
        : q( parent ) {
    }

    QList<CompletionItem> items;

private:
    CompletionModel* q;
};


Nepomuk::CompletionModel::CompletionModel( QObject* parent )
    : QAbstractItemModel( parent ),
      d( new Private( this ) )
{
}


Nepomuk::CompletionModel::~CompletionModel()
{
    delete d;
}


void Nepomuk::CompletionModel::clear()
{
    d->items.clear();
    reset();
}


QList<Nepomuk::CompletionItem> Nepomuk::CompletionModel::allItems() const
{
    return d->items;
}


void Nepomuk::CompletionModel::addCompletionItem( const CompletionItem& item )
{
    beginInsertRows( QModelIndex(), d->items.count(), d->items.count() );
    d->items.append( item );
    endInsertRows();
}


QVariant Nepomuk::CompletionModel::data( const QModelIndex& index, int role ) const
{
    if ( index.isValid() &&
         index.row() < d->items.count() &&
         index.column() == 0 ) {

        CompletionItem item = d->items[index.row()];

        switch( role ) {
        case Qt::EditRole:
            return item.completionText();

        case Qt::DisplayRole:
            return item.text();

        case Qt::ToolTipRole:
            return item.toolTip();

        case Qt::DecorationRole:
            return item.icon();

        case UserDataRole:
            return item.userData();

        case CompletionItemRole:
            return QVariant::fromValue( item );

        default:
            return QVariant();
        }
    }

    return QVariant();
}


int Nepomuk::CompletionModel::columnCount( const QModelIndex& ) const
{
    return 1;
}


int Nepomuk::CompletionModel::rowCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) {
        return 0;
    }
    else {
        return d->items.count();
    }
}


QModelIndex Nepomuk::CompletionModel::index( int row, int column, const QModelIndex& ) const
{
    if ( row < d->items.count() && column == 0 ) {
        return createIndex( row, column );
    }
    return QModelIndex();
}


QModelIndex Nepomuk::CompletionModel::parent( const QModelIndex& ) const
{
    return QModelIndex();
}

#include "completionmodel.moc"
