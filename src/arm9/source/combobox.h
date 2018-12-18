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

#include "windows.h"
#include "listbox.h"

#ifndef __COMBOBOXH__
#define __COMBOBOXH__

#define CB_ERR				LB_ERR

#define CBS_DROPDOWN		0x100000

#define CB_RESETCONTENT		LB_RESETCONTENT
#define CB_ADDSTRING		LB_ADDSTRING
#define CB_GETCOUNT			LB_GETCOUNT
#define CB_SETCURSEL		LB_SETCURSEL
#define CB_GETCURSEL		LB_GETCURSEL
#define CB_GETLBTEXT		LB_GETITEM

#define CBN_SELENDOK		1

extern int create_combobox_win(unsigned char screen,int x,int y,int width,int height,int style);

#endif 