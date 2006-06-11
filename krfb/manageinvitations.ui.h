/*
     Copyright 2002 Tim Jansen <tim@tjansen.de>
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
void ManageInvitationsDialog::listSizeChanged(int i) {
	deleteAllButton->setEnabled(i);
}

void ManageInvitationsDialog::listSelectionChanged() {
	Q3ListViewItem *i = listView->firstChild();
	while(i) {
		if (i->isSelected()) {
			deleteOneButton->setEnabled(true);
			return;
		}
		i = i->nextSibling();
	}
	deleteOneButton->setEnabled(false);
}
