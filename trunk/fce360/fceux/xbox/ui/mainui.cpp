#include "fceux\git.h"
#include "fceux\driver.h"
#include "fceux\video.h"
#include <xtl.h>
#include <xui.h>
#include <xuiapp.h>
#include <xuihtml.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <tchar.h>
#include "..\Cemulator.h"

extern Cemulator emul;//smsplus_pc.cpp

std::wstring strtowstr(std::string str)
{
	wchar_t buffer[1000];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer, 1000);
	std::wstring s = buffer;
	return s;
}
	
class CXuiEmulationScene: public CXuiSceneImpl{
protected:
	void EnableTab(bool enable){
		 // Get the parent tabbed scene.
		CXuiTabScene tabbedScene( m_hObj );
		if( FAILED( tabbedScene.GetParent( &tabbedScene ) ) )
		{
			return;
		}
		else
		{
			tabbedScene.EnableTabbing(enable);
		}
	}
	void GoToNext()
	{
		 // Get the parent tabbed scene.
		CXuiTabScene tabbedScene( m_hObj );
		if( FAILED( tabbedScene.GetParent( &tabbedScene ) ) )
		{
			return;
		}
		else
		{
			EnableTab(true);
			tabbedScene.GotoNext();
			EnableTab(false);
		}
	}
	void Goto(int n)
	{
		CXuiTabScene tabbedScene( m_hObj );
		if( FAILED( tabbedScene.GetParent( &tabbedScene ) ) )
		{
			return;
		}
		else
		{
			EnableTab(true);
			tabbedScene.Goto(n);
			EnableTab(false);
		}

	}
	void GotoPrev()
	{
		 // Get the parent tabbed scene.
		CXuiTabScene tabbedScene( m_hObj );
		if( FAILED( tabbedScene.GetParent( &tabbedScene ) ) )
		{
			return;
		}
		else
		{
			EnableTab(true);
			tabbedScene.GotoPrev();
			EnableTab(false);
		}
	}
};

class XuiRunner:public CXuiEmulationScene{
/*
	Display (nothing) when emulation is running
*/
	CXuiControl XuiNext;
	
public:
	XUI_IMPLEMENT_CLASS( XuiRunner, L"XuiRunner", XUI_CLASS_SCENE );

    XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_LEAVE_TAB ( OnLeaveTab )
		XUI_ON_XM_ENTER_TAB ( OnEnterTab )
    XUI_END_MSG_MAP()
	
	HRESULT OnEnterTab( BOOL &bHandled){
		emul.RenderEmulation = true;//run emulation
		return S_OK;
	}

	HRESULT OnLeaveTab( BOOL &bHandled){
		emul.RenderEmulation = false;//stop emulation
		return S_OK;
	}

    //--------------------------------------------------------------------------------------
    // Name: OnInit
    // Desc: Message handler for XM_INIT
    //--------------------------------------------------------------------------------------
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
    {
		EnableTab(false);

		HRESULT hr = GetChildById( L"XuiNext", &XuiNext );
        if( FAILED( hr ) ){	return hr;	}
		
        return S_OK;
    }


	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
	{
		HRESULT hr = S_OK;
		if( hObjPressed == XuiNext )
		{
			GoToNext();
		}
		return S_OK;
	}
};

class Osd: public CXuiEmulationScene
{
private:
	CXuiSlider XuiSaveStateSlot;

	CXuiControl	XuiSaveState;
	CXuiControl	XuiLoadState;
	CXuiControl XuiReset;
	CXuiControl XuiBack;
	CXuiControl XuiLoadGame;

	CXuiCheckbox XuiNormal;
	CXuiCheckbox XuiHq2x;
	
	CXuiCheckbox XuiTv;
	CXuiCheckbox XuiFullscren;

public:
	XUI_IMPLEMENT_CLASS( Osd, L"Osd", XUI_CLASS_SCENE );

    XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
    		XUI_ON_XM_LEAVE_TAB ( OnLeaveTab )
		XUI_ON_XM_ENTER_TAB ( OnEnterTab )
    XUI_END_MSG_MAP()
	
	HRESULT OnEnterTab( BOOL &bHandled){
		emul.RenderEmulation = true;//run emulation
		FCEUI_SetEmulationPaused(1);
		return S_OK;
	}

	HRESULT OnLeaveTab( BOOL &bHandled){
		emul.RenderEmulation = false;//stop emulation
		FCEUI_SetEmulationPaused(0);
		return S_OK;
	}

