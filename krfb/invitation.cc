/***************************************************************************
                               invitation.cpp
                             -------------------
    begin                : Sat Mar 30 2002
    copyright            : (C) 2002 by Tim Jansen
                           (C) Stefan Taferner (password encryption)
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

#include "invitation.h"

/*
 * Function for (en/de)crypting strings for config file, taken from KMail
 * Author: Stefan Taferner <taferner@alpin.or.at>
 */
QString cryptStr(const QString &aStr) {
        QString result;
        for (unsigned int i = 0; i < aStr.length(); i++)
                result += (aStr[i].unicode() < 0x20) ? aStr[i] :
                                QChar(0x1001F - aStr[i].unicode());
        return result;
}

// a random string that doesn't contain i, I, o, O, 1, 0
// based on KApplication::randomString()
static QString readableRandomString(int length) {
   QString str;
   while (length)
   {
      int r = KApplication::random() % 62;
      r += 48;
      if (r > 57) 
	      r += 7;
      if (r > 90) 
	      r += 6;
      char c = char(r);
      if ((c == 'i') || 
	  (c == 'I') ||
	  (c == '1') ||
	  (c == 'o') ||
	  (c == 'O') ||
	  (c == '0'))
	      continue;
      str += c;
      length--;
   }
   return str;
}

Invitation::Invitation() :
	m_viewItem(0) {
	m_password = readableRandomString(4)+"-"+readableRandomString(3);
	m_creationTime = QDateTime::currentDateTime();
	m_expirationTime = QDateTime::currentDateTime().addSecs(INVITATION_DURATION);
}

Invitation::Invitation(const Invitation &x) :
	m_password(x.m_password),
	m_creationTime(x.m_creationTime),
	m_expirationTime(x.m_expirationTime),
	m_viewItem(0) {
}

Invitation::Invitation(KConfig* config, int num) {
	m_password = cryptStr(config->readEntry(QString("password%1").arg(num), ""));
	m_creationTime = config->readDateTimeEntry(QString("creation%1").arg(num));
	m_expirationTime = config->readDateTimeEntry(QString("expiration%1").arg(num));
	m_viewItem = 0;
}

Invitation::~Invitation() {
	if (m_viewItem)
		delete m_viewItem;
}

Invitation &Invitation::operator= (const Invitation&x) {
	m_password = x.m_password;
	m_creationTime = x.m_creationTime;
	m_expirationTime = x.m_expirationTime;
	if (m_viewItem)
		delete m_viewItem;
	m_viewItem = 0;
	return *this;
}

void Invitation::save(KConfig *config, int num) const {
	config->writeEntry(QString("password%1").arg(num), cryptStr(m_password));
	config->writeEntry(QString("creation%1").arg(num), m_creationTime);
	config->writeEntry(QString("expiration%1").arg(num), m_expirationTime);
}

QString Invitation::password() const {
	return m_password;
}

QDateTime Invitation::expirationTime() const {
	return m_expirationTime;
}

QDateTime Invitation::creationTime() const {
	return m_creationTime;
}

bool Invitation::isValid() const {
	return m_expirationTime > QDateTime::currentDateTime();
}

void Invitation::setViewItem(KListViewItem *i) {
	if (m_viewItem)
		delete m_viewItem;
	m_viewItem = i;
}

KListViewItem *Invitation::getViewItem() const{
	return m_viewItem;
}
