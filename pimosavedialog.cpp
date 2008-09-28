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

#include "pimosavedialog.h"
#include "pimo.h"
#include "typecompletion.h"

#include <Nepomuk/Types/Class>
#include <Nepomuk/Resource>
#include <Nepomuk/Variant>
#include <Nepomuk/ResourceManager>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/XMLSchema>
#include <Soprano/Vocabulary/Xesam>

#include <KTitleWidget>
#include <KLineEdit>
#include <KLocale>
#include <KStandardDirs>

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QListView>
#include <QtGui/QGroupBox>
#include <QtGui/QStandardItemModel>
#include <QtGui/QStandardItem>
#include <QtCore/QDir>
#include <QtCore/QFile>


static const int TypeUriRole = 7777;

class Nepomuk::FileDialog::Private
{
public:
    Private()
        : baseType( Vocabulary::PIMO::Document() ) {
    }

    Nepomuk::Types::Class baseType;
    QString extension;
    KUrl lastUrl;

    KLineEdit* editName;
    QListView* typesView;
    KLineEdit* editType;

    TypeCompletion* typeCompletion;

    QStandardItemModel* typeModel;

    QStandardItem* createItem( Nepomuk::Types::Class type );
    void _k_typeActivated( const Nepomuk::CompletionItem& );
    void _k_dialogFinished();
    QList<QUrl> types() const;
    KUrl createUniqueUrl() const;
};


QStandardItem* Nepomuk::FileDialog::Private::createItem( Nepomuk::Types::Class type )
{
    QStandardItem* item = new QStandardItem( type.icon(), type.label() );
    item->setToolTip( type.comment() );
    item->setCheckable( true );
    item->setData( type.uri(), TypeUriRole );
    return item;
}


void Nepomuk::FileDialog::Private::_k_typeActivated( const Nepomuk::CompletionItem& item )
{
    QUrl ru = item.userData().toUrl();
    typeModel->appendRow( createItem( Types::Class( ru ) ) );
}


void Nepomuk::FileDialog::Private::_k_dialogFinished()
{
    lastUrl = createUniqueUrl();
}


QList<QUrl> Nepomuk::FileDialog::Private::types() const
{
    QList<QUrl> tl;
    for ( int i = 0; i < typeModel->rowCount(); ++i ) {
        QStandardItem* item = typeModel->item( i );
        if ( item->checkState() == Qt::Checked ) {
            tl << item->data( TypeUriRole ).toUrl();
        }
    }
    return tl;
}


KUrl Nepomuk::FileDialog::Private::createUniqueUrl() const
{
    QString basePath = QDir::homePath() + "/Nepomuk/";
    KStandardDirs::makeDir( basePath );
    QString name = editName->text() + '.' + extension;
    int i = 0;
    while ( QFile::exists( basePath + name ) ) {
        name = editName->text() + QString::number( ++i ) + '.' + extension;
    }
    return basePath + name;
}



Nepomuk::FileDialog::FileDialog( QWidget* parent )
    : KDialog( parent ),
      d( new Private() )
{
    KTitleWidget* title = new KTitleWidget( mainWidget() );
    title->setText( i18n( "Save Document" ) );
    title->setComment( i18n( "...on the Semantic Desktop" ) );
    title->setPixmap( KIcon( "nepomuk" ).pixmap( 32, 32 ), KTitleWidget::ImageRight );

    QLabel* label = new QLabel( i18n( "Name/Description:" ), mainWidget() );
    d->editName = new KLineEdit( mainWidget() );
    QHBoxLayout* nameLay = new QHBoxLayout;
    nameLay->addWidget( label );
    nameLay->addWidget( d->editName );

    QGroupBox* typeGroup = new QGroupBox( i18n( "Type" ), mainWidget() );
    d->typesView = new QListView( typeGroup );
    d->editType = new KLineEdit( typeGroup );
    QVBoxLayout* typeLay = new QVBoxLayout( typeGroup );
    typeLay->addWidget( d->typesView );
    typeLay->addWidget( d->editType );

    QVBoxLayout* mainLay = new QVBoxLayout( mainWidget() );
    mainLay->addWidget( title );
    mainLay->addItem( nameLay );
    mainLay->addWidget( typeGroup );

    d->typeCompletion = new TypeCompletion( d->editType );
    d->typeCompletion->setCaseSensitivity(Qt::CaseInsensitive);
    d->editType->setCompleter( d->typeCompletion );
    connect( d->editType, SIGNAL( textEdited( QString ) ),
             d->typeCompletion, SLOT( setCompletionText( QString ) ) );

    d->typeModel = new QStandardItemModel( d->typesView );
    d->typesView->setModel( d->typeModel );

    connect( d->editType->completer(), SIGNAL( activated( Nepomuk::CompletionItem ) ),
             this, SLOT( _k_typeActivated( Nepomuk::CompletionItem ) ) );

    connect( this, SIGNAL( finished() ),
             this, SLOT( _k_dialogFinished() ) );
}


Nepomuk::FileDialog::~FileDialog()
{
    delete d;
}


Nepomuk::Types::Class Nepomuk::FileDialog::baseType() const
{
    return d->baseType;
}


void Nepomuk::FileDialog::setBaseType( const Types::Class& type )
{
    d->baseType = type;
}


void Nepomuk::FileDialog::setExtension( const QString& ext )
{
    d->extension = ext;
}


KUrl Nepomuk::FileDialog::selectedFile() const
{
    return d->lastUrl;
}


void Nepomuk::FileDialog::saveMetaData()
{
    Nepomuk::Resource res( selectedFile(), Soprano::Vocabulary::Xesam::File() );
    res.setProperty( Soprano::Vocabulary::Xesam::url(), selectedFile().url() );

    Nepomuk::Resource pimoRes;
    pimoRes.setLabel( d->editName->text() );
    pimoRes.setProperty( Vocabulary::PIMO::groundingOccurrence(), res );
    pimoRes.setTypes( d->types() );

//     NRLModel nrlModel( ResourceManager::instance()->mainModel() );
//     QUrl uri = nrlModel.createResource( d->baseType.uri(),
//                                         d->editName.text() ); // FIXME: comment
//     nrlModel.addStatements( QList<Statement>()
//                             << Statement( uri, Vocabulary::PIMO::groundingResource(), res.uri() )
}


KUrl Nepomuk::FileDialog::getSaveUrl( const QString& extension,
                                      QWidget* parent,
                                      const QString& caption )
{
    FileDialog dlg( parent );
    dlg.setCaption( caption );
    dlg.setExtension( extension );
    if ( dlg.exec() ) {
        dlg.d->lastUrl = dlg.d->createUniqueUrl();
        dlg.saveMetaData();
        return dlg.selectedFile();
    }
    else {
        return QString();
    }
}

#include "pimosavedialog.moc"
