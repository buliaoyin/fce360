#include "xbox/fceusupport.h"
#include "stdafx.h"

#ifdef _XBOX
#	include <xtl.h>
#	include <fxl.h>
#	include <xui.h>
#else
#	include <windows.h>
#	include <d3d9.h>
#	include <d3dx9tex.h>
#	include <XInput.h>
#endif
#include <string>
#include <vector>
#include <xaudio2.h>
extern "C"
{
}
#include "Cemulator.h"
#include "audio.h"
#include "input.h"

//#define printf writeline

//ATG
VOID UnloadFile( VOID* pFileData );
HRESULT LoadFile( const CHAR* strFileName, VOID** ppFileData, DWORD* pdwFileSize );

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9             g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // Buffer to hold vertices
IDirect3DTexture9 * g_texture = NULL;
IDirect3DTexture9 * g_bg_texture = NULL;
D3DPRESENT_PARAMETERS g_d3dpp;
LPD3DXEFFECT g_effect = NULL; //handle to D3DXEffect

// Rendering surfaces and textures
#ifdef _XBOX
D3DSurface*                 m_pBackBuffer;
D3DSurface*                 m_pDepthBuffer;
D3DTexture*                 m_pFrontBuffer;
#endif

//-------------------------------------------------------------------------------------
// Shader
//-------------------------------------------------------------------------------------
IDirect3DVertexDeclaration9* g_pVertexDecl;   // Vertex format decl

D3DXMATRIX g_matWorld;
D3DXMATRIX g_matProj;
D3DXMATRIX g_matView;
D3DXMATRIX g_matWorldViewProjection;

//-------------------------------------------------------------------------------------
// Audio
//-------------------------------------------------------------------------------------
#define SOUND_SAMPLES_SIZE  2048

uint8 * bitmap;// = (uint8*)malloc(256*256);
int16 g_sound_buffer[48000];

IXAudio2* g_pXAudio2 = NULL;
IXAudio2MasteringVoice* g_pMasteringVoice = NULL;
IXAudio2SourceVoice* g_pSourceVoice = NULL;
WAVEFORMATEXTENSIBLE wfx;
XAUDIO2_BUFFER g_SoundBuffer;


//-------------------------------------------------------------------------------------
// Input
//-------------------------------------------------------------------------------------
//GAMEPAD* m_pGamepad;
uint32 powerpadbuf[2]={0,0};

//-------------------------------------------------------------------------------------
// TEXTURE
//-------------------------------------------------------------------------------------
struct TEXTURED
{
    FLOAT x, y, z;      // The untransformed, 3D position for the vertex
    FLOAT u,v;			// The texture coordonate
};

#define D3DFVF_TEXTURED (D3DFVF_XYZ|D3DFVF_TEX1)

