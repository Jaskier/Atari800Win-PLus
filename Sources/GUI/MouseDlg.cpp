/****************************************************************************
File    : MouseDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CMouseDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "MouseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_MOUSE_FIRST		IDC_MOUSE_EMULATEDDEVICE
#define IDC_MOUSE_LAST		IDC_MOUSE_CANCEL

#define MAX_MOUSE_SPEED		 99
#define MAX_MIN_POT			228
#define MAX_MAX_POT			228
#define MAX_JOY_INERTIA		 99
#define MAX_PEN_H_OFFSET	 44
#define MAX_PEN_V_OFFSET	 32


/////////////////////////////////////////////////////////////////////////////
// CMouseDlg dialog

BEGIN_MESSAGE_MAP(CMouseDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CMouseDlg)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MOUSE_PENHORZSPIN, OnDeltaposPenHorzSpin)
	ON_EN_KILLFOCUS(IDC_MOUSE_PENHORZOFFSET, OnKillfocusPenHorzOffset)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MOUSE_PENVERTSPIN, OnDeltaposPenVertSpin)
	ON_EN_KILLFOCUS(IDC_MOUSE_PENVERTOFFSET, OnKillfocusPenVertOffset)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MOUSE_MOUSESPIN, OnDeltaposMouseSpin)
	ON_EN_KILLFOCUS(IDC_MOUSE_MOUSESPEED, OnKillfocusMouseSpeed)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MOUSE_MINSPIN, OnDeltaposMinSpin)
	ON_EN_KILLFOCUS(IDC_MOUSE_MINPOT, OnKillfocusMinPot)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MOUSE_MAXSPIN, OnDeltaposMaxSpin)
	ON_EN_KILLFOCUS(IDC_MOUSE_MAXPOT, OnKillfocusMaxPot)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MOUSE_JOYSPIN, OnDeltaposJoySpin)
	ON_EN_KILLFOCUS(IDC_MOUSE_JOYINERTIA, OnKillfocusJoyInertia)
	ON_CBN_SELCHANGE(IDC_MOUSE_EMULATEDDEVICE, OnSelchangeEmulatedDevice)
	ON_CBN_SELCHANGE(IDC_MOUSE_PORT, OnSelchangePort)
	ON_BN_CLICKED(IDC_MOUSE_MOUSECAPTURE, OnMouseCapture)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_MOUSE_OK, OnOK)
	ON_BN_CLICKED(IDC_MOUSE_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CMouseDlg::CMouseDlg
=========================================================*/
/* #FN#
   Standard constructor */
CMouseDlg::
CMouseDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CMouseDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CMouseDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_MOUSE_FIRST;
	m_nLastCtrl  = IDC_MOUSE_LAST;

} /* #OF# CMouseDlg::CMouseDlg */

