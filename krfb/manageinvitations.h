/****************************************************************************
** Form interface generated from reading ui file './manageinvitations.ui'
**
** Created: Fri Mar 29 18:27:37 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef MANAGEINVITATIONSDIALOG_H
#define MANAGEINVITATIONSDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KListView;
class QFrame;
class QListViewItem;
class QPushButton;

class ManageInvitationsDialog : public QDialog
{ 
    Q_OBJECT

public:
    ManageInvitationsDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ManageInvitationsDialog();

    KListView* listView;
    QPushButton* closeButton;
    QFrame* Line1;
    QPushButton* deleteOneButton;
    QPushButton* newButton;
    QPushButton* deleteAllButton;


protected:
    QGridLayout* ManageInvitationsDialogLayout;
};

#endif // MANAGEINVITATIONSDIALOG_H
