void ManageInvitationsDialog::listSizeChanged(int i) {
	deleteAllButton->setEnabled(i);
}

void ManageInvitationsDialog::listSelectionChanged() {
	QListViewItem *i = listView->firstChild();
	while(i) {
		if (i->isSelected()) {
			deleteOneButton->setEnabled(true);
			return;
		}
		i = i->nextSibling();
	}
	deleteOneButton->setEnabled(false);
}
