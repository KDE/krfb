/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
   Copyright (C) 2013 Amandeep Singh <aman.dedman@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef MANAGEINVITATIONSDIALOG_H
#define MANAGEINVITATIONSDIALOG_H

#include "ui_mainwidget.h"

#include <KXmlGuiWindow>

class KLineEdit;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow() override;

    public Q_SLOTS:
        void showConfiguration();

    protected:
        void readProperties(const KConfigGroup & group) override;
        void saveProperties(KConfigGroup & group) override;

    private Q_SLOTS:
        void editPassword();
        void editUnattendedPassword();
        void toggleDesktopSharing(bool enable);
        void passwordChanged(const QString&);
        void aboutConnectionAddress();
        void aboutUnattendedMode();

    private:
        Ui::MainWidget m_ui;
        bool m_passwordEditable;
        KLineEdit *m_passwordLineEdit;
};

#endif
