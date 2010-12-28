// config.cpp : définit le point d'entrée pour l'application console.
#include "config_reader.h"
#include <iostream>

using namespace std;

Config fcecfg;

void ReadConfig()
{
	/*
	*/
	//SetDefault Value overwritten during loading
	fcecfg.Set("sound","enabled", 1);
	fcecfg.Set("sound","rate", 48000);
	fcecfg.Set("sound","bufsize", 128);
	fcecfg.Set("sound","volume", 100);
	fcecfg.Set("sound","trianglevolume", 256);
	fcecfg.Set("sound","square1volume", 256);
	fcecfg.Set("sound","square2volume", 256);
	fcecfg.Set("sound","noisevolume", 256);
	fcecfg.Set("sound","pcmvolume", 256);

	fcecfg.Set("video","region","NTSC"); //not used
	fcecfg.Set("video","swfilter", 1);
	fcecfg.Set("video","screenaspect", 1);
	
	//load the ini file
	fcecfg.Load("game:\\fceui.ini");
	fcecfg.Save("game:\\fceui.ini");
	
}

