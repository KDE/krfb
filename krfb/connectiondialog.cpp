/* This file is part of the KDE project
   Copyright (C) 2010 Collabora Ltd <info@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
   Copyright (C) 2004 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "connectiondialog.h"

#include <QCheckBox>
#include <QIcon>

#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <KGuiItem>
#include <QVBoxLayout>

template <typename UI>
ConnectionDialog<UI>::ConnectionDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("New Connection"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ConnectionDialog<UI>::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConnectionDialog<UI>::reject);
    buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);
    setModal(true);

    setMinimumSize(500, 200);

    m_connectWidget = new QWidget(this);
    m_ui.setupUi(m_connectWidget);

    m_ui.pixmapLabel->setPixmap(QIcon::fromTheme(QStringLiteral("krfb")).pixmap(128));

    KGuiItem accept = KStandardGuiItem::ok();
    accept.setText(i18n("Accept Connection"));
    KGuiItem::assign(okButton, accept);

    KGuiItem refuse = KStandardGuiItem::cancel();
    refuse.setText(i18n("Refuse Connection"));
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), refuse);

    mainLayout->addWidget(m_connectWidget);
    mainLayout->addWidget(buttonBox);
}

//**********

InvitationsConnectionDialog::InvitationsConnectionDialog(QWidget *parent)
    : ConnectionDialog<Ui::ConnectionWidget>(parent)
{
}

void InvitationsConnectionDialog::setRemoteHost(const QString &host)
{
    m_ui.remoteHost->setText(host);
}
