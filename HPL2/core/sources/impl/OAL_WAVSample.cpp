/**
	@file OAL_WAVSample.cpp
	@author Luis Rodero
	@date 2006-10-02
	@version 0.1
	Derived class for containing WAV Sample data 
*/

#include "impl/OAL_WAVSample.h"
#include "impl/OAL_Buffer.h"
#include "impl/OAL_Helper.h"
#include "impl/OAL_Device.h"
#include "impl/WAVLoader.h"
#include "system/LowLevelSystem.h"
#include "system/String.h"

//------------------------------------------------------------------

///////////////////////////////////////////////////////////
//	void Load ( const string &asFilename )
//	-	Loads sample data from a WAV file
///////////////////////////////////////////////////////////

//------------------------------------------------------------------

bool cOAL_WAVSample::CreateFromFile(const wstring &asFilename)
{
	DEF_FUNC_NAME("cOAL_WAVSample::Load()");
	FUNC_USES_AL;
	
	if(mbStatus==false)
		return false;

	Reset();

	ALenum	status;
	ALvoid	*pPCMBuffer = NULL;
	ALsizei	lSize;

	msFilename = asFilename;

	// Convert filename to 8-bit string for file I/O
	string sFilename = hpl::cString::To8Char(asFilename);

	// Load WAV file using custom loader
	status = WAV_LoadFile(sFilename.c_str(), &mFormat, &pPCMBuffer, &lSize, &mlFrequency);
	if (status != WAV_NO_ERROR)
	{
		mbStatus = false;
		return false;
	}

	cOAL_Buffer* pBuffer = mvBuffers.front();
	if(pBuffer->Feed(pPCMBuffer, lSize)==false)
	{
		mlBuffersUsed = 1;
		WAV_UnloadData(pPCMBuffer);
		mbStatus = false;
		return false;
	}

	RUN_AL_FUNC(alGetBufferi( pBuffer->GetObjectID(), AL_CHANNELS, &mlChannels ));
	
	mlSamples = lSize/(mlChannels*GetBytesPerSample());

	mfTotalTime = ((double)mlSamples)/mlFrequency;

	// Free the WAV data buffer
	WAV_UnloadData(pPCMBuffer);

	return true;
}

//------------------------------------------------------------------