static const D3DVERTEXELEMENT9 g_ElementsTextured[4] =
{
    { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    D3DDECL_END()
};

struct TEXTURED g_VerticesTextured[] =
{
	//square
	{ -1.0f, -1.0f, 0.0f,  0.0f,  1.0f },//1
	{ -1.0f,  1.0f, 0.0f,  0.0f,  0.0f },//2
	{  1.0f, -1.0f, 0.0f,  1.0f,  1.0f },//3
	{  1.0f,  1.0f, 0.0f,  1.0f,  0.0f }//4
};

static const D3DRECT g_tiles[3] = 
{
    {             0,              0,  g_dwTileWidth,  g_dwTileHeight },
    {             0, g_dwTileHeight,  g_dwTileWidth, g_dwTileHeight * 2 },
    {             0, g_dwTileHeight * 2,  g_dwTileWidth, g_dwFrameHeight },
};

//-------------------------------------------------------------------------------------
// TEXTURE
//-------------------------------------------------------------------------------------
D3DXHANDLE  g_MaterialAmbientColor;
D3DXHANDLE  g_MaterialDiffuseColor;
D3DXHANDLE  g_mWorldViewProjection;
D3DXHANDLE  g_MeshTexture;
D3DXHANDLE  g_technique_bg;
D3DXHANDLE  g_technique_model;
D3DXHANDLE  g_technique_model_tv;
D3DXHANDLE  g_technique_model_fullscreen;

D3DXHANDLE  g_TexelSize;
D3DXHANDLE  g_fTime;
LPD3DXBUFFER materialBuffer;
DWORD numMaterials;            // Note: DWORD is a typedef for unsigned long
LPD3DXMESH mesh;

float g_pTexelSize[2];

//SwFilter
enum SwFilter{
	NORMAL=1,
	HQ2X=2,
	HQ3X=3
};
DWORD SelectedSwFilter = HQ3X;

//-------------------------------------------------------------------------------------
// Cemulator
//-------------------------------------------------------------------------------------
Cemulator::Cemulator(void)
{
	end=false;
	//defaut_rom=(DEFAULT_GAME);
	RenderEmulation = false;//Display xui at first

	SelectedVertexFilter = FullScreen;
}

HRESULT Cemulator::InitVideo(){

//-------------------------------------------------------------------------------------
// Init Os
//-------------------------------------------------------------------------------------
	//InitWindows();

//-------------------------------------------------------------------------------------
// Create d3d device
//-------------------------------------------------------------------------------------
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if( !g_pD3D )
        return E_FAIL;

    ZeroMemory( &g_d3dpp, sizeof(g_d3dpp) );
#ifdef _XBOX
	// Set up the structure used to create the D3DDevice.
	XVIDEO_MODE VideoMode;
	ZeroMemory( &VideoMode, sizeof( VideoMode ) );
	XGetVideoMode( &VideoMode );
	BOOL bEnable720p = ( VideoMode.dwDisplayHeight >= 720 ) ? TRUE : FALSE;
	SetSystemWidth(( bEnable720p ) ? 1280 : 640);
	SetSystemHeight(( bEnable720p ) ?  720 : 480);
#else
	SetSystemWidth(720);SetSystemHeight(480);
#endif

#ifndef _XBOX
    d3dpp.Windowed = TRUE;
	d3dpp.BackBufferWidth        = 1280;
    d3dpp.BackBufferHeight       = 720;
	d3dpp.BackBufferFormat       = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount        = 1;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE ;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
#else
	g_d3dpp.BackBufferWidth = GetSystemWidth();
	g_d3dpp.BackBufferHeight = GetSystemHeight();
    g_d3dpp.BackBufferFormat = ( D3DFORMAT )MAKESRGBFMT( D3DFMT_A8R8G8B8 );
    g_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    g_d3dpp.MultiSampleQuality = 0;
    g_d3dpp.BackBufferCount = 0;
    g_d3dpp.EnableAutoDepthStencil = FALSE;
    g_d3dpp.DisableAutoBackBuffer = TRUE;
    g_d3dpp.DisableAutoFrontBuffer = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
#endif

#ifdef _XBOX
	D3DSURFACE_PARAMETERS params = {0};
	g_pd3dDevice->CreateRenderTarget( g_dwTileWidth, g_dwTileHeight, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_4_SAMPLES, 0, 0, &m_pBackBuffer, &params );
	params.Base = m_pBackBuffer->Size / GPU_EDRAM_TILE_SIZE;
	params.HierarchicalZBase = D3DHIZFUNC_GREATER_EQUAL;

	g_pd3dDevice->CreateDepthStencilSurface( g_dwTileWidth, g_dwTileHeight, D3DFMT_D24S8, D3DMULTISAMPLE_4_SAMPLES, 0, 0, &m_pDepthBuffer, &params );
	g_pd3dDevice->CreateTexture( g_dwFrameWidth, g_dwFrameHeight, 1, 0, D3DFMT_LE_X8R8G8B8, 0, &m_pFrontBuffer, NULL );
	if( FAILED( g_pD3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL, D3DCREATE_BUFFER_2_FRAMES, &g_d3dpp, &g_pd3dDevice ) ) )
	{
		printf("CreateDevice failed\n");
        return E_FAIL;
	}

#else
	if( FAILED( g_pD3D->CreateDevice( 0, D3DDEVTYPE_HAL, hWnd,
										D3DCREATE_SOFTWARE_VERTEXPROCESSING,
										&d3dpp, &g_pd3dDevice ) ) )
	{
		printf("CreateDevice failed\n");
        return E_FAIL;
	}
#endif
//-------------------------------------------------------------------------------------
// MSAA surface
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// Create the buffer, and load the effect from the file.
//-------------------------------------------------------------------------------------
	HRESULT Result;
	ID3DXEffectCompiler* pCompiler = NULL;
	ID3DXBuffer* pCompiledData = NULL;

//-------------------------------------------------------------------------------------
// Create a ID3DXEffectCompiler interface for the effect that was just loaded.
//-------------------------------------------------------------------------------------
	Result = D3DXCreateEffectCompilerFromFileA(
         EFFECT_FILE,
         NULL,
         NULL,
         0,
         &pCompiler,
         NULL
	);
	if (FAILED(Result))
    {
		printf( "D3DXCreateEffectCompiler FAILED\n" );
        return Result;
    }

    // Compile the effect by using the ID3DXEffectCompiler interface, and then release the compiler.
    Result = pCompiler->CompileEffect(
		D3DXSHADER_DEBUG , 
		&pCompiledData, 
		NULL
	);

    pCompiler->Release();
    if (FAILED(Result))
    {
		printf( "CompileEffect FAILED\n" );
        return Result;
    }

    // Create the effect that was just compiled.
    Result = D3DXCreateEffect(g_pd3dDevice, (DWORD*)pCompiledData->GetBufferPointer(), pCompiledData->GetBufferSize(),
                                        NULL, NULL, 0, NULL, &g_effect, NULL);

//-------------------------------------------------------------------------------------
// Create the model
//-------------------------------------------------------------------------------------

	HRESULT hr=D3DXLoadMeshFromXA(X_FILE, D3DXMESH_SYSTEMMEM, 
                             g_pd3dDevice, NULL, 
                             &materialBuffer,NULL, &numMaterials, 
                             &mesh );
//-------------------------------------------------------------------------------------
// Load the bg
//-------------------------------------------------------------------------------------
	D3DXCreateTextureFromFileA(g_pd3dDevice,BG_FILE,&g_bg_texture);

//-------------------------------------------------------------------------------------
// Create SMS RenderTexture
//-------------------------------------------------------------------------------------
	D3DXCreateTexture(
		g_pd3dDevice, GetWidth(),
		GetHeight(), D3DX_DEFAULT, 0, D3DFMT_LIN_A8R8G8B8, D3DPOOL_MANAGED,
		&g_texture
	);

//-------------------------------------------------------------------------------------
// Create VB
//-------------------------------------------------------------------------------------
	g_pd3dDevice->CreateVertexDeclaration( g_ElementsTextured, &g_pVertexDecl );

    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 4*sizeof(TEXTURED),
                                                  D3DUSAGE_WRITEONLY, 
                                                  NULL,
                                                  D3DPOOL_MANAGED, 
                                                  &g_pVB, 
                                                  NULL ) ) )
	{
		printf("CreateVertexBuffer failed\n");
        return E_FAIL;
	}

	TEXTURED* pVertices;
    if( FAILED( g_pVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
        return E_FAIL;
    memcpy( pVertices, g_VerticesTextured, 4*sizeof(TEXTURED) );
    g_pVB->Unlock();

//-------------------------------------------------------------------------------------
// Param from effectfile
//-------------------------------------------------------------------------------------	
	g_technique_bg = g_effect->GetTechniqueByName("RenderFullScreen");
	g_technique_model = g_effect->GetTechniqueByName("RenderModel");
	g_technique_model_tv = g_effect->GetTechniqueByName("RenderModelTv");
	g_technique_model_fullscreen = g_effect->GetTechniqueByName("RenderModelFullScreen");

	g_mWorldViewProjection = g_effect->GetParameterByName(NULL,  "g_mWorldViewProjection" );
	g_MaterialAmbientColor = g_effect->GetParameterByName(NULL,  "g_MaterialAmbientColor" );
	g_MaterialDiffuseColor = g_effect->GetParameterByName(NULL,  "g_MaterialDiffuseColor" );
	g_mWorldViewProjection = g_effect->GetParameterByName(NULL,  "g_mWorldViewProjection" );
	g_MeshTexture = g_effect->GetParameterByName(NULL,  "g_MeshTexture" );
	g_TexelSize = g_effect->GetParameterByName(NULL,  "g_TexelSize" );
	g_fTime = g_effect->GetParameterByName(NULL,  "g_fTime" );

//-------------------------------------------------------------------------------------
// Default Renderstates
//-------------------------------------------------------------------------------------
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

	return S_OK;
};


void Cemulator::UpdateVideo(){
//-------------------------------------------------------------------------------------
// Refresh texture cache
//-------------------------------------------------------------------------------------
	RECT d3dr;

	d3dr.left=0;
	d3dr.top=0;
	d3dr.right=GetWidth();
	d3dr.bottom=GetHeight();

	D3DLOCKED_RECT texture_info;
	
	g_texture->LockRect( 0,  &texture_info, &d3dr, NULL );
	g_texture->UnlockRect(0);

//-------------------------------------------------------------------------------------
// Initialise view
//-------------------------------------------------------------------------------------
	// Initialize the world matrix
	D3DXMatrixIdentity(&g_matWorld);
	D3DXMATRIX scale;

    // Initialize the projection matrix
    FLOAT fAspect = 16.0f / 9.0f; 
	//FLOAT fAspect = 1.0f;
	D3DXMatrixPerspectiveFovLH(&g_matProj, D3DX_PI/4, fAspect, 1.0f, 200.0f );

    // Initialize the view matrix
	D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.f, 0.f, -8.f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.f, 0.f, 0.f );
    D3DXVECTOR3 vUp       = D3DXVECTOR3( 0.f, 1.f, 1.f );
	D3DXMatrixLookAtLH(&g_matView,&vEyePt, &vLookatPt, &vUp );

	g_matWorldViewProjection = g_matWorld * g_matView * g_matProj;
};

