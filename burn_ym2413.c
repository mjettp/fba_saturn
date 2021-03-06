#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym2413.h"

void (*BurnYM2413Render)(short* pSoundBuf, int nSegmentLength);

//unsigned char BurnYM2151Registers[0x0100];
//unsigned int nBurnCurrentYM2151Register;

/*static*/ int nBurnYM2413SoundRate;

/*static*/ short* pBuffer;
/*static*/ short* pYM2413Buffer[2];

/*static*/ int nBurnPosition;
/*static*/ unsigned int nSampleSize;
/*static*/ unsigned int nFractionalPosition;
/*static*/ unsigned int nSamplesRendered;

/*static*/ int nYM2413Volume;

/*static*/ void YM2413RenderResample(short* pSoundBuf, int nSegmentLength)
{
	unsigned int i;
	nBurnPosition += nSegmentLength;

	if (nBurnPosition >= nBurnSoundRate) {
		nBurnPosition = nSegmentLength;

		pYM2413Buffer[0][1] = pYM2413Buffer[0][(nFractionalPosition >> 16) - 3];
		pYM2413Buffer[0][2] = pYM2413Buffer[0][(nFractionalPosition >> 16) - 2];
		pYM2413Buffer[0][3] = pYM2413Buffer[0][(nFractionalPosition >> 16) - 1];

		pYM2413Buffer[1][1] = pYM2413Buffer[1][(nFractionalPosition >> 16) - 3];
		pYM2413Buffer[1][2] = pYM2413Buffer[1][(nFractionalPosition >> 16) - 2];
		pYM2413Buffer[1][3] = pYM2413Buffer[1][(nFractionalPosition >> 16) - 1];

		nSamplesRendered -= (nFractionalPosition >> 16) - 4;

		for (i = 0; i <= nSamplesRendered; i++) {
			pYM2413Buffer[0][4 + i] = pYM2413Buffer[0][(nFractionalPosition >> 16) + i];
			pYM2413Buffer[1][4 + i] = pYM2413Buffer[1][(nFractionalPosition >> 16) + i];
		}

		nFractionalPosition &= 0x0000FFFF;
		nFractionalPosition |= 4 << 16;
	}

	pYM2413Buffer[0] = pBuffer + 4 + nSamplesRendered;
	pYM2413Buffer[1] = pBuffer + 4 + nSamplesRendered + 65536;

	YM2413UpdateOne(0, pYM2413Buffer, (unsigned int)(nBurnPosition + 1) * nBurnYM2413SoundRate / nBurnSoundRate - nSamplesRendered);
	nSamplesRendered += (unsigned int)(nBurnPosition + 1) * nBurnYM2413SoundRate / nBurnSoundRate - nSamplesRendered;

	pYM2413Buffer[0] = pBuffer;
	pYM2413Buffer[1] = pBuffer + 65536;

	nSegmentLength <<= 1;

	for (i = 0; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {

		// Left channel
		pSoundBuf[i + 0] = INTERPOLATE4PS_CUSTOM((nFractionalPosition >> 4) & 0x0FFF,
												 pYM2413Buffer[0][(nFractionalPosition >> 16) - 3],
												 pYM2413Buffer[0][(nFractionalPosition >> 16) - 2],
												 pYM2413Buffer[0][(nFractionalPosition >> 16) - 1],
												 pYM2413Buffer[0][(nFractionalPosition >> 16) - 0],
												 nYM2413Volume);

		// Right channel
		pSoundBuf[i + 1] = INTERPOLATE4PS_CUSTOM((nFractionalPosition >> 4) & 0x0FFF,
												 pYM2413Buffer[1][(nFractionalPosition >> 16) - 3],
												 pYM2413Buffer[1][(nFractionalPosition >> 16) - 2],
												 pYM2413Buffer[1][(nFractionalPosition >> 16) - 1],
												 pYM2413Buffer[1][(nFractionalPosition >> 16) - 0],
												 nYM2413Volume);
	}
}

/*static*/ void YM2413RenderNormal(short* pSoundBuf, int nSegmentLength)
{
	int n;
	nBurnPosition += nSegmentLength;

	pYM2413Buffer[0] = pBuffer;
	pYM2413Buffer[1] = pBuffer + nSegmentLength;

	YM2413UpdateOne(0, pYM2413Buffer, nSegmentLength);

/*	if (bBurnUseMMX) {
		BurnSoundCopy_FM_A(pYM2413Buffer[0], pYM2413Buffer[1], pSoundBuf, nSegmentLength, nYM2413Volume, nYM2413Volume);
	} else */
	{
		for (n = 0; n < nSegmentLength; n++) {
			pSoundBuf[(n << 1) + 0] = (pYM2413Buffer[0][n] * nYM2413Volume) >> 12;
			pSoundBuf[(n << 1) + 1] = (pYM2413Buffer[1][n] * nYM2413Volume) >> 12;
		}
	}
}

void BurnYM2413Reset()
{
	YM2413ResetChip(0);
}

void BurnYM2413Exit()
{
	YM2413Shutdown();

	free(pBuffer);
}

int BurnYM2413Init(int nClockFrequency, float nVolume)
{
	if (nBurnSoundRate <= 0) {
		YM2413Init(1, nClockFrequency, 11025);
		return 0;
	}
/*
	if (nFMInterpolation == 3) {
		// Set YM2413 core samplerate to match the hardware
		nBurnYM2413SoundRate = nClockFrequency >> 6;
		// Bring YM2413 core samplerate within usable range
		while (nBurnYM2413SoundRate > nBurnSoundRate * 3) {
			nBurnYM2413SoundRate >>= 1;
		}

		BurnYM2413Render = YM2413RenderResample;

		nYM2413Volume = (int)((double)16384.0 * 100.0 / nVolume);
	} else 
	*/
	{
		nBurnYM2413SoundRate = nBurnSoundRate;
		BurnYM2413Render = YM2413RenderNormal;

		nYM2413Volume = (int)((double)65536.0 * 100.0 / nVolume);
	}

	YM2413Init(1, nClockFrequency, nBurnYM2413SoundRate);

	pBuffer = (short*)BurnMalloc(65536 * 2 * sizeof(short));
	memset(pBuffer, 0, 65536 * 2 * sizeof(short));

	nSampleSize = (unsigned int)nBurnYM2413SoundRate * (1 << 16) / nBurnSoundRate;
	nFractionalPosition = 4 << 16;
	nSamplesRendered = 0;
	nBurnPosition = 0;

	return 0;
}

void BurnYM2413IncreaseVolume(int nFactor)
{
	nYM2413Volume *= nFactor;
}
