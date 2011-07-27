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
my $Description = "Assembler/Disassembler for the WP 34S calculator.";
#
#-----------------------------------------------------------------------
#
# Language:         Perl script
#
my $SVN_Current_Revision  =  '$Revision$';
#
#-----------------------------------------------------------------------

use strict;
use File::Basename;

# ---------------------------------------------------------------------

my $debug = 0;
my $quiet = 1;
my (%mnem2hex, %hex2mnem, %ord2escaped_alpha, %escaped_alpha2ord);
my @files = ();

my $outfile = "-";

my $DEFAULT_OP_SVN = "-- unknown --";
my $op_svn = $DEFAULT_OP_SVN;
my $report_op_svn = 1;

my $FLASH_LENGTH = 512;
my $DEFAULT_FLASH_BLANK_INSTR = "ERR 03";
my $user_flash_blank_fill = "";

my $STD_EXTERNAL_OP_TABLE = "wp34s.op";
my $USE_INTERNAL_OPCODE_MAP = "-- internal table --";
my $DEFAULT_OPCODE_MAP_FILE = $USE_INTERNAL_OPCODE_MAP;
my $opcode_map_file = $DEFAULT_OPCODE_MAP_FILE;

# If this variable is set to a non-NULL value, it will be used as the filename of the
# expanded opcode dump file. This *should* be equivalent to the "a a" file from the
# 'calc a a' program.
my $expanded_op_file = "";

# If this variable is set to a non-NULL value, it will be used as the filename of the
# dumped escaped alpha table.
my $dump_escaped_alpha_table = "";

# Number of '*' to prepend to labels in disassembly -- just makes them easier to find.
my $DEFAULT_STAR_LABELS = 0;
my $star_labels = $DEFAULT_STAR_LABELS;

my $MAX_FLASH_WORDS = 506;
my $disable_flash_limit = 0;

my $no_step_numbers = 0;
my $zero_last_4 = 0;

my $MAGIC_MARKER = 0xA53C;
my $CRC_INITIALIZER = 0x5aa5;
my $use_magic_marker = 0;

my $build_an_empty_image = 0;

my $ASSEMBLE_MODE = "assemble";
my $DISASSEMBLE_MODE = "disassemble";
my $DEFAULT_MODE = $ASSEMBLE_MODE;
my $mode = $DEFAULT_MODE;

# There are 2 XROM modes:
# 1) XROM output into a C array (use '-c_xrom' switch)
# 2) XROM output into a binary image (use '-xrom' switch)
my $xrom_c_mode = 0;
my $xrom_leader = "const s_opcode xrom[] = (";
my $xrom_indent_spaces = 8;
my $xrom_bin_mode = 0;

# These are used to convert all the registers into numeric offsets.
#
# There are 2 groups of named register offsets; the 112 group (XYZTABCDLIJK) and the 104
# group (ABCD). We have to extract the opcode's numeric value based on the op-type. The value of
# max will give us this (112,111,104,100,etc.). Note that for all values of max<=100, either group is
# valid since these are purely numeric.
my @reg_offset_112 = (0 .. 99, "X", "Y", "Z", "T", "A", "B", "C", "D", "L", "I", "J", "K");
my $MAX_INDIRECT_112 = 112;
my @reg_offset_104 = (0 .. 99, "A", "B", "C", "D");
my $MAX_INDIRECT_104 = 104;

# The register numeric value is flagged as an indirect reference by setting bit 7.
my $INDIRECT_FLAG = 0x80;

# There are several instructions that require a "custom" format for printing the
# "arg" type descriptor. Access that format through this table.
my %table_exception_format = ( "PRCL"   => "%01d",
                               "PSTO"   => "%01d",
                               "P[<->]" => "%01d",
                             );

# ---------------------------------------------------------------------

# Automatically extract the name of the executable that was used. Also extract
# the directory so we can potentially source other information from this location.
my $script_executable = $0;
my ($script_name, $script_dir, $script_suffix) = fileparse($script_executable);

my $script  = "$script_name  - $Description";
my $usage = <<EOM;

$script

Usage:
   $script_name src_file [src_file2 [src_file3]] -o out_binary  # assembly mode
   $script_name in_binary -dis [-o out_binary] > src_file       # disassembly mode

Parameters:
   src_file         One or more WP34s program source files. Conventionally, "wp34s" is used
                    as the filename extension.
   -o outfile       Output produced by the tool. In assembler mode, this is required and will be
                    the binary flash image. Assembler output extension is conventionally ".dat".
                    In disassembler mode, this is optional and will be the output ASCII source
                    listing, conventionally uses the ".wp34s" extension. I/O redirection can be
                    used as an alternate method of capturing the ASCII source listing in disassembler
                    mode.
   -dis             Disassemble the binary image file.
   -op infile       Optional opcode file to parse.              [Default: $DEFAULT_OPCODE_MAP_FILE]
   -fill fill_hex   Optional value to prefill flash image with. [Default: instruction '$DEFAULT_FLASH_BLANK_INSTR']
   -s number        Optional number of asterisks (stars) to prepend to labels in
                    disassembly mode.                           [Default: $DEFAULT_STAR_LABELS]
   -syntax outfile  Turns on syntax guide file dumping. Output will be sent to 'outfile'.
   -ns              Turn off step numbers in disassembler listing.
   -no_svn          Suppress report of compressed opcode table SVN version number.
   -svn             Report compressed opcode table SVN version number to STDOUT. [default]
   -h               This help script.

Examples:
  \$ $script_name great_circle.wp34s -o wp34s-3.dat
  - Assembles the named WP34s program source file producing a flash image for the WP34s.
    MUST use -o to name the output file in assembler mode -- cannot use output redirection.

  \$ $script_name  great_circle.wp34s floating_point.wp34s -o wp34s-1.dat  -fill FFFF
  - Assembles multiple WP34s program source files into a single contiguous flash image for
    the WP34s. Uses 0xFFFF as the optional fill value. Allows (and encourages) use of libraries
    of programs by concatenating the flash image from several source files.

  \$ $script_name -dis wp34s-1.dat -s 3 > myProg.wp34s
  \$ $script_name -dis wp34s-1.dat -s 3 -o myProg.wp34s
  - Disassembles a flash image from the WP34s. Prepend 3 asterisks to the front to each label to
    make then easier to find in the listing (they are ignored during assembly). Both invocation
    result in identical behaviour.

  \$ $script_name -dis wp34s-0.dat -o test.wp34s; $script_name test.wp34s -o wp34s-0a.dat; diff wp34s-0.dat wp34s-0a.dat
  - An end-to-end test of the tool. Note that the blank fill mode will have to have been the same
    for the binaries to match.

Notes:
  1) Step numbers can be used in the source file but they are ignored. Since they are ignored,
     it doesn\'t matter if they are not contiguous (ie: 000, 003, 004) or not monotonic (ie: 000,
     004, 003). The disassembler does produce step numbers that are both contiguous and monotonic
     -- when not suppressed by the -ns switch.
  2) You can name a different opcode table using the -op switch. This can be used to translate a source
     written for a different SVN revision of the WP34s to move the program to a modern version of
     the WP34s. Typically, disassemble the old flash using the old opcode table and reassemble using
     the default (internal) table. This is also an insurance policy against the opcodes evolving as well.
     Simply target newer opcode tables as they become available. To generate an opcode table, using the
     following (Linux version shown. Windows is likely similar, though I have never tried):
      \$ svn up
      \$ cd ./trunk
      \$ make
      \$ ./Linux/calc opcodes > new_opcodes.map
      \$ $script_name -dis wp34s-0.dat -op new_opcodes.map > source.wp34s
  3) The prefill-value will be interpreted as decimal if it contains only decimal digits. If it
     contains any hex digits or it starts with a "0x", it will be interpreted as a hex value. Thus
     "1234" will be decimal 1234 while "0x1234" will be the decimal value 4660. Both "EFA2" and "0xEFA2"
     will be interpreted as a hex value as well (61346). The leading "0x" is optional in this case.
  4) The order the command lines switches is not significant. There is no fixed order.
  5) There is subtle difference in using the disassembler with the "-o" switch and output redirection.
     With the "-o" switch, the resulting file will not contain the commented statistics -- these will
     be displayed on the screen. With redirection, all printed lines go to the file.
EOM

my $extended_help = <<EXTENDED_HELP;
Extended parameters:
   -d level         Set the debug output verbosity level.       [Default: 0]
   -dl              Disable flash length limit.
   -c               Output the assembler result in C-array mode. Primarily for XROM assembly.
   -xrom            Assemble in XROM mode. Automatically turns off the flash length limit (-dl).
   -alpha  file     Turns on alpha table dumping. Output will be sent to 'file'.
   -empty           Used in assembler mode to generate an empty flash image. Needs '-o FILE.dat'
                    as well.
   -more_help       This extended help script.

