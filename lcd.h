#ifndef _LCD_H
#define _LCD_H

/*

	DMG LCD screen:
		- 160 x 144 dots
		- 8kB or 64kb LCD Display RAM
		- Character data $8000 - $97FF
		- BG Display data 1 $9800 - $9BFF
		- BG Display data 2 $9C00 - $9FFF
		- Grayscale :
			- BG & Window : 4 shades, 1 palette
			- Objects : 3 shades, 2 palettes
		- Display data for object characters is stored in OAM ($FE00 - $FE9F) as follows:
			- y-axis
			- x-axis
			- character code
			- attribute data

*/

#define LCD_WIDTH (160)
#define LCD_HEIGHT (144)


#endif