/****************************************************************************
** Form interface generated from reading ui file './personalinvitation.ui'
**
** Created: Sat Mar 30 16:29:05 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef PERSONALINVITATIONDIALOG_H
#define PERSONALINVITATIONDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QFrame;
class QLabel;
class QPushButton;

class PersonalInvitationDialog : public QDialog
{ 
    Q_OBJECT

public:
    PersonalInvitationDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PersonalInvitationDialog();

    QLabel* PixmapLabel1;
    QPushButton* closeButton;
    QFrame* Frame22;
    QLabel* TextLabel11;
    QLabel* TextLabel3;
    QFrame* Frame21;
    QLabel* hostLabel;
    QLabel* TextLabel4;
    QLabel* TextLabel5;
    QLabel* passwordLabel;
    QLabel* TextLabel6;
    QLabel* expirationLabel;
    QFrame* Frame23;


protected:
    QGridLayout* PersonalInvitationDialogLayout;
    QVBoxLayout* Frame22Layout;
    QGridLayout* Frame21Layout;
};

#endif // PERSONALINVITATIONDIALOG_H