Examples:
  \$ calc xrom | wp34s_conv.pl > xrom.wp34s
  \$ $script_name xrom.wp34s -dl -o xrom.dat
  - Assembles the XROM output into a binary image. (Mostly for test purposes.)

  \$ calc xrom | wp34s_conv.pl > xrom.wp34s
  \$ $script_name xrom.wp34s -c -o xrom.c
  - Creates a C-array of the XROM.

  \$ $script_name -syntax opcode_syntax.txt
  - Creates an opcode syntax file call 'opcode_syntax.txt' matching the current internal
    opcode table. Useful as reference when writing or transcribing source in an editor.

  \$ $script_name -empty -o wp34s-0.dat
  - Creates an empty flash image. (Could not figure out how clear an image otherwise though
    there probably is a mechanism I don't know about.)

Notes:
  1) XROM mode skips the first 2 words normally written to the binary image. This means that
     the CRC16 and the "next step" indicator are suppressed. This binary output is currently
     *not* able to be disassembled.
EXTENDED_HELP

#######################################################################

get_options();

# We load a compressed opcode <-> mnemonic table and expand it into an uncompressed
# version, By default, the compressed table is contained in a 'DATA' section at the
# end of the script. A newer (or older!) compressed table can also be read in in order
# to translate to newer (or older) opcode maps.
load_opcode_tables($opcode_map_file);

print "// Opcode SVN version: $op_svn\n" if $report_op_svn;

# Decide what value is to be used to fill the unused portion of the flash image.
# By default, the operation "ERR 03" has been used in the standard image that the
# calculator produces itself. However, there are some minor advantages to making this
# the value "FFFF". Use the the '-fill <VALUE>' switch to modify the value,
my $flash_blank_fill_hex_str = ($user_flash_blank_fill) ? $user_flash_blank_fill : $mnem2hex{$DEFAULT_FLASH_BLANK_INSTR};

# This is where the script decides what it is going to do.
if( $build_an_empty_image ) {
  build_empty_flash_image( $outfile );
  warn "Built empty flash image...\n";
} elsif( $expanded_op_file ) {
  write_expanded_opcode_table_to_file( $expanded_op_file );
} elsif( $dump_escaped_alpha_table ) {
  write_escaped_alpha_table_to_file( $dump_escaped_alpha_table );
} elsif( $mode eq $ASSEMBLE_MODE ) {
  assemble( $outfile, @files );
} elsif( $mode eq $DISASSEMBLE_MODE ) {
  disassemble( $outfile, @files );
} else {
  warn "Internal error: Huh! How did I get here?\n";
}


#######################################################################
#
# Assemble the source files
#
sub assemble {
  my $outfile = shift;
  my @files = @_;
  local $_;
  my ($crc16);
  my @words = pre_fill_flash($flash_blank_fill_hex_str); # Fill value is a hex4 string.
  my $next_free_word = 1;
  my $steps_used = 0;
  my ($alpha_text);

  foreach my $file (@files) {
    my @lines = read_file_and_redact_comments($file);
    for my $k (1 .. (scalar @lines)) {
      $_ = $lines[$k-1];

      next if /^\s*$/; # Skip any blank lines.
      next if /^\s*\d{3}\:{0,1}\s*$/; # Skip any lines that have just a 3-digit step number
                                      # and no actual mnemonic.
      chomp; chomp;

      s/\s+$//; # Remove any trailing blanks.
      ($_, $alpha_text) = assembler_preformat_handling($_);

      if( not exists $mnem2hex{$_} ) {
        die "ERROR: Cannot recognize mnemonic at line $k of '$file': $_\n";
      } else {
        $steps_used++;
        # Alpha text needs to be treated separately since it encodes to 2 words.
        if( $alpha_text ) {
          # See if the text has any escaped characters in it. Subsitute them if found.
          # There may be more than one so loop until satisfied.
          while( $alpha_text =~ /\[(.+?)\]/ ) {
            my $escaped_alpha = $1;
            if( exists $escaped_alpha2ord{$escaped_alpha} ) {
              my $char = chr($escaped_alpha2ord{$escaped_alpha});
              $alpha_text =~ s/\[$escaped_alpha\]/$char/;
            } else {
              die "ERROR: Cannot locate escaped alpha: [$escaped_alpha] in table.\n";
            }
          }

          my @chars = split "", $alpha_text;
          $words[++$next_free_word] = hex2dec($mnem2hex{$_}) | ord($chars[0]);
          if( $chars[1] and $chars[2] ) {
            $words[++$next_free_word] = ord($chars[1]) | (ord($chars[2]) << 8);
          } elsif( $chars[1] ) {
            $words[++$next_free_word] = ord($chars[1]);
          } else {
            $words[++$next_free_word] = 0;
          }

        # "Normal" opcodes...
        } else {
          $words[++$next_free_word] = hex2dec($mnem2hex{$_});
        }
      }
      if( ($next_free_word >= $MAX_FLASH_WORDS) and not $disable_flash_limit ) {
        die "ERROR: Too many program steps encounterd.\n";
      }
    }
  }

  # We have 3 output modes:
  #  - A C-array for use with XROM
  #  - An XROM binary with no output of the CRC or step count.
  #  - Standard assembler binary output.
  if( $xrom_c_mode ) {
    dump_c_array( $outfile, $xrom_leader, $xrom_indent_spaces, @words[2 .. $next_free_word] );

  } elsif( $xrom_bin_mode ) {
    $crc16 = calc_crc16( @words[2 .. $next_free_word] );
    write_binary( $outfile, @words[2 .. $next_free_word] ); # Limit the words that are processed.
    print "// XROM mode -- NOTE: Currently cannot be disassembled.\n";
    print "// CRC16: ", dec2hex4($crc16), "\n";
    print "// Total words: ", $next_free_word-1, "\n";
    print "// Total steps: $steps_used\n";

  # Normal mode.
  } else {
    $words[1] = $next_free_word;
    $crc16 = ($use_magic_marker) ? $MAGIC_MARKER : calc_crc16( @words[2 .. $next_free_word] );
    $words[0] = $crc16;
    write_binary( $outfile, @words );
    print "// CRC16: ", dec2hex4($crc16), "\n";
    print "// Total words: ", $next_free_word-1, "\n";
    print "// Total steps: $steps_used\n";
  }

  return;
} # assemble


#######################################################################
#
# Disassemble the source files
#
sub disassemble {
  my $outfile = shift;
  my @files = @_;

  my $text_lo_limit = hex2dec($mnem2hex{"LBL'"});
  my $text_hi_limit = hex2dec($mnem2hex{"INT'"}) + 256;

  open OUT, "> $outfile" or die "ERROR: Cannot open file '$outfile' for writing: $!\n";

  print "// Source file(s): ", join(", ", @files), "\n";

  foreach my $file (@files) {
    my $double_words = 0;
    open DAT, $file or die "ERROR: Cannot open dat file '$file' for reading: $!\n";

    # This trick will read in the binary image in 16-bit words of the correct endian.
    local $/;
    my @words = unpack("S*", <DAT>);

    my $len = $words[1] - 1;
    my $total_steps = 0;
    for( my $k = 0; $k < $len; $k++ ) {
      my $word = $words[$k+2];
      $total_steps++;

      # Check if we are in the 2 words opcode range: LBL' to INT'. These require special treatment.
      if( ($word >= $text_lo_limit) and ($word < $text_hi_limit) ) {
        my $base_op = $word & 0xFF00;
        my (@chars);
        $chars[0] = $word & 0xFF;
        $word = $words[$k+3];
        $double_words++;
        ($chars[2], $chars[1]) = (($word >> 8), ($word & 0xFF));
        print_disassemble_text($total_steps, $base_op, @chars);
        $k++;
      } else {
        print_disassemble_normal($total_steps, $word);
      }
    }
    print "// $total_steps total instructions used.\n";
    print "// $len total words used.\n";
    print "// ", $len - (2*$double_words), " single word instructions.\n";
    print "// $double_words double word instructions.\n";
    close DAT;
  }
  close OUT;
  return;
} # disassemble


#######################################################################
#
# Print the lines which do not include quoted characters.
#
sub print_disassemble_normal {
  my $idx = shift;
  my $word = shift;
  my $hex_str = lc dec2hex4($word);
  if( not exists $hex2mnem{$hex_str} ) {
    die "ERROR: Opcode '$hex_str' does not exist at line ", $idx+1, " at print_disassemble_normal\n";
  }

  # Set up the correct number of leading asterisks if requested.
  my $label_flag = (($hex2mnem{$hex_str} =~ /LBL/) and $star_labels) ? "*" x $star_labels : "";

  if( $no_step_numbers ) {
    printf OUT "%0s%0s\n", $label_flag, $hex2mnem{$hex_str};
  } elsif( $debug ) {
    printf OUT "%03d /* %04s */ %0s%0s\n", $idx, $hex_str, $label_flag, $hex2mnem{$hex_str};
  } else {
    printf OUT "%03d %0s%0s\n", $idx, $label_flag, $hex2mnem{$hex_str};
  }
  return;
} # print_disassemble_normal


