/* This program is free software: you can redistribute it and/or modify
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "keyboard.h"

//---------------------------------------------------------------------------------
void ChangeKeyboardType(void)
{
	u16 value;
	s16 charset;
	u8 blocksize;
	
	PA_Keyboard_Struct.Type = !PA_Keyboard_Struct.Type;
	value = _REG16(REG_BGCNT(0, PA_Keyboard_Struct.Bg));
	value &= ~(0x1F << SCREEN_SHIFT);
	
	blocksize = PA_BgInfo[0][PA_Keyboard_Struct.Bg].mapsize;
	charset = PA_BgInfo[0][PA_Keyboard_Struct.Bg].mapchar + (PA_Keyboard_Struct.Type * blocksize);	
	
	value |= (charset << SCREEN_SHIFT);
	_REG16(REG_BGCNT(0, PA_Keyboard_Struct.Bg)) = value;
}
//---------------------------------------------------------------------------------
char CheckKeyboard(void) 
{
	s16 x = Stylus.X;
	s16 y = Stylus.Y;  

	x -= PA_Keyboard_Struct.ScrollX + 8;
	y -= PA_Keyboard_Struct.ScrollY + 8;
		
	if ((x >= 0) && (x < 192) && (y >= 0) && (y < 80)) {		
		y = y >> 4;  
		x = x >> 3;		
		if (Stylus.Newpress) {
			PA_Keyboard_Struct.oldX = x;
			PA_Keyboard_Struct.oldY = y;
			PA_Keyboard_Struct.Repeat = 50;
			
			if (PA_Keyboard[PA_Keyboard_Struct.Type][y][x] == PA_CAPS) {
				ChangeKeyboardType();
			}	
			if (PA_Keyboard_Struct.Letter == PA_SHIFT) {
				// Si on rappuye sur Shift ca le vire sans rien faire
				if(PA_Keyboard[PA_Keyboard_Struct.Type][y][x] == PA_SHIFT) PA_Keyboard_Struct.Letter = 0;
				else PA_Keyboard_Struct.Letter = PA_Keyboard[PA_Keyboard_Struct.Type][y][x];
				
				PA_SetLetterPal(0, 3, 15); // On efface le shift
				ChangeKeyboardType();				
			}
			else {
				PA_Keyboard_Struct.Letter = PA_Keyboard[PA_Keyboard_Struct.Type][y][x];
				if (PA_Keyboard_Struct.Letter == PA_SHIFT) ChangeKeyboardType();
			}
			PA_SetLetterPal(x, y, 14);	
			
			return (PA_Keyboard_Struct.Letter);  // Renvoie la valeur dans le clavier...	si nouvelle pression
		}
		else {
			if (Stylus.Held && (PA_Keyboard[PA_Keyboard_Struct.Type][y][x] == PA_Keyboard_Struct.Letter)) {
				--PA_Keyboard_Struct.Repeat;
				if (PA_Keyboard_Struct.Repeat == 0) {
					PA_Keyboard_Struct.Repeat = 10;
					return(PA_Keyboard_Struct.Letter);
				}
			}
		}	
	}
	if (!Stylus.Held) {
		if (PA_Keyboard_Struct.Letter != PA_SHIFT) 
			PA_SetLetterPal(PA_Keyboard_Struct.oldX, PA_Keyboard_Struct.oldY, 15);		
	}
	return 0;
}