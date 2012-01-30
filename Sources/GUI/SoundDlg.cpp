/****************************************************************************
File    : SoundDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CSoundDlg implementation file
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 30.03.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "SoundDlg.h"
#include "SDL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_SOUND_FIRST		IDC_SOUND_VOLUMESLIDER
#define IDC_SOUND_LAST		IDC_SOUND_CANCEL

#define MIN_SOUND_LATENCY	10
#define MAX_SOUND_LATENCY	50
#define MIN_SOUND_QUALITY	0
#define MAX_SOUND_QUALITY	2

#define SR_8BIT				0
#define SR_16BIT			1

/////////////////////////////////////////////////////////////////////////////
// CSoundDlg dialog

BEGIN_MESSAGE_MAP(CSoundDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CSoundDlg)
	ON_WM_HSCROLL()
	ON_CBN_SELCHANGE(IDC_SOUND_RESOLUTION, OnSelchangeSoundResolution)
	ON_EN_KILLFOCUS(IDC_SOUND_LATENCY, OnKillfocusSoundLatency)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SOUND_LATENCYSPIN, OnDeltaposSoundLatencySpin)
	ON_EN_KILLFOCUS(IDC_SOUND_QUALITY, OnKillfocusSoundQuality)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SOUND_QUALITYSPIN, OnDeltaposSoundQualitySpin)
	ON_BN_CLICKED(IDC_SOUND_MUTE, OnSoundMute)
	ON_BN_CLICKED(IDC_SOUND_STEREO, OnSoundStereo)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SOUND_OK, OnOK)
	ON_BN_CLICKED(IDC_SOUND_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CSoundDlg::CSoundDlg
=========================================================*/
/* #FN#
   Standard constructor */
CSoundDlg::
CSoundDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CSoundDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CSoundDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bModeChanged = FALSE;
	m_nFirstCtrl   = IDC_SOUND_FIRST;
	m_nLastCtrl    = IDC_SOUND_LAST;

} /* #OF# CSoundDlg::CSoundDlg */

/*========================================================
Method   : CSoundDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CSoundDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CSoundDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CSoundDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CSoundDlg implementation

/*========================================================
Method   : CSoundDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CSoundDlg::
SetDlgState()
{
	CSliderCtrl *pSlider = (CSliderCtrl *)GetDlgItem( IDC_SOUND_VOLUMESLIDER );
	ASSERT(NULL != pSlider);

	/* Set up Volume slider */
	pSlider->SetRange( 0, SDL_MIX_MAXVOLUME, FALSE );
	pSlider->SetTicFreq( 16 );
	pSlider->SetPageSize( 16 );
	pSlider->SetPos( m_nSoundVol );

	_SetChkBttn( IDC_SOUND_MUTE,          _IsFlagSet( m_ulSoundState, SS_NO_SOUND ) );
	_SetChkBttn( IDC_SOUND_STEREO,        m_bEnableStereo );

	/* Set up Driver Type and Sound Resolution combos */
	_SetSelCbox( IDC_SOUND_RESOLUTION,    _IsFlagSet( m_ulSoundState, SS_16BIT_AUDIO ) ? SR_16BIT   : SR_8BIT    );

	/* Set up Playback Rate combo */
	switch( m_nSoundRate )
	{
		case 8000:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 0 );
			break;

		case 11025:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 1 );
			break;

		case 21280:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 2 );
			break;

		case 22050:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 3 );
			break;

		case 31920:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 4 );
			break;

		case 44100:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 5 );
			break;

		case 48000:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 6 );
			break;

		default:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 5 );
			m_nSoundRate = 44100;
	}

	SetDlgItemInt( IDC_SOUND_LATENCY, m_nSoundLatency, FALSE );
	SetDlgItemInt( IDC_SOUND_QUALITY, m_nSoundQuality, FALSE );

} /* #OF# CSoundDlg::SetDlgState */

