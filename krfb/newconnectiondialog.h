/****************************************************************************
** Form interface generated from reading ui file './newconnectiondialog.ui'
**
** Created: Sun Mar 24 01:48:54 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KRFBCONNECTIONDIALOG_H
#define KRFBCONNECTIONDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QFrame;
class QLabel;
class QPushButton;

class KRFBConnectionDialog : public QDialog
{ 
    Q_OBJECT

public:
    KRFBConnectionDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~KRFBConnectionDialog();

    QFrame* Frame12;
    QPushButton* acceptConnectionButton;
    QPushButton* refuseConnectionButton;
    QFrame* someframe;
    QFrame* Frame11;
    QLabel* TextLabel5;
    QLabel* mainTextLabel;
    QFrame* Frame6;
    QLabel* TextLabel1;
    QLabel* ipLabel;
    QCheckBox* allowRemoteControlCB;
    QLabel* PixmapLabel1;


protected:
    QGridLayout* KRFBConnectionDialogLayout;
    QHBoxLayout* Frame12Layout;
    QVBoxLayout* someframeLayout;
    QVBoxLayout* Frame11Layout;
    QHBoxLayout* Frame6Layout;
};

#endif // KRFBCONNECTIONDIALOG_H
