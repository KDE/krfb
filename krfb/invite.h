/****************************************************************************
** Form interface generated from reading ui file './invite.ui'
**
** Created: Sat Mar 30 14:06:36 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef INVITATIONDIALOG_H
#define INVITATIONDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QFrame;
class QLabel;
class QPushButton;

class InvitationDialog : public QDialog
{ 
    Q_OBJECT

public:
    InvitationDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~InvitationDialog();

    QPushButton* closeButton;
    QLabel* PixmapLabel1;
    QFrame* Frame19;
    QLabel* TextLabel2;
    QPushButton* createInvitationButton;
    QPushButton* createInvitationEMailButton;
    QCheckBox* dontShowOnStartupButton;
    QLabel* TextLabel1;


protected:
    QGridLayout* InvitationDialogLayout;
    QGridLayout* Frame19Layout;
};

#endif // INVITATIONDIALOG_H
