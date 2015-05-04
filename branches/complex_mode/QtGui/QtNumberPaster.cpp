// Next method, struct & defines are copied from system.h & HP20b_cDLG.cpp hence the copyright
// Some reformatting and renaming done though

/*
Copyright (c) 2007 Hewlett-Packard development company llc
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the disclaimer below.
//
//  - Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the disclaimer below in the documentation and/or
//  other materials provided with the distribution. Unless a specific license is granted
//  to te licencee
//
//  - This software can only be used on a HP hardware. You are not permited to leaverage
//  any part of this software for products/project that do not work on hardware sold by HP
//
//  HP's name may not be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
//  DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY HP "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
//  DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
//  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  ----------------------------------------------------------------------------
Original file created by Cyrille de Brebisson
*/

#include "QtNumberPaster.h"

#define KEYf 9
#define KEYSHIFT KEYf
#define KEYg 10
#define KEYh 11
#define KEYENTER 12
#define KEYINPUT KEYENTER
#define KEYCHS 14
#define KEYPLUSMOINS KEYCHS
#define KEYEEX 15
#define KEYBACK 16
#define KEY7 19
#define KEY8 20
#define KEY9 21
#define KEYDIV 22
#define KEYUP 24
#define KEY4 25
#define KEY5 26
#define KEY6 27
#define KEYMUL 28
#define KEYDOWN 30
#define KEY1 31
#define KEY2 32
#define KEY3 33
#define KEYMINUS 34
#define KEYEXIT 36
#define KEYON KEYEXIT
#define KEY0 37
#define KEYDOT 38
#define KEYPLUS 40

#define KEYSHIFTPLAN 42

struct
{
  QChar c;
  unsigned int  keys;
}
const keydefs[] =
{
  { '0', KEY0 },
  { '1', KEY1 },
  { '2', KEY2 },
  { '3', KEY3 },
  { '4', KEY4 },
  { '5', KEY5 },
  { '6', KEY6 },
  { '7', KEY7 },
  { '8', KEY8 },
  { '9', KEY9 },
  { 'e', KEYEEX },
  { 'E', KEYEEX },
  { '-', KEYCHS },
  { '.', KEYDOT },
  { ',', KEYDOT },
  { ' ', - 1 },
  { 0, 0}
};

void QtNumberPaster::paste(const QString& aString, QtKeyboard& aKeyboard)
{
	QString toPaste=aString.trimmed();
	if (!toPaste.isEmpty())
	{
		bool pushSign=false;

		for (int i = 0; i < toPaste.length(); i++)
		{
			QChar currentChar = toPaste.at(i);
			int j = 0;

			while (keydefs[j].c != 0 && keydefs[j].c != currentChar)
			{
				j++;
			}

			if (keydefs[j].c == currentChar)
			{
				int keys = keydefs[j].keys;

				while (keys > 0)
				{
					if ((keys & 0xff) > KEYSHIFTPLAN)
					{
						keys = keys - KEYSHIFTPLAN;
						aKeyboard.putKey(KEYSHIFT);
					}
					if ((keys & 0xff) == KEYCHS && i == 0)
					{
						pushSign = true;
					}
					else
					{
						aKeyboard.putKey(keys & 0xff);
						if (pushSign)
						{
							pushSign = false;
							aKeyboard.putKey(KEYCHS);
						}
					}
					keys >>= 8;
				}
			}
		}
	}
}