    //--------------------------------------------------------------------------------------
    // Name: OnInit
    // Desc: Message handler for XM_INIT
    //--------------------------------------------------------------------------------------
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
    {
		EnableTab(false);

		HRESULT hr = GetChildById( L"XuiSaveStateSlot", &XuiSaveStateSlot );
        if( FAILED( hr ) ){	return hr;	}

		hr = GetChildById( L"XuiSaveState", &XuiSaveState );
        if( FAILED( hr ) ){	return hr;	}

		hr = GetChildById( L"XuiBack", &XuiBack );
        if( FAILED( hr ) ){	return hr;	}

		hr = GetChildById( L"XuiLoadState", &XuiLoadState );
        if( FAILED( hr ) ){	return hr;	}

		hr = GetChildById( L"XuiReset", &XuiReset );
        if( FAILED( hr ) ){	return hr;	}
		
		hr = GetChildById( L"XuiLoadGame", &XuiLoadGame );
        if( FAILED( hr ) ){	return hr;	}
#if 0
		//
		hr = GetChildById( L"XuiNormal", &XuiNormal );
        if( FAILED( hr ) ){	return hr;	}

		hr = GetChildById( L"XuiFullscren", &XuiFullscren );
        if( FAILED( hr ) ){	return hr;	}

		hr = GetChildById( L"XuiHq2x", &XuiHq2x );
        if( FAILED( hr ) ){	return hr;	}

		hr = GetChildById( L"XuiTv", &XuiTv );
        if( FAILED( hr ) ){	return hr;	}

		XuiNormal.SetCheck(true);
		XuiFullscren.SetCheck(true);

		XuiHq2x.SetCheck(false);
		XuiTv.SetCheck(false);
#endif
        return S_OK;
    }

	void SwFilter()
	{
		bool b_hq2x = XuiHq2x.IsChecked();
		bool b_normal = XuiNormal.IsChecked();

		if(b_hq2x) XuiNormal.SetCheck(false);
		if(b_normal) XuiHq2x.SetCheck(false);
	}

	void VertexFilter()
	{
		bool b_fullscreen = XuiFullscren.IsChecked();
		if(b_fullscreen)
		{
			XuiTv.SetCheck(false);
			XuiFullscren.SetCheck(true);
			emul.SetVertexFilter(TvScreen);
		}

		bool b_tv = XuiTv.IsChecked();
		if(b_tv)
		{
			XuiFullscren.SetCheck(false);
			XuiTv.SetCheck(true);
			emul.SetVertexFilter(FullScreen);
		}
		
	}

	//----------------------------------------------------------------------------------
	// Name: OnNotifyPress
	// Desc: Handler for the button press message.
	//----------------------------------------------------------------------------------
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
	{
		HRESULT hr = S_OK;
		//sw filter
		if( hObjPressed == XuiNormal )
		{
			SwFilter();
		}
		if( hObjPressed == XuiHq2x )
		{
			SwFilter();
		}
		//vertex
		if( hObjPressed == XuiTv )
		{
			VertexFilter();
		}
		if( hObjPressed == XuiFullscren )
		{
			VertexFilter();
		}

		if( hObjPressed == XuiSaveState )
		{
			int val = 0;
			char state_name[512];
			char rom_name[512];
	
			extern FCEUGI * GameInfo;

			XuiSaveStateSlot.GetValue(&val);

			strcpy(rom_name,"");
			for(int x=0;x<16;x++)
				sprintf(rom_name, "%s%02x",rom_name,GameInfo->MD5[x]);

			sprintf(state_name, "game:\\states\\%s-%d.sav",rom_name,val);
			
			FCEUI_SaveState(state_name);
			bHandled = TRUE;
		}
		if( hObjPressed == XuiLoadState )
		{
			int val = 0;
			char state_name[512];
			char rom_name[512];
	
			XuiSaveStateSlot.GetValue(&val);

			extern FCEUGI * GameInfo;

			strcpy(rom_name,"");
			for(int x=0;x<16;x++)
				sprintf(rom_name, "%s%02x",rom_name,GameInfo->MD5[x]);

			sprintf(state_name, "game:\\states\\%s-%d.sav",rom_name,val);
		
			FCEUI_LoadState(state_name);

			bHandled = TRUE;
		}
		if( hObjPressed == XuiReset )
		{
			FCEUI_ResetNES();
			bHandled = TRUE;
		}
		if( hObjPressed == XuiBack )
		{
			GotoPrev();
			bHandled = TRUE;
		}
		if( hObjPressed == XuiLoadGame)
		{
			Goto(0);
			bHandled = TRUE;
		}
		return S_OK;
	}

};

class LoadGame: public CXuiEmulationScene
{
private:
	CXuiList XuiRomList;

	typedef struct s_rom_item{
		std::string path;
		std::string filename;
		std::wstring affichage;
	} rom_item;

	//list des roms
	std::vector<rom_item> m_rom_list;

public:
	XUI_IMPLEMENT_CLASS( LoadGame, L"RomChoose", XUI_CLASS_SCENE );

    XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
    XUI_END_MSG_MAP()