#######################################################################
#
# Print the lines that include up to 3 quoted characters For example "LBL'ABC'".
# Substitute the characters which exist in the escaped-alpha translation
# table with their escaped counterparts. For example, the 2-word opcode
# "12a3 0058" will get translated to "LBL'[delta]X'".
#
sub print_disassemble_text {
  my $idx = shift;
  my $opcode = shift;
  my @chars = @_;
  my $hex_str = lc dec2hex4($opcode);
  if( not exists $hex2mnem{$hex_str} ) {
    die "ERROR: Opcode '$hex_str' does not exist at line ", $idx+1, " at print_disassemble_text\n";
  }

  # Set up the correct number of leading asterisks if requested.
  my $label_flag = (($hex2mnem{$hex_str} =~ /LBL/) and $star_labels) ? "*" x $star_labels : "";

  if( $no_step_numbers ) {
    printf OUT "%0s%0s", $label_flag, $hex2mnem{$hex_str};
  } elsif( $debug ) {
    my $op_hex = sprintf "%04s %04s", lc dec2hex4(hex2dec($hex_str)+$chars[0]), lc dec2hex4($chars[2]*256+$chars[1]);
    printf OUT "%03d /* %04s */ %0s%0s", $idx, $op_hex, $label_flag, $hex2mnem{$hex_str};
  } else {
    printf OUT "%03d %0s%0s", $idx, $label_flag, $hex2mnem{$hex_str};
  }

  # Check to see if the character is an escaped-alpha. If so, print the escaped string
  # rather than the actual characters.
  foreach my $ord (@chars) {
    if( exists $ord2escaped_alpha{$ord} ) {
      print OUT "[${ord2escaped_alpha{$ord}}]" if $ord;
    } else {
      print OUT chr($ord) if $ord;
    }
  }
  print OUT "'\n"; # Append the trailing single quote.
  return;
} # print_disassemble_normal


#######################################################################
#
# Compute a CRC16 over the stream of bytes. We have convert the word array to
# a byte array to do this.
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
# Handle any lines that need special work. If the opcode has single
# quotes in it ('xxx'), it needs to have the alpha string extracted to the
# "$alpha_text" variable. If "$alpha_text" is returned as the NULL string,
# the opcode was not of this type. Thus, this "$alpha_text" flag is used
# above to signal the assembler to swicth into the 2-word opcode translation
# mode required for these quoted alpha mnemonics.
#
sub assembler_preformat_handling {
  my $line = shift;
  my $alpha_text = "";

  $line =~ s/^\s*\d{3}\:{0,1}\s+//; # Remove any line numbers with or without a colon
  $line =~ s/^\s+//;      # Remove leading whitespace
  $line =~ s/\s+$//;      # Remove trailing whitespace

  # Labels sometimes are displayed with an asterisk to make them easier to locate.
  $line =~ s/^\*+LBL/LBL/;

  # Remove the actual label string so we can do a lookup based on the base mnemonic.
  if( $line =~ /\S+\'(\S+)\'/ ) {
    $alpha_text = $1;
    $line =~ s/\'\S+/\'/;
  }

  return ($line, $alpha_text);
} # assembler_preformat_handling


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
  # If strict image compatibility is required, zero these words.
  if( $zero_last_4 ) {
    $flash[-1] = 0;
    $flash[-2] = 0;
    $flash[-3] = 0;
    $flash[-4] = 0;
  }
  return @flash;
} # pre_fill_flash


#######################################################################
#
# Load the opcode tables.
#
sub load_opcode_tables {
  my $file = shift;

  # If the user has specified an opcode table via the -opcodes switch, use that one.
  if( $file ne $USE_INTERNAL_OPCODE_MAP ) {
    open DATA, $file or die "ERROR: Cannot open opcode map file '$file' for reading: $!\n";
    $file .= " (specified)";

  # Search the current directory for an opcode table.
  } elsif( -e "${STD_EXTERNAL_OP_TABLE}" ) {
    $file = "${STD_EXTERNAL_OP_TABLE}";
    open DATA, $file or die "ERROR: Cannot open opcode map file '$file' for reading: $!\n";
    $file .= " (local directory)";

  # Search the directory the script is running out of.
  } elsif( -e "${script_dir}${STD_EXTERNAL_OP_TABLE}" ) {
    $file = "${script_dir}${STD_EXTERNAL_OP_TABLE}";
    open DATA, $file or die "ERROR: Cannot open opcode map file '$file' for reading: $!\n";

  } else {
  }
  print "// Opcode map source: $file\n";

  # Read from whatever source happened to be caught. If none of the above opened
  # a file, read the attached DATA segment at the end of the script as the fallback.
  while(<DATA>) {
    # Extract the SVN version of the opcode table -- if it exists.
    if( /^# Generated .+ svn_(\d+)\.op/ ) {
      $op_svn = $1;
    }

    # Remove any comments ('#' or '//') and any blank lines.
    next if /^\s*#/;
    next if /^\s*$/;
    next if m<^\s*//>;
    chomp; chomp;

    # From about WP34s SVN 1000 on, the console can spit out a "compressed" opcode map
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

      # We need to harvest the escaped-alpha table for later substitutions.
      if( /\[alpha\]\s+\[(.+)\]/ ) {
        my $escaped_alpha = $1;
        load_escaped_alpha_tables($hex_str, $escaped_alpha, $line_num);
      }

      # As a convenience, create a secondary entry for iC commands that have a 2-digit
      # number. This form can be used instead of entering the longer constant form. For
      # example, "iC 03" will be accepted as well as "iC 5.01402". Note that the offset
      # will are in decimal so "iC 12" is treated as "iC 0.56275713" and not "iC 0.03255816",
      # which would have been the hex offset of 0x12.
      if( $mnemonic =~ /iC/ ) {
        load_2digit_iC_entry($hex_str, $mnemonic, $line_num);
      }

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
# Write the expanded opcode table to a file.
#
sub write_expanded_opcode_table_to_file {
  my $file = shift;
  local $_;
  open EXPND, "> $file" or die "ERROR: Cannot open expanded opcode file '$file' for writing: $!\n";
  print EXPND "// Opcode SVN version: $op_svn\n";
  for my $hex_str (sort keys %hex2mnem) {
    print EXPND "$hex_str  ${hex2mnem{$hex_str}}\n";
  }
  close EXPND;
  return;
} # write_expanded_opcode_table_to_file


#######################################################################
#
# Write the escaped alpha table to a file.
#
sub write_escaped_alpha_table_to_file {
  my $file = shift;
  local $_;
  open ALPHA, "> $file" or die "ERROR: Cannot open escaped alpha file '$file' for writing: $!\n";
  for my $ord (sort {$a <=> $b} keys %ord2escaped_alpha) {
    print ALPHA "ord = $ord (decimal), 0x", dec2hex2($ord), "(hex); alpha = '${ord2escaped_alpha{$ord}}'\n";
  }
  close ALPHA;
  return;
} # write_escaped_alpha_table_to_file


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
      if( $direct_max > $MAX_INDIRECT_104 ) {
        $reg_str = $reg_offset_112[$offset];
      } else {
        $reg_str = $reg_offset_104[$offset];
      }
      # "Correct" the format if it is in the numeric range [0-99]. However, a few of the instructions
      # are limited to a single digit. Detect these and format accordingly.
      if( not exists $table_exception_format{$base_mnemonic} ) {
        $reg_str = sprintf("%02d", $offset) if $offset < 100;
      } else {
        $reg_str = sprintf($table_exception_format{$base_mnemonic}, $offset) if $offset < 100;
      }
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
      my $mnemonic_str = "$base_mnemonic";
      $mnemonic_str .= "[->]";
      my ($hex_str, $mnemonic) = construct_offset_mnemonic($base_hex_str, $indirect_offset, $mnemonic_str, $reg_str);
      load_table_entry($hex_str, $mnemonic, $line_num);
    }
  }

  return;
} # parse_arg_type


#######################################################################
#
#
#
sub load_escaped_alpha_tables {
  my $hex_str = shift;
  my $escaped_alpha = shift;
  my $line_num = shift;
  my $ord = hex2dec($hex_str) & 0xFF;

  # Build the table to convert ordinals to escaped alpha, ie: 0x1D = 29 => "[approx]".
  if( not exists $ord2escaped_alpha{$ord} ) {
    $ord2escaped_alpha{$ord} = $escaped_alpha;
  } else {
    warn "# WARNING: Duplicate ordinal for escaped alpha: $ord (0x", dec2hex2($ord), ") at line $line_num\n" unless $quiet;
  }

  # Build the table to convert escaped alpha to ordinals, ie: "[approx]" => 0x1D = 29.
  if( not exists $escaped_alpha2ord{$escaped_alpha} ) {
    $escaped_alpha2ord{$escaped_alpha} = $ord;
  } else {
    warn "# WARNING: Duplicate escaped alpha: '$escaped_alpha' at line $line_num\n" unless $quiet;
  }
  return;
} # load_escaped_alpha_tables


#######################################################################
#
# Make a duplciate entry for the "iC' menomics to allow a 2-digit offsets version of
# the mnemonic. Only load this the the mnemonic to hex table to avoid multiple definitions
# in the hex to mnemonic table.
#
sub load_2digit_iC_entry {
  my $hex_str = shift;
  my $mnemonic = shift;
  my $line_num = shift;
  my $dup_mnemonic = sprintf("iC %02d", hex2dec($hex_str) & 0xFF);
  load_mnem2hex_entry($hex_str, $dup_mnemonic, $line_num);
  return;
} # load_2digit_iC_entry


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
  return (dec2hex4(hex2dec($base_hex_str) + $offset), "${mnemonic}${reg_name_str}");
} # construct_offset_mnemonic


