/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>
   SPDX-FileCopyrightText: 2013 Amandeep Singh <aman.dedman@gmail.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRFB_MAINWINDOW_H
#define KRFB_MAINWINDOW_H

#include "ui_mainwidget.h"

#include <KXmlGuiWindow>

class QLineEdit;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public Q_SLOTS:
    void showConfiguration();

protected:
    void readProperties(const KConfigGroup &group) override;
    void saveProperties(KConfigGroup &group) override;

private Q_SLOTS:
    void editPassword();
    void editUnattendedPassword();
    void toggleDesktopSharing(bool enable);
    void passwordChanged(const QString &);
    void aboutConnectionAddress();
    void aboutUnattendedMode();

private:
    Ui::MainWidget m_ui;
    bool m_passwordEditable;
    QLineEdit *m_passwordLineEdit = nullptr;
};

#endif
