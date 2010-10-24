#! /bin/sh
$EXTRACTRC ui/*.ui *.kcfg >> rc.cpp
$XGETTEXT *.cpp -o $podir/krfb.pot