HRESULT Cemulator::InitAudio()
{
	memset(g_sound_buffer,0,48000);

	//return S_OK;
//-------------------------------------------------------------------------------------
// Initialise Audio
//-------------------------------------------------------------------------------------	
	HRESULT hr;
	if( FAILED( hr = XAudio2Create( &g_pXAudio2, 0 ) ) )
    {
        printf( "Failed to init XAudio2 engine: %#X\n", hr );
        return E_FAIL;
    }

//-------------------------------------------------------------------------------------
// Create a mastering voice
//-------------------------------------------------------------------------------------	
    if( FAILED( hr = g_pXAudio2->CreateMasteringVoice( &g_pMasteringVoice ) ) )
    {
		printf( "Failed creating mastering voice: %#X\n", hr );
        return E_FAIL;
    }

//-------------------------------------------------------------------------------------
// Create source voice
//-------------------------------------------------------------------------------------	
	WAVEFORMATEXTENSIBLE wfx;
	memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));
#if 0
	wfx.Format.wFormatTag           = WAVE_FORMAT_EXTENSIBLE;
	wfx.Format.nSamplesPerSec       = 44100;
	wfx.Format.nChannels            = 2;
	wfx.Format.wBitsPerSample       = 16;
	wfx.Format.nBlockAlign          = wfx.Format.nChannels*wfx.Format.wBitsPerSample/8;
	wfx.Format.nAvgBytesPerSec      = 44100 * wfx.Format.nBlockAlign;
	wfx.Format.cbSize               = sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
	wfx.Samples.wValidBitsPerSample = wfx.Format.wBitsPerSample;
	wfx.dwChannelMask               = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
	wfx.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;
