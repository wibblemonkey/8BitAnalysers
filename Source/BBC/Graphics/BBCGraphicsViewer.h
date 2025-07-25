#pragma once

#include "CodeAnalyser/UI/GraphicsViewer.h"

class FBBCEmulator;

class FBBCGraphicsViewer : public FGraphicsViewer
{
	friend class FC64ScreenAccessGrid;
public:
	FBBCGraphicsViewer(FBBCEmulator* pEmu);
	bool	Init() override;
	void	Shutdown() override;
	void	DrawUI() override;

	void	DrawScreenViewer() override;

	uint32_t GetRGBValueForPixel(int colourIndex, uint32_t heatMapCol) const;
	void	UpdateScreenPixelImage();
	void	UpdateScreenTeletextImage();

	void	UpdateGraphicsScreen1bpp();
	void	UpdateGraphicsScreen2bpp();
	void	UpdateGraphicsScreen4bpp();

	uint16_t WrapAddress(uint16_t addr) const;

private:
	//FGraphicsView* ScreenView = nullptr;
	FBBCEmulator* pBBCEmu = nullptr;


	int			DisplayAddress = 0x0000;
	int			WidthChars = 40;
	int			HeightChars = 25;
	int			CharacterHeight = 8;
	int 		CharacterWidth = 8;

	uint32_t*	pPaletteColours = nullptr;
	bool		bShowReadsWrites = false;
};