#pragma once

#include <Xaudio2.h>

#ifdef _XBOX
HRESULT InitUi(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS d3dpp);
HRESULT RenderXui(IDirect3DDevice9* pDevice);
void UpdateUI();
#endif

//-----------------------------------------------------------------------------
// Path
//-----------------------------------------------------------------------------
#ifdef _XBOX
#	define EFFECT_FILE "game:\\media\\effect.fx"
#	define DEFAULT_GAME "game:\\roms\\sonic.ss"
#	define X_FILE "game:\\media\\screen.x"
#	define BG_FILE "game:\\media\\background.png"
#else
#	define EFFECT_FILE "media\\effect.fx"
#	define DEFAULT_GAME "roms\\sonic.sms"
#	define X_FILE "media\\test.x"
#	define BG_FILE "media\\bg.jpg"
#endif

#define g_dwTileWidth   1280
#define g_dwTileHeight  256
#define g_dwFrameWidth  1280
#define g_dwFrameHeight 720

enum VIDEO_VERTEX_FILTER{
	FullScreen,//Default
	TvScreen
};

class Cemulator
{
public:
	DWORD SelectedVertexFilter;

	void SetVertexFilter(int i){
		SelectedVertexFilter = i;
	}

	DWORD GetVertexFilter(){
		return SelectedVertexFilter;
	}

private:
	std::string defaut_rom;
	int mWidth;
	int mHeight;

	void SetSystemWidth(int w)
	{
		mWidth=w;
	};

	void SetSystemHeight(int h)
	{
		mHeight=h;
	};

	int GetSystemWidth()
	{
		return mWidth;
	};

	int GetSystemHeight()
	{
		return mHeight;
	};

	int GetWidth()
	{
		return 256;
	};
	int GetHeight()
	{
		return 240;
	};

	bool end;

	void SendExitSignal()
	{
		end=true;
	}
		
	// Synchronization Event
	class XAudio2_BufferNotify : public IXAudio2VoiceCallback
	{
	public:
		HANDLE hBufferEndEvent;

		XAudio2_BufferNotify() {
			hBufferEndEvent = NULL;
			hBufferEndEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
			//ASSERT( hBufferEndEvent != NULL );
		}

		~XAudio2_BufferNotify() {
			CloseHandle( hBufferEndEvent );
			hBufferEndEvent = NULL;
		}

		STDMETHOD_( void, OnBufferEnd ) ( void *pBufferContext ) {
			//ASSERT( hBufferEndEvent != NULL );
			SetEvent( hBufferEndEvent );
		}


		// dummies:
		STDMETHOD_( void, OnVoiceProcessingPassStart ) ( UINT32 BytesRequired ) {}
		STDMETHOD_( void, OnVoiceProcessingPassEnd ) () {}
		STDMETHOD_( void, OnStreamEnd ) () {}
		STDMETHOD_( void, OnBufferStart ) ( void *pBufferContext ) {}
		STDMETHOD_( void, OnLoopEnd ) ( void *pBufferContext ) {}
		STDMETHOD_( void, OnVoiceError ) ( void *pBufferContext, HRESULT Error ) {};
	};

	XAudio2_BufferNotify    notify; // buffer end notification
public:
	Cemulator(void);

	//render sms or ui
	bool RenderEmulation;

	//PC SIDE
	HRESULT InitVideo();
	HRESULT InitAudio();
	HRESULT InitInput();
	HRESULT CloseVideo();
	HRESULT CloseAudio();
	HRESULT CloseInput();
	HRESULT Finish(){
		SendExitSignal();
		return S_OK;
	};

	HRESULT LoadGame(std::string name, bool restart);

	//SYSTEM SIDE
	HRESULT InitSystem();
	HRESULT CloseSystem();

	void Render();
	void UpdateVideo();
	void UpdateAudio(int * snd, int sndsize);
	void UpdateInput();

	void SetStartGame(std::string gamename)
	{
		defaut_rom=gamename;
	}
	//Run
	HRESULT Run();
};
