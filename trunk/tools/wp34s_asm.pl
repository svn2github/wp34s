#!/usr/bin/perl -w
#-----------------------------------------------------------------------
#
#  This file is part of 34S.
#
#  34S is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  34S is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with 34S.  If not, see <http://www.gnu.org/licenses/>.
#
#-----------------------------------------------------------------------
#
my $Description = "Assembler/Disassembler for the WP34S calculator.";
#
#-----------------------------------------------------------------------
#
# Language:         Perl script
#
my $SVN_Filename          =  '$HeadURL:  $';
my $SVN_Current_Revision  =  '$Revision: $';
my $SVN_Last_Updated      =  '$Date:  $';
#
#-----------------------------------------------------------------------

use strict;

# ---------------------------------------------------------------------

my $debug = 0;
my $quiet = 1;
my (%mnem2hex, %hex2mnem);
my @files = ();

my $outfile = "-";

my $FLASH_LENGTH = 512;
my $DEFAULT_FLASH_BLANK_INSTR = "ERR 03";
my $user_flash_blank_fill = "";

my $INTERNAL_OPCODE_MAP = "--internal table--";
my $DEFAULT_OPCODE_MAP_FILE = $INTERNAL_OPCODE_MAP;
my $opcode_map_file = $DEFAULT_OPCODE_MAP_FILE;

# Number of '*' to prepend to labels in disassembly -- just makes them easier to find.
my $DEFAULT_STAR_LABELS = 0;
my $star_labels = $DEFAULT_STAR_LABELS;

my $MAX_FLASH_WORDS = 506;

my $MAGIC_MARKER = 0xA53C;
my $CRC_INITIALIZER = 0x5aa5;

my $ASSEMBLE_MODE = "assemble";
my $DISASSEMBLE_MODE = "disassemble";
my $DEFAULT_MODE = $ASSEMBLE_MODE;
my $mode = $DEFAULT_MODE;

# These are used to convert all the registers into numeric offsets.
#
# There are 2 groups of named register offsets; the 112 group (XYZTABCDLIJK) and the 104
# group (ABCD). We have to extract the opcode's numeric value based on the op-type. The value of
# max will give us this (112,104,100,etc.). Note that for all values of max<=100, either group is
# valid since these are purely numeric.
my @reg_offset_112 = (0 .. 99, "X", "Y", "Z", "T", "A", "B", "C", "D", "L", "I", "J", "K");
my $MAX_INDIRECT_112 = 112;
my @reg_offset_104 = (0 .. 99, "A", "B", "C", "D");
my $INDIRECT_FLAG = 0x80;


# ---------------------------------------------------------------------

my $script_executable = $0;
my $script_name       = "";
if( $script_executable =~ /[\.\/]*(\w+\.pl)/ ) {
  $script_name     = "$1";
}

my $script  = <<SCRIPT;
$script_name  - $Description
SCRIPT

my $usage = <<EOM;

$script
Usage:
   $script_name src_file [src_file2 [src_file3]] -o out_binary  # assembly mode
   $script_name in_binary -dis > src_file                       # disassembly mode

Parameters:
   src_file         One or more WP34S program source files.
   -o out_binary    Flash image produced by assembler. Required in assembler mode.
   -dis             Disassemble the 'dat' file.
   -opcode file     Opcode file to parse.                       [Default: $DEFAULT_OPCODE_MAP_FILE]
   -fill fill_hex   Hex value to prefill flash with.            [Default: instruction '$DEFAULT_FLASH_BLANK_INSTR']
   -s stars         Stars to prepend to labels in disassembly.  [Default: $DEFAULT_STAR_LABELS]
   -h               This help script.

Examples:
  \$ $script_name great_circle.wp34s -o wp34s-3.dat
  - Assembles the named WP34S program source file producing a flash image for the WP34S.

  \$ $script_name great_circle.wp34s floating_point.wp34s -o wp34s-1.dat -fill FFFF
  - Assembles multiple WP34S program source files into a single flash image for the WP34S.
    Uses FFFF as the optional fill value.

  \$ $script_name -dis wp34s-1.dat -s 3
  - Disassembles a flash image from the WP34S. Prepend 3 stars to the front to each label to
    make then easier to find in the listing (they are ignored during assembly).

  \$ $script_name -dis wp34s-0.dat > test.wp34s ; $script_name test.wp34s -o wp34s-0a.dat
  \$ diff wp34s-0.dat wp34s-0a.dat
  - An end-to-end test of the tool. Note that the blank fill mode will have to be the same
    for the binaries to match.

Notes:
  1) Line numbers can be used in the source file but they are ignored.
  2) You can name a different opcode table using -opcode. This can be used to translate a source
     written for a different SVN revison of the WP34S to move it to a modern version. Disassembler
     the old flash using the old opcode table and reassemble using the default (internal) table.
     This is also an insurance policy against the opcodes evolving as well. Simply target newer
     opcode tables as they become available. To generate an opcode table, using the following:

      \$ cd ./trunk
      \$ make
      \$ ./Linux/calc opcodes > opcodes.map
      \$ $script_name -dis wp34s-0.dat -opcode opcodes.map > source.wp34s
EOM

#######################################################################

get_options();
load_opcode_tables($opcode_map_file);

my $flash_blank_fill_hex_str = ($user_flash_blank_fill) ? $user_flash_blank_fill : $mnem2hex{$DEFAULT_FLASH_BLANK_INSTR};

if( $mode eq $ASSEMBLE_MODE ) {
  assemble( $outfile, @files );
} elsif( $mode eq $DISASSEMBLE_MODE ) {
  disassemble( $outfile, @files );
} else {
}


#######################################################################
#
# Assemble the source files
#
sub assemble {
  my $outfile = shift;
  my @files = @_;
  local $_;
  my @words = pre_fill_flash($flash_blank_fill_hex_str); # A hex string.
  my $next_free_word = 1;

  foreach my $file (@files) {
    open SRC, $file or die "ERROR: Cannot open source '$file' for reading: $!\n";
    while(<SRC>) {
      # Remove any comments ('#' or '//') and any blank lines.
      next if /^\s*#/;
      next if /^\s*$/;
      next if m<^\s*//>;
      chomp; chomp;

      my $text_flag;
      ($_, $text_flag) = assemble_special_handling($_);
      print "\n" if 0; # Possible breakpoint

      if( not exists $mnem2hex{$_} ) {
        die "ERROR: Cannot recognize mnemonic at line $. of '$file': $_\n";
      } else {
        if( $text_flag ) {
          my @chars = split "", $text_flag;
          $words[++$next_free_word] = hex2dec($mnem2hex{$_}) | ord($chars[0]);
          if( $chars[1] and $chars[2] ) {
            $words[++$next_free_word] = ord($chars[1]) | (ord($chars[2]) << 8);
          } elsif( $chars[1] ) {
            $words[++$next_free_word] = ord($chars[1]);
          } else {
            $words[++$next_free_word] = 0;
          }
        } else {
          $words[++$next_free_word] = hex2dec($mnem2hex{$_});
        }
      }
      if( $next_free_word >= $MAX_FLASH_WORDS ) {
        die "ERROR: Too many program steps encounterd.\n";
      }
      print "\n" if 0; # Possible breakpoint
    }
  }
  $words[1] = $next_free_word;
  $words[0] = calc_crc16( @words[2 .. $next_free_word] );

  write_binary( $outfile, @words );

  return;
} # assemble


#######################################################################
#
# Disassemble the source files
#
sub disassemble {
  my $outfile = shift;
  my @files = @_;
  local $_;
  my (@img);

  my $text_lo_limit = hex2dec($mnem2hex{"LBL'"});
  my $text_hi_limit = hex2dec($mnem2hex{"INT'"}) + 256;

  open OUT, "> $outfile" or die "ERROR: Cannot open file '$outfile' for writing: $!\n";

  foreach my $file (@files) {
    open DAT, $file or die "ERROR: Cannot open dat file '$file' for reading: $!\n";
    local $/;
    my @words = unpack("S*", <DAT>);
    my $len = $words[1] - 1;
    for( my $k = 0; $k < $len; $k++ ) {
      my $word = $words[$k+2];
      # Check if we are in the 2 words opcode range: LBL' to INT'
      if( ($word >= $text_lo_limit) and ($word < $text_hi_limit) ) {
        my $base_op = $word & 0xFF00;
        my (@chars);
        $chars[0] = $word & 0xFF;
        $word = $words[$k+3];
        ($chars[2], $chars[1]) = (($word >> 8), ($word & 0xFF));
        print_disassemble_text($k, $base_op, @chars);
        $k++;
      } else {
        print_disassemble_normal($k, $word);
      }
      print "\n" if 0; # breakpoint
    }
    close DAT;
  }
  close OUT;
  return;
} # disassemble


