#include "Mode7Screen.h"
#include "../BBCEmulator.h"
#include "Util/GraphicsView.h"

#include "ttxfont.h"

const int kNoSixels = 64;
unsigned short g_Sixels[kNoSixels][20];
unsigned short g_SixelsSeparated[kNoSixels][20];

const uint16_t kSixelPatterns[] = { 0x0000,0xff00,0x00ff,0xffff };
const uint16_t kSixelPatternsSeparated[] = { 0x0000,0x3c00,0x003c,0x3c3c };

const uint32_t kTeletextColours[] =
{
	0xff000000,	// Black
	0xff0000ff,	// Red
	0xff00ff00,	// Green
	0xff00ffff,	// Yellow
	0xffff0000,	// Blue
	0xffff00ff,	// Magenta
	0xffffff00,	// Cyan
	0xffffffff,	// White
};

void InitMode7ScreenViewer(FBBCEmulator* pEmu)
{
	// Set up the graphics characters 'sixels' for mode 7
	for (int i = 0; i < kNoSixels; i++)
	{
		// continuous
		{
			const uint16_t topRow = kSixelPatterns[i & 0x3];
			const uint16_t middleRow = kSixelPatterns[(i >> 2) & 0x3];
			const uint16_t bottomRow = kSixelPatterns[(i >> 4) & 0x3];

			for (int j = 0; j < 20; j++)	// each line
			{
				if (j < 7)
					g_Sixels[i][j] = topRow;		// 7 pixels
				else if (j < 13)
					g_Sixels[i][j] = middleRow;		// 6 pixels
				else
					g_Sixels[i][j] = bottomRow;		// 7 pixels
			}
		}

		// separated
		{
			const uint16_t topRow = kSixelPatternsSeparated[i & 0x3];
			const uint16_t middleRow = kSixelPatternsSeparated[(i >> 2) & 0x3];
			const uint16_t bottomRow = kSixelPatternsSeparated[(i >> 4) & 0x3];

			for (int j = 0; j < 20; j++)	// each line
			{
				if (j > 1 && j < 5)
					g_SixelsSeparated[i][j] = topRow;		// 7 pixels
				else if (j > 8 && j < 11)
					g_SixelsSeparated[i][j] = middleRow;		// 6 pixels
				else if (j > 16 && j < 19)
					g_SixelsSeparated[i][j] = bottomRow;		// 7 pixels
			}
		}
	}

}

void DrawMode7ScreenToGraphicsView(FBBCEmulator* pBBCEmu, FGraphicsView* pScreenView)
{
	const bbc_t& bbc = pBBCEmu->GetBBC();
	int widthChars = 40;
	int heightChars = 25;

	int characterHeight = 20;
	int characterWidth = 16;

	int screenWidth = widthChars * characterWidth;
	int screenHeight = heightChars * characterHeight;
	const mc6845_t& crtc = bbc.crtc;
	const uint16_t displayAddress = (crtc.start_addr_hi << 8) | crtc.start_addr_lo;

	uint32_t backgroundColour = 0xff000000;
	uint32_t foregroundColour = 0xffffffff;
	uint32_t graphicsColour = 0xffffffff;

	uint16_t currentScreenAddress = displayAddress;//(bbc.crtc.ma & 0x800) << 3 | 0x3C00 | (bbc.crtc.ma & 0x3FF);

	bool bSetBackgroundColour = false;
	bool bGraphicsMode = false;
	bool bGraphicsSeparated = false;
	bool bDoubleHeight = false;
	bool bDoubleWidth = false;
	bool bDoubleHeightSecondRow = false;

	for (int y = 0; y < heightChars; y++)
	{
		// reset parameters for each line
		backgroundColour = 0xff000000;
		foregroundColour = 0xffffffff;
		graphicsColour = 0xffffffff;
		bSetBackgroundColour = false;
		bGraphicsMode = false;
		bGraphicsSeparated = false;
		if (bDoubleHeight && bDoubleHeightSecondRow == false)
			bDoubleHeightSecondRow = true;
		else
			bDoubleHeightSecondRow = false;

		bDoubleHeight = false;
		bDoubleWidth = false;

		for (int x = 0; x < widthChars; x++)
		{
			const uint16_t ttxScreenAddress = (currentScreenAddress & 0x800) << 3 | 0x3C00 | (currentScreenAddress & 0x3FF);
			const int charCode = pBBCEmu->ReadByte(ttxScreenAddress);
			int drawChar = 0;	// space as default
			currentScreenAddress++;

			if (charCode < 0x20)
			{
				// TODO: process control character
			}
			else if (charCode < 128)
			{
				drawChar = charCode - 0x20;
			}
			else if (charCode < 136) // 128-135 : Set colour	
			{
				if(bSetBackgroundColour)
				{
					backgroundColour = kTeletextColours[charCode - 128];
					bSetBackgroundColour = false;
				}
				else
				{
					foregroundColour = kTeletextColours[charCode - 128];
				}
			}
			else if (charCode == 136) //136	flash
			{

			}
			else if (charCode == 137) //137	steady
			{
			}
			else if (charCode == 140) //140	normal height
			{
				bDoubleHeight = false;
			}
			else if (charCode == 141) //141	double height
			{
				bDoubleHeight = true;
			}
			else if (charCode > 144 && charCode < 152)
			{
				graphicsColour = kTeletextColours[charCode - 144];
				bGraphicsMode = true;
			}
			else if (charCode == 156) //156	black background
			{
				backgroundColour = 0xff000000;
			}
			else if (charCode == 157) //157	new background
			{
				bSetBackgroundColour = true;
			}

			// Draw the visible character
			const uint16_t* pChar = mode7font[drawChar];

			for (int l = 0; l < characterHeight; l++)
			{
				uint32_t* pCurPixBufAddr = pScreenView->GetPixelBuffer() + (screenWidth * (y * 20 + l)) + (x * 16);
				uint16_t characterLine = bDoubleHeight ? pChar[(l >> 1) + ((bDoubleHeightSecondRow == true) ? 10 : 0)] : pChar[l];
				for (int c = 0; c < characterWidth; c++)
				{
					const uint32_t pixelColour = characterLine & 0x8000 ? foregroundColour : backgroundColour;
					*pCurPixBufAddr = pixelColour;
					pCurPixBufAddr++;
					characterLine <<= 1;
				}
			}
			
		}
	}
}