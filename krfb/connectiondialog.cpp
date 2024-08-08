/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2010 Collabora Ltd <info@collabora.co.uk>
   SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
   SPDX-FileCopyrightText: 2004 Nadeem Hasan <nhasan@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "connectiondialog.h"

#include <QCheckBox>
#include <QIcon>

#include <KConfigGroup>
#include <KGuiItem>
#include <KLocalizedString>
#include <KStandardGuiItem>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

template<typename UI>
ConnectionDialog<UI>::ConnectionDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("New Connection"));
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout;
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