	//----------------------------------------------------------------------------------
	// Name: OnNotifyPress
	// Desc: Handler for the button press message.
	//----------------------------------------------------------------------------------
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
	{
		HRESULT hr = S_OK;
		if( hObjPressed == XuiRomList )
		{
			std::string sRom = m_rom_list.at(XuiRomList.GetCurSel()).path;
			sRom += m_rom_list.at(XuiRomList.GetCurSel()).filename;
			emul.LoadGame( sRom ,true);

			GoToNext();
		}
		return S_OK;
	}
    //--------------------------------------------------------------------------------------
    // Name: OnInit
    // Desc: Message handler for XM_INIT
    //--------------------------------------------------------------------------------------
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
    {
		EnableTab(false);

        HRESULT hr = GetChildById( L"XuiRomList", &XuiRomList );
        if( FAILED( hr ) )
		{
			return hr;
		}
		else
		{
			//XuiRomList.addi
			if(!FAILED(ScanDir()))
			{
				std::vector<rom_item>::iterator it;
				XuiRomList.InsertItems(0, m_rom_list.size());
				int i = 0;
				for ( it=m_rom_list.begin() ; it < m_rom_list.end(); it++ )
				{
					XuiRomList.SetText(i, (*it).affichage.c_str());
					i++;
				}
			}
		}

        return S_OK;
    }
private:
	HRESULT ScanDir()
	{
		HANDLE				hFind;                   // Handle to file
		WIN32_FIND_DATA	FileInformation;         // File information

		m_rom_list.clear();

		hFind = FindFirstFile( "game:\\roms\\*", &FileInformation );
		if( hFind != INVALID_HANDLE_VALUE )
		{
			do
			{
				{
					if(!(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ))
					{
						if(
							strstr(FileInformation.cFileName,".nes") ||
							//strstr(FileInformation.cFileName,".gg") ||
							strstr(FileInformation.cFileName,".zip")
						)
						{
							rom_item s;
							s.path="game:\\roms\\";
							s.filename= FileInformation.cFileName;
							s.affichage = strtowstr(FileInformation.cFileName);
							m_rom_list.push_back(s);
						}
					}
				}
			}
			while( FindNextFile( hFind, &FileInformation ) == TRUE );
			FindClose( hFind );
		}
		//Tri alphabetic
		return S_OK;
	}
};

//--------------------------------------------------------------------------------------
// Main xui 
//--------------------------------------------------------------------------------------
class XboxUI : public CXuiModule
{
	public:
		XboxUI()
        {
        }
        ~XboxUI()
        {
        }
	protected:
		HRESULT RegisterXuiClasses(){
			XuiVideoRegister();
			XuiSoundXACTRegister();
			XuiSoundXAudioRegister();
			XuiHtmlRegister();

			LoadGame::Register();
			Osd::Register();
			XuiRunner::Register();
			return S_OK;
		};
		HRESULT UnregisterXuiClasses(){
			XuiVideoUnregister();
			XuiSoundXACTUnregister();
			XuiSoundXAudioUnregister();
			XuiHtmlUnregister();

			LoadGame::Unregister();
			Osd::Unregister();
			XuiRunner::Unregister();
			return S_OK;
		};
};

XboxUI gUi;

HRESULT InitUi(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS d3dpp)
{
	HRESULT hr = gUi.InitShared( pDevice, &d3dpp, XuiPNGTextureLoader );
    if( FAILED( hr ) )
        return hr;

	 // Register a default typeface
    hr = gUi.RegisterDefaultTypeface( L"Arial", L"file://game:/media/xarialuni.ttf" );
    if( FAILED( hr ) )
        return hr;
    hr = gUi.LoadSkin( L"file://game:/media/ui.xzp#media\\xui\\skin_default.xur" );
    if( FAILED( hr ) )
        return hr;

    //hr = gUi.LoadFirstScene( L"file://game:/media/ui.xzp#media\\xui\\", L"main.xur" );
	hr = gUi.LoadFirstScene( L"file://game:/media/ui.xzp#media\\xui\\", L"LoadGame.xur" );
    if( FAILED( hr ) )
        return hr;

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Update the UI
//--------------------------------------------------------------------------------------

HRESULT RenderXui(IDirect3DDevice9* pDevice)
{
	//update ui
	gUi.RunFrame();

	XuiTimersRun();

	XuiRenderBegin( gUi.GetDC(), D3DCOLOR_ARGB( 255, 0, 0, 0 ) );

    D3DXMATRIX matOrigView;
    XuiRenderGetViewTransform( gUi.GetDC(), &matOrigView );

	XUIMessage msg;
    XUIMessageRender msgRender;
    XuiMessageRender( &msg, &msgRender, gUi.GetDC(), 0xffffffff, XUI_BLEND_NORMAL );
    XuiSendMessage( gUi.GetRootObj(), &msg );

    XuiRenderSetViewTransform( gUi.GetDC(), &matOrigView );

    XuiRenderEnd( gUi.GetDC() );
	
    pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

	return S_OK;
}