/*========================================================
Method   : CMouseDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CMouseDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CMouseDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CMouseDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CMouseDlg implementation

/*========================================================
Method   : CMouseDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CMouseDlg::
SetDlgState()
{
	_SetSelCbox( IDC_MOUSE_EMULATEDDEVICE, m_nMouseMode );
	_SetSelCbox( IDC_MOUSE_PORT,           m_nMousePort );

	/* Enable/Disable appriopriate controls */
	_EnableCtrl( IDC_MOUSE_MOUSESPEED,          MOUSE_OFF != m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_MOUSESPEED_LABEL,    MOUSE_OFF != m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_MOUSESPIN,           MOUSE_OFF != m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_MINPOT,              MOUSE_PAD == m_nMouseMode || MOUSE_TOUCH == m_nMouseMode || MOUSE_KOALA == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_MINPOT_LABEL,        MOUSE_PAD == m_nMouseMode || MOUSE_TOUCH == m_nMouseMode || MOUSE_KOALA == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_MINSPIN,             MOUSE_PAD == m_nMouseMode || MOUSE_TOUCH == m_nMouseMode || MOUSE_KOALA == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_MAXPOT,              MOUSE_PAD == m_nMouseMode || MOUSE_TOUCH == m_nMouseMode || MOUSE_KOALA == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_MAXPOT_LABEL,        MOUSE_PAD == m_nMouseMode || MOUSE_TOUCH == m_nMouseMode || MOUSE_KOALA == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_MAXSPIN,             MOUSE_PAD == m_nMouseMode || MOUSE_TOUCH == m_nMouseMode || MOUSE_KOALA == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_PENHORZOFFSET,       MOUSE_PEN == m_nMouseMode || MOUSE_GUN == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_PENHORZOFFSET_LABEL, MOUSE_PEN == m_nMouseMode || MOUSE_GUN == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_PENHORZSPIN,         MOUSE_PEN == m_nMouseMode || MOUSE_GUN == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_PENVERTOFFSET,       MOUSE_PEN == m_nMouseMode || MOUSE_GUN == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_PENVERTOFFSET_LABEL, MOUSE_PEN == m_nMouseMode || MOUSE_GUN == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_PENVERTSPIN,         MOUSE_PEN == m_nMouseMode || MOUSE_GUN == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_JOYINERTIA,          MOUSE_JOY == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_JOYINERTIA_LABEL,    MOUSE_JOY == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_JOYSPIN,             MOUSE_JOY == m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_PORT,                MOUSE_OFF != m_nMouseMode );
	_EnableCtrl( IDC_MOUSE_PORT_LABEL,          MOUSE_OFF != m_nMouseMode );

	/* Set up check button state */
	_SetChkBttn( IDC_MOUSE_MOUSECAPTURE, _IsFlagSet( m_ulInputState, IS_CAPTURE_MOUSE ) );

	/* Set some parameter values */
	SetDlgItemInt( IDC_MOUSE_MOUSESPEED,    m_nMouseSpeed, FALSE );
	SetDlgItemInt( IDC_MOUSE_MINPOT,        m_nPotMin,     FALSE );
	SetDlgItemInt( IDC_MOUSE_MAXPOT,        m_nPotMax,     FALSE );
	SetDlgItemInt( IDC_MOUSE_JOYINERTIA,    m_nJoyInertia, FALSE );
	SetDlgItemInt( IDC_MOUSE_PENHORZOFFSET, m_nPenOffsetX, FALSE );
	SetDlgItemInt( IDC_MOUSE_PENVERTOFFSET, m_nPenOffsetY, FALSE );

