#!/bin/sh
cd library
DEBUG=0
TOOLS=../trunk/tools
ASM=$TOOLS/wp34s_asm.pl
LIB="$TOOLS/wp34s_lib.pl -d $DEBUG"
PP=$TOOLS/wp34s_pp.pl

DAT=wp34s-lib.dat

$LIB -pp matrix.wp34s matrixedit.wp34s vectors.wp34s digamma.wp34s invgamma.wp34s coordinates.wp34s modified-AGM.wp34s elliptic.wp34s -olib $DAT || exit
$LIB TVM.wp34s -ilib $DAT -olib $DAT || exit
$LIB -pp TRIGON.wp34s PF.wp34s -ilib $DAT -olib $DAT || exit

$LIB -cat -ilib $DAT >library.cat

cp $DAT ../trunk/windows/wp34sgui
cp $DAT ../trunk/realbuild

cd ../trunk/realbuild
cat calc.bin $DAT > calc_full.bin
cat calc_xtal.bin $DAT > calc_xtal_full.bin
cat calc_ir.bin $DAT > calc_ir_full.bin