#else
	wfx.Format.wFormatTag           = WAVE_FORMAT_EXTENSIBLE ;
	wfx.Format.nSamplesPerSec       = 48000;
	wfx.Format.nChannels            = 1;
	wfx.Format.wBitsPerSample       = 16;
	wfx.Format.nBlockAlign          = wfx.Format.nChannels*wfx.Format.wBitsPerSample/8;
	wfx.Format.nAvgBytesPerSec      = wfx.Format.nSamplesPerSec * wfx.Format.nBlockAlign;
	wfx.Format.cbSize               = sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
	wfx.Samples.wValidBitsPerSample = wfx.Format.wBitsPerSample;
	wfx.dwChannelMask               = SPEAKER_MONO;
	wfx.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;
#endif
//-------------------------------------------------------------------------------------
//	Csv
//-------------------------------------------------------------------------------------
	if(FAILED( g_pXAudio2->CreateSourceVoice(&g_pSourceVoice,(WAVEFORMATEX*)&wfx,0, 1.0f, &notify)	))
	{
		printf("CreateSourceVoice failed\n");
		return E_FAIL;
	}

//-------------------------------------------------------------------------------------
// Start sound
//-------------------------------------------------------------------------------------	
	if ( FAILED(g_pSourceVoice->Start( 0 ) ) )
	{
		printf("g_pSourceVoice failed\n");
		return E_FAIL;
	}
	return S_OK;

};

