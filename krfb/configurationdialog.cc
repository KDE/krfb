#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './configurationdialog.ui'
**
** Created: Sun Mar 24 02:03:14 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "configurationdialog.h"

#include <qvariant.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

static const char* const image0_data[] = { 
"48 48 202 2",
"Qt c None",
"#9 c #000085",
"#W c #00008b",
"#V c #000094",
"#8 c #00009b",
"af c #0000a0",
"#7 c #0000b0",
"#K c #0000b5",
"#J c #0000b7",
"#I c #0000be",
"#D c #0000c0",
"#X c #0000c2",
"#6 c #0000c5",
"#L c #0000c9",
"#H c #0000ca",
"#C c #0000d0",
"#5 c #0000da",
"aa c #0000e0",
"#M c #0000ec",
"#G c #0000f4",
"aw c #0004ca",
"aD c #0012ca",
"ax c #0012f4",
"ao c #0020ef",
"aE c #0059f4",
"ag c #01017e",
"#B c #0404d4",
"ah c #070775",
"al c #080873",
"#x c #0808a8",
"#w c #0808c2",
"a. c #0c0c6c",
"aL c #0c0c9c",
"#v c #1010c1",
"#A c #1414a3",
"#Y c #1515e5",
"ay c #1555ff",
"a# c #181858",
"aK c #18187d",
"aJ c #181892",
"aQ c #1a1a71",
"ai c #1c1c51",
"aj c #212148",
"am c #28283d",
"#r c #282848",
"#q c #28284d",
"aU c #292945",
"ab c #2a2aff",
"aM c #2a37d0",
"ap c #2a60ff",
"ak c #2c2c37",
"#p c #2f2f52",
".# c #303030",
".a c #333333",
"aV c #363a4d",
"#u c #373737",
"bh c #393939",
"#S c #3a3a3a",
"#t c #3c3c3c",
"#y c #3d3d3d",
"aZ c #3e3e3e",
".b c #404040",
"#4 c #4040ef",
"an c #414141",
"aY c #424242",
".c c #434343",
"aX c #434445",
"#i c #444444",
"aW c #444b50",
"#h c #474747",
"av c #4848f9",
"#o c #4a4a52",
"#j c #4b4b4b",
"aH c #4e4e4e",
"#Z c #4e4eff",
"aR c #4e64c2",
"az c #4ea7ff",
"#n c #515151",
"aO c #518ad2",
".i c #525252",
"aB c #535353",
"aN c #5374dd",
"#g c #555555",
"#m c #575757",
"#l c #585858",
"#z c #585897",
"aI c #585898",
"#a c #595959",
".G c #5a5a5a",
"## c #5b5b5b",
".6 c #5e5e5e",
"#R c #5f5f5f",
"#c c #606060",
".R c #616161",
".F c #626262",
".7 c #636363",
"#0 c #646464",
".P c #656565",
".B c #686868",
"aC c #68686f",
".z c #696969",
"#N c #6a6afa",
"aF c #6aaaff",
".1 c #6c6c6c",
".V c #6e6e6e",
".0 c #6f6f6f",
"#s c #737373",
".Z c #747474",
"aP c #7494b4",
".T c #757575",
"#b c #767676",
"#. c #777777",
"ad c #787878",
".H c #797979",
".E c #7b7b7b",
".y c #7c7c7c",
".d c #7e7e7e",
".Q c #7f7f7f",
".t c #808080",
"#F c #8080ff",
"#d c #828282",
".S c #838383",
".l c #858585",
".O c #868686",
".2 c #878787",
".I c #888888",
"#k c #898989",
".4 c #8a8a8a",
".u c #8b8b8b",
"a4 c #8d7878",
".e c #8d8d8d",
".s c #8e8e8e",
"aT c #8e969b",
".U c #8f8f8f",
".h c #909090",
".o c #929292",
".N c #939393",
"aS c #93b7cf",
".n c #949494",
"#3 c #9595ff",
".W c #969696",
"b# c #977474",
"#f c #979797",
"ae c #989898",
".D c #999999",
".m c #9b9b9b",
".x c #9c9c9c",
".g c #9d9d9d",
"bf c #9e9797",
".q c #9f9f9f",
".f c #a0a0a0",
"bc c #a19c9c",
"#e c #a1a1a1",
".k c #a2a2a2",
"a0 c #a39393",
"bg c #a39c9c",
".5 c #a3a3a3",
"a7 c #a48a8a",
".M c #a5a5a5",
".9 c #a6a6a6",
"a5 c #a89393",
".C c #a8a8a8",
".J c #a9a9a9",
"bd c #ab8585",
"be c #ac8989",
".A c #acacac",
"a1 c #ad5d5d",
"ba c #ad8a8a",
".r c #adadad",
".Y c #b2b2b2",
".3 c #b3b3b3",
".p c #b4b4b4",
"a8 c #b63131",
".v c #b8b8b8",
".X c #b9b9b9",
".K c #bababa",
".8 c #bcbcbc",
".w c #bdbdbd",
"au c #bfbfe2",
"a2 c #c05050",
".L c #c1c1c1",
"a3 c #c25858",
".j c #c3c3c3",
"as c #c4c4c4",
"a6 c #c85d5d",
"ar c #cbcbcb",
"ac c #d4d4ff",
"aq c #d4dfff",
"a9 c #d51b1b",
"#T c #dcdcdc",
"#O c #dcdcff",
"aA c #dcedff",
"at c #dddddd",
"b. c #de2c2c",
"bb c #e23131",
"#E c #e2e2e2",
"#2 c #eaeaff",
"#1 c #eeeeee",
"#P c #f8f8ff",
"aG c #f8fbff",
"#U c #f9f9f9",
"#Q c #ffffff",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.a.b.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.b.a",
".#.b.d.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.d.b",
".#.c.e.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.f.e.c",
".#.c.e.f.g.h.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.h.g.f.e.c",
".#.c.e.f.h.i.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.i.h.f.e.c",
".#.c.e.f.e.c.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQtQtQtQtQtQtQtQtQtQt.j.j.j.j.j.j.j.j.j.j.j.jQtQtQtQtQtQtQtQtQtQtQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQtQtQtQtQtQtQtQtQtQt.j.j.j.j.j.j.j.j.j.j.j.jQtQtQtQtQtQtQtQtQtQtQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQtQtQtQtQt.j.j.k.l.m.n.o.p.q.o.p.r.k.k.s.s.s.t.u.v.j.j.jQtQtQtQtQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQtQt.j.j.j.w.v.x.l.m.y.z.A.u.B.C.D.E.u.F.G.H.l.I.J.K.L.M.tQtQtQtQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQtQt.j.j.j.v.N.O.l.m.y.P.D.Q.R.S.T.B.N.H.F.o.U.y.V.W.X.e.tQtQtQtQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQt.j.X.M.Y.p.E.d.e.Q.Z.0.H.Z.1.1.1.0.H.Z.0.H.O.2.R.l.J.I.W.j.j.jQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQt.w.3.4.D.5.1.H.4.F.z.Z.6.z.Z.6.z.Z.6.z.T.7.H.u.B.H.h.d.m.8.X.vQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQt.9.f.O.T.1#..0.F.G##.6#a##.6#a##.6#a##.7.T.0.B#b.0.7#c#d#e.h.uQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#.j.j.A#f.T.F##.z.F#g#h#i#i#i#i#i#i#i#i#i#i#j.P.F##.z.F##.z.Z.H.0#k.j.j.#.c.e.f.e.c",
".#.c.e.f.e.c.#.w.w.J.4.F#l#m.i#n#o#p#q#r#r#r#r#r#r#r#r#q#p#o#n#n#n#n#l#s.z#a.6.s.w.w.#.c.e.f.e.c",
".#.c.e.f.e.c.#.9.9.l.F.G#l.i#t#u#p#v#w#x#x#x#x#x#x#x#x#w#v#p#u#u#u#u#y#l##.6.Z.h.9.9.#.c.e.f.e.c",
".#.c.e.f.e.c.#.h.h.Z#g#h#i.i.4#z#A#B#C#D#D#D#D#D#D#D#D#C#B#A#z#f#f#f.4.i#i#j.P.Z.y.y.#.c.e.f.e.c",
".#.c.e.f.e.c.##s#s.F.i.i.i.0#E#F#G#H#D#I#J#K#K#K#K#J#I#D#L#M#N#O#P#Q#E.0.i.i.i#l#R#R.#.c.e.f.e.c",
".#.c.e.f.e.c.##y#y#S.i.L#T#E#U#F#G#H#D#J#V#W#W#W#W#V#J#D#X#L#Y#Z#O#Q#U#E#T.L.i#l.H.H.#.c.e.f.e.c",
".#.c.e.f.e.c.#.#.##0.J#1#Q#2#3#4#5#6#7#8#9a.a#a#a.#9#8#7#D#Daaabac#Q#Q#Q#Q#1.Jad#l#l.#.c.e.f.e.c",
".#.c.e.f.e.c.#.#.#ae#Q#Q#Qacabaa#D#Dafagahai.#.#aiahagaf#D#Daaabac#Q#Q#Q#Q#Q#Qae.#.#.#.c.e.f.e.c",
".#.c.e.f.e.c.#.#.#ae#Q#Q#Qacabaa#D#Dafahajak.#.#akajahaf#D#Daaabac#Q#Q#Q#Q#Q#Qae.#.#.#.c.e.f.e.c",
".#.c.e.f.e.c.#.#.#ae#Q#Q#Qacabaa#D#Dafalam.#.#.#.#amalaf#D#Daaabac#Q#Q#Q#Q#Q#Qae.#.#.#.c.e.f.e.c",
".#.c.e.f.e.c.#.#.#ae#Q#Q#Qacabaa#D#Dafahajak.#.#akajahaf#D#Daaabac#Q#Q#Q#Q#U#E.O.#.#.#.c.e.f.e.c",
".#.c.e.f.e.c.#.#.#ae#Q#Q#Qacabaa#D#Dafagahai.#.#aiahagaf#D#Daaabac#Q#Q#Q#Q#E.0an.#.#.#.c.e.f.e.c",
".#.c.e.f.e.c.#.#.#.T.J#1#Qacabaa#D#D#7#8#9a.a#a#a.#9#8#7#6#5aoapaq#Q#Q#Qar.Oan.#.#.#.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQt.##nasatauav#Y#L#X#D#J#V#W#W#W#W#V#J#DawaxayazaA#Q#U#E.s.#.#.#QtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQt.#.c#naBaCau#N#M#L#D#I#J#K#K#K#K#J#I#DaDaEaFaAaG#Q#E.0aH.#.#.#QtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQtQt.#.#.#aH.saIaJaKaL#D#D#D#D#D#D#D#DaMaNaOaPaeaeae.saH.#QtQtQtQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQtQtQtQtQt.#.#.#.#.#aQ#x#x#x#x#x#x#x#xaRaSaT.7.#.#.#.#.#QtQtQtQtQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQtQtQtQtQt.#.#.#.#.#aU#r#r#r#r#r#r#r#raVaWaX.b.#.#.#.#.#QtQtQtQtQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQtQtQtQtQtQtQtQtQtQt.#.#.#.#.#.#.#.#.#.#.#.#.#QtQtQtQtQtQtQtQtQtQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#QtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQtQt.#.c.e.f.e.c",
".#.c.e.f.e.c.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.c.e.f.e.c",
".#.c.e.f.h.i.caYaZ.b.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.i.h.f.e.c",
".#.c.e.f.g.h.e#k#..t.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.e.h.g.f.e.c",
".#.c.e.fa0a1a2a3a4.ha5a6a2a1a0.f.f.fae.h.h.h.N.g.f.g.N.h.N.g.f.f.fae.h.h.h.h.h.h.N.g.f.f.f.f.e.c",
".#.c.e.fa7a8a9b.b#.Nbabba9a8a7.f.f.f.N.l.l.l.4.x.f.x.4.l.4.x.f.f.f.N.l.l.l.l.l.l.4.x.f.f.f.f.e.c",
".#.c.e.fbca7bdbebf.gbgbabda7bc.f.f.f.g.m.m.m.x.q.f.q.x.m.x.q.f.f.f.g.m.m.m.m.m.m.x.q.f.f.f.f.e.c",
".#bh#R.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B.B#Rbh",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#"};


/* 
 *  Constructs a ConfigurationDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ConfigurationDialog::ConfigurationDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    QPixmap image0( ( const char** ) image0_data );
    if ( !name )
	setName( "ConfigurationDialog" );
    resize( 335, 255 ); 
    setCaption( tr2i18n( "Configuration - Desktop Sharing" ) );
    setIcon( image0 );
    ConfigurationDialogLayout = new QVBoxLayout( this, 11, 6, "ConfigurationDialogLayout"); 

    Frame7 = new QFrame( this, "Frame7" );
    Frame7->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, Frame7->sizePolicy().hasHeightForWidth() ) );
    Frame7->setFrameShape( QFrame::NoFrame );
    Frame7->setFrameShadow( QFrame::Plain );
    Frame7Layout = new QVBoxLayout( Frame7, 11, 6, "Frame7Layout"); 

    allowUninvitedCB = new QCheckBox( Frame7, "allowUninvitedCB" );
    allowUninvitedCB->setText( tr2i18n( "Allow &uninvited connections" ) );
    allowUninvitedCB->setChecked( TRUE );
    QToolTip::add( allowUninvitedCB, QString::null );
    QWhatsThis::add( allowUninvitedCB, tr2i18n( "Allow connections that do not follow a previous invitation. Be careful to protect your computer when you allow this." ) );
    Frame7Layout->addWidget( allowUninvitedCB );

    askOnConnectCB = new QCheckBox( Frame7, "askOnConnectCB" );
    askOnConnectCB->setText( tr2i18n( "Confirm connection &before accepting" ) );
    QWhatsThis::add( askOnConnectCB, tr2i18n( "If this option is enabled you will be asked whenever a remote user tries to connect to your display. This allows you to deny the request or to remove things that the remote user should not see before he connects. It is highly recommended to turn this on while you are working on this computer. If you do not enable this option your only hint that somebody is connected is the small icon in your panel showing an open eye." ) );
    Frame7Layout->addWidget( askOnConnectCB );

    allowDesktopControlCB = new QCheckBox( Frame7, "allowDesktopControlCB" );
    allowDesktopControlCB->setText( tr2i18n( "Allow &remote client to control the desktop" ) );
    QWhatsThis::add( allowDesktopControlCB, tr2i18n( "If you turn this option on the remote user can enter keystrokes and use your mouse pointer. This gives him full control over your computer, so be careful.  When the option is disabled the remote user can only watch your screen." ) );
    Frame7Layout->addWidget( allowDesktopControlCB );
    ConfigurationDialogLayout->addWidget( Frame7 );

    Frame4 = new QFrame( this, "Frame4" );
    Frame4->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, Frame4->sizePolicy().hasHeightForWidth() ) );
    Frame4->setFrameShape( QFrame::NoFrame );
    Frame4->setFrameShadow( QFrame::Plain );
    Frame4Layout = new QVBoxLayout( Frame4, 6, 6, "Frame4Layout"); 

    TextLabel1 = new QLabel( Frame4, "TextLabel1" );
    TextLabel1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, TextLabel1->sizePolicy().hasHeightForWidth() ) );
    TextLabel1->setText( tr2i18n( "Password:" ) );
    Frame4Layout->addWidget( TextLabel1 );

    passwordInput = new QLineEdit( Frame4, "passwordInput" );
    passwordInput->setMaxLength( 9 );
    passwordInput->setEchoMode( QLineEdit::Password );
    QWhatsThis::add( passwordInput, tr2i18n( "If you want to protect your system with a password enter one here. It is highly recommended that you either do this or enable \"ask before remote client connects\"." ) );
    Frame4Layout->addWidget( passwordInput );
    ConfigurationDialogLayout->addWidget( Frame4 );

    Frame6 = new QFrame( this, "Frame6" );
    Frame6->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, Frame6->sizePolicy().hasHeightForWidth() ) );
    Frame6->setFrameShape( QFrame::NoFrame );
    Frame6->setFrameShadow( QFrame::Plain );
    Frame6Layout = new QHBoxLayout( Frame6, 4, 6, "Frame6Layout"); 

    applyButton = new QPushButton( Frame6, "applyButton" );
    applyButton->setText( tr2i18n( "&Apply" ) );
    QWhatsThis::add( applyButton, tr2i18n( "Click this to apply the changes immediately." ) );
    Frame6Layout->addWidget( applyButton );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Frame6Layout->addItem( spacer );

    okButton = new QPushButton( Frame6, "okButton" );
    okButton->setText( tr2i18n( "&Ok" ) );
    okButton->setDefault( TRUE );
    QWhatsThis::add( okButton, tr2i18n( "Apply changes and close window." ) );
    Frame6Layout->addWidget( okButton );

    cancelButton = new QPushButton( Frame6, "cancelButton" );
    cancelButton->setText( tr2i18n( "&Cancel" ) );
    QWhatsThis::add( cancelButton, tr2i18n( "Discard all changes and close window." ) );
    Frame6Layout->addWidget( cancelButton );
    ConfigurationDialogLayout->addWidget( Frame6 );

    // signals and slots connections
    connect( allowUninvitedCB, SIGNAL( toggled(bool) ), askOnConnectCB, SLOT( setEnabled(bool) ) );
    connect( allowUninvitedCB, SIGNAL( toggled(bool) ), allowDesktopControlCB, SLOT( setEnabled(bool) ) );
    connect( allowUninvitedCB, SIGNAL( toggled(bool) ), Frame4, SLOT( setEnabled(bool) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
ConfigurationDialog::~ConfigurationDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "configurationdialog.moc"
