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
file created by Cyrille de Brebisson
modified by Marcus von Cube
*/
#ifdef __cplusplus
extern "C" {
#else
#define bool int
#define true 1
#define false 0
#endif

#ifdef WIN32
#ifdef _WINDLL
#define EXPORT __declspec(dllexport)
extern unsigned int *LcdData;
#else
#ifdef WINGUI
#define EXPORT __declspec(dllimport)
#else
#define EXPORT
#endif
extern unsigned int LcdData[20];
#endif
#define __noinit
#define __arm
#undef AT91C_SLCDC_MEM
#define AT91C_SLCDC_MEM LcdData
#define Lcd_Enable()
void EXPORT UpdateDlgScreen(bool force);
void EXPORT WindowsBlink(bool a);
void EXPORT WindowsSwapBuffers();
void EXPORT AddKey(int k,bool ifnotfull);
#else
#define UpdateScreenContent()
#define WindowsBlink(a)
#define WindowsSwapBuffers()
//#define UpdateDlgScreen()
#define UpdateDlgScreen(a)
#endif

#define makeindic(bit,seg) (bit<<4)+seg
#define BIG_EQUAL	  makeindic(39,1)
#define DOWN_ARROW	  makeindic(39,0)
#define INPUT		  makeindic(39,9)
#define SMALL_EQUAL	  makeindic(39,8)
#define BAT		  makeindic(39,7)
#define BEG		  makeindic(39,6)
#define STO		  makeindic(39,5)
#define RCL		  makeindic(39,4)
#define RAD		  makeindic(39,3)
#define THREE_SIXTY       makeindic(39,2)
#define RPN		  makeindic(38,2)
//#undef makeindic

#include "types.h"
/**! \fn DispGraphic(unsigned long long int *grob)
 *  @ingroup lcddriver
 * \brief display the 43*6 gaphic pointed by grob on the 43*6 matrix on the screen
 **/
__arm void DispGraphic(u64 const *grob);
__arm void GetGraphic(u64 *grob);
/**! \fn DispChars(char const *s, int offset)
 *  @ingroup lcddriver
 * \brief display the text message pointed by s on the matrix area of the screen. the first chr is offseted to the left by offset pixcels
 * \if loop is true, then display the message in a loop (ie, if it reaches the end of the text before the end of the screen, it adds a space and restarts from the begining of the text)
 * \ returns the number of times that the function had to go though the whole message in loop mode...
 *  \if loop is set to false, returns the number of pixcel that it took to display the text...
 **/

extern __arm int DispChars(char const *s, int offset, bool loop);
void DispNumber(char const *s);
void DispOnNumber(int chrNumber, char character, int coma); // disp character at poisition chrNumber, coma=0 for no coma, 1 for . and 2 for ,

/**! \fn DispIndicator(unsigned long long indic, bool on)
 *  @ingroup lcddriver
 * \brief turn given indicator on or off
 **/
void DispIndicator(unsigned int indic, bool on);
bool isIndicatorOn(unsigned int indic);

#ifdef __cplusplus
}
#endif