#######################################################################
#
# Load the translation hash tables with opcode (as a hex string) and the mnemonic.
#
sub load_table_entry {
  my $op_hex_str = shift;
  my $mnemonic = shift;
  my $line_num = shift;
  load_mnem2hex_entry($op_hex_str, $mnemonic, $line_num);
  load_hex2mnem_entry($op_hex_str, $mnemonic, $line_num);
  return;
} # load_table_entry


#######################################################################
#
# Load the translation hash table with opcode (as a hex string) and the mnemonic.
#
sub load_mnem2hex_entry {
  my $op_hex_str = shift;
  my $mnemonic = shift;
  my $line_num = shift;

  if( not exists $mnem2hex{$mnemonic} ) {
    $mnem2hex{$mnemonic} = lc $op_hex_str;
  } else {
    warn "# WARNING: Duplicate mnemonic: '$mnemonic' at line $line_num (new definition: '$op_hex_str', previous definition: '${mnem2hex{$mnemonic}}')\n" unless $quiet;
  }
  return;
} # load_mnem2hex_entry


#######################################################################
#
# Load the translation hash tables with opcode (as a hex string) and the mnemonic.
#
sub load_hex2mnem_entry {
  my $op_hex_str = shift;
  my $mnemonic = shift;
  my $line_num = shift;

  if( not exists $hex2mnem{$op_hex_str} ) {
    $hex2mnem{lc $op_hex_str} = $mnemonic;
  } else {
    warn "# WARNING: Duplicate opcode hex: '$op_hex_str' at line $line_num (new defintion: '$mnemonic', previous definition: '${hex2mnem{$op_hex_str}}')\n" unless $quiet;
  }
  return;
} # load_hex2mnem_entry


#######################################################################
#
# Convert a decimal number to 4-digit hex string
#
sub dec2hex4 {
  my $dec = shift;
  my $hex_str = sprintf "%04X", $dec;
  return ( $hex_str );
} # dec2hex4

sub d2h { # Quickie version for use with Perl debugger.
  return dec2hex4(@_);
} # d2h


#######################################################################
#
# Convert a decimal number to 2-digit hex string
#
sub dec2hex2 {
  my $dec = shift;
  my $hex_str = sprintf "%02X", $dec;
  return ( $hex_str );
} # dec2hex2

sub d2h2 { # Quickie version for use with Perl debugger.
  return dec2hex2(@_);
} # d2h2

#######################################################################
#
# Convert a hex string to a decimal number
#
sub hex2dec {
  my $hex_str = shift;
  my $dec = hex("0x" . $hex_str);
  return ( $dec );
} # hex2dec

sub h2d { # Quickie version for use with Perl debugger.
  return hex2dec(@_);
} # h2d


#######################################################################
#
# Read in a file and redact any commented sections.
#
# Currently this will handle "//", "/* ... */" on a single line, "/*" alone
# on a line up to and including the next occurance of "*/", plus more than one
# occurance of "/* ... */ ... /* ... */" on a single line. There may be some
# weird cases that it does not handle.
#
sub read_file_and_redact_comments {
  my $file = shift;
  local $_;
  # Oh no!! Slanted toothpick syndrome! :-)
  my $slc  = "\/\/"; # Single line comment marker.
  my $mlcs = "/\\*"; # Multiline comment start marker.
  my $mlce = "\\*/"; # Multiline comment end marker.
  my $in_mlc = 0;    # In-multiline-comment flag.
  my (@lines);
  open FILE, $file or die "ERROR: Cannot open input file '$file' for reading: $!\n";
  while( <FILE> ) {
    chomp; chomp;

    # Detect if we are currently in a multiline comment and we see the end of a
    # multiline comment. If so, remove up to and including the trailing multiline
    # comment marker.
    if( $in_mlc and m<${mlce}> ) {
      $in_mlc = 0;
      s<^.*?\*/><>;
    }

    # Remove any single line comment sections from the line.
    s<${slc}.*$><>;

    # Detect complete multiline comments ...and remove them. (don't be greedy)
    while( m<${mlcs}.+?${mlce}> ) {
      s<${mlcs}.*?${mlce}><>;
    }

    # If we still have a start of a multiline comment, set the flag on (possibly again).
    # It means we don't have a terminated comment set so delete to the end of the line.
    if( m<${mlcs}> ) {
      $in_mlc = 1;
      s<${mlcs}.*$><>;
    }

    # If we are still in a multiline comment and we get here, scrub the entire line.
    if( $in_mlc ) {
      $_ = "";
    }

    # Store anything that is left.
    push @lines, $_;
  }
  close FILE;
  return @lines;
} # read_file_and_redact_comments


#######################################################################
#
# Evaluate a possible hex value from a string.
#
sub eval_possible_hex {
  my $num = shift; # Either a decimal value or a hex string (the latter with or without the leading "0x").
  my $result = 0;
  if( $num =~ /^\s*0x/ ) {
    $result = hex($num);
  } elsif( $num =~ /[a-f-A-F]+/ ) {
    $result = hex("0x" . $num);
  } else {
    $result = $num;
  }
  return $result; # A decimal value
} # eval_possible_hex


#######################################################################
#
# Build an emtpy flash image.
#
sub build_empty_flash_image {
  my $outfile = shift;
  my ($crc16);
  my @words = pre_fill_flash($flash_blank_fill_hex_str); # A hex string.
  my $next_free_word = 1;
  $words[1] = $next_free_word;
  $crc16 = ($use_magic_marker) ? $MAGIC_MARKER : calc_crc16( @words[2 .. $next_free_word] );
  $words[0] = $crc16;
  write_binary( $outfile, @words );
  print "// CRC16: ", dec2hex4($crc16), "\n";
  print "// Total words: ", $next_free_word-1, "\n";
  return;
} # build_empty_flash_image


#######################################################################
#
# Dump the output into a C-array.
#
sub dump_c_array {
  my $file = shift;
  my $leader = shift;
  my $indent_spaces = shift;
  my @val_array = @_;
  open OUT, "> $file" or die "ERROR: Could not open C-array file '$file' for writing: $!\n";
  print OUT "$leader\n";
  for my $hex_str (0 .. (scalar @val_array)-2) {
    printf OUT "%0s0x%04x,\n", " " x $indent_spaces, $val_array[$hex_str];
  }
  printf OUT "%0s0x%04x );\n", " " x $indent_spaces, $val_array[-1];
  close OUT;
  return;
} # dump_c_array


#######################################################################
#
#
#
sub extract_svn_version {
  my $svn_rev = "";
  if( $SVN_Current_Revision =~ /Revision: (.+)\s*\$/ ) {
    $svn_rev = $1;
  }
  return $svn_rev;
} # extract_svn_version


#######################################################################
#######################################################################
#
# Process the command line option list.
#
sub get_options {
  my ($arg);
  while ($arg = shift(@ARGV)) {

    # See if help is asked for
    if( $arg eq "-h" ) {
      print "$usage\n";
      die "\n";
    }

    if( $arg eq "-more_help" ) {
      print "$usage\n";
      print "$extended_help\n";
      die "\n";
    }

    elsif( ($arg eq "--version") or ($arg eq "-V") ) {
      print "$script\n";
      my $svn_rev = extract_svn_version();
      print "SVN version: $svn_rev\n" if $svn_rev;
      die "\n";
    }

    elsif( ($arg eq "-opcodes") or ($arg eq "-opcode") or ($arg eq "-map") or  ($arg eq "-op") ) {
      $opcode_map_file = shift(@ARGV);
    }

    elsif( $arg eq "-dis" ) {
      $mode = $DISASSEMBLE_MODE;
    }

    elsif( $arg eq "-d" ) {
      $debug = shift(@ARGV);
    }

    elsif( $arg eq "-v" ) {
      $quiet = 0;
    }

    elsif( $arg eq "-svn" ) {
      $report_op_svn = 1;
    }

    elsif( $arg eq "-no_svn" ) {
      $report_op_svn = 0;
    }

    elsif( ($arg eq "-s") or ($arg eq "-stars") ) {
      $star_labels = shift(@ARGV);
    }

    elsif( ($arg eq "-f") or ($arg eq "-fill") ) {
      $user_flash_blank_fill = dec2hex4(eval_possible_hex(shift(@ARGV)));
    }

    elsif( $arg eq "-dl" ) {
      $disable_flash_limit = 1;
    }

    elsif( $arg eq "-ns" ) {
      $no_step_numbers = 1;
    }

    elsif( $arg eq "-o" ) {
      $outfile = shift(@ARGV);
    }

    # Undocumented debug hook to zero the last 4 words.
    elsif( $arg eq "-04" ) {
      $zero_last_4 = 1;
    }

    # Undocumented debug hook to generate a "universal" CRC.
    elsif( $arg eq "-magic" ) {
      $use_magic_marker = 1;
    }

    # Undocumented debug hook to generate an empty flash image.
    elsif( $arg eq "-empty" ) {
      $build_an_empty_image = 1;
    }

    # Undocumented debug hook to generate a XROM C-array.
    # Automatically disables the flash length limit since the XROM can be larger than a
    # flash image.
    elsif( ($arg eq "-c_xrom") or ($arg eq "-c") ) {
      $xrom_c_mode = 1;
      $disable_flash_limit = 1;
    }

    # Undocumented debug hook to generate a XROM binary image.
    # Automatically disables the flash length limit since the XROM can be larger than a
    # flash image.
    elsif( $arg eq "-xrom" ) {
      $xrom_bin_mode = 1;
      $disable_flash_limit = 1;
    }

    # This is typically only used for debug at the moment. It will dump the expanded tables
    # into a file of this name. Just having the file named is sufficient to trigger this mode.
    elsif( ($arg eq "-e") or ($arg eq "-expand") or ($arg eq "-syntax") ) {
      $expanded_op_file = shift(@ARGV);
    }

    # This is typically only used for debug at the moment. It will dump the escaped alpha table
    # into a file of this name. Just having the file named is sufficient to trigger this mode.
    elsif( ($arg eq "-da") or ($arg eq "-alpha") ) {
      $dump_escaped_alpha_table = shift(@ARGV);
    }

    else {
      push @files, $arg;
    }
  }

  #----------------------------------------------
  # Verify that we have an output file if we are in assembler mode.
  if( ($mode eq $ASSEMBLE_MODE) and ($outfile eq "-") and not $expanded_op_file ) {
    warn "ERROR: Must enter an output file name in assembler mode (-o SomeFileName).\n";
    die  "       Enter '$script_name -h' for help.\n";
  }

  return;
} # get_options

