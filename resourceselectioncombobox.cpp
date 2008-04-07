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

#include "resourceselectioncombobox.h"

class Nepomuk::ResourceSelectionComboBox::Private
{
public:
    Types::Class baseType;
    // FIXME: use a model that loads data only when requested (QAbstractItemModel::canFetchMore)
    SimpleResourceModel* model;
};


Nepomuk::ResourceSelectionComboBox::ResourceSelectionComboBox( QWidget* parent )
    : QComboBox( parent ),
      d( new Private() )
{
    d->model = new SimpleResourceModel( this );
    setModel( d->model );
}


Nepomuk::ResourceSelectionComboBox::~ResourceSelectionComboBox()
{
    delete d;
}


void Nepomuk::ResourceSelectionComboBox::setType( const Types::Class& type )
{
    d->baseType = type;

    // select all resources.
    // FIXME: here we need inference
    // FIXME: This can become a really big list!
    QList<Resource> rl;

}


Nepomuk::Resource Nepomuk::ResourceSelectionComboBox::selectedResource() const
{
}

#include "resourceselectioncombobox.moc"
