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

#include "resourcepropertydelegate.h"
#include "resourceinputwidget.h"
#include "resourcepropertymodel.h"

#include <Nepomuk/Types/Class>
#include <Nepomuk/Types/Property>
#include <Nepomuk/Variant>

#include <KDebug>


Q_DECLARE_METATYPE( Nepomuk::Types::Property )
Q_DECLARE_METATYPE( Nepomuk::Variant )


Nepomuk::ResourcePropertyDelegate::ResourcePropertyDelegate( QObject* parent )
    : QStyledItemDelegate( parent )
{
}


Nepomuk::ResourcePropertyDelegate::~ResourcePropertyDelegate()
{
}


QWidget* Nepomuk::ResourcePropertyDelegate::createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
//    kDebug() << parent << option << index;
    QVariant v = index.data( ResourcePropertyEditModel::PropertyRole );
    if ( v.isValid() ) {
        Types::Property property = v.value<Types::Property>();
        if ( property.range().isValid() ) {
            ResourceInputWidget* w = new ResourceInputWidget( parent );
            w->setType( property.range() );
            w->setFocusPolicy( Qt::StrongFocus );
            w->setFrame( false );
            return w;
        }
    }

    return QStyledItemDelegate::createEditor( parent, option, index );
}


void Nepomuk::ResourcePropertyDelegate::setEditorData( QWidget* editor, const QModelIndex& index ) const
{
    kDebug() << editor << index;
    QVariant v = index.data( ResourcePropertyEditModel::PropertyRole );
    if ( v.isValid() ) {
        Types::Property property = v.value<Types::Property>();
        if ( property.range().isValid() ) {
            ResourceInputWidget* w = qobject_cast<ResourceInputWidget*>( editor );
            w->setResource( index.data( Qt::EditRole ).value<Nepomuk::Resource>() );
            kDebug() << index.data( Qt::EditRole ).value<Nepomuk::Resource>().resourceUri();
            return;
        }
    }

    QStyledItemDelegate::setEditorData( editor, index );
}


void Nepomuk::ResourcePropertyDelegate::setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
    kDebug() << editor << model << index;
    QVariant v = index.data( ResourcePropertyEditModel::PropertyRole );
    if ( v.isValid() ) {
        Types::Property property = v.value<Types::Property>();
        if ( property.range().isValid() ) {
            ResourceInputWidget* w = qobject_cast<ResourceInputWidget*>( editor );
            kDebug() << w->selectedResource().uri();
            model->setData( index, w->selectedResource().resourceUri() );
            return;
        }
    }

    QStyledItemDelegate::setModelData( editor, model, index );
}

#include "resourcepropertydelegate.moc"
