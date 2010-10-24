/***************************************************************************
                                invitation.h
                             -------------------
    begin                : Sat Mar 30 2002
    copyright            : (C) 2002 by Tim Jansen
    email                : tim@tjansen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INVITATION_H
#define INVITATION_H

#include <KApplication>
#include <KConfig>

#include <QtCore/QDateTime>
#include <QtCore/QObject>

const int INVITATION_DURATION = 60 * 60;

class Invitation
{
public:
    Invitation();
    ~Invitation();
    Invitation(const KConfigGroup &config);
    Invitation(const Invitation &x);
    Invitation &operator= (const Invitation &x);

    bool operator == (const Invitation &ot);

    QString password() const;
    QDateTime expirationTime() const;
    QDateTime creationTime() const;
    bool isValid() const;

    void save(KConfigGroup &config) const;
private:
    QString m_password;
    QDateTime m_creationTime;
    QDateTime m_expirationTime;

};

#endif
