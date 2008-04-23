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

#include "newclassdialog.h"
#include "nrlmodel.h"
#include "pimoitemmodel.h"

#include <nepomuk/class.h>
#include <nepomuk/property.h>
#include <nepomuk/resource.h>
#include <nepomuk/resourcemanager.h>

#include <KDebug>
#include <KLocale>
#include <KTitleWidget>

#include <QtGui/QTreeView>
#include <QtGui/QHeaderView>

#include <Soprano/Vocabulary/RDFS>


NewClassDialog::NewClassDialog( QWidget* parent )
    : KDialog( parent )
{
    setCaption( i18n( "Create new Type" ) );
    setButtons( Ok|Cancel );
    enableButtonOk( false );

    setupUi( mainWidget() );

    connect( m_editClassLabel, SIGNAL( textChanged(const QString&) ),
             this, SLOT( slotLabelChanged(const QString&) ) );
}


NewClassDialog::~NewClassDialog()
{
}


void NewClassDialog::slotLabelChanged( const QString& text )
{
    enableButtonOk( !text.isEmpty() );
}


Nepomuk::Types::Class NewClassDialog::createClass( Nepomuk::Types::Class parentClass, QWidget* parent )
{
    NewClassDialog dlg( parent );
    dlg.m_labelTitle->setText( i18n( "Create New Type" ) );
    dlg.m_labelTitle->setComment( i18n( "based on <i>%1</i>", parentClass.label() )  );
    dlg.m_rangeWidget->hide();
    QIcon icon = parentClass.icon();
    if ( icon.isNull() ) {
        icon = KIcon( "nepomuk" );
    }
    dlg.m_labelTitle->setPixmap( icon.pixmap( 32, 32 ) );

    if ( dlg.exec() ) {
        QString name = dlg.m_editClassLabel->text();
        QString comment = dlg.m_editClassComment->text();
        QString icon = dlg.m_buttonClassIcon->icon();

        Soprano::NRLModel nrlModel( Nepomuk::ResourceManager::instance()->mainModel() );
        nrlModel.setBaseNamespace( QUrl( "nepomuk:/classes" ) ); // FIXME: Use the PIMO NS
        return nrlModel.createClass( parentClass.uri(),
                                     name,
                                     comment,
                                     icon != "unknown" ? icon : QString() );
    }
    else {
        return Nepomuk::Types::Class();
    }
}


Nepomuk::Types::Property NewClassDialog::createProperty( Nepomuk::Types::Class parentClass, QWidget* parent )
{
    NewClassDialog dlg( parent );
    dlg.m_labelTitle->setText( i18n( "Create New Property" ) );
    dlg.m_labelTitle->setComment( i18n( "for <i>%1</i>", parentClass.label() )  );
    QIcon icon = parentClass.icon();
    if ( icon.isNull() ) {
        icon = KIcon( "nepomuk" );
    }
    dlg.m_labelTitle->setPixmap( icon.pixmap( 32, 32 ) );

    // FIXME: hopefully at some point QComboBox will support QTreeView
    // we can then use the PIMOItemModel.
    Nepomuk::Types::Class base( Soprano::Vocabulary::RDFS::Resource() );
    dlg.m_propertyRangeCombo->addItem( base.label(), base.uri() );
    foreach( Nepomuk::Types::Class c, base.allSubClasses() ) {
        dlg.m_propertyRangeCombo->addItem( c.label(), c.uri() );
    }

    if ( dlg.exec() ) {
        QString name = dlg.m_editClassLabel->text();
        QString comment = dlg.m_editClassComment->text();
        QString icon = dlg.m_buttonClassIcon->icon();

        Soprano::NRLModel nrlModel( Nepomuk::ResourceManager::instance()->mainModel() );
        nrlModel.setBaseNamespace( QUrl( "nepomuk:/properties" ) ); // FIXME: Use the PIMO NS
        return nrlModel.createProperty( parentClass.uri(),
                                        dlg.m_propertyRangeCombo->itemData( dlg.m_propertyRangeCombo->currentIndex() ).toUrl(),
                                        name,
                                        comment,
                                        icon != "unknown" ? icon : QString() );
    }
    else {
        return Nepomuk::Types::Property();
    }
}


Nepomuk::Resource NewClassDialog::createResource( Nepomuk::Types::Class type, QWidget* parent )
{
    NewClassDialog dlg( parent );
    dlg.m_labelTitle->setText( i18n( "Create New Resource" ) );
    dlg.m_labelTitle->setComment( i18n( "of type <i>%1</i>", type.label() )  );
    dlg.m_rangeWidget->hide();
    QIcon icon = type.icon();
    if ( icon.isNull() ) {
        icon = KIcon( "nepomuk" );
    }
    dlg.m_labelTitle->setPixmap( icon.pixmap( 32, 32 ) );

    if ( dlg.exec() ) {
        QString name = dlg.m_editClassLabel->text();
        QString comment = dlg.m_editClassComment->text();
        QString icon = dlg.m_buttonClassIcon->icon();

        Soprano::NRLModel nrlModel( Nepomuk::ResourceManager::instance()->mainModel() );
        nrlModel.setBaseNamespace( QUrl( "nepomuk:/instances" ) ); // FIXME: Use the PIMO NS
        return nrlModel.createResource( type.uri(),
                                        name,
                                        comment,
                                        icon != "unknown" ? icon : QString() );
    }
    else {
        return Nepomuk::Resource();
    }
}

#include "newclassdialog.moc"
