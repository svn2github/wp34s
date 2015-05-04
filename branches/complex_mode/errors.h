#ifndef __ERRORS_H__
#define __ERRORS_H__

#define	ERR_NONE		0
#define	ERR_DOMAIN		1
#define	ERR_BAD_DATE		2
#define	ERR_PROG_BAD		3
#define	ERR_INFINITY		4
#define	ERR_MINFINITY		5
#define	ERR_NO_LBL		6
#define	ERR_ILLEGAL		7
#define	ERR_RANGE		8
#define	ERR_DIGIT		9
#define	ERR_TOO_LONG		10
#define	ERR_RAM_FULL		11
#define	ERR_STK_CLASH		12
#define	ERR_BAD_MODE		13
#define	ERR_INT_SIZE		14
#define	ERR_MORE_POINTS		15
#define	ERR_BAD_PARAM		16
#define	ERR_IO			17
#define	ERR_INVALID		18
#define	ERR_READ_ONLY		19
#define	ERR_SOLVE		20
#define	ERR_MATRIX_DIM		21
#define	ERR_SINGULAR		22
#define	ERR_FLASH_FULL		23
#define ERR_NO_CRYSTAL		24

#ifdef SHIFT_EXPONENT

#  define	MSG_INTEGRATE		25

#  define	MAX_ERROR		25

#  if INTERRUPT_XROM_TICKS > 0
#    define	ERR_INTERRUPTED		26
#    define	MAX_MESSAGE		27
#  else
#    define	MAX_MESSAGE		26
#  endif

#else

#  define	ERR_TOO_SMALL		25
#  define	ERR_TOO_BIG		26

#  define	MSG_INTEGRATE		27

#  define	MAX_ERROR		27

#  if INTERRUPT_XROM_TICKS > 0
#    define	ERR_INTERRUPTED		28
#    define	MAX_MESSAGE		29
#  else
#    define	MAX_MESSAGE		28
#  endif

#endif

#endif
