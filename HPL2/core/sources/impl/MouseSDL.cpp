/*
 * Copyright © 2009-2020 Frictional Games
 * 
 * This file is part of Amnesia: The Dark Descent.
 * 
 * Amnesia: The Dark Descent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. 

 * Amnesia: The Dark Descent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "impl/MouseSDL.h"

#if USE_SDL3
#include "SDL3/SDL.h"
#else
#include "SDL3/SDL.h"
#endif

#if SDL_VERSION_ATLEAST(3, 0, 0)
#define HPL_MOUSE_EVENT_MOTION SDL_EVENT_MOUSE_MOTION
#define HPL_MOUSE_EVENT_BUTTON_DOWN SDL_EVENT_MOUSE_BUTTON_DOWN
#define HPL_MOUSE_EVENT_BUTTON_UP SDL_EVENT_MOUSE_BUTTON_UP
#define HPL_MOUSE_EVENT_WHEEL SDL_EVENT_MOUSE_WHEEL
#else
#define HPL_MOUSE_EVENT_MOTION SDL_MOUSEMOTION
#define HPL_MOUSE_EVENT_BUTTON_DOWN SDL_MOUSEBUTTONDOWN
#define HPL_MOUSE_EVENT_BUTTON_UP SDL_MOUSEBUTTONUP
#define HPL_MOUSE_EVENT_WHEEL SDL_MOUSEWHEEL
#endif

#include "graphics/LowLevelGraphics.h"
#include "impl/LowLevelInputSDL.h"
#include "math/Math.h"

#include "system/LowLevelSystem.h"
#include "system/Platform.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cMouseSDL::cMouseSDL(cLowLevelInputSDL *apLowLevelInputSDL) : iMouse("SDL Portable Mouse")
	{
		mvMButtonArray.resize(eMouseButton_LastEnum);
		mvMButtonArray.assign(mvMButtonArray.size(),false);

		mpLowLevelInputSDL = apLowLevelInputSDL;

		mvMouseRelPos = cVector2l(0,0);
		mvMouseAbsPos = cVector2l(0,0);

		mbWheelUpMoved = false;
		mbWheelDownMoved = false;

		mbFirstTime = true;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////
	
	//-----------------------------------------------------------------------

	void cMouseSDL::Update()
	{
		///////////////////////////////////////
		//On first update make sure to clear all previous events.
		if(mbFirstTime)
		{
			//Clear all relative movement
			for(int i=0; i<10; ++i)
			{
				SDL_PumpEvents();
#if SDL_VERSION_ATLEAST(3, 0, 0)
				float lX = 0.0f;
				float lY = 0.0f;
#else
				int lX = 0;
				int lY = 0;
#endif
				SDL_GetRelativeMouseState(&lX, &lY);
			}
			mbFirstTime = false;
		}
		
		iLowLevelGraphics *pLowLevelGfx = mpLowLevelInputSDL->GetLowLevelGraphics();
		//mvMouseRelPos = cVector2f(0,0);
		mbWheelUpMoved = false;
		mbWheelDownMoved = false;

		std::list<SDL_Event>::iterator it = mpLowLevelInputSDL->mlstEvents.begin();
		for(; it != mpLowLevelInputSDL->mlstEvents.end(); ++it)
		{
			SDL_Event *pEvent = &(*it);

			if (pEvent->type != HPL_MOUSE_EVENT_MOTION &&
				pEvent->type != HPL_MOUSE_EVENT_BUTTON_DOWN &&
#if SDL_VERSION_ATLEAST(2, 0, 0)
				pEvent->type != HPL_MOUSE_EVENT_WHEEL &&
#endif
				pEvent->type != HPL_MOUSE_EVENT_BUTTON_UP)
			{
				continue;
			}

			if (pEvent->type == HPL_MOUSE_EVENT_MOTION)
			{
#if SDL_VERSION_ATLEAST(3, 0, 0) && _WIN32
				/*if(pLowLevelGfx->GetFullscreenModeActive() == false)
				{
					/////////////
					// Only use abs position if not in fullscreen mode
					mvMouseAbsPos = cVector2l(pEvent->motion.x,pEvent->motion.y);
				}*/
				mvMouseAbsPos = cVector2l(pEvent->motion.x,pEvent->motion.y);