#######################################################################
#
#
#
sub print_disassemble_normal {
  my $idx = shift;
  my $word = shift;
  my $hex_str = lc dec2hex($word);
  if( not exists $hex2mnem{$hex_str} ) {
    die "ERROR: Opcode '$hex_str' does not exist at line ", $idx+1, " at print_disassemble_normal\n";
  }
  my $label_flag = (($hex2mnem{$hex_str} =~ /LBL/) and $star_labels) ? "*" x $star_labels : "";
  if( $debug ) {
    printf OUT "%03d /* %04s */ %0s%0s\n", $idx+1, $hex_str, $label_flag, $hex2mnem{$hex_str};
  } else {
    printf OUT "%03d %0s%0s\n", $idx+1, $label_flag, $hex2mnem{$hex_str};
  }
  return;
} # print_disassemble_normal


#######################################################################
#
#
#
sub print_disassemble_text {
  my $idx = shift;
  my $word = shift;
  my @chars = @_;
  my $hex_str = lc dec2hex($word);
  if( not exists $hex2mnem{$hex_str} ) {
    die "ERROR: Opcode '$hex_str' does not exist at line ", $idx+1, " at print_disassemble_text\n";
  }
  my $label_flag = (($hex2mnem{$hex_str} =~ /LBL/) and $star_labels) ? "*" x $star_labels : "";
  if( $debug ) {
    printf OUT "%03d /* %04s */ %0s%0s", $idx+1, $hex_str, $label_flag, $hex2mnem{$hex_str};
  } else {
    printf OUT "%03d %0s%0s", $idx+1, $label_flag, $hex2mnem{$hex_str};
  }
  foreach (@chars) {
    print OUT chr($_) if $_;
  } print OUT "\n";
  return;
} # print_disassemble_normal


#######################################################################
#
# From: http://www.eagleairaust.com.au/code/crc16.htm
#
sub calc_crc16 {
  my @byteArray = wordArray2byteArray(@_);
  my $crc = $CRC_INITIALIZER;
  foreach (@byteArray) {
    $crc = (($crc >> 8) & 0xFFFF) | (($crc << 8) & 0xFFFF);
    $crc ^= $_;
    $crc ^= ($crc & 0xFF) >> 4;
    $crc ^= ($crc << 12) & 0xFFFF;
    $crc ^= (($crc & 0xFF) << 5) & 0xFFFF;
  }
# Until we fix this calculation, this might do the "universal" trick.
#$crc = $MAGIC_MARKER;
  return $crc;
} # calc_crc16


#######################################################################
#
#
#
sub wordArray2byteArray {
  my @wordArray = @_;
  my (@byteArray);
  local $_;
  foreach (@wordArray) {
    push @byteArray, ($_ & 0xFF); # Low byte
    push @byteArray, (($_ >> 8) & 0xFF); # High byte
  }
  return @byteArray;
} # wordArray2byteArray


#######################################################################
#
#
#
sub write_binary {
  my $outfile = shift;
  my @words = @_;
  open OUT, "> $outfile" or die "ERROR: Cannot open file '$outfile' for writing: $!\n";
  foreach (@words) {
    my $bin_lo = $_ & 0xFF;
    my $bin_hi = $_ >> 8;
    print OUT chr($bin_lo), chr($bin_hi);
  }
  close OUT;
  return;
} # write_binary


#######################################################################
#
# Handle any lines that need special work. If the opcode is in
# the range "LBL'" through "INT'" it needs to have the extra
# label character masked off and a flag set to non-0 to indicate
# that the opcode is this type. The $text_flag is set to a non-0
# label characters if this is the case.
#
sub assemble_special_handling {
  my $line = shift;
  my $text_flag = 0;

  $line =~ s/^\d{3}\s+//; # Remove any line numbers.
  $line =~ s/^\s+//;      # Remove leading whitespace

  # Labels sometimes are displayed with and asterisk -- presumably
  # to make them easier to locate.
  $line =~ s/^\*+LBL/LBL/;

  if( $line =~ /\S+\'(\S+)/ ) {
    $text_flag = $1;
    $line =~ s/\'\S+/\'/;
  }

  return ($line, $text_flag);
} # assemble_special_handling


#######################################################################
#
# Prefill an array with an empty flash image.
#
sub pre_fill_flash {
  my $fill_value = shift;
  my $start = 0;
  my $length = $FLASH_LENGTH;
  my (@flash);
  for(my $k = 0; $k < $FLASH_LENGTH; $k++ ) {
    $flash[$k] = hex2dec($fill_value);
  }

  # Overwrite the 'special' locations.
  $flash[0] = $MAGIC_MARKER;
  $flash[1] = 1;
#  $flash[-1] = 0;
#  $flash[-2] = 0;
#  $flash[-3] = 0;
#  $flash[-4] = 0;
  return @flash;
} # pre_fill_flash


#######################################################################
#
# Load the opcode tables.
#
sub load_opcode_tables {
  my $file = shift;
  if( $opcode_map_file ne $INTERNAL_OPCODE_MAP ) {
    open DATA, $file or die "ERROR: Cannot open opcode map file '$opcode_map_file' for reading: $!\n";
  }
  while(<DATA>) {
    # Remove any comments ('#' or '//') and any blank lines.
    next if /^\s*#/;
    next if /^\s*$/;
    next if m<^\s*//>;
    chomp; chomp;

    # From about WP34S SVN 1000 on, the console can spit out a "compressed" opcode map
    # when run using: "./trunk/Linux/calc opcodes > opcode.map"
    #
    # From Paul Dale (:
    #
    # Fields are tab separated.
    #
    # The first field is the numeric op-code in hex.
    #
    # The second field is the type of op-code. cmd is a standard command. mult is a
    # multi-character alpha command (double length). arg is a command that takes an
    # argument.
    #
    # The third field is the op-code itself.
    #
    # For argument commands there is a fourth field that can contain up to four pieces
    # of information comma separated:
    #
    # 'max=nnn' nnn is the maximum value + 1 permitted as an argument. I.e. legal arguments
    # are 0 through nnn-1 inclusive.
    #
    # 'indirect' is present if the command is permitted an indirect argument. If it is, the
    # argument can of course be any register including the stack (0 - 111 inclusive).
    #
    # 'stack' is present if a stack register is allowed as an argument.
    #
    # 'complex' is present if the argument specifies a complex pair of registers. This
    # basically restricts numeric registers to be 0 - 98 instead of 0 - 99 and it restricts
    # lettered registers to be X, Z, A, C, L and J.
    #

    # cmd-type line
    if( /0x([0-9a-fA-F]{4})\s+cmd\s+(.+)$/ ) {
      my $hex_str = $1;
      my $mnemonic = $2;
      my $line_num = $.;
      load_table_entry($hex_str, $mnemonic, $line_num);

    # arg-type line
    } elsif( /0x([0-9a-fA-F]{4})\s+arg\s+(.+)$/ ) {
      my $hex_str = $1;
      my $parameter = $2;
      my $line_num = $.;
      parse_arg_type($hex_str, $parameter, $line_num);

    # mult-type line
    } elsif( /0x([0-9a-fA-F]{4})\s+mult\s+(.+)$/ ) {
      my $hex_str = $1;
      my $mnemonic = "${2}'"; # append a single quote.
      my $line_num = $.;
      load_table_entry($hex_str, $mnemonic, $line_num);

    } else {
      die "ERROR: Cannot parse opcode table line $.: '$_'\n";
    }
  }
  close DATA;
  return;
} # load_opcode_tables

