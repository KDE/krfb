/****************************************************************************
** Form interface generated from reading ui file './configurationdialog.ui'
**
** Created: Sun Mar 24 01:48:53 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;

class ConfigurationDialog : public QDialog
{ 
    Q_OBJECT

public:
    ConfigurationDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ConfigurationDialog();

    QFrame* Frame7;
    QCheckBox* allowUninvitedCB;
    QCheckBox* askOnConnectCB;
    QCheckBox* allowDesktopControlCB;
    QFrame* Frame4;
    QLabel* TextLabel1;
    QLineEdit* passwordInput;
    QFrame* Frame6;
    QPushButton* applyButton;
    QPushButton* okButton;
    QPushButton* cancelButton;


protected:
    QVBoxLayout* ConfigurationDialogLayout;
    QVBoxLayout* Frame7Layout;
    QVBoxLayout* Frame4Layout;
    QHBoxLayout* Frame6Layout;
};

#endif // CONFIGURATIONDIALOG_H