__DATA__
# Generated by "./trunk/Linux/calc opcodes > svn_1333.op"
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
0x0112  cmd [SIGMA]x[^2]y
0x0113  cmd [SIGMA]x
0x0114  cmd [SIGMA]x[^2]
0x0115  cmd [SIGMA]y
0x0116  cmd [SIGMA]y[^2]
0x0117  cmd [SIGMA]xy
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
0x0130  cmd LinF
0x0131  cmd ExpF
0x0132  cmd PowerF
0x0133  cmd LogF
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
0x0149  cmd DENANY
0x014a  cmd DENFIX
0x014b  cmd DENFAC
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
0x0161  cmd D.MY
0x0162  cmd Y.MD
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
0x017c  cmd XEQUSR
0x017d  cmd [infinity]?
0x017e  cmd NaN?
0x017f  cmd SPEC?
0x0180  cmd PRIME?
0x0181  cmd INT?
0x0182  cmd FP?
0x0183  cmd EVEN?
0x0184  cmd ODD?
0x0185  cmd ENTRY?
0x0186  cmd TICKS
0x0187  cmd BATT
0x0188  cmd SETEUR
0x0189  cmd SETUK
0x018a  cmd SETUSA
0x018b  cmd SETIND
0x018c  cmd SETCHN
0x018d  cmd SLVQ
0x018e  cmd NEXTP
0x018f  cmd XEQ[alpha]
0x0190  cmd GTO[alpha]
0x0191  cmd RCFRG
0x0192  cmd RCFST
0x0193  cmd SAVE
0x0194  cmd LOAD
0x0195  cmd RM?
0x0196  cmd SLOW
0x0197  cmd FAST
0x0198  cmd SENDP
0x0199  cmd SENDR
0x019a  cmd SENDA
0x019b  cmd RECV
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
0x026b  cmd WDAY
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
0x0277  cmd YEAR
0x0278  cmd MONTH
0x0279  cmd DAY
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
0x0500  cmd [cmplx]FP
0x0504  cmd [cmplx]IP
0x0505  cmd [cmplx]ABS
0x0506  cmd [cmplx]ROUND
0x0507  cmd [cmplx]SIGN
0x0508  cmd [cmplx]LN
0x0509  cmd [cmplx]e[^x]
0x050a  cmd [cmplx][sqrt]
0x050b  cmd [cmplx]1/x
0x050c  cmd [cmplx](-1)[^x]
0x050d  cmd [cmplx]LOG[sub-1][sub-0]
0x050e  cmd [cmplx]LOG[sub-2]
0x050f  cmd [cmplx]2[^x]
0x0510  cmd [cmplx]10[^x]
0x0511  cmd [cmplx]LN1+x
0x0512  cmd [cmplx]e[^x]-1
0x0513  cmd [cmplx]W
0x0514  cmd [cmplx]W[^-1]
0x0515  cmd [cmplx]x[^2]
0x0516  cmd [cmplx]CUBE
0x0517  cmd [cmplx]CUBERT
0x0518  cmd [cmplx]FIB
0x051f  cmd [cmplx]SIN
0x0520  cmd [cmplx]COS
0x0521  cmd [cmplx]TAN
0x0522  cmd [cmplx]ASIN
0x0523  cmd [cmplx]ACOS
0x0524  cmd [cmplx]ATAN
0x0525  cmd [cmplx]SINC
0x0526  cmd [cmplx]SINH
0x0527  cmd [cmplx]COSH
0x0528  cmd [cmplx]TANH
0x0529  cmd [cmplx]ASINH
0x052a  cmd [cmplx]ACOSH
0x052b  cmd [cmplx]ATANH
0x052c  cmd [cmplx]x!
0x052d  cmd [cmplx][GAMMA]
0x052e  cmd [cmplx]LN[GAMMA]
0x0535  cmd [cmplx]+/-
0x0536  cmd [cmplx]CONJ
0x0600  cmd [cmplx]y[^x]
0x0601  cmd [cmplx]+
0x0602  cmd [cmplx]-
0x0603  cmd [cmplx][times]
0x0604  cmd [cmplx]/
0x0606  cmd [cmplx]LOGx
0x060a  cmd [cmplx][beta]
0x060b  cmd [cmplx]LN[beta]
0x060d  cmd [cmplx]COMB
0x060e  cmd [cmplx]PERM
0x0611  cmd [cmplx]||
0x0612  cmd [cmplx]AGM
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
0x2000  cmd # a
0x2001  cmd # a[sub-0]
0x2002  cmd # a[sub-m][terra]
0x2003  cmd # a[terra][sol]
0x2004  cmd # c
0x2005  cmd # c[sub-1]
0x2006  cmd # c[sub-2]
0x2007  cmd # e
0x2008  cmd # eE
0x2009  cmd # F
0x200a  cmd # F[alpha]
0x200b  cmd # F[delta]
0x200c  cmd # g
0x200d  cmd # G
0x200e  cmd # G[sub-0]
0x200f  cmd # Gc
0x2010  cmd # g[sub-e]
0x2011  cmd # GM
0x2012  cmd # h
0x2013  cmd # [h-bar]
0x2014  cmd # k
0x2015  cmd # l[sub-p]
0x2016  cmd # m[sub-e]
0x2017  cmd # m[sub-e]c[^2]
0x2018  cmd # M[sub-m]
0x2019  cmd # m[sub-n]
0x201a  cmd # m[sub-n]c[^2]
0x201b  cmd # m[sub-p]
0x201c  cmd # M[sub-p]
0x201d  cmd # m[sub-p]c[^2]
0x201e  cmd # m[sub-u]
0x201f  cmd # m[sub-u]c[^2]
0x2020  cmd # m[sub-mu]
0x2021  cmd # m[sub-mu]c[^2]
0x2022  cmd # M[sol]
0x2023  cmd # M[terra]
0x2024  cmd # N[sub-A]
0x2025  cmd # NaN
0x2026  cmd # p[sub-0]
0x2027  cmd # q[sub-p]
0x2028  cmd # R
0x2029  cmd # r[sub-e]
0x202a  cmd # R[sub-k]
0x202b  cmd # R[sub-m]
0x202c  cmd # R[sub-infinity]
0x202d  cmd # R[sol]
0x202e  cmd # R[terra]
0x202f  cmd # Sa
0x2030  cmd # Sb
0x2031  cmd # Se[^2]
0x2032  cmd # Se'[^2]
0x2033  cmd # Sf[^-1]
0x2034  cmd # T[sub-0]
0x2035  cmd # t[sub-p]
0x2036  cmd # T[sub-p]
0x2037  cmd # V[sub-m]
0x2038  cmd # Z[sub-0]
0x2039  cmd # [alpha]
0x203a  cmd # [gamma]EM
0x203b  cmd # [gamma][sub-p]
0x203c  cmd # [epsilon][sub-0]
0x203d  cmd # [lambda][sub-c]
0x203e  cmd # [lambda][sub-c][sub-n]
0x203f  cmd # [lambda][sub-c][sub-p]
0x2040  cmd # [mu][sub-0]
0x2041  cmd # [mu][sub-B]
0x2042  cmd # [mu][sub-e]
0x2043  cmd # [mu][sub-n]
0x2044  cmd # [mu][sub-p]
0x2045  cmd # [mu][sub-u]
0x2046  cmd # [mu][sub-mu]
0x2047  cmd # [pi]
0x2048  cmd # [sigma][sub-B]
0x2049  cmd # [PHI]
0x204a  cmd # [PHI][sub-0]
0x204b  cmd # [omega]
0x204c  cmd # [infinity]
0x2100  cmd [cmplx]# a
0x2101  cmd [cmplx]# a[sub-0]
0x2102  cmd [cmplx]# a[sub-m][terra]
0x2103  cmd [cmplx]# a[terra][sol]
0x2104  cmd [cmplx]# c
0x2105  cmd [cmplx]# c[sub-1]
0x2106  cmd [cmplx]# c[sub-2]
0x2107  cmd [cmplx]# e
0x2108  cmd [cmplx]# eE
0x2109  cmd [cmplx]# F
0x210a  cmd [cmplx]# F[alpha]
0x210b  cmd [cmplx]# F[delta]
0x210c  cmd [cmplx]# g
0x210d  cmd [cmplx]# G
0x210e  cmd [cmplx]# G[sub-0]
0x210f  cmd [cmplx]# Gc
0x2110  cmd [cmplx]# g[sub-e]
0x2111  cmd [cmplx]# GM
0x2112  cmd [cmplx]# h
0x2113  cmd [cmplx]# [h-bar]
0x2114  cmd [cmplx]# k
0x2115  cmd [cmplx]# l[sub-p]
0x2116  cmd [cmplx]# m[sub-e]
0x2117  cmd [cmplx]# m[sub-e]c[^2]
0x2118  cmd [cmplx]# M[sub-m]
0x2119  cmd [cmplx]# m[sub-n]
0x211a  cmd [cmplx]# m[sub-n]c[^2]
0x211b  cmd [cmplx]# m[sub-p]
0x211c  cmd [cmplx]# M[sub-p]
0x211d  cmd [cmplx]# m[sub-p]c[^2]
0x211e  cmd [cmplx]# m[sub-u]
0x211f  cmd [cmplx]# m[sub-u]c[^2]
0x2120  cmd [cmplx]# m[sub-mu]
0x2121  cmd [cmplx]# m[sub-mu]c[^2]
0x2122  cmd [cmplx]# M[sol]
0x2123  cmd [cmplx]# M[terra]
0x2124  cmd [cmplx]# N[sub-A]
0x2125  cmd [cmplx]# NaN
0x2126  cmd [cmplx]# p[sub-0]
0x2127  cmd [cmplx]# q[sub-p]
0x2128  cmd [cmplx]# R
0x2129  cmd [cmplx]# r[sub-e]
0x212a  cmd [cmplx]# R[sub-k]
0x212b  cmd [cmplx]# R[sub-m]
0x212c  cmd [cmplx]# R[sub-infinity]
0x212d  cmd [cmplx]# R[sol]
0x212e  cmd [cmplx]# R[terra]
0x212f  cmd [cmplx]# Sa
0x2130  cmd [cmplx]# Sb
0x2131  cmd [cmplx]# Se[^2]
0x2132  cmd [cmplx]# Se'[^2]
0x2133  cmd [cmplx]# Sf[^-1]
0x2134  cmd [cmplx]# T[sub-0]
0x2135  cmd [cmplx]# t[sub-p]
0x2136  cmd [cmplx]# T[sub-p]
0x2137  cmd [cmplx]# V[sub-m]
0x2138  cmd [cmplx]# Z[sub-0]
0x2139  cmd [cmplx]# [alpha]
0x213a  cmd [cmplx]# [gamma]EM
0x213b  cmd [cmplx]# [gamma][sub-p]
0x213c  cmd [cmplx]# [epsilon][sub-0]
0x213d  cmd [cmplx]# [lambda][sub-c]
0x213e  cmd [cmplx]# [lambda][sub-c][sub-n]
0x213f  cmd [cmplx]# [lambda][sub-c][sub-p]
0x2140  cmd [cmplx]# [mu][sub-0]
0x2141  cmd [cmplx]# [mu][sub-B]
0x2142  cmd [cmplx]# [mu][sub-e]
0x2143  cmd [cmplx]# [mu][sub-n]
0x2144  cmd [cmplx]# [mu][sub-p]
0x2145  cmd [cmplx]# [mu][sub-u]
0x2146  cmd [cmplx]# [mu][sub-mu]
0x2147  cmd [cmplx]# [pi]
0x2148  cmd [cmplx]# [sigma][sub-B]
0x2149  cmd [cmplx]# [PHI]
0x214a  cmd [cmplx]# [PHI][sub-0]
0x214b  cmd [cmplx]# [omega]
0x214c  cmd [cmplx]# [infinity]
0x2200  cmd iC 0
0x2200  arg iC  max=0,indirect
0x2201  cmd iC 1
0x2202  cmd iC 3.75312
0x2203  cmd iC 13.76707
0x2204  cmd iC 0.19883478
0x2205  cmd iC 0.99364934
0x2206  cmd iC 0.01129389
0x2207  cmd iC 0.98238587
0x2208  cmd iC 0.05395845
0x2209  cmd iC 0.76281312
0x220a  cmd iC 0.09238421
0x220b  cmd iC 0.54734932
0x220c  cmd iC 0.11867175
0x220d  cmd iC 0.29697204
0x220e  cmd iC 0.13835773
0x220f  cmd iC 0.95630418
0x2210  cmd iC 0.06591106
0x2211  cmd iC 0.03999630
0x2212  cmd iC 0.85804795
0x2213  cmd iC 0.19884013
0x2214  cmd iC 0.07377747
0x2215  cmd iC 0.66900995
0x2216  cmd iC 0.29186994
0x2217  cmd iC 0.18516678
0x2218  cmd iC 0.48437399
0x2219  cmd iC 0.25345911
0x221a  cmd iC 0.12958509
0x221b  cmd iC 0.13685673
0x221c  cmd iC 0.27931721
0x221d  cmd iC 0.19533070
0x2300  arg ERR max=19,indirect
0x2400  arg STO max=112,indirect,stack
0x2500  arg STO+  max=112,indirect,stack
0x2600  arg STO-  max=112,indirect,stack
0x2700  arg STO[times]  max=112,indirect,stack
0x2800  arg STO/  max=112,indirect,stack
0x2900  arg STO[v]  max=112,indirect,stack
0x2a00  arg STO[^]  max=112,indirect,stack
0x2b00  arg RCL max=112,indirect,stack
0x2c00  arg RCL+  max=112,indirect,stack
0x2d00  arg RCL-  max=112,indirect,stack
0x2e00  arg RCL[times]  max=112,indirect,stack
0x2f00  arg RCL/  max=112,indirect,stack
0x3000  arg RCL[v]  max=112,indirect,stack
0x3100  arg RCL[^]  max=112,indirect,stack
0x3200  arg x[<->]  max=112,indirect,stack
0x3300  arg [cmplx]STO  max=111,indirect,stack,complex
0x3400  arg [cmplx]STO+ max=111,indirect,stack,complex
0x3500  arg [cmplx]STO- max=111,indirect,stack,complex
0x3600  arg [cmplx]STO[times] max=111,indirect,stack,complex
0x3700  arg [cmplx]STO/ max=111,indirect,stack,complex
0x3800  arg [cmplx]RCL  max=111,indirect,stack,complex
0x3900  arg [cmplx]RCL+ max=111,indirect,stack,complex
0x3a00  arg [cmplx]RCL- max=111,indirect,stack,complex
0x3b00  arg [cmplx]RCL[times] max=111,indirect,stack,complex
0x3c00  arg [cmplx]RCL/ max=111,indirect,stack,complex
0x3d00  arg [cmplx]x[<->] max=111,indirect,stack,complex
0x3e00  arg VIEW  max=112,indirect,stack
0x3f00  arg STOS  max=97,indirect
0x4000  arg RCLS  max=97,indirect
0x4101  cmd [alpha] [x-bar]
0x4102  cmd [alpha] [y-bar]
0x4103  cmd [alpha] [sqrt]
0x4104  cmd [alpha] [integral]
0x4105  cmd [alpha] [degree]
0x4106  cmd [alpha] [space]
0x4107  cmd [alpha] [grad]
0x4108  cmd [alpha] [+/-]
0x4109  cmd [alpha] [<=]
0x410a  cmd [alpha] [>=]
0x410b  cmd [alpha] [!=]
0x410c  cmd [alpha] [euro]
0x410d  cmd [alpha] [->]
0x410e  cmd [alpha] [<-]
0x410f  cmd [alpha] [v]
0x4110  cmd [alpha] [^]
0x4111  cmd [alpha] [f-shift]
0x4112  cmd [alpha] [g-shift]
0x4113  cmd [alpha] [h-shift]
0x4114  cmd [alpha] [cmplx]
0x4115  cmd [alpha] [O-slash]
0x4116  cmd [alpha] [o-slash]
0x4117  cmd [alpha] [<->]
0x4118  cmd [alpha] [sz]
0x4119  cmd [alpha] [x-hat]
0x411a  cmd [alpha] [y-hat]
0x411b  cmd [alpha] [sub-m]
0x411c  cmd [alpha] [times]
0x411d  cmd [alpha] [approx]
0x411e  cmd [alpha] [pound]
0x411f  cmd [alpha] [yen]
0x4120  cmd [alpha] [space]
0x4121  cmd [alpha] !
0x4122  cmd [alpha] "
0x4123  cmd [alpha] #
0x4124  cmd [alpha] $
0x4125  cmd [alpha] %
0x4126  cmd [alpha] &
0x4127  cmd [alpha] '
0x4128  cmd [alpha] (
0x4129  cmd [alpha] )
0x412a  cmd [alpha] *
0x412b  cmd [alpha] +
0x412c  cmd [alpha] ,
0x412d  cmd [alpha] -
0x412e  cmd [alpha] .
0x412f  cmd [alpha] /
0x4130  cmd [alpha] 0
0x4131  cmd [alpha] 1
0x4132  cmd [alpha] 2
0x4133  cmd [alpha] 3
0x4134  cmd [alpha] 4
0x4135  cmd [alpha] 5
0x4136  cmd [alpha] 6
0x4137  cmd [alpha] 7
0x4138  cmd [alpha] 8
0x4139  cmd [alpha] 9
0x413a  cmd [alpha] :
0x413b  cmd [alpha] ;
0x413c  cmd [alpha] <
0x413d  cmd [alpha] =
0x413e  cmd [alpha] >
0x413f  cmd [alpha] ?
0x4140  cmd [alpha] @
0x4141  cmd [alpha] A
0x4142  cmd [alpha] B
0x4143  cmd [alpha] C
0x4144  cmd [alpha] D
0x4145  cmd [alpha] E
0x4146  cmd [alpha] F
0x4147  cmd [alpha] G
0x4148  cmd [alpha] H
0x4149  cmd [alpha] I
0x414a  cmd [alpha] J
0x414b  cmd [alpha] K
0x414c  cmd [alpha] L
0x414d  cmd [alpha] M
0x414e  cmd [alpha] N
0x414f  cmd [alpha] O
0x4150  cmd [alpha] P
0x4151  cmd [alpha] Q
0x4152  cmd [alpha] R
0x4153  cmd [alpha] S
0x4154  cmd [alpha] T
0x4155  cmd [alpha] U
0x4156  cmd [alpha] V
0x4157  cmd [alpha] W
0x4158  cmd [alpha] X
0x4159  cmd [alpha] Y
0x415a  cmd [alpha] Z
0x415b  cmd [alpha] [
0x415c  cmd [alpha] \
0x415d  cmd [alpha] ]
0x415e  cmd [alpha] ^
0x415f  cmd [alpha] _
0x4160  cmd [alpha] `
0x4161  cmd [alpha] a
0x4162  cmd [alpha] b
0x4163  cmd [alpha] c
0x4164  cmd [alpha] d
0x4165  cmd [alpha] e
0x4166  cmd [alpha] f
0x4167  cmd [alpha] g
0x4168  cmd [alpha] h
0x4169  cmd [alpha] i
0x416a  cmd [alpha] j
0x416b  cmd [alpha] k
0x416c  cmd [alpha] l
0x416d  cmd [alpha] m
0x416e  cmd [alpha] n
0x416f  cmd [alpha] o
0x4170  cmd [alpha] p
0x4171  cmd [alpha] q
0x4172  cmd [alpha] r
0x4173  cmd [alpha] s
0x4174  cmd [alpha] t
0x4175  cmd [alpha] u
0x4176  cmd [alpha] v
0x4177  cmd [alpha] w
0x4178  cmd [alpha] x
0x4179  cmd [alpha] y
0x417a  cmd [alpha] z
0x417b  cmd [alpha] {
0x417c  cmd [alpha] |
0x417d  cmd [alpha] }
0x417e  cmd [alpha] ~
0x417f  cmd [alpha] [del]
0x4180  cmd [alpha] [ALPHA]
0x4181  cmd [alpha] [BETA]
0x4182  cmd [alpha] [GAMMA]
0x4183  cmd [alpha] [DELTA]
0x4184  cmd [alpha] [EPSILON]
0x4185  cmd [alpha] [ZETA]
0x4186  cmd [alpha] [ETA]
0x4187  cmd [alpha] [THETA]
0x4188  cmd [alpha] [IOTA]
0x4189  cmd [alpha] [KAPPA]
0x418a  cmd [alpha] [LAMBDA]
0x418b  cmd [alpha] [MU]
0x418c  cmd [alpha] [NU]
0x418d  cmd [alpha] [XI]
0x418e  cmd [alpha] [sol]
0x418f  cmd [alpha] [PI]
0x4190  cmd [alpha] [RHO]
0x4191  cmd [alpha] [SIGMA]
0x4192  cmd [alpha] [TAU]
0x4193  cmd [alpha] [UPSILON]
0x4194  cmd [alpha] [PHI]
0x4195  cmd [alpha] [CHI]
0x4196  cmd [alpha] [PSI]
0x4197  cmd [alpha] [OMEGA]
0x4198  cmd [alpha] [sub-B]
0x4199  cmd [alpha] [sub-mu]
0x419a  cmd [alpha] [^2]
0x419b  cmd [alpha] [sub-infinity]
0x419c  cmd [alpha] [^x]
0x419d  cmd [alpha] [^-1]
0x419e  cmd [alpha] [h-bar]
0x419f  cmd [alpha] [infinity]
0x41a0  cmd [alpha] [alpha]
0x41a1  cmd [alpha] [beta]
0x41a2  cmd [alpha] [gamma]
0x41a3  cmd [alpha] [delta]
0x41a4  cmd [alpha] [epsilon]
0x41a5  cmd [alpha] [zeta]
0x41a6  cmd [alpha] [eta]
0x41a7  cmd [alpha] [theta]
0x41a8  cmd [alpha] [iota]
0x41a9  cmd [alpha] [kappa]
0x41aa  cmd [alpha] [lambda]
0x41ab  cmd [alpha] [mu]
0x41ac  cmd [alpha] [nu]
0x41ad  cmd [alpha] [xi]
0x41ae  cmd [alpha] [terra]
0x41af  cmd [alpha] [pi]
0x41b0  cmd [alpha] [rho]
0x41b1  cmd [alpha] [sigma]
0x41b2  cmd [alpha] [tau]
0x41b3  cmd [alpha] [upsilon]
0x41b4  cmd [alpha] [phi]
0x41b5  cmd [alpha] [chi]
0x41b6  cmd [alpha] [psi]
0x41b7  cmd [alpha] [omega]
0x41b8  cmd [alpha] [sub-0]
0x41b9  cmd [alpha] [sub-1]
0x41ba  cmd [alpha] [sub-2]
0x41bb  cmd [alpha] [sub-c]
0x41bc  cmd [alpha] [sub-e]
0x41bd  cmd [alpha] [sub-n]
0x41be  cmd [alpha] [sub-p]
0x41bf  cmd [alpha] [sub-u]
0x41c0  cmd [alpha] [A-grave]
0x41c1  cmd [alpha] [A-acute]
0x41c2  cmd [alpha] [A-tilde]
0x41c3  cmd [alpha] [A-umlaut]
0x41c4  cmd [alpha] [A-dot]
0x41c5  cmd [alpha] [C-acute]
0x41c6  cmd [alpha] [C-hook]
0x41c7  cmd [alpha] [C-cedilla]
0x41c8  cmd [alpha] [E-grave]
0x41c9  cmd [alpha] [E-acute]
0x41ca  cmd [alpha] [E-filde]
0x41cb  cmd [alpha] [E-trema]
0x41cc  cmd [alpha] [I-grave]
0x41cd  cmd [alpha] [I-acute]
0x41ce  cmd [alpha] [I-tilde]
0x41cf  cmd [alpha] [I-trema]
0x41d0  cmd [alpha] [N-tilde]
0x41d1  cmd [alpha] [O-grave]
0x41d2  cmd [alpha] [O-acute]
0x41d3  cmd [alpha] [O-tilde]
0x41d4  cmd [alpha] [O-umlaut]
0x41d5  cmd [alpha] [R-hook]
0x41d6  cmd [alpha] [S-hook]
0x41d7  cmd [alpha] [sub-A]
0x41d8  cmd [alpha] [U-grave]
0x41d9  cmd [alpha] [U-acute]
0x41da  cmd [alpha] [U-tilde]
0x41db  cmd [alpha] [U-umlaut]
0x41dc  cmd [alpha] [U-dot]
0x41dd  cmd [alpha] [Y-acute]
0x41de  cmd [alpha] [Y-trema]
0x41df  cmd [alpha] [Z-hook]
0x41e0  cmd [alpha] [a-grave]
0x41e1  cmd [alpha] [a-acute]
0x41e2  cmd [alpha] [a-tilde]
0x41e3  cmd [alpha] [a-umlaut]
0x41e4  cmd [alpha] [a-dot]
0x41e5  cmd [alpha] [c-acute]
0x41e6  cmd [alpha] [c-hook]
0x41e7  cmd [alpha] [c-cedilla]
0x41e8  cmd [alpha] [e-grave]
0x41e9  cmd [alpha] [e-acute]
0x41ea  cmd [alpha] [e-tilde]
0x41eb  cmd [alpha] [e-trema]
0x41ec  cmd [alpha] [i-grave]
0x41ed  cmd [alpha] [i-acute]
0x41ee  cmd [alpha] [i-tilde]
0x41ef  cmd [alpha] [i-trema]
0x41f0  cmd [alpha] [n-tilde]
0x41f1  cmd [alpha] [o-grave]
0x41f2  cmd [alpha] [o-acute]
0x41f3  cmd [alpha] [o-tilde]
0x41f4  cmd [alpha] [o-umlaut]
0x41f5  cmd [alpha] [r-hook]
0x41f6  cmd [alpha] [s-hook]
0x41f7  cmd [alpha] [sub-k]
0x41f8  cmd [alpha] [u-grave]
0x41f9  cmd [alpha] [u-acute]
0x41fa  cmd [alpha] [u-tilde]
0x41fb  cmd [alpha] [u-umlaut]
0x41fc  cmd [alpha] [u-dot]
0x41fd  cmd [alpha] [y-acute]
0x41fe  cmd [alpha] [y-trema]
0x41ff  cmd [alpha] [z-hook]
0x4200  arg [alpha]RC#  max=112,indirect,stack
0x4300  arg [alpha]STO  max=112,indirect,stack
0x4400  arg [alpha]RCL  max=112,indirect,stack
0x4500  arg [alpha]IP max=112,indirect,stack
0x4600  arg [alpha]RL max=31,indirect
0x4700  arg [alpha]RR max=31,indirect
0x4800  arg [alpha]SL max=32,indirect
0x4900  arg [alpha]SR max=32,indirect
0x4a00  arg x=? max=112,indirect,stack
0x4b00  arg x[!=]?  max=112,indirect,stack
0x4c00  arg x[approx]?  max=112,indirect,stack
0x4d00  arg x<? max=112,indirect,stack
0x4e00  arg x[<=]?  max=112,indirect,stack
0x4f00  arg x>? max=112,indirect,stack
0x5000  arg x[>=]?  max=112,indirect,stack
0x5100  arg [cmplx]x=?  max=111,indirect,stack,complex
0x5200  arg [cmplx]x[!=]? max=111,indirect,stack,complex
0x5300  arg SKIP  max=100,indirect
0x5400  arg BACK  max=100,indirect
0x5500  arg DSE max=112,indirect,stack
0x5600  arg ISG max=112,indirect,stack
0x5700  arg DSZ max=112,indirect,stack
0x5800  arg ISZ max=112,indirect,stack
0x5900  arg DEC max=112,indirect,stack
0x5a00  arg INC max=112,indirect,stack
0x5b00  arg LBL max=104
0x5c00  arg LBL?  max=104,indirect
0x5d00  arg XEQ max=104,indirect
0x5e00  arg GTO max=104,indirect
0x5f00  arg [SIGMA] max=104,indirect
0x6000  arg [PI]  max=104,indirect
0x6100  arg SLV max=104,indirect
0x6200  arg f'(x) max=104,indirect
0x6300  arg f"(x) max=104,indirect
0x6400  arg INT max=104,indirect
0x6500  arg ALL max=12,indirect
0x6600  arg FIX max=12,indirect
0x6700  arg SCI max=12,indirect
0x6800  arg ENG max=12,indirect
0x6900  arg DISP  max=12,indirect
0x6a00  arg SF  max=104,indirect
0x6b00  arg CF  max=104,indirect
0x6c00  arg FF  max=104,indirect
0x6d00  arg FS? max=104,indirect
0x6e00  arg FC? max=104,indirect
0x6f00  arg FS?C  max=104,indirect
0x7000  arg FS?S  max=104,indirect
0x7100  arg FS?F  max=104,indirect
0x7200  arg FC?C  max=104,indirect
0x7300  arg FC?S  max=104,indirect
0x7400  arg FC?F  max=104,indirect
0x7500  arg WSIZE max=65,indirect
0x7600  arg RL  max=64,indirect
0x7700  arg RR  max=64,indirect
0x7800  arg RLC max=65,indirect
0x7900  arg RRC max=65,indirect
0x7a00  arg SL  max=65,indirect
0x7b00  arg SR  max=65,indirect
0x7c00  arg ASR max=65,indirect
0x7d00  arg SB  max=64,indirect
0x7e00  arg CB  max=64,indirect
0x7f00  arg FB  max=64,indirect
0x8000  arg BS? max=64,indirect
0x8100  arg BC? max=64,indirect
0x8200  arg MASKL max=65,indirect
0x8300  arg MASKR max=65,indirect
0x8400  arg BASE  max=17,indirect
0x8500  cmd kg[->]lb
0x8501  cmd lb[->]kg
0x8502  cmd kg[->]stone
0x8503  cmd stone[->]kg
0x8504  cmd kg[->]cwt
0x8505  cmd cwt[->]kg
0x8506  cmd kg[->]s.cwt
0x8507  cmd s.cwt[->]kg
0x8508  cmd g[->]oz
0x8509  cmd oz[->]g
0x850a  cmd g[->]tr.oz
0x850b  cmd tr.oz[->]g
0x850c  cmd l[->]galUK
0x850d  cmd galUK[->]l
0x850e  cmd l[->]galUS
0x850f  cmd galUS[->]l
0x8510  cmd l[->]cft
0x8511  cmd cft[->]l
0x8512  cmd ml[->]flozUK
0x8513  cmd flozUK[->]ml
0x8514  cmd ml[->]flozUS
0x8515  cmd flozUS[->]ml
0x8516  cmd cm[->]inches
0x8517  cmd inches[->]cm
0x8518  cmd m[->]fathom
0x8519  cmd fathom[->]m
0x851a  cmd m[->]feet
0x851b  cmd feet[->]m
0x851c  cmd m[->]yards
0x851d  cmd yards[->]m
0x851e  cmd km[->]miles
0x851f  cmd miles[->]km
0x8520  cmd km[->]l.y.
0x8521  cmd l.y.[->]km
0x8522  cmd km[->]pc
0x8523  cmd pc[->]km
0x8524  cmd km[->]AU
0x8525  cmd AU[->]km
0x8526  cmd km[->]nmi
0x8527  cmd nmi[->]km
0x8528  cmd ha[->]acres
0x8529  cmd acres[->]ha
0x852a  cmd N[->]lbf
0x852b  cmd lbf[->]N
0x852c  cmd J[->]Btu
0x852d  cmd Btu[->]J
0x852e  cmd J[->]cal
0x852f  cmd cal[->]J
0x8530  cmd J[->]kWh
0x8531  cmd kWh[->]J
0x8532  cmd Pa[->]atm
0x8533  cmd atm[->]Pa
0x8534  cmd Pa[->]bar
0x8535  cmd bar[->]Pa
0x8536  cmd Pa[->]mmHg
0x8537  cmd mmHg[->]Pa
0x8538  cmd Pa[->]psi
0x8539  cmd psi[->]Pa
0x853a  cmd Pa[->]inHg
0x853b  cmd inHg[->]Pa
0x853c  cmd Pa[->]torr
0x853d  cmd torr[->]Pa
0x853e  cmd W[->]hpUK
0x853f  cmd hpUK[->]W
0x8540  cmd W[->]hp
0x8541  cmd hp[->]W
0x8542  cmd W[->]PS(hp)
0x8543  cmd PS(hp)[->]W
0x8544  cmd W[->]HP[sub-e]
0x8545  cmd HP[sub-e][->]W
0x8546  cmd t[->]tons
0x8547  cmd tons[->]t
0x8548  cmd t[->]s.tons
0x8549  cmd s.tons[->]t
0x8600  arg SLVI  max=91,indirect
0x8700  arg SLVS  max=91,indirect
0x8800  arg PSE max=100,indirect
0x8900  arg KEY?  max=112,indirect,stack
0x8a00  arg [alpha]XEQ  max=112,indirect,stack
0x8b00  arg [alpha]GTO  max=112,indirect,stack
0x8c00  arg PRCL  max=9,indirect
0x8d00  arg PSTO  max=9,indirect
0x8e00  arg P[<->]  max=9,indirect
0x8f00  arg RCF max=112,indirect,stack
0x9000  arg RCF+  max=112,indirect,stack
0x9100  arg RCF-  max=112,indirect,stack
0x9200  arg RCF[times]  max=112,indirect,stack
0x9300  arg RCF/  max=112,indirect,stack
0x9400  arg RCF[v]  max=112,indirect,stack
0x9500  arg RCF[^]  max=112,indirect,stack
0x9600  arg [cmplx]RCF  max=111,indirect,stack,complex
0x9700  arg [cmplx]RCF+ max=111,indirect,stack,complex
0x9800  arg [cmplx]RCF- max=111,indirect,stack,complex
0x9900  arg [cmplx]RCF[times] max=111,indirect,stack,complex
0x9a00  arg [cmplx]RCF/ max=111,indirect,stack,complex
0x9b00  arg S.L max=100,indirect
0x9c00  arg S.R max=100,indirect
0x9d00  arg VW[alpha]+  max=112,indirect,stack
0x9e00  arg RM  max=7,indirect
0x9f00  arg STOM  max=112,indirect,stack
0xa000  arg RCLM  max=112,indirect,stack
