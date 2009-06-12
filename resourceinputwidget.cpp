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

#include "resourceinputwidget.h"
#include "resourcecompletion.h"
#include "kcompletionitem.h"

#include <Nepomuk/Resource>
#include <Nepomuk/Types/Class>

#include <QtGui/QHBoxLayout>
#include <QtGui/QCompleter>

#include <KDebug>


class Nepomuk::ResourceInputWidget::Private
{
public:
    ResourceCompletion* completion;
    Resource selectedResource;
};


Nepomuk::ResourceInputWidget::ResourceInputWidget( QWidget* parent )
    : KLineEdit( parent ),
      d( new Private() )
{
    // setup completion
    d->completion = new ResourceCompletion( this );
    d->completion->setCaseSensitivity(Qt::CaseInsensitive);
    setCompleter( d->completion );

    connect( this, SIGNAL( textEdited( QString ) ),
             this, SLOT( slotTextChanged( QString ) ) );
    connect( d->completion, SIGNAL( activated( KCompletionItem ) ),
             this, SLOT( slotActivated( KCompletionItem ) ) );
}


Nepomuk::ResourceInputWidget::~ResourceInputWidget()
{
    delete d;
}


Nepomuk::Types::Class Nepomuk::ResourceInputWidget::type() const
{
    return d->completion->type();
}


void Nepomuk::ResourceInputWidget::setType( const Nepomuk::Types::Class& type )
{
    d->completion->setType( type );
}


void Nepomuk::ResourceInputWidget::setResource( const Resource& res )
{
    d->selectedResource = res;
    setText( res.genericLabel() );
}


Nepomuk::Resource Nepomuk::ResourceInputWidget::selectedResource() const
{
    return d->selectedResource;
}


void Nepomuk::ResourceInputWidget::slotTextChanged( const QString& s )
{
    d->selectedResource = Resource();
    d->completion->setCompletionText( s );
}


void Nepomuk::ResourceInputWidget::slotActivated( const KCompletionItem& item )
{
    kDebug() << "selected resource" << item.userData().toUrl();
    d->selectedResource = item.userData().toUrl();
}

#include "resourceinputwidget.moc"