/////////////////////////////////////////////////////////////////////////////
// CSoundDlg message handlers

/*========================================================
Method   : CSoundDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CSoundDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	m_ulSoundState   = g_Sound.ulState;
	m_nSoundRate     = g_Sound.nRate;
	m_nSoundVol      = g_Sound.nVolume;
	m_nSoundLatency  = g_Sound.nLatency;
	m_nSoundQuality  = g_Sound.nQuality;
	m_bEnableStereo  = POKEYSND_stereo_enabled;

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CSoundDlg::OnInitDialog */

/*========================================================
Method   : CSoundDlg::OnHScroll
=========================================================*/
/* #FN#
   The framework calls this member function when the user
   clicks a window’s horizontal scroll bar */
void
/* #AS#
   Nothing */
CSoundDlg::
OnHScroll(
	UINT        nSBCode,   /* #IN# A scroll-bar code that indicates the user’s scrolling request */
	UINT        nPos,      /* #IN# Specifies the scroll-box position */
	CScrollBar *pScrollBar /* #IN# A pointer to the scroll-bar control control */
)
{
	if( TB_THUMBTRACK == nSBCode || SB_THUMBPOSITION == nSBCode )
		m_nSoundVol = nPos;
	else
	{
//		CSliderCtrl *pSlider = (CSliderCtrl *)GetDlgItem( IDC_SOUND_VOLUMESLIDER );
//		ASSERT(NULL != pSlider);
		if( NULL != pScrollBar )
			m_nSoundVol = ((CSliderCtrl *)pScrollBar)->GetPos();
	}
	CCommonDlg::OnHScroll( nSBCode, nPos, pScrollBar );

} /* #OF# CSoundDlg::OnHScroll */

/*========================================================
Method   : CSoundDlg::OnSelchangeSoundResolution
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CSoundDlg::
OnSelchangeSoundResolution()
{
	int nSelection = _GetSelCbox( IDC_SOUND_RESOLUTION );

	if( CB_ERR == nSelection )
		return;

	_ModifyFlag( SR_16BIT == nSelection, m_ulSoundState, SS_16BIT_AUDIO );

} /* #OF# CSoundDlg::OnSelchangeSoundResolution */

/*========================================================
Method   : CSoundDlg::OnSoundMute
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CSoundDlg::
OnSoundMute()
{
	_ClickButton( IDC_SOUND_MUTE, m_ulSoundState, SS_NO_SOUND );

} /* #OF# CSoundDlg::OnSoundMute */

/*========================================================
Method   : CSoundDlg::OnKillfocusSoundLatency
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CSoundDlg::
OnKillfocusSoundLatency()
{
	_KillfocusSpin( IDC_SOUND_LATENCY, m_nSoundLatency, MIN_SOUND_LATENCY, MAX_SOUND_LATENCY );

} /* #OF# CSoundDlg::OnKillfocusSoundLatency */

/*========================================================
Method   : CSoundDlg::OnKillfocusSoundQuality
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CSoundDlg::
OnKillfocusSoundQuality()
{
	_KillfocusSpin( IDC_SOUND_QUALITY, m_nSoundQuality, MIN_SOUND_QUALITY, MAX_SOUND_QUALITY );

} /* #OF# CSoundDlg::OnKillfocusSoundQuality */

/*========================================================
Method   : CSoundDlg::OnDeltaposSoundLatencySpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CSoundDlg::
OnDeltaposSoundLatencySpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_SOUND_LATENCY, m_nSoundLatency, MIN_SOUND_LATENCY, MAX_SOUND_LATENCY );
	*pResult = 0;

} /* #OF# CSoundDlg::OnDeltaposSoundLatencySpin */

/*========================================================
Method   : CSoundDlg::OnDeltaposSoundQualitySpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CSoundDlg::
OnDeltaposSoundQualitySpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_SOUND_QUALITY, m_nSoundQuality, MIN_SOUND_QUALITY, MAX_SOUND_QUALITY );

	*pResult = 0;

} /* #OF# CSoundDlg::OnDeltaposSoundQualitySpin */