#######################################################################
#
# Parse the arg-type
#
# For argument commands there is a fourth field that can contain up to four pieces
# of information comma separated:
#
# 'max=nnn' nnn is the maximum value + 1 permitted as an argument. I.e. legal arguments
# are 0 through nnn-1 inclusive.
#
# 'indirect' is present if the command is permitted an indirect argument. If it is, the
# argument can of course be any register including the stack (0 - 111 inclusive).
#
# 'stack' is present if a stack register is allowed as an argument.
#
# 'complex' is present if the argument specifies a complex pair of registers. This
# basically restricts numeric registers to be 0 - 98 instead of 0 - 99 and it restricts
# lettered registers to be X, Z, A, C, L and J.
#
sub parse_arg_type {
  my $base_hex_str = shift;
  my $arg = shift;
  my $line_num = shift;

  my ($base_mnemonic);
  my $direct_max = 0;
  my $indirect_modifier = 0;
  my $stack_modifier = 0;
  my $complex_modifier = 0;
  
  if( $arg =~ /(\S+)\s+/ ) {
    $base_mnemonic = $1;
  } else {
    die "ERROR: Cannot parse base mnemonic from arg-type line $line_num: '$arg'\n";
  }

  if( $arg =~ /max=(\d+)/ ) {
    $direct_max = $1;
  } else {
    die "ERROR: Cannot parse max parameter from arg-type line $line_num: '$arg'\n";
  }

  $indirect_modifier = 1 if $arg =~ /indirect/;
  $stack_modifier    = 1 if $arg =~ /stack/;
  $complex_modifier  = 1 if $arg =~ /complex/;

  # Load the direct argument variants
  if( $direct_max ) {
    for my $offset (0 .. ($direct_max-1)) {
      my ($reg_str);
      # There are 2 groups of named register offsets; the 112 group (XYZTABCDLIJK) and the 104
      # group (ABCD). We have to extract the opcode's numeric offset based on the op-type. The value of
      # max will give us this (112,104,100,etc.). Note that for all values of max<=100, either group is
      # valid since these are purely numeric.
      if( $direct_max == $MAX_INDIRECT_112 ) {
        $reg_str = $reg_offset_112[$offset];
      } else {
        $reg_str = $reg_offset_104[$offset];
      }
      # "Correct" the format if it is in the numeric range [0-99].
      $reg_str = sprintf("%02d", $offset) if $offset < 100;
      my ($hex_str, $mnemonic) = construct_offset_mnemonic($base_hex_str, $offset, "$base_mnemonic ", $reg_str);
      load_table_entry($hex_str, $mnemonic, $line_num);
    }
  }

  # Load the indirect argument variants. These always comes from the 112 set.
  if( $indirect_modifier ) {
    for my $offset (0 .. ($MAX_INDIRECT_112 - 1)) {
      # "Correct" the format if it is in the numeric range [0-99].
      my $reg_str = ($offset < 100) ? sprintf("%02d", $offset) : $reg_offset_112[$offset];
      my $indirect_offset = $offset + $INDIRECT_FLAG;
      my ($hex_str, $mnemonic) = construct_offset_mnemonic($base_hex_str, $indirect_offset, "${base_mnemonic}[->]", $reg_str);
      load_table_entry($hex_str, $mnemonic, $line_num);
    }
  }

  return;
} # parse_arg_type

#######################################################################
#
# Build an entry pair similar to the older format which includes the opcode (in a hex
# string) offset by the correct amount and the reconstructed mnemonic name.
#
sub construct_offset_mnemonic {
  my $base_hex_str = shift;
  my $offset = shift; # Decimal
  my $mnemonic = shift;
  my $reg_name_str = shift;
  return (dec2hex(hex2dec($base_hex_str) + $offset), "${mnemonic}${reg_name_str}");
} # construct_offset_mnemonic


#######################################################################
#
# Load the translation hash tables with opcode (as a hex string) and the mnemonic.
#
sub load_table_entry {
  my $op_hex_str = shift;
  my $mnemonic = shift;
  my $line_num = shift;

  # No idea why but chomp does not seem to work on these guys so I rolled my own.
  $mnemonic = myChomp($mnemonic);
#  chomp $mnemonic;
#  chomp $mnemonic;

  # Load the mnemonic to hex table
  if( not exists $mnem2hex{$mnemonic} ) {
    $mnem2hex{$mnemonic} = lc $op_hex_str;
  } else {
    my @cc = split "", $mnemonic;
    warn "# WARNING: Duplicate mnemonic: '$mnemonic' at line $line_num (new definition: '$op_hex_str', previous definition: '${mnem2hex{$mnemonic}}')\n" unless $quiet;
  }

  # Load the hex to mnemonic table
  if( not exists $hex2mnem{$op_hex_str} ) {
    $hex2mnem{lc $op_hex_str} = $mnemonic;
  } else {
    warn "# WARNING: Duplicate opcode hex: '$op_hex_str' at line $line_num (new defintion: '$mnemonic', previous definition: '${hex2mnem{$op_hex_str}}')\n" unless $quiet;
  }
  return;
} # load_table_entry


#######################################################################
#
# No idea why but chomp does not seem to work on some input so I rolled my own.
# Ugly but it works (late at night)!
#
sub myChomp {
  my $str = shift;
  my @cc = split "", $str;
  $str = "";
  local $_;
  foreach (@cc) {
    if( ($_ ne chr(0x0D)) and ($_ ne chr(0x0A)) ) {
      $str .= $_;
    }
  }
  return $str;
} # myChomp


#######################################################################
#
# Convert a decimal number to 4-digit hex string
#
sub dec2hex {
  my $dec = shift;
  my $hex_str = sprintf "%04X", $dec;
  return ( $hex_str );
} # dec2hex

sub d2h {
  return dec2hex(@_);
} # d2h

#######################################################################
#
# Convert a hex string to a decimal number
#
sub hex2dec {
  my $hex_str = shift;
  my $dec = hex("0x" . $hex_str);
  return ( $dec );
} # hex2dec

sub h2d {
  return hex2dec(@_);
} # h2d


#######################################################################
#
# Debug stubs
#
sub combien {
  my $ref_hash = shift;
  my $count = 0;
  local $_;
  foreach (keys %{$ref_hash} ) {
    $count++;
  }
  print "The hash has $count keys\n";
  return;
} # combien

sub many {
  return(combien(shift));
} # many


#######################################################################
#######################################################################
#
# Process the command line option list.
#
sub get_options {
  my ($arg);
  while ($arg = shift(@ARGV)) {

    # See if help is asked for
    if( $arg eq '-h' ) {
      print "$usage\n";
      die "\n";
    }

    elsif( ($arg eq '--version') or ($arg eq '-V') ) {
      print "$script\n";
      if( $SVN_Current_Revision =~ /Revision: (.+)\s*\$/ ) {
        print "Version: $1\n";
        if( $SVN_Last_Updated =~ /Date: (.+)\s*\$/ ) {
          print "Date:    $1\n";
        }
      }
      die "\n";
    }

    elsif( ($arg eq '-opcodes') or  ($arg eq '-map') or  ($arg eq '-op') ) {
      $opcode_map_file = shift(@ARGV);
      print "# NOTE: Opcode map file: $opcode_map_file\n";
    }

    elsif( $arg eq '-dis' ) {
      $mode = $DISASSEMBLE_MODE;
    }

    elsif( $arg eq '-d' ) {
      $debug = shift(@ARGV);
    }

    elsif( $arg eq '-v' ) {
      $quiet = 0;
    }

    elsif( ($arg eq '-s') or ($arg eq '-stars') ) {
      $star_labels = shift(@ARGV);
    }

    elsif( ($arg eq '-f') or ($arg eq '-fill') ) {
      $user_flash_blank_fill = shift(@ARGV);
      $user_flash_blank_fill =~ s/^0x//;
    }

    elsif( $arg eq '-o' ) {
      $outfile = shift(@ARGV);
    }

    else {
      push @files, $arg;
    }
  }

  #----------------------------------------------
  if( ($mode eq $ASSEMBLE_MODE) and ($outfile eq "-") ) {
    warn "ERROR: Must enter an output file name in assembler mode.\n";
    die  "       Enter '$script_name -h' for help.\n";
  }

  if( $user_flash_blank_fill ne "" and $user_flash_blank_fill =~ /[0-9a-fA-F]{,4}/ ) {
    warn "ERROR: Must a valid hex number for flash blank file.\n";
    die  "       Enter '$script_name -h' for help.\n";
  }

  return;
} # get_options