#else
				mvMouseAbsPos = cVector2l(pEvent->motion.x,pEvent->motion.y);
#endif
				
				Uint8 buttonState = pEvent->motion.state;

				//Set button here as well just to be sure
				/*if(buttonState & SDL_BUTTON(1)) mvMButtonArray[eMouseButton_Left] = true;
				if(buttonState & SDL_BUTTON(2)) mvMButtonArray[eMouseButton_Middle] = true;
				if(buttonState & SDL_BUTTON(3)) mvMButtonArray[eMouseButton_Right] = true;*/
			}
#if SDL_VERSION_ATLEAST(2, 0, 0)
            else if(pEvent->type == SDL_MOUSEWHEEL)
            {
                if (pEvent->wheel.y > 0) {
                    mvMButtonArray[eMouseButton_WheelUp] = true;
                    mbWheelUpMoved = true;
                } else {
                    mvMButtonArray[eMouseButton_WheelDown] = true;
                    mbWheelDownMoved = true;
                }
                break;
            }
#endif
			else
			{
				bool bButtonIsDown = pEvent->type==SDL_MOUSEBUTTONDOWN;

				//if(pEvent->button.button == SDL_BUTTON_WHEELUP)Log(" Wheel %d!\n",bButtonIsDown);

				switch(pEvent->button.button)
				{
					case SDL_BUTTON_LEFT: mvMButtonArray[eMouseButton_Left] = bButtonIsDown;break;
					case SDL_BUTTON_MIDDLE: mvMButtonArray[eMouseButton_Middle] = bButtonIsDown;break;
					case SDL_BUTTON_RIGHT: mvMButtonArray[eMouseButton_Right] = bButtonIsDown;break;
					case SDL_BUTTON_X1: mvMButtonArray[eMouseButton_Button6] = bButtonIsDown;break;
					case SDL_BUTTON_X2: mvMButtonArray[eMouseButton_Button7] = bButtonIsDown;break;
#if !SDL_VERSION_ATLEAST(2, 0, 0)
					case SDL_BUTTON_WHEELUP: 
						mvMButtonArray[eMouseButton_WheelUp] = bButtonIsDown;
						if(bButtonIsDown) mbWheelUpMoved = true;
						break;
					case SDL_BUTTON_WHEELDOWN: 
						mvMButtonArray[eMouseButton_WheelDown] = bButtonIsDown;
						if(bButtonIsDown) mbWheelDownMoved = true;
						break;
#endif
				}
			}
		}

		if(mbWheelDownMoved)	mvMButtonArray[eMouseButton_WheelDown] = true;
		else					mvMButtonArray[eMouseButton_WheelDown] = false;
		if(mbWheelUpMoved)		mvMButtonArray[eMouseButton_WheelUp] = true;
		else					mvMButtonArray[eMouseButton_WheelUp] = false;
		
		int lX,lY; 
		SDL_GetRelativeMouseState(&lX, &lY);
		mvMouseRelPos = cVector2l(lX,lY);

		
	}
	
	//-----------------------------------------------------------------------
	
	bool cMouseSDL::ButtonIsDown(eMouseButton mButton)
	{
		return mvMButtonArray[mButton];
	}

	//-----------------------------------------------------------------------

	cVector2l cMouseSDL::GetAbsPosition()
	{
		return mvMouseAbsPos;
	}
	
	//-----------------------------------------------------------------------

	cVector2l cMouseSDL::GetRelPosition()
	{
		cVector2l vPos = mvMouseRelPos;
		mvMouseRelPos = cVector2l(0,0);
		
		return vPos;
	}

	//-----------------------------------------------------------------------
	
	/////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	/////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	
	//-----------------------------------------------------------------------

}