void Cemulator::UpdateAudio(int * snd, int sndsize)
{
	if(sndsize==0)
		return;

	XAUDIO2_VOICE_STATE VoiceState;

	while( true ) 
	{
		g_pSourceVoice->GetState(&VoiceState);
		if(VoiceState.BuffersQueued < 4)
		{
			break;
		}
		else
		{
			WaitForSingleObject( notify.hBufferEndEvent, INFINITE );
		}
	}

//-------------------------------------------------------------------------------------
// Rebuild sound stream
//-------------------------------------------------------------------------------------	
	int submit_size = sndsize;

	if(submit_size % 4)
		submit_size =( ( sndsize / 4 ) + 1 ) * 4;

	int iii = 0;
	for(int i=0;i<sndsize;i++)
	{
		g_sound_buffer[i]=snd[i];
	}
	
	g_SoundBuffer.AudioBytes = submit_size * sizeof(int16);	//size of the audio buffer in bytes
	g_SoundBuffer.pAudioData = (BYTE*)g_sound_buffer;		//buffer containing audio data
	//g_SoundBuffer.Flags = XAUDIO2_END_OF_STREAM;

//-------------------------------------------------------------------------------------
// Send sound stream
//-------------------------------------------------------------------------------------	
	if( FAILED(g_pSourceVoice->SubmitSourceBuffer( &g_SoundBuffer ) ) )
	{
		printf("SubmitSourceBuffer failed\n");
		return ;
	}
};


HRESULT Cemulator::InitInput()
{
	return S_OK;
}