/*========================================================
Method   : CSoundDlg::OnSoundStereo
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CSoundDlg::
OnSoundStereo()
{
	m_bEnableStereo = _GetChkBttn( IDC_SOUND_STEREO );

} /* #OF# CSoundDlg::OnSoundStereo */

/*========================================================
Method   : CSoundDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CSoundDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	switch( nCtrlID )
	{
		case IDC_SOUND_LATENCY:
			OnKillfocusSoundLatency();
			break;
		case IDC_SOUND_QUALITY:
			OnKillfocusSoundQuality();
			break;
	}
} /* #OF# CSoundDlg::ReceiveFocused */

/*========================================================
Method   : CSoundDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CSoundDlg::
OnOK()
{
	CSliderCtrl *pSlider = (CSliderCtrl *)GetDlgItem( IDC_SOUND_VOLUMESLIDER );
	ASSERT(NULL != pSlider);

	BOOL bCommit = TRUE;

	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	/* Check if the video recording should be stopped */
	if(	m_nSoundRate != g_Sound.nRate || m_bEnableStereo != (BOOL)POKEYSND_stereo_enabled )
	{
		if( _IsFlagSet( g_Misc.ulState, MS_VIDEO_AND_SOUND ) ) /* When streaming video with sound */
			bCommit = StreamWarning( IDS_WARN_RECORD_SNDOUT, SRW_VIDEO_STREAM );
	}
	if( bCommit )
	{
		if( m_nSoundRate    != g_Sound.nRate        ||
			m_bEnableStereo != (BOOL)POKEYSND_stereo_enabled ||
			m_ulSoundState  != g_Sound.ulState      ||
			m_nSoundLatency != g_Sound.nLatency     ||
			m_nSoundQuality != g_Sound.nQuality )
		{
			bCommit = StreamWarning( IDS_WARN_RECORD_SNDOUT, SRW_SOUND_STREAM );
		}
	}
	if( bCommit )
	{
		if( m_ulSoundState != g_Sound.ulState )
		{
			g_Sound.ulState = m_ulSoundState;
			WriteRegDWORD( NULL, REG_SOUND_STATE, g_Sound.ulState );

			m_bModeChanged = TRUE;
		}
		if( m_nSoundRate != g_Sound.nRate )
		{
			g_Sound.nRate = m_nSoundRate;
			WriteRegDWORD( NULL, REG_SOUND_RATE, g_Sound.nRate );

			m_bModeChanged = TRUE;
		}
		if( m_nSoundLatency != g_Sound.nLatency )
		{
			g_Sound.nLatency = m_nSoundLatency;
			WriteRegDWORD( NULL, REG_SOUND_LATENCY, g_Sound.nLatency );

			m_bModeChanged = TRUE;
		}
		if( m_nSoundQuality != g_Sound.nQuality )
		{
			g_Sound.nQuality = m_nSoundQuality;
			WriteRegDWORD( NULL, REG_SOUND_QUALITY, g_Sound.nQuality );

			m_bModeChanged = TRUE;
		}
		if( m_bEnableStereo != (BOOL)POKEYSND_stereo_enabled )
		{
			POKEYSND_stereo_enabled = (int)m_bEnableStereo;
			WriteRegDWORD( NULL, REG_ENABLE_STEREO, POKEYSND_stereo_enabled );

			m_bModeChanged = TRUE;
		}
		/* Set new sound volume */
		if( m_nSoundVol != g_Sound.nVolume )
		{
			g_Sound.nVolume = m_nSoundVol;
			WriteRegDWORD( NULL, REG_SOUND_VOLUME, g_Sound.nVolume );
		}
		CCommonDlg::OnOK();
	}
} /* #OF# CSoundDlg::OnOK */