__DATA__
# Generated by "./trunk/Linux/calc opcodes" from SVN 1119
# "Adjusted" to have the iC data split into 2 types of encoding.
0x0000  cmd ENTER[^]
0x0001  cmd CLx
0x0002  cmd EEX
0x0003  cmd +/-
0x0004  cmd .
0x0005  cmd 0
0x0006  cmd 1
0x0007  cmd 2
0x0008  cmd 3
0x0009  cmd 4
0x000a  cmd 5
0x000b  cmd 6
0x000c  cmd 7
0x000d  cmd 8
0x000e  cmd 9
0x000f  cmd A
0x0010  cmd B
0x0011  cmd C
0x0012  cmd D
0x0013  cmd E
0x0014  cmd F
0x0015  cmd [SIGMA]+
0x0016  cmd [SIGMA]-
0x0017  cmd x=0?
0x0018  cmd x[!=]0?
0x0019  cmd x[approx]0?
0x001a  cmd x<0?
0x001b  cmd x[<=]0?
0x001c  cmd x>0?
0x001d  cmd x[>=]0?
0x001e  cmd x=1?
0x001f  cmd x[!=]1?
0x0020  cmd x[approx]1?
0x0021  cmd x<1?
0x0022  cmd x[<=]1?
0x0023  cmd x>1?
0x0024  cmd x[>=]1?
0x0025  cmd [cmplx]x=0?
0x0026  cmd [cmplx]x[!=]0?
0x0100  cmd NOP
0x0101  cmd VERS
0x0102  cmd OFF
0x0103  cmd SSIZE?
0x0104  cmd SSIZE4
0x0105  cmd SSIZE8
0x0106  cmd WSIZE?
0x0107  cmd x[<->]y
0x0108  cmd [cmplx]x[<->]y
0x0109  cmd R[v]
0x010a  cmd R[^]
0x010b  cmd [cmplx]R[v]
0x010c  cmd [cmplx]R[^]
0x010d  cmd [cmplx]ENTER
0x010e  cmd FILL
0x010f  cmd [cmplx]FILL
0x0110  cmd DROP
0x0111  cmd [cmplx]DROP
0x0112  cmd [SIGMA]x
0x0113  cmd [SIGMA]y
0x0114  cmd [SIGMA]x[^2]
0x0115  cmd [SIGMA]y[^2]
0x0116  cmd [SIGMA]xy
0x0117  cmd [SIGMA]x[^2]y
0x0118  cmd n[SIGMA]
0x0119  cmd [SIGMA]lnx
0x011a  cmd [SIGMA]ln[^2]x
0x011b  cmd [SIGMA]lny
0x011c  cmd [SIGMA]ln[^2]y
0x011d  cmd [SIGMA]lnxy
0x011e  cmd [SIGMA]xlny
0x011f  cmd [SIGMA]ylnx
0x0120  cmd s
0x0121  cmd [sigma]
0x0122  cmd [epsilon]
0x0123  cmd [epsilon][sub-p]
0x0124  cmd sw
0x0125  cmd [sigma]w
0x0126  cmd [x-bar]
0x0127  cmd [x-bar]w
0x0128  cmd [x-bar]g
0x0129  cmd CORR
0x012a  cmd LR
0x012b  cmd SERR
0x012c  cmd [epsilon]m
0x012d  cmd SERRw
0x012e  cmd COV
0x012f  cmd sxy
0x0130  cmd ExpF
0x0131  cmd LinF
0x0132  cmd LogF
0x0133  cmd PowerF
0x0134  cmd BestF
0x0135  cmd RAN#
0x0136  cmd SEED
0x0137  cmd DEG
0x0138  cmd RAD
0x0139  cmd GRAD
0x013a  cmd RTN
0x013b  cmd RTN+1
0x013c  cmd STOP
0x013d  cmd PROMPT
0x013e  cmd CL[SIGMA]
0x013f  cmd CLREG
0x0140  cmd CLx
0x0141  cmd CLSTK
0x0142  cmd CLALL
0x0143  cmd RESET
0x0144  cmd CLFLAG
0x0145  cmd [->]POL
0x0146  cmd [->]REC
0x0147  cmd DENMAX
0x0148  cmd DECOMP
0x0149  cmd DENFIX
0x014a  cmd DENFAC
0x014b  cmd DENANY
0x014c  cmd IMPFRC
0x014d  cmd PROFRC
0x014e  cmd RDX.
0x014f  cmd RDX,
0x0150  cmd E3ON
0x0151  cmd E3OFF
0x0152  cmd SCIOVR
0x0153  cmd ENGOVR
0x0154  cmd 2COMPL
0x0155  cmd 1COMPL
0x0156  cmd UNSIGN
0x0157  cmd SIGNMT
0x0158  cmd DECM
0x0159  cmd H.MS
0x015a  cmd FRACT
0x015b  cmd LZON
0x015c  cmd LZOFF
0x015d  cmd LJ
0x015e  cmd RJ
0x015f  cmd DBL[times]
0x0160  cmd SUM
0x0161  cmd Y.MD
0x0162  cmd D.MY
0x0163  cmd M.DY
0x0164  cmd JG1752
0x0165  cmd JG1582
0x0166  cmd LEAP?
0x0167  cmd [alpha]DAY
0x0168  cmd [alpha]MONTH
0x0169  cmd [alpha]DATE
0x016a  cmd [alpha]TIME
0x016b  cmd DATE
0x016c  cmd TIME
0x016d  cmd 24H
0x016e  cmd 12H
0x016f  cmd SETDAT
0x0170  cmd SETTIM
0x0171  cmd CL[alpha]
0x0172  cmd [alpha]VIEW
0x0173  cmd [alpha]LENG
0x0174  cmd [alpha][->]x
0x0175  cmd x[->][alpha]
0x0176  cmd [alpha]ON
0x0177  cmd [alpha]OFF
0x0178  cmd R-COPY
0x0179  cmd R-SWAP
0x017a  cmd R-CLR
0x017b  cmd R-SORT
0x017c  cmd RCLM
0x017d  cmd STOM
0x017e  cmd XEQUSR
0x017f  cmd [infinity]?
0x0180  cmd NaN?
0x0181  cmd SPEC?
0x0182  cmd PRIME?
0x0183  cmd INT?
0x0184  cmd FP?
0x0185  cmd EVEN?
0x0186  cmd ODD?
0x0187  cmd ENTRY?
0x0188  cmd TICKS
0x0189  cmd BATT
0x018a  cmd SETEUR
0x018b  cmd SETUK
0x018c  cmd SETUSA
0x018d  cmd SETIND
0x018e  cmd SETCHN
0x018f  cmd SLVQ
0x0190  cmd XEQ[alpha]
0x0191  cmd GTO[alpha]
0x0192  cmd RCFRG
0x0193  cmd RCFST
0x0194  cmd SAVE
0x0195  cmd LOAD
0x0200  cmd FP
0x0201  cmd FLOOR
0x0202  cmd CEIL
0x0203  cmd ROUNDI
0x0204  cmd IP
0x0205  cmd ABS
0x0206  cmd ROUND
0x0207  cmd SIGN
0x0208  cmd LN
0x0209  cmd e[^x]
0x020a  cmd [sqrt]
0x020b  cmd 1/x
0x020c  cmd (-1)[^x]
0x020d  cmd LOG[sub-1][sub-0]
0x020e  cmd LOG[sub-2]
0x020f  cmd 2[^x]
0x0210  cmd 10[^x]
0x0211  cmd LN1+x
0x0212  cmd e[^x]-1
0x0213  cmd W
0x0214  cmd W[^-1]
0x0215  cmd x[^2]
0x0216  cmd CUBE
0x0217  cmd CUBERT
0x0218  cmd FIB
0x0219  cmd [->]DEG
0x021a  cmd [->]RAD
0x021b  cmd [->]GRAD
0x021c  cmd DEG[->]
0x021d  cmd RAD[->]
0x021e  cmd GRAD[->]
0x021f  cmd SIN
0x0220  cmd COS
0x0221  cmd TAN
0x0222  cmd ASIN
0x0223  cmd ACOS
0x0224  cmd ATAN
0x0225  cmd SINC
0x0226  cmd SINH
0x0227  cmd COSH
0x0228  cmd TANH
0x0229  cmd ASINH
0x022a  cmd ACOSH
0x022b  cmd ATANH
0x022c  cmd x!
0x022d  cmd [GAMMA]
0x022e  cmd LN[GAMMA]
0x022f  cmd [degree][->]rad
0x0230  cmd rad[->][degree]
0x0231  cmd [degree][->]G
0x0232  cmd G[->][degree]
0x0233  cmd rad[->]G
0x0234  cmd G[->]rad
0x0235  cmd +/-
0x0236  cmd CONJ
0x0237  cmd erf
0x0238  cmd erfc
0x0239  cmd [phi](x)
0x023a  cmd [PHI](x)
0x023b  cmd [PHI][^-1](p)
0x023c  cmd [chi][^2][sub-p]
0x023d  cmd [chi][^2]
0x023e  cmd [chi][^2]INV
0x023f  cmd t[sub-p](x)
0x0240  cmd t(x)
0x0241  cmd t[^-1](p)
0x0242  cmd F[sub-p](x)
0x0243  cmd F(x)
0x0244  cmd F[^-1](p)
0x0245  cmd Weibl[sub-p]
0x0246  cmd Weibl
0x0247  cmd Weibl[^-1]
0x0248  cmd Expon[sub-p]
0x0249  cmd Expon
0x024a  cmd Expon[^-1]
0x024b  cmd Binom[sub-p]
0x024c  cmd Binom
0x024d  cmd Binom[^-1]
0x024e  cmd Poiss[sub-p]
0x024f  cmd Poiss
0x0250  cmd Poiss[^-1]
0x0251  cmd Geom[sub-p]
0x0252  cmd Geom
0x0253  cmd Geom[^-1]
0x0254  cmd Norml[sub-p]
0x0255  cmd Norml
0x0256  cmd Norml[^-1]
0x0257  cmd LgNrm[sub-p]
0x0258  cmd LgNrm
0x0259  cmd LgNrm[^-1]
0x025a  cmd Logis[sub-p]
0x025b  cmd Logis
0x025c  cmd Logis[^-1]
0x025d  cmd Cauch[sub-p]
0x025e  cmd Cauch
0x025f  cmd Cauch[^-1]
0x0260  cmd [x-hat]
0x0261  cmd [y-hat]
0x0262  cmd %[SIGMA]
0x0263  cmd %
0x0264  cmd [DELTA]%
0x0265  cmd %T
0x0266  cmd [->]HR
0x0267  cmd [->]H.MS
0x0268  cmd NOT
0x0269  cmd nBITS
0x026a  cmd MIRROR
0x026b  cmd DAY
0x026c  cmd D[->]J
0x026d  cmd J[->]D
0x026e  cmd [degree]C[->][degree]F
0x026f  cmd [degree]F[->][degree]C
0x0270  cmd dB[->]ar.
0x0271  cmd ar.[->]dB
0x0272  cmd dB[->]pr.
0x0273  cmd pr.[->]dB
0x0274  cmd [zeta]
0x0275  cmd B[sub-n]
0x0276  cmd B[sub-n]*
0x0300  cmd y[^x]
0x0301  cmd +
0x0302  cmd -
0x0303  cmd [times]
0x0304  cmd /
0x0305  cmd RMDR
0x0306  cmd LOGx
0x0307  cmd MIN
0x0308  cmd MAX
0x0309  cmd ANGLE
0x030a  cmd [beta]
0x030b  cmd LN[beta]
0x030c  cmd I[GAMMA]
0x030d  cmd COMB
0x030e  cmd PERM
0x030f  cmd %+MG
0x0310  cmd %MG
0x0311  cmd ||
0x0312  cmd AGM
0x0313  cmd H.MS+
0x0314  cmd H.MS-
0x0315  cmd GCD
0x0316  cmd LCM
0x0317  cmd AND
0x0318  cmd OR
0x0319  cmd XOR
0x031a  cmd NAND
0x031b  cmd NOR
0x031c  cmd XNOR
0x031d  cmd DAYS+
0x031e  cmd [DELTA]DAYS
0x031f  cmd P[sub-n]
0x0320  cmd T[sub-n]
0x0321  cmd U[sub-n]
0x0322  cmd L[sub-n]
0x0323  cmd H[sub-n]
0x0324  cmd H[sub-n][sub-p]
0x0400  cmd I[beta]
0x0401  cmd DBL/
0x0402  cmd DBLR
0x0403  cmd %MRR
0x0404  cmd L[sub-n][alpha]
0x0500  cmd FP
0x0501  cmd FLOOR
0x0502  cmd CEIL
0x0503  cmd ROUNDI
0x0504  cmd IP
0x0505  cmd ABS
0x0506  cmd ROUND
0x0507  cmd SIGN
0x0508  cmd LN
0x0509  cmd e[^x]
0x050a  cmd [sqrt]
0x050b  cmd 1/x
0x050c  cmd (-1)[^x]
0x050d  cmd LOG[sub-1][sub-0]
0x050e  cmd LOG[sub-2]
0x050f  cmd 2[^x]
0x0510  cmd 10[^x]
0x0511  cmd LN1+x
0x0512  cmd e[^x]-1
0x0513  cmd W
0x0514  cmd W[^-1]
0x0515  cmd x[^2]
0x0516  cmd CUBE
0x0517  cmd CUBERT
0x0518  cmd FIB
0x0519  cmd [->]DEG
0x051a  cmd [->]RAD
0x051b  cmd [->]GRAD
0x051c  cmd DEG[->]
0x051d  cmd RAD[->]
0x051e  cmd GRAD[->]
0x051f  cmd SIN
0x0520  cmd COS
0x0521  cmd TAN
0x0522  cmd ASIN
0x0523  cmd ACOS
0x0524  cmd ATAN
0x0525  cmd SINC
0x0526  cmd SINH
0x0527  cmd COSH
0x0528  cmd TANH
0x0529  cmd ASINH
0x052a  cmd ACOSH
0x052b  cmd ATANH
0x052c  cmd x!
0x052d  cmd [GAMMA]
0x052e  cmd LN[GAMMA]
0x052f  cmd [degree][->]rad
0x0530  cmd rad[->][degree]
0x0531  cmd [degree][->]G
0x0532  cmd G[->][degree]
0x0533  cmd rad[->]G
0x0534  cmd G[->]rad
0x0535  cmd +/-
0x0536  cmd CONJ
0x0537  cmd erf
0x0538  cmd erfc
0x0539  cmd [phi](x)
0x053a  cmd [PHI](x)
0x053b  cmd [PHI][^-1](p)
0x053c  cmd [chi][^2][sub-p]
0x053d  cmd [chi][^2]
0x053e  cmd [chi][^2]INV
0x053f  cmd t[sub-p](x)
0x0540  cmd t(x)
0x0541  cmd t[^-1](p)
0x0542  cmd F[sub-p](x)
0x0543  cmd F(x)
0x0544  cmd F[^-1](p)
0x0545  cmd Weibl[sub-p]
0x0546  cmd Weibl
0x0547  cmd Weibl[^-1]
0x0548  cmd Expon[sub-p]
0x0549  cmd Expon
0x054a  cmd Expon[^-1]
0x054b  cmd Binom[sub-p]
0x054c  cmd Binom
0x054d  cmd Binom[^-1]
0x054e  cmd Poiss[sub-p]
0x054f  cmd Poiss
0x0550  cmd Poiss[^-1]
0x0551  cmd Geom[sub-p]
0x0552  cmd Geom
0x0553  cmd Geom[^-1]
0x0554  cmd Norml[sub-p]
0x0555  cmd Norml
0x0556  cmd Norml[^-1]
0x0557  cmd LgNrm[sub-p]
0x0558  cmd LgNrm
0x0559  cmd LgNrm[^-1]
0x055a  cmd Logis[sub-p]
0x055b  cmd Logis
0x055c  cmd Logis[^-1]
0x055d  cmd Cauch[sub-p]
0x055e  cmd Cauch
0x055f  cmd Cauch[^-1]
0x0560  cmd [x-hat]
0x0561  cmd [y-hat]
0x0562  cmd %[SIGMA]
0x0563  cmd %
0x0564  cmd [DELTA]%
0x0565  cmd %T
0x0566  cmd [->]HR
0x0567  cmd [->]H.MS
0x0568  cmd NOT
0x0569  cmd nBITS
0x056a  cmd MIRROR
0x056b  cmd DAY
0x056c  cmd D[->]J
0x056d  cmd J[->]D
0x056e  cmd [degree]C[->][degree]F
0x056f  cmd [degree]F[->][degree]C
0x0570  cmd dB[->]ar.
0x0571  cmd ar.[->]dB
0x0572  cmd dB[->]pr.
0x0573  cmd pr.[->]dB
0x0574  cmd [zeta]
0x0575  cmd B[sub-n]
0x0576  cmd B[sub-n]*
0x0600  cmd y[^x]
0x0601  cmd +
0x0602  cmd -
0x0603  cmd [times]
0x0604  cmd /
0x0605  cmd RMDR
0x0606  cmd LOGx
0x0607  cmd MIN
0x0608  cmd MAX
0x0609  cmd ANGLE
0x060a  cmd [beta]
0x060b  cmd LN[beta]
0x060c  cmd I[GAMMA]
0x060d  cmd COMB
0x060e  cmd PERM
0x060f  cmd %+MG
0x0610  cmd %MG
0x0611  cmd ||
0x0612  cmd AGM
0x0613  cmd H.MS+
0x0614  cmd H.MS-
0x0615  cmd GCD
0x0616  cmd LCM
0x0617  cmd AND
0x0618  cmd OR
0x0619  cmd XOR
0x061a  cmd NAND
0x061b  cmd NOR
0x061c  cmd XNOR
0x061d  cmd DAYS+
0x061e  cmd [DELTA]DAYS
0x061f  cmd P[sub-n]
0x0620  cmd T[sub-n]
0x0621  cmd U[sub-n]
0x0622  cmd L[sub-n]
0x0623  cmd H[sub-n]
0x0624  cmd H[sub-n][sub-p]
0x1000  mult  LBL
0x1100  mult  LBL?
0x1200  mult  XEQ
0x1300  mult  GTO
0x1400  mult  [SIGMA]
0x1500  mult  [PI]
0x1600  mult  SLV
0x1700  mult  f'(x)
0x1800  mult  f"(x)
0x1900  mult  INT
0x8000  cmd # a
0x8001  cmd # a[sub-0]
0x8002  cmd # c
0x8003  cmd # c[sub-1]
0x8004  cmd # c[sub-2]
0x8005  cmd # e
0x8006  cmd # eE
0x8007  cmd # F
0x8008  cmd # g
0x8009  cmd # G
0x800a  cmd # G[sub-0]
0x800b  cmd # g[sub-e]
0x800c  cmd # h
0x800d  cmd # [h-bar]
0x800e  cmd # k
0x800f  cmd # l[sub-p]
0x8010  cmd # m[sub-e]
0x8011  cmd # m[sub-n]
0x8012  cmd # M[sub-p]
0x8013  cmd # m[sub-p]
0x8014  cmd # m[sub-u]
0x8015  cmd # m[sub-mu]
0x8016  cmd # N[sub-A]
0x8017  cmd # NaN
0x8018  cmd # p[sub-0]
0x8019  cmd # q[sub-p]
0x801a  cmd # R
0x801b  cmd # r[sub-e]
0x801c  cmd # R[sub-infinity]
0x801d  cmd # T[sub-0]
0x801e  cmd # t[sub-p]
0x801f  cmd # T[sub-p]
0x8020  cmd # V[sub-m]
0x8021  cmd # Z[sub-0]
0x8022  cmd # [alpha]
0x8023  cmd # [gamma]EM
0x8024  cmd # [gamma][sub-p]
0x8025  cmd # [epsilon][sub-0]
0x8026  cmd # [lambda][sub-c]
0x8027  cmd # [lambda][sub-c][sub-n]
0x8028  cmd # [lambda][sub-c][sub-p]
0x8029  cmd # [mu][sub-0]
0x802a  cmd # [mu][sub-B]
0x802b  cmd # [mu][sub-e]
0x802c  cmd # [mu][sub-n]
0x802d  cmd # [mu][sub-p]
0x802e  cmd # [mu][sub-u]
0x802f  cmd # [mu][sub-mu]
0x8030  cmd # [pi]
0x8031  cmd # [sigma][sub-B]
0x8032  cmd # [PHI]
0x8033  cmd # [PHI][sub-0]
0x8034  cmd # [infinity]
0x8100  cmd [cmplx]# a
0x8101  cmd [cmplx]# a[sub-0]
0x8102  cmd [cmplx]# c
0x8103  cmd [cmplx]# c[sub-1]
0x8104  cmd [cmplx]# c[sub-2]
0x8105  cmd [cmplx]# e
0x8106  cmd [cmplx]# eE
0x8107  cmd [cmplx]# F
0x8108  cmd [cmplx]# g
0x8109  cmd [cmplx]# G
0x810a  cmd [cmplx]# G[sub-0]
0x810b  cmd [cmplx]# g[sub-e]
0x810c  cmd [cmplx]# h
0x810d  cmd [cmplx]# [h-bar]
0x810e  cmd [cmplx]# k
0x810f  cmd [cmplx]# l[sub-p]
0x8110  cmd [cmplx]# m[sub-e]
0x8111  cmd [cmplx]# m[sub-n]
0x8112  cmd [cmplx]# M[sub-p]
0x8113  cmd [cmplx]# m[sub-p]
0x8114  cmd [cmplx]# m[sub-u]
0x8115  cmd [cmplx]# m[sub-mu]
0x8116  cmd [cmplx]# N[sub-A]
0x8117  cmd [cmplx]# NaN
0x8118  cmd [cmplx]# p[sub-0]
0x8119  cmd [cmplx]# q[sub-p]
0x811a  cmd [cmplx]# R
0x811b  cmd [cmplx]# r[sub-e]
0x811c  cmd [cmplx]# R[sub-infinity]
0x811d  cmd [cmplx]# T[sub-0]
0x811e  cmd [cmplx]# t[sub-p]
0x811f  cmd [cmplx]# T[sub-p]
0x8120  cmd [cmplx]# V[sub-m]
0x8121  cmd [cmplx]# Z[sub-0]
0x8122  cmd [cmplx]# [alpha]
0x8123  cmd [cmplx]# [gamma]EM
0x8124  cmd [cmplx]# [gamma][sub-p]
0x8125  cmd [cmplx]# [epsilon][sub-0]
0x8126  cmd [cmplx]# [lambda][sub-c]
0x8127  cmd [cmplx]# [lambda][sub-c][sub-n]
0x8128  cmd [cmplx]# [lambda][sub-c][sub-p]
0x8129  cmd [cmplx]# [mu][sub-0]
0x812a  cmd [cmplx]# [mu][sub-B]
0x812b  cmd [cmplx]# [mu][sub-e]
0x812c  cmd [cmplx]# [mu][sub-n]
0x812d  cmd [cmplx]# [mu][sub-p]
0x812e  cmd [cmplx]# [mu][sub-u]
0x812f  cmd [cmplx]# [mu][sub-mu]
0x8130  cmd [cmplx]# [pi]
0x8131  cmd [cmplx]# [sigma][sub-B]
0x8132  cmd [cmplx]# [PHI]
0x8133  cmd [cmplx]# [PHI][sub-0]
0x8134  cmd [cmplx]# [infinity]
0x8200  cmd iC 0
0x8201  cmd iC 1
0x8202  cmd iC 5.01402
0x8203  cmd iC 15.02903
0x8204  cmd iC 0.14944555
0x8205  cmd iC 0.99565716
0x8206  cmd iC 0.01169463
0x8207  cmd iC 0.93015749
0x8208  cmd iC 0.05475589
0x8209  cmd iC 0.78081772
0x820a  cmd iC 0.09312545
0x820b  cmd iC 0.56275713
0x820c  cmd iC 0.12349197
0x820d  cmd iC 0.29439286
0x820e  cmd iC 0.14277593
0x820f  cmd iC 0.97390652
0x8210  cmd iC 0.06667134
0x8211  cmd iC 0.03255816
0x8212  cmd iC 0.86506336
0x8213  cmd iC 0.14945134
0x8214  cmd iC 0.07503967
0x8215  cmd iC 0.67940956
0x8216  cmd iC 0.21908636
0x8217  cmd iC 0.10938715
0x8218  cmd iC 0.43339539
0x8219  cmd iC 0.26926671
0x821a  cmd iC 0.13470921
0x821b  cmd iC 0.14887433
0x821c  cmd iC 0.29552422
0x821d  cmd iC 0.14773910
0x8200  arg iC  max=0,indirect
0x8300  arg ERR max=19,indirect
0x8400  arg STO max=112,indirect,stack
0x8500  arg STO+  max=112,indirect,stack
0x8600  arg STO-  max=112,indirect,stack
0x8700  arg STO[times]  max=112,indirect,stack
0x8800  arg STO/  max=112,indirect,stack
0x8900  arg STO[v]  max=112,indirect,stack
0x8a00  arg STO[^]  max=112,indirect,stack
0x8b00  arg RCL max=112,indirect,stack
0x8c00  arg RCL+  max=112,indirect,stack
0x8d00  arg RCL-  max=112,indirect,stack
0x8e00  arg RCL[times]  max=112,indirect,stack
0x8f00  arg RCL/  max=112,indirect,stack
0x9000  arg RCL[v]  max=112,indirect,stack
0x9100  arg RCL[^]  max=112,indirect,stack
0x9200  arg x[<->]  max=112,indirect,stack
0x9300  arg [cmplx]STO  max=112,indirect,stack,complex
0x9400  arg [cmplx]STO+ max=112,indirect,stack,complex
0x9500  arg [cmplx]STO- max=112,indirect,stack,complex
0x9600  arg [cmplx]STO[times] max=112,indirect,stack,complex
0x9700  arg [cmplx]STO/ max=112,indirect,stack,complex
0x9800  arg [cmplx]RCL  max=112,indirect,stack,complex
0x9900  arg [cmplx]RCL+ max=112,indirect,stack,complex
0x9a00  arg [cmplx]RCL- max=112,indirect,stack,complex
0x9b00  arg [cmplx]RCL[times] max=112,indirect,stack,complex
0x9c00  arg [cmplx]RCL/ max=112,indirect,stack,complex
0x9d00  arg [cmplx]x[<->] max=112,indirect,stack,complex
0x9e00  arg VIEW  max=112,indirect,stack
0x9f00  arg STOS  max=97,indirect
0xa000  arg RCLS  max=97,indirect
0xa101  cmd [alpha] [x-bar]
0xa102  cmd [alpha] [y-bar]
0xa103  cmd [alpha] [sqrt]
0xa104  cmd [alpha] [integral]
0xa105  cmd [alpha] [degree]
0xa106  cmd [alpha] [space]
0xa107  cmd [alpha] [grad]
0xa108  cmd [alpha] [+/-]
0xa109  cmd [alpha] [<=]
0xa10a  cmd [alpha] [>=]
0xa10b  cmd [alpha] [!=]
0xa10c  cmd [alpha] [euro]
0xa10d  cmd [alpha] [->]
0xa10e  cmd [alpha] [<-]
0xa10f  cmd [alpha] [v]
0xa110  cmd [alpha] [^]
0xa111  cmd [alpha] [f-shift]
0xa112  cmd [alpha] [g-shift]
0xa113  cmd [alpha] [h-shift]
0xa114  cmd [alpha] [cmplx]
0xa115  cmd [alpha] [O-slash]
0xa116  cmd [alpha] [o-slash]
0xa117  cmd [alpha] [<->]
0xa118  cmd [alpha] [sz]
0xa119  cmd [alpha] [x-hat]
0xa11a  cmd [alpha] [y-hat]
0xa11b  cmd [alpha] [sub-m]
0xa11c  cmd [alpha] [times]
0xa11d  cmd [alpha] [approx]
0xa11e  cmd [alpha] [pound]
0xa11f  cmd [alpha] [yen]
0xa120  cmd [alpha]
0xa121  cmd [alpha] !
0xa122  cmd [alpha] "
0xa123  cmd [alpha] #
0xa124  cmd [alpha] $
0xa125  cmd [alpha] %
0xa126  cmd [alpha] &
0xa127  cmd [alpha] '
0xa128  cmd [alpha] (
0xa129  cmd [alpha] )
0xa12a  cmd [alpha] *
0xa12b  cmd [alpha] +
0xa12c  cmd [alpha] ,
0xa12d  cmd [alpha] -
0xa12e  cmd [alpha] .
0xa12f  cmd [alpha] /
0xa130  cmd [alpha] 0
0xa131  cmd [alpha] 1
0xa132  cmd [alpha] 2
0xa133  cmd [alpha] 3
0xa134  cmd [alpha] 4
0xa135  cmd [alpha] 5
0xa136  cmd [alpha] 6
0xa137  cmd [alpha] 7
0xa138  cmd [alpha] 8
0xa139  cmd [alpha] 9
0xa13a  cmd [alpha] :
0xa13b  cmd [alpha] ;
0xa13c  cmd [alpha] <
0xa13d  cmd [alpha] =
0xa13e  cmd [alpha] >
0xa13f  cmd [alpha] ?
0xa140  cmd [alpha] @
0xa141  cmd [alpha] A
0xa142  cmd [alpha] B
0xa143  cmd [alpha] C
0xa144  cmd [alpha] D
0xa145  cmd [alpha] E
0xa146  cmd [alpha] F
0xa147  cmd [alpha] G
0xa148  cmd [alpha] H
0xa149  cmd [alpha] I
0xa14a  cmd [alpha] J
0xa14b  cmd [alpha] K
0xa14c  cmd [alpha] L
0xa14d  cmd [alpha] M
0xa14e  cmd [alpha] N
0xa14f  cmd [alpha] O
0xa150  cmd [alpha] P
0xa151  cmd [alpha] Q
0xa152  cmd [alpha] R
0xa153  cmd [alpha] S
0xa154  cmd [alpha] T
0xa155  cmd [alpha] U
0xa156  cmd [alpha] V
0xa157  cmd [alpha] W
0xa158  cmd [alpha] X
0xa159  cmd [alpha] Y
0xa15a  cmd [alpha] Z
0xa15b  cmd [alpha] [
0xa15c  cmd [alpha] \
0xa15d  cmd [alpha] ]
0xa15e  cmd [alpha] ^
0xa15f  cmd [alpha] _
0xa160  cmd [alpha] `
0xa161  cmd [alpha] a
0xa162  cmd [alpha] b
0xa163  cmd [alpha] c
0xa164  cmd [alpha] d
0xa165  cmd [alpha] e
0xa166  cmd [alpha] f
0xa167  cmd [alpha] g
0xa168  cmd [alpha] h
0xa169  cmd [alpha] i
0xa16a  cmd [alpha] j
0xa16b  cmd [alpha] k
0xa16c  cmd [alpha] l
0xa16d  cmd [alpha] m
0xa16e  cmd [alpha] n
0xa16f  cmd [alpha] o
0xa170  cmd [alpha] p
0xa171  cmd [alpha] q
0xa172  cmd [alpha] r
0xa173  cmd [alpha] s
0xa174  cmd [alpha] t
0xa175  cmd [alpha] u
0xa176  cmd [alpha] v
0xa177  cmd [alpha] w
0xa178  cmd [alpha] x
0xa179  cmd [alpha] y
0xa17a  cmd [alpha] z
0xa17b  cmd [alpha] {
0xa17c  cmd [alpha] |
0xa17d  cmd [alpha] }
0xa17e  cmd [alpha] ~
0xa17f  cmd [alpha] [del]
0xa180  cmd [alpha] [ALPHA]
0xa181  cmd [alpha] [BETA]
0xa182  cmd [alpha] [GAMMA]
0xa183  cmd [alpha] [DELTA]
0xa184  cmd [alpha] [EPSILON]
0xa185  cmd [alpha] [ZETA]
0xa186  cmd [alpha] [ETA]
0xa187  cmd [alpha] [THETA]
0xa188  cmd [alpha] [IOTA]
0xa189  cmd [alpha] [KAPPA]
0xa18a  cmd [alpha] [LAMBDA]
0xa18b  cmd [alpha] [MU]
0xa18c  cmd [alpha] [NU]
0xa18d  cmd [alpha] [XI]
0xa18e  cmd [alpha] [OMICRON]
0xa18f  cmd [alpha] [PI]
0xa190  cmd [alpha] [RHO]
0xa191  cmd [alpha] [SIGMA]
0xa192  cmd [alpha] [TAU]
0xa193  cmd [alpha] [UPSILON]
0xa194  cmd [alpha] [PHI]
0xa195  cmd [alpha] [CHI]
0xa196  cmd [alpha] [PSI]
0xa197  cmd [alpha] [OMEGA]
0xa198  cmd [alpha] [sub-B]
0xa199  cmd [alpha] [sub-mu]
0xa19a  cmd [alpha] [^2]
0xa19b  cmd [alpha] [sub-infinity]
0xa19c  cmd [alpha] [^x]
0xa19d  cmd [alpha] [^-1]
0xa19e  cmd [alpha] [h-bar]
0xa19f  cmd [alpha] [infinity]
0xa1a0  cmd [alpha] [alpha]
0xa1a1  cmd [alpha] [beta]
0xa1a2  cmd [alpha] [gamma]
0xa1a3  cmd [alpha] [delta]
0xa1a4  cmd [alpha] [epsilon]
0xa1a5  cmd [alpha] [zeta]
0xa1a6  cmd [alpha] [eta]
0xa1a7  cmd [alpha] [theta]
0xa1a8  cmd [alpha] [iota]
0xa1a9  cmd [alpha] [kappa]
0xa1aa  cmd [alpha] [lambda]
0xa1ab  cmd [alpha] [mu]
0xa1ac  cmd [alpha] [nu]
0xa1ad  cmd [alpha] [xi]
0xa1ae  cmd [alpha] [omicron]
0xa1af  cmd [alpha] [pi]
0xa1b0  cmd [alpha] [rho]
0xa1b1  cmd [alpha] [sigma]
0xa1b2  cmd [alpha] [tau]
0xa1b3  cmd [alpha] [upsilon]
0xa1b4  cmd [alpha] [phi]
0xa1b5  cmd [alpha] [chi]
0xa1b6  cmd [alpha] [psi]
0xa1b7  cmd [alpha] [omega]
0xa1b8  cmd [alpha] [sub-0]
0xa1b9  cmd [alpha] [sub-1]
0xa1ba  cmd [alpha] [sub-2]
0xa1bb  cmd [alpha] [sub-c]
0xa1bc  cmd [alpha] [sub-e]
0xa1bd  cmd [alpha] [sub-n]
0xa1be  cmd [alpha] [sub-p]
0xa1bf  cmd [alpha] [sub-u]
0xa1c0  cmd [alpha] [A-grave]
0xa1c1  cmd [alpha] [A-acute]
0xa1c2  cmd [alpha] [A-tilde]
0xa1c3  cmd [alpha] [A-umlaut]
0xa1c4  cmd [alpha] [A-dot]
0xa1c5  cmd [alpha] [C-acute]
0xa1c6  cmd [alpha] [C-hook]
0xa1c7  cmd [alpha] [C-cedilla]
0xa1c8  cmd [alpha] [E-grave]
0xa1c9  cmd [alpha] [E-acute]
0xa1ca  cmd [alpha] [E-filde]
0xa1cb  cmd [alpha] [E-trema]
0xa1cc  cmd [alpha] [I-grave]
0xa1cd  cmd [alpha] [I-acute]
0xa1ce  cmd [alpha] [I-tilde]
0xa1cf  cmd [alpha] [I-trema]
0xa1d0  cmd [alpha] [N-tilde]
0xa1d1  cmd [alpha] [O-grave]
0xa1d2  cmd [alpha] [O-acute]
0xa1d3  cmd [alpha] [O-tilde]
0xa1d4  cmd [alpha] [O-umlaut]
0xa1d5  cmd [alpha] [R-hook]
0xa1d6  cmd [alpha] [S-hook]
0xa1d7  cmd [alpha] [sub-A]
0xa1d8  cmd [alpha] [U-grave]
0xa1d9  cmd [alpha] [U-acute]
0xa1da  cmd [alpha] [U-tilde]
0xa1db  cmd [alpha] [U-umlaut]
0xa1dc  cmd [alpha] [U-dot]
0xa1dd  cmd [alpha] [Y-acute]
0xa1de  cmd [alpha] [Y-trema]
0xa1df  cmd [alpha] [Z-hook]
0xa1e0  cmd [alpha] [a-grave]
0xa1e1  cmd [alpha] [a-acute]
0xa1e2  cmd [alpha] [a-tilde]
0xa1e3  cmd [alpha] [a-umlaut]
0xa1e4  cmd [alpha] [a-dot]
0xa1e5  cmd [alpha] [c-acute]
0xa1e6  cmd [alpha] [c-hook]
0xa1e7  cmd [alpha] [c-cedilla]
0xa1e8  cmd [alpha] [e-grave]
0xa1e9  cmd [alpha] [e-acute]
0xa1ea  cmd [alpha] [e-tilde]
0xa1eb  cmd [alpha] [e-trema]
0xa1ec  cmd [alpha] [i-grave]
0xa1ed  cmd [alpha] [i-acute]
0xa1ee  cmd [alpha] [i-tilde]
0xa1ef  cmd [alpha] [i-trema]
0xa1f0  cmd [alpha] [n-tilde]
0xa1f1  cmd [alpha] [o-grave]
0xa1f2  cmd [alpha] [o-acute]
0xa1f3  cmd [alpha] [o-tilde]
0xa1f4  cmd [alpha] [o-umlaut]
0xa1f5  cmd [alpha] [r-hook]
0xa1f6  cmd [alpha] [s-hook]
0xa1f7  cmd [alpha] [sub-k]
0xa1f8  cmd [alpha] [u-grave]
0xa1f9  cmd [alpha] [u-acute]
0xa1fa  cmd [alpha] [u-tilde]
0xa1fb  cmd [alpha] [u-umlaut]
0xa1fc  cmd [alpha] [u-dot]
0xa1fd  cmd [alpha] [y-acute]
0xa1fe  cmd [alpha] [y-trema]
0xa1ff  cmd [alpha] [z-hook]
0xa200  arg [alpha]RC#  max=112,indirect,stack
0xa300  arg [alpha]STO  max=112,indirect,stack
0xa400  arg [alpha]RCL  max=112,indirect,stack
0xa500  arg [alpha]IP max=112,indirect,stack
0xa600  arg [alpha]RL max=31,indirect
0xa700  arg [alpha]RR max=31,indirect
0xa800  arg [alpha]SL max=32,indirect
0xa900  arg [alpha]SR max=32,indirect
0xaa00  arg x=? max=112,indirect,stack
0xab00  arg x[!=]?  max=112,indirect,stack
0xac00  arg x[approx]?  max=112,indirect,stack
0xad00  arg x<? max=112,indirect,stack
0xae00  arg x[<=]?  max=112,indirect,stack
0xaf00  arg x>? max=112,indirect,stack
0xb000  arg x[>=]?  max=112,indirect,stack
0xb100  arg [cmplx]x=?  max=112,indirect,stack,complex
0xb200  arg [cmplx]x[!=]? max=112,indirect,stack,complex
0xb300  arg SKIP  max=100,indirect
0xb400  arg BACK  max=100,indirect
0xb500  arg DSE max=112,indirect,stack
0xb600  arg ISG max=112,indirect,stack
0xb700  arg DSZ max=112,indirect,stack
0xb800  arg ISZ max=112,indirect,stack
0xb900  arg DEC max=112,indirect,stack
0xba00  arg INC max=112,indirect,stack
0xbb00  arg LBL max=104
0xbc00  arg LBL?  max=104,indirect
0xbd00  arg XEQ max=104,indirect
0xbe00  arg GTO max=104,indirect
0xbf00  arg [SIGMA] max=104,indirect
0xc000  arg [PI]  max=104,indirect
0xc100  arg SLV max=104,indirect
0xc200  arg f'(x) max=104,indirect
0xc300  arg f"(x) max=104,indirect
0xc400  arg INT max=104,indirect
0xc500  arg ALL max=12,indirect
0xc600  arg FIX max=12,indirect
0xc700  arg SCI max=12,indirect
0xc800  arg ENG max=12,indirect
0xc900  arg DISP  max=12,indirect
0xca00  arg SF  max=104,indirect
0xcb00  arg CF  max=104,indirect
0xcc00  arg FF  max=104,indirect
0xcd00  arg FS? max=104,indirect
0xce00  arg FC? max=104,indirect
0xcf00  arg FS?C  max=104,indirect
0xd000  arg FS?S  max=104,indirect
0xd100  arg FS?F  max=104,indirect
0xd200  arg FC?C  max=104,indirect
0xd300  arg FC?S  max=104,indirect
0xd400  arg FC?F  max=104,indirect
0xd500  arg WSIZE max=65,indirect
0xd600  arg RL  max=64,indirect
0xd700  arg RR  max=64,indirect
0xd800  arg RLC max=65,indirect
0xd900  arg RRC max=65,indirect
0xda00  arg SL  max=65,indirect
0xdb00  arg SR  max=65,indirect
0xdc00  arg ASR max=65,indirect
0xdd00  arg SB  max=64,indirect
0xde00  arg CB  max=64,indirect
0xdf00  arg FB  max=64,indirect
0xe000  arg BS? max=64,indirect
0xe100  arg BC? max=64,indirect
0xe200  arg MASKL max=65,indirect
0xe300  arg MASKR max=65,indirect
0xe400  arg BASE  max=17,indirect
0xe500  cmd kg[->]lb
0xe501  cmd lb[->]kg
0xe502  cmd kg[->]stone
0xe503  cmd stone[->]kg
0xe504  cmd g[->]oz
0xe505  cmd oz[->]g
0xe506  cmd g[->]tr.oz
0xe507  cmd tr.oz[->]g
0xe508  cmd l[->]galUK
0xe509  cmd galUK[->]l
0xe50a  cmd l[->]galUS
0xe50b  cmd galUS[->]l
0xe50c  cmd l[->]cft
0xe50d  cmd cft[->]l
0xe50e  cmd ml[->]flozUK
0xe50f  cmd flozUK[->]ml
0xe510  cmd ml[->]flozUS
0xe511  cmd flozUS[->]ml
0xe512  cmd cm[->]inches
0xe513  cmd inches[->]cm
0xe514  cmd m[->]fathom
0xe515  cmd fathom[->]m
0xe516  cmd m[->]feet
0xe517  cmd feet[->]m
0xe518  cmd m[->]yards
0xe519  cmd yards[->]m
0xe51a  cmd km[->]miles
0xe51b  cmd miles[->]km
0xe51c  cmd km[->]l.y.
0xe51d  cmd l.y.[->]km
0xe51e  cmd km[->]pc
0xe51f  cmd pc[->]km
0xe520  cmd km[->]AU
0xe521  cmd AU[->]km
0xe522  cmd km[->]nmi
0xe523  cmd nmi[->]km
0xe524  cmd ha[->]acres
0xe525  cmd acres[->]ha
0xe526  cmd N[->]lbf
0xe527  cmd lbf[->]N
0xe528  cmd J[->]Btu
0xe529  cmd Btu[->]J
0xe52a  cmd J[->]cal
0xe52b  cmd cal[->]J
0xe52c  cmd J[->]kWh
0xe52d  cmd kWh[->]J
0xe52e  cmd Pa[->]atm
0xe52f  cmd atm[->]Pa
0xe530  cmd Pa[->]bar
0xe531  cmd bar[->]Pa
0xe532  cmd Pa[->]mmHg
0xe533  cmd mmHg[->]Pa
0xe534  cmd Pa[->]psi
0xe535  cmd psi[->]Pa
0xe536  cmd Pa[->]inHg
0xe537  cmd inHg[->]Pa
0xe538  cmd Pa[->]torr
0xe539  cmd torr[->]Pa
0xe53a  cmd W[->]bhp
0xe53b  cmd bhp[->]W
0xe53c  cmd W[->]PS(hp)
0xe53d  cmd PS(hp)[->]W
0xe53e  cmd W[->]HP[sub-e]
0xe53f  cmd HP[sub-e][->]W
0xe540  cmd t[->]tons
0xe541  cmd tons[->]t
0xe542  cmd t[->]s.tons
0xe543  cmd s.tons[->]t
0xe600  arg SLVI  max=91,indirect
0xe700  arg SLVS  max=91,indirect
0xe800  arg PSE max=100,indirect
0xe900  arg KEY?  max=112,indirect,stack
0xea00  arg [alpha]XEQ  max=112,indirect,stack
0xeb00  arg [alpha]GTO  max=112,indirect,stack
0xec00  arg PRCL  max=4,indirect
0xed00  arg PSTO  max=4,indirect
0xee00  arg P[<->]  max=4,indirect
0xef00  arg RCF max=112,indirect,stack
0xf000  arg RCF+  max=112,indirect,stack
0xf100  arg RCF-  max=112,indirect,stack
0xf200  arg RCF[times]  max=112,indirect,stack
0xf300  arg RCF/  max=112,indirect,stack
0xf400  arg RCF[v]  max=112,indirect,stack
0xf500  arg RCF[^]  max=112,indirect,stack
0xf600  arg [cmplx]RCF  max=112,indirect,stack,complex
0xf700  arg [cmplx]RCF+ max=112,indirect,stack,complex
0xf800  arg [cmplx]RCF- max=112,indirect,stack,complex
0xf900  arg [cmplx]RCF[times] max=112,indirect,stack,complex
0xfa00  arg [cmplx]RCF/ max=112,indirect,stack,complex
0xfb00  arg S.L max=100,indirect
0xfc00  arg S.R max=100,indirect
0xfd00  arg VW[alpha]+  max=112,indirect,stack