#ifdef WIN_NETWORK_GAMES
	_EnableCtrl( IDC_MOUSE_MOUSECAPTURE, !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMouseDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CMouseDlg message handlers

/*========================================================
Method   : CMouseDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CMouseDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	m_ulInputState = g_Input.ulState;
	m_nMouseMode   = mouse_mode;
	m_nMousePort   = mouse_port;
	m_nMouseSpeed  = mouse_speed;
	m_nPotMin      = mouse_pot_min;
	m_nPotMax      = mouse_pot_max;
	m_nJoyInertia  = mouse_joy_inertia;
	m_nPenOffsetX  = mouse_pen_ofs_h;
	m_nPenOffsetY  = mouse_pen_ofs_v;

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CMouseDlg::OnInitDialog */

/*========================================================
Method   : CMouseDlg::OnMouseCapture
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CMouseDlg::
OnMouseCapture()
{
	_ClickButton( IDC_MOUSE_MOUSECAPTURE, m_ulInputState, IS_CAPTURE_MOUSE );

} /* #OF# CMouseDlg::OnMouseCapture */

/*========================================================
Method   : CMouseDlg::OnDeltaposMouseSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnDeltaposMouseSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_MOUSE_MOUSESPEED, m_nMouseSpeed, 1, MAX_MOUSE_SPEED );
	*pResult = 0;

} /* #OF# CMouseDlg::OnDeltaposMouseSpin */

/*========================================================
Method   : CMouseDlg::OnKillfocusMouseSpeed
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnKillfocusMouseSpeed()
{
	_KillfocusSpin( IDC_MOUSE_MOUSESPEED, m_nMouseSpeed, 1, MAX_MOUSE_SPEED );

} /* #OF# CMouseDlg::OnKillfocusMouseSpeed */

/*========================================================
Method   : CMouseDlg::OnDeltaposJoySpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnDeltaposJoySpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_MOUSE_JOYINERTIA, m_nJoyInertia, 1, MAX_JOY_INERTIA );
	*pResult = 0;

} /* #OF# CMouseDlg::OnDeltaposJoySpin */

/*========================================================
Method   : CMouseDlg::OnKillfocusJoyInertia
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnKillfocusJoyInertia()
{
	_KillfocusSpin( IDC_MOUSE_JOYINERTIA, m_nJoyInertia, 1, MAX_JOY_INERTIA );

} /* #OF# CMouseDlg::OnKillfocusJoyInertia */

/*========================================================
Method   : CMouseDlg::OnDeltaposMinSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnDeltaposMinSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_MOUSE_MINPOT, m_nPotMin, 0, MAX_MIN_POT );
	*pResult = 0;

} /* #OF# CMouseDlg::OnDeltaposMinSpin */

/*========================================================
Method   : CMouseDlg::OnKillfocusMinPot
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnKillfocusMinPot()
{
	_KillfocusSpin( IDC_MOUSE_MINPOT, m_nPotMin, 0, MAX_MIN_POT );

} /* #OF# CMouseDlg::OnKillfocusMinPot */

/*========================================================
Method   : CMouseDlg::OnDeltaposMaxSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnDeltaposMaxSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_MOUSE_MAXPOT, m_nPotMax, 0, MAX_MAX_POT );
	*pResult = 0;

} /* #OF# CMouseDlg::OnDeltaposMaxSpin */

/*========================================================
Method   : CMouseDlg::OnKillfocusMaxPot
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnKillfocusMaxPot()
{
	_KillfocusSpin( IDC_MOUSE_MAXPOT, m_nPotMax, 0, MAX_MAX_POT );

} /* #OF# CMouseDlg::OnKillfocusMaxPot */

/*========================================================
Method   : CMouseDlg::OnDeltaposPenHorzSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnDeltaposPenHorzSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_MOUSE_PENHORZOFFSET, m_nPenOffsetX, 0, MAX_PEN_H_OFFSET );
	*pResult = 0;

} /* #OF# CMouseDlg::OnDeltaposPenHorzSpin */

/*========================================================
Method   : CMouseDlg::OnKillfocusPenHorzOffset
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnKillfocusPenHorzOffset()
{
	_KillfocusSpin( IDC_MOUSE_PENHORZOFFSET, m_nPenOffsetX, 0, MAX_PEN_H_OFFSET );

} /* #OF# CMouseDlg::OnKillfocusPenHorzOffset */

/*========================================================
Method   : CMouseDlg::OnDeltaposPenVertSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnDeltaposPenVertSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_MOUSE_PENVERTOFFSET, m_nPenOffsetY, 0, MAX_PEN_V_OFFSET );
	*pResult = 0;

} /* #OF# CMouseDlg::OnDeltaposPenVertSpin */

/*========================================================
Method   : CMouseDlg::OnKillfocusPenVertOffset
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CMouseDlg::
OnKillfocusPenVertOffset()
{
	_KillfocusSpin( IDC_MOUSE_PENVERTOFFSET, m_nPenOffsetY, 0, MAX_PEN_V_OFFSET );

} /* #OF# CMouseDlg::OnKillfocusPenVertOffset */

/*========================================================
Method   : CMouseDlg::OnSelchangeEmulatedDevice
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CMouseDlg::
OnSelchangeEmulatedDevice()
{
	if( CB_ERR == (m_nMouseMode = _GetSelCbox( IDC_MOUSE_EMULATEDDEVICE )) )
	{
		_SetSelCbox( IDC_MOUSE_EMULATEDDEVICE, 0 ),
		m_nMouseMode = 0;
	}
	SetDlgState();

} /* #OF# CMouseDlg::OnSelchangeEmulatedDevice */

/*========================================================
Method   : CMouseDlg::OnSelchangePort
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CMouseDlg::
OnSelchangePort()
{
	if( CB_ERR == (m_nMousePort = _GetSelCbox( IDC_MOUSE_PORT )) )
	{
		_SetSelCbox( IDC_MOUSE_PORT, 0 ),
		m_nMousePort = 0;
	}
} /* #OF# CMouseDlg::OnSelchangePort */

/*========================================================
Method   : CMouseDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CMouseDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	switch( nCtrlID )
	{
		case IDC_MOUSE_MINPOT:
			OnKillfocusMinPot();
			break;
		case IDC_MOUSE_MAXPOT:
			OnKillfocusMaxPot();
			break;
		case IDC_MOUSE_MOUSESPEED:
			OnKillfocusMouseSpeed();
			break;
		case IDC_MOUSE_JOYINERTIA:
			OnKillfocusJoyInertia();
			break;
		case IDC_MOUSE_PENHORZOFFSET:
			OnKillfocusPenHorzOffset();
			break;
		case IDC_MOUSE_PENVERTOFFSET:
			OnKillfocusPenVertOffset();
			break;
	}
} /* #OF# CMouseDlg::ReceiveFocused */

/*========================================================
Method   : CMouseDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CMouseDlg::
OnOK() 
{
	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	if( m_nMouseMode != mouse_mode )
	{
		mouse_mode = m_nMouseMode;
		WriteRegDWORD( NULL, REG_MOUSE_MODE, mouse_mode );

		/* Update main window's indicator */
		UpdateIndicator( ID_INDICATOR_MSE );
	}
	if( m_nMousePort != mouse_port )
	{
		mouse_port = m_nMousePort;
		WriteRegDWORD( NULL, REG_MOUSE_PORT, mouse_port );
	}
	if( m_nMouseSpeed != mouse_speed )
	{
		mouse_speed = m_nMouseSpeed;
		WriteRegDWORD( NULL, REG_MOUSE_SPEED, mouse_speed );
	}
	if( m_nPotMin != mouse_pot_min )
	{
		mouse_pot_min = m_nPotMin;
		WriteRegDWORD( NULL, REG_POT_MIN, mouse_pot_min );
	}
	if( m_nPotMax != mouse_pot_max )
	{
		mouse_pot_max = m_nPotMax;
		WriteRegDWORD( NULL, REG_POT_MAX, mouse_pot_max );
	}
	if( m_nJoyInertia != mouse_joy_inertia )
	{
		mouse_joy_inertia = m_nJoyInertia;
		WriteRegDWORD( NULL, REG_JOY_INERTIA, mouse_joy_inertia );
	}
	if( m_nPenOffsetX != mouse_pen_ofs_h )
	{
		mouse_pen_ofs_h = m_nPenOffsetX;
		WriteRegDWORD( NULL, REG_PEN_XOFFSET, mouse_pen_ofs_h );
	}
	if( m_nPenOffsetY != mouse_pen_ofs_v )
	{
		mouse_pen_ofs_v = m_nPenOffsetY;
		WriteRegDWORD( NULL, REG_PEN_YOFFSET, mouse_pen_ofs_v );
	}

	if( m_ulInputState != g_Input.ulState )
	{
		if( g_Input.ulState & IS_CAPTURE_MOUSE ^
			m_ulInputState & IS_CAPTURE_MOUSE )
		{
			Input_ToggleMouseCapture();

			/* Update main window's indicator */
			UpdateIndicator( ID_INDICATOR_MSE );
		}
		g_Input.ulState = m_ulInputState;
		WriteRegDWORD( NULL, REG_INPUT_STATE, g_Input.ulState );
	}
	CCommonDlg::OnOK();

} /* #OF# CMouseDlg::OnOK */