void Cemulator::UpdateInput()
{
	// Get input from all the gamepads
	GAMEPAD Gamepads[XUSER_MAX_COUNT];
    Input::GetInput( Gamepads );

	unsigned char pad[4];
    memset(pad, 0, sizeof(char) * 4);

	for( DWORD dwUser = 0; dwUser < 2; dwUser++ )
	{
		if(!FCEUI_EmulationPaused()){
			/* Player #1 inputs */
			if(Gamepads[dwUser].wLastButtons & XINPUT_GAMEPAD_DPAD_UP)
				pad[dwUser] |= JOY_UP;

			if(Gamepads[dwUser].wLastButtons & XINPUT_GAMEPAD_DPAD_DOWN)
				pad[dwUser] |= JOY_DOWN;

			if(Gamepads[dwUser].wLastButtons & XINPUT_GAMEPAD_DPAD_LEFT)
				pad[dwUser] |= JOY_LEFT;

			if(Gamepads[dwUser].wLastButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
				pad[dwUser] |= JOY_RIGHT;

			if(Gamepads[dwUser].wLastButtons & XINPUT_GAMEPAD_A)
				pad[dwUser] |= JOY_A;

			if(Gamepads[dwUser].wLastButtons & XINPUT_GAMEPAD_B)
				pad[dwUser] |= JOY_B;

			if(Gamepads[dwUser].wLastButtons & XINPUT_GAMEPAD_START)
				pad[dwUser] |= JOY_START;

			
			if(Gamepads[dwUser].wLastButtons & XINPUT_GAMEPAD_BACK)
				pad[dwUser] |= JOY_SELECT;
/*
			if(Gamepads[dwUser].wLastButtons & XINPUT_GAMEPAD_X)
			{
				char state_name[512];
				int val = 0;

				sprintf(state_name, "game:\\states\\%s-%d.sav","mario",val);
		
				FCEUI_LoadState(state_name);
			}
			*/
		}

		//fé par xui ..
		/*
		if(Gamepads[dwUser].wLastButtons & XINPUT_GAMEPAD_Y)
			FCEUI_ToggleEmulationPause();
			*/
/*
		//Pause and display xui
		if(Gamepads[dwUser].wLastButtons & XINPUT_GAMEPAD_BACK)
		{
			mRenderSms=false;
		}
		*/
	}

	//JSReturn = pad[0] | pad[1] << 8 | pad[2] << 16 | pad[3] << 24;

	//powerpadbuf[0] = pad[0] | pad[1] << 8 | pad[2] << 16 | pad[3] << 24;;
	powerpadbuf[0] = pad[0];
	powerpadbuf[1] = pad[1];
};

HRESULT Cemulator::InitSystem()
{
//-------------------------------------------------------------------------------------
// Set up rendering texture
//-------------------------------------------------------------------------------------
	RECT d3dr;

	d3dr.left=0;
	d3dr.top=0;
	d3dr.right=GetWidth();
	d3dr.bottom=GetHeight();

	D3DLOCKED_RECT texture_info;
	g_texture->LockRect( 0,  &texture_info, &d3dr, NULL );

//-------------------------------------------------------------------------------------
// Set up bitmap structure
//-------------------------------------------------------------------------------------
   

	g_texture->UnlockRect(0);

//-------------------------------------------------------------------------------------
// Set up sound
//-------------------------------------------------------------------------------------
#if 0
	snd.fm_which = SND_EMU2413;
    snd.fps = (1) ? FPS_NTSC : FPS_PAL;
    snd.fm_clock = (1) ? CLOCK_NTSC : CLOCK_PAL;
    snd.psg_clock = (1) ? CLOCK_NTSC : CLOCK_PAL;
	snd.sample_rate = 44100;
	sms.territory = 0;
    sms.use_fm = 1;
	//sms.save=1;
#endif
//-------------------------------------------------------------------------------------
// Load roms
//-------------------------------------------------------------------------------------
#ifndef _XBOX
	if(FAILED(LoadGame((char*)defaut_rom.c_str(),false)))
	{
		return E_FAIL;
	}
#endif
	return S_OK;
//-------------------------------------------------------------------------------------
// Start system
//-------------------------------------------------------------------------------------

	return S_OK;
};

HRESULT Cemulator::LoadGame(std::string name, bool restart)
{
//-------------------------------------------------------------------------------------
// Load roms
//-------------------------------------------------------------------------------------

	FCEUI_SetBaseDirectory("game:");
	FCEUI_SetVidSystem(0);
	if(FCEUI_LoadGame(name.c_str() ,0)!=NULL)
	{
		
		FCEUI_SetInput(0, SI_GAMEPAD, (void*)&powerpadbuf[0], 0);
		FCEUI_SetInput(1, SI_GAMEPAD, (void*)&powerpadbuf[1], 0);

			//FCEUI_SetVidSystem(0);

		//set to ntsc
		extern FCEUGI * GameInfo;
		GameInfo->vidsys=GIV_NTSC;

		InitSound();

		if(restart)
			ResetNES();

		return S_OK;
	}
	
	return E_FAIL;
};

float ftime=0.f;

void Cemulator::Render()
{
	g_pd3dDevice->BeginScene();

	// Set effect variables as needed
    D3DXCOLOR colorMtrlDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
    D3DXCOLOR colorMtrlAmbient( 0.35f, 0.35f, 0.35f, 0 );

//-------------------------------------------------------------------------------------
// Clear screen
//-------------------------------------------------------------------------------------	
#ifdef _XBOX
	g_pd3dDevice->SetRenderTarget( 0, m_pBackBuffer );
	g_pd3dDevice->SetDepthStencilSurface( m_pDepthBuffer );

	const D3DVECTOR4 clearColor = { 0.f, 0.f, 0.f, 1.f };
    g_pd3dDevice->BeginTiling( 0, ARRAYSIZE(g_tiles), g_tiles, &clearColor, 1, 0 );

	g_pd3dDevice->SetPredication( D3DPRED_TILE( 0 ) );
	//g_pd3dDevice->Clear( D3DCLEAR_TARGET1 | D3DCLEAR_TARGET2 );
	g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_ZBUFFER |D3DCLEAR_TARGET0| D3DCLEAR_TARGET,D3DCOLOR_XRGB(70,140,255), 1.0f, 0L);
	g_pd3dDevice->SetPredication( 0 );
#else
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER |D3DCLEAR_TARGET,  D3DCOLOR_XRGB(70,140,255), 1.0f, 0 );
#endif

