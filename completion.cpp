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

#include "completion.h"
#include "completionmodel.h"


Q_DECLARE_METATYPE( Nepomuk::CompletionItem )


class Nepomuk::Completion::Private
{
public:
    void _k_activated( const QModelIndex& index );

    CompletionModel* model;

    Completion* q;
};


void Nepomuk::Completion::Private::_k_activated( const QModelIndex& index )
{
    emit q->activated( index.data( CompletionModel::CompletionItemRole ).value<CompletionItem>() );
}


Nepomuk::Completion::Completion( QObject* parent )
    : QCompleter( parent ),
      d( new Private() )
{
    d->q = this;
    d->model = new CompletionModel( this );
    setModel( d->model );

    connect( this, SIGNAL( activated( QModelIndex ) ),
             this, SLOT( _k_activated( QModelIndex ) ) );
}


Nepomuk::Completion::~Completion()
{
    delete d;
}


void Nepomuk::Completion::setCompletionText( const QString& text )
{
    d->model->clear();
    makeCompletion( text );
    setCompletionPrefix( text );
}


QList<Nepomuk::CompletionItem> Nepomuk::Completion::allItems() const
{
    return d->model->allItems();
}


void Nepomuk::Completion::addCompletion( const CompletionItem& item )
{
    d->model->addCompletionItem( item );
}

#include "completion.moc"
