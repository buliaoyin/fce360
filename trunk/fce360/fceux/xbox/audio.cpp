/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/// \file
/// \brief Handles sound emulation using the SDL.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "xbox/fceusupport.h"
#include "audio.h"

/**
 * Initialize the audio subsystem.
 */
int
InitSound()
{
    int sound, soundrate, soundbufsize, soundvolume, soundtrianglevolume, soundsquare1volume, soundsquare2volume, soundnoisevolume, soundpcmvolume, soundq;

    // load configuration variables
	sound = 1;
    soundrate = 48000;
    soundbufsize = 128;
	soundvolume = 100;
    soundq = 1;
	soundtrianglevolume = 256;
	soundsquare1volume = 256;
	soundsquare2volume = 256;
	soundnoisevolume = 256;
	soundpcmvolume = 256;

    FCEUI_Sound(soundrate);
	FCEUI_SetSoundVolume(soundvolume);
	FCEUI_SetLowPass(1);
	//FCEUI_SetSoundQuality(soundq);
    FCEUI_SetTriangleVolume(soundtrianglevolume);
    FCEUI_SetSquare1Volume(soundsquare1volume);
    FCEUI_SetSquare2Volume(soundsquare2volume);
    FCEUI_SetNoiseVolume(soundnoisevolume);
    FCEUI_SetPCMVolume(soundpcmvolume);

    return(1);
}
void ResetAudio(){};