//-------------------------------------------------------------------------------------
// Setup technique
//-------------------------------------------------------------------------------------	
	g_pTexelSize[0]=1.f/float(GetWidth());
	g_pTexelSize[1]=1.f/float(GetHeight());
	ftime+=0.1f;
	g_effect->SetValue( g_MaterialAmbientColor, &colorMtrlAmbient, sizeof( D3DXCOLOR ) );
    g_effect->SetValue( g_MaterialDiffuseColor, &colorMtrlDiffuse, sizeof( D3DXCOLOR ) );
	g_effect->SetValue ( g_fTime, &ftime, sizeof(float));
	g_effect->SetMatrix( g_mWorldViewProjection, &g_matWorldViewProjection );
	g_effect->SetTexture( g_MeshTexture, g_texture );
	g_effect->SetTechnique( g_technique_model );
    g_effect->SetFloatArray(g_TexelSize,g_pTexelSize,2);

	g_pd3dDevice->SetFVF( D3DFVF_XYZ|D3DFVF_TEX1 );

//-------------------------------------------------------------------------------------
// Render
//-------------------------------------------------------------------------------------	
	unsigned int iPass, cPasses;

//-------------------------------------------------------------------------------------
// Draw Bg
//-------------------------------------------------------------------------------------	
	g_effect->SetTechnique( g_technique_bg );
	g_effect->SetTexture( g_MeshTexture, g_bg_texture );
	g_effect->Begin( &cPasses, 0 );
	g_pd3dDevice->SetVertexDeclaration( g_pVertexDecl );
	g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(TEXTURED) );
	for( iPass = 0; iPass < cPasses; iPass++ )
	{
		g_effect->BeginPass( iPass );
		g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, g_VerticesTextured, sizeof( TEXTURED ) );
		g_effect->EndPass();
	}
	g_effect->End();

//-------------------------------------------------------------------------------------
// Draw Game
//-------------------------------------------------------------------------------------	
	if(RenderEmulation==true)
	{
		switch(SelectedVertexFilter)
		{
			case FullScreen:
				g_effect->SetTechnique( g_technique_model_fullscreen );
				break;
			case TvScreen:
				g_effect->SetTechnique( g_technique_model_tv );
				break;
			default: 
				g_effect->SetTechnique( g_technique_model_tv );
				break;
		}
		
		g_effect->SetTexture( g_MeshTexture, g_texture );
		g_effect->Begin( &cPasses, 0 );
		for( iPass = 0; iPass < cPasses; iPass++ )
		{
			g_effect->BeginPass( iPass );
			mesh->DrawSubset(0);
			g_effect->EndPass();
		}
		g_effect->End();
	}

//-------------------------------------------------------------------------------------
// Ui
//-------------------------------------------------------------------------------------	
	//toujours
	RenderXui(g_pd3dDevice);

//-------------------------------------------------------------------------------------
// Affiche
//-------------------------------------------------------------------------------------	
	g_pd3dDevice->EndScene();
#ifdef _XBOX
	g_pd3dDevice->SetPredication( 0 );
	g_pd3dDevice->EndTiling( 0, NULL, m_pFrontBuffer, NULL, 1, 0, NULL );
    
    // Present the backbuffer contents to the display
    g_pd3dDevice->SynchronizeToPresentationInterval();
	g_pd3dDevice->Swap( m_pFrontBuffer, NULL );
#else
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
#endif
};

LARGE_INTEGER ts_old={0};
LARGE_INTEGER ts_new={0};
DOUBLE ms_sec={0};
#define TARGET_FPS 16666 //16.66 ms

void FrameLimitInit()
{
	LARGE_INTEGER ts_frequency={0};
	QueryPerformanceFrequency(&ts_frequency);
	QueryPerformanceCounter(&ts_old);
	QueryPerformanceCounter(&ts_new);
	ms_sec = ts_frequency.QuadPart* 0.000001;
}

void FrameLimit()
{
	QueryPerformanceCounter(&ts_new);
	while(((ts_new.QuadPart - ts_old.QuadPart)/ms_sec)<TARGET_FPS)
	{
		QueryPerformanceCounter(&ts_new);
		//Sleep(
	}
	ts_old = ts_new;
}
//to move


HRESULT Cemulator::Run()
{
	//FrameLimitInit();
//-------------------------------------------------------------------------------------
// Lance l'application
//-------------------------------------------------------------------------------------	
	if(FAILED( InitVideo()	))
	{
		printf("InitVideo failed\n");
		return E_FAIL;
	}
	if(FAILED( InitAudio()	))
	{
		printf("InitAudio failed\n");
		return E_FAIL;
	}
	if(FAILED( InitInput()	))
	{
		printf("InitInput failed\n");
		return E_FAIL;
	}
#if 0
	if(FAILED( InitSystem()	))
	{
		printf("InitSystem failed\n");
		return E_FAIL;
	}
#endif
	nesrom = (unsigned char *)memalign(32,1024*1024*4); // 4 MB should be plenty

	FCEUI_Initialize();

	RECT d3dr;

	d3dr.left=0;
	d3dr.top=0;
	d3dr.right=GetWidth();
	d3dr.bottom=GetHeight();

	D3DLOCKED_RECT texture_info;
	
	DWORD pitch;

	g_texture->LockRect( 0,  &texture_info, &d3dr, NULL );

	pitch = (DWORD) texture_info.Pitch;
	unsigned int * data = ((unsigned int*) texture_info.pBits);

	g_texture->UnlockRect(0);

	InitUi(g_pd3dDevice, g_d3dpp);

//-------------------------------------------------------------------------------------
// Looop :D
//-------------------------------------------------------------------------------------	
	if(true)
	{
		//move to update ...
		int32 * snd;
		int32 sndsize;

		unsigned int nesBitmap[256*240];
		while(end==false)
		{
			if(RenderEmulation == true)
			{

				FCEUI_Emulate(&bitmap, &snd, &sndsize, 0);
				for(int i = 0;i<(256*240);i++)
				{
					//Make an ARGB bitmap
					data[i] = ( (pcpalette[bitmap[i]].r) << 16) | ( (pcpalette[bitmap[i]].g) << 8 ) | ( pcpalette[bitmap[i]].b ) | ( 0xFF << 24 );
				}

				//applique un filtre sur l'image
				//hq3x_32(nesBitmap, data, 256, 240);

				UpdateAudio(snd, sndsize);
				UpdateInput();
			}
			
			UpdateVideo();
			Render();
		}
	}
	CloseSystem();
	CloseVideo();
	//CloseAudio();
	CloseInput();
	return S_OK;
};


HRESULT Cemulator::CloseVideo()
{
	g_texture->Release();
	g_pd3dDevice->Release();
	g_pD3D->Release();

	return S_OK;	
};
HRESULT Cemulator::CloseAudio()
{
	g_pMasteringVoice->DestroyVoice();
	g_pSourceVoice->DestroyVoice();
	free(&g_SoundBuffer);
	return S_OK;	
};

HRESULT Cemulator::CloseInput()
{
	return S_OK;	
};

HRESULT Cemulator::CloseSystem()
{
#if 0
	//save states
	save_state(AUTO_STATES);
	system_poweroff();
	//system_manage_sram(cart.sram,1,SRAM_SAVE);
	system_shutdown();
#endif
	return S_OK;	
};

//-------------------------------------------------------------------------------------
// ATG !!
//-------------------------------------------------------------------------------------	

//--------------------------------------------------------------------------------------
// Name: LoadFile()
// Desc: Helper function to load a file
//--------------------------------------------------------------------------------------
HRESULT LoadFile( const CHAR* strFileName, VOID** ppFileData, DWORD* pdwFileSize )
{
	//assert( ppFileData );
    if( pdwFileSize )
        *pdwFileSize = 0L;

    // Open the file for reading
    HANDLE hFile = CreateFileA( strFileName, GENERIC_READ, 0, NULL,
                               OPEN_EXISTING, 0, NULL );

    if( INVALID_HANDLE_VALUE == hFile )
        return E_HANDLE;

    DWORD dwFileSize = GetFileSize( hFile, NULL );
    VOID* pFileData = malloc( dwFileSize );

    if( NULL == pFileData )
    {
        CloseHandle( hFile );
        return E_OUTOFMEMORY;
    }

    DWORD dwBytesRead;
    if( !ReadFile( hFile, pFileData, dwFileSize, &dwBytesRead, NULL ) )
    {
        CloseHandle( hFile );
        free( pFileData );
        return E_FAIL;
    }

    // Finished reading file
    CloseHandle( hFile );

    if( dwBytesRead != dwFileSize )
    {
        free( pFileData );
        return E_FAIL;
    }

    if( pdwFileSize )
        *pdwFileSize = dwFileSize;
    *ppFileData = pFileData;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: UnloadFile()
// Desc: Matching unload
//--------------------------------------------------------------------------------------
VOID UnloadFile( VOID* pFileData )
{
	//assert( pFileData != NULL );
    free( pFileData );
}
