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
my $Description = "Symbolic Preprocessor for the WP 34S Assembler.";
#
my $SVN_Current_Revision = '$Rev$';
#
#-----------------------------------------------------------------------
#
# Language:         Perl script
#
#-----------------------------------------------------------------------

use strict;
use File::Basename;

# ---------------------------------------------------------------------

my $debug = 0;
my $stderr2stdout = 0;
my (%LBLs, %targets, %branches, @branches, @steps);

# Allow these to be over-ridden using environment variables.
my $v3_mode = (exists $ENV{WP34S_PP_V3}) ? $ENV{WP34S_PP_V3} : 0;
my $renumber_steps = (exists $ENV{WP34S_PP_RENUM}) ? $ENV{WP34S_PP_RENUM} : 0;
my $step = (exists $ENV{WP34S_PP_RENUM_START}) ? $ENV{WP34S_PP_RENUM_START} : 1;

my $step_digits = 3; # Default to older style 3-digit step numbers.
my $override_step_digits = 0;

my $DEFAULT_MAX_JMP_OFFSET = 99;
my $MAX_JMP_OFFSET = $DEFAULT_MAX_JMP_OFFSET; # Maximum offset for a BACK/SKIP/BSRF/BSRB statement.

my $MAX_LABEL_NUM = 99;  # Maximum label number
my $last_label_used = $MAX_LABEL_NUM+1; # It will be decremented before being used.
my $DEFAULT_NEXT_LABEL_DIR = -1;
my $next_label_dir = $DEFAULT_NEXT_LABEL_DIR ;

my $branch_digits = 2; # Default to V2
my $label_digits = 2;
my $label_digits_range = "2";

my $OPEN_COMMENT = "/*";  # These are only used because my editor's syntax based chroma-coding sucks.
my $CLOSE_COMMENT = "*/"; # These are only used because my editor's syntax based chroma-coding sucks.
my $DOUBLE_QUOTE = "\"";

# The labels used as targets for program branches are only allowed to be made from
# the following characters. Additionally, they are always terminated by a double colon.
my $label_spec = "A-Za-z0-9_";
my $longest_label = 0; # Recorded for pretty print purposes.

my $NUM_TARGET_LABEL_COLONS = 2;

my $xrom_mode = 0;
my %xlbl = (); # keys LBL with ASCII label, and entry WORD number.
my $DEFAULT_XLBL_FILE = "xrom_labels.h";
my $xlbl_file = $DEFAULT_XLBL_FILE;
my $DEFAULT_XOFFSET_FILE = "xrom_targets.c";
my $xoffset_file = $DEFAULT_XOFFSET_FILE;
my $DEFAULT_XOFFSET_LEADER = "\t";
my $xoffset_leader = $DEFAULT_XOFFSET_LEADER;

my @files;
my @cmd_args;

# ANSI colour codes.
my $ansi_normal           = "\e[0m";
my $ansi_red_bg           = "\e[41;33;1m";
my $ansi_green_bg         = "\e[42;33;1m";
my $ansi_rev_green_bg     = "\e[42;1;7;33;1m";
my $ansi_rev_red_bg       = "\e[41;1;7;33;1m";
my $ansi_rev_blue_bg      = "\e[47;1;7;34;1m";
my $ansi_rev_cyan_bg      = "\e[30;46m";

my $DEFAULT_USE_ANSI_COLOUR = (exists $ENV{WP34S_ASM_COLOUR})
                            ? $ENV{WP34S_ASM_COLOUR}
                            : 0;
my $use_ansi_colour       = $DEFAULT_USE_ANSI_COLOUR;

my $ENUM_OP = "OP";
my $ENUM_NUM_WORDS = "WORDS";
my $ENUM_STEP = "STEP";
my $ENUM_PRECOMMENT = "PRECMNT";
my $ENUM_POSTCOMMENT = "POSTCMNT";

my $prt_step_num = exists $ENV{WP34S_PP_PRT_STEP}
                 ? $ENV{WP34S_PP_PRT_STEP}
                 : 1;
my $show_catalogue = 0;
my $show_targets = 0;

my $max_skip = 0;
my $max_back = 0;
my $max_bsrb = 0;
my $max_bsrf = 0;

# These instructions are eligible for use with the symbolic label. They all can take a symbolic label
# which will eventualy be replaced with a numeric LBL. The value in the hash means:
#   1 = key is used as-is.
#   2 = key will be surrounded by "[" and "]" when used in some cases.
#
# XXX This section is a work-in-progress.
my %LBLd_ops = (
        'GTO' => 1,
        'INT' => 1,
        'SLV' => 1,
        'XEQ' => 1,
        'integral' => 2,
        'PI' => 2,
        'SIGMA' => 2,
# XXX Not realy sure what to do about these. Refine them later.
        'f\"\(x\)' => 1,
        'f"(x)' => 1,
        'f\'\(x\)' => 1,
        'f\'(x)' => 1,
        );

# Build a table that will translate any O/S reported to what we should be using.
# Note: At one point I thought I might need to convert cygwin to Windows type
#       so I thought I need a translation table. I worked around that but
#       will keep the table anyway because it allows me to detect O/S that
#       are "approved" with the script.
my %useable_OS = ("linux"   => "linux",
                  "MSWin32" => "MSWin32",
                  "MSDOS"   => "MSDOS",
                  "cygwin"  => "cygwin",
                  "Darwin"  => "Darwin",
                  "darwin"  => "darwin",
                  "MacOS"   => "MacOS",
                 );

if( exists $useable_OS{$^O} ) {
  fileparse_set_fstype($useable_OS{$^O});

  # Check for OS-specific options to enable or disable.
  if ($^O =~ /MSDOS/) {
    $use_ansi_colour = 0; # Supposedly, terminal emulators under MS-DOS have a hard time running
                          # in full ANSI mode. Sigh! So, no matter what the user tries, make it
                          # difficult to turn ANSI codes on.

  # Conversely, if the OS is linux, turn colour on.
  } elsif ($^O =~ /linux/i) {
    $use_ansi_colour = 1;
  }

} else {
  # If we get here, the set file system type function has NOT been run and we may not
  # be able to locate "relative" file such as the standard external opcode table or the
  # PP script. If worst comes to worse, we can specify both of these from the command line.
  print "// WARNING: Unrecognized O/S ($^O). Some features may not work as expected.\n";
}


# ---------------------------------------------------------------------

# Automatically extract the name of the executable that was used. Also extract
# the directory so we can potentially source other information from this location.
my $script_executable = $0;
fileparse_set_fstype($^O);
my ($script_name, $script_dir, $script_suffix) = fileparse($script_executable);

my $script  = "$script_name  - $Description";
my $usage = <<EOM;

$script

      The output is intended to be fed into the WP 34S assembler (wp34s_asm.pl).
Usage:
   $script_name src_file [src_file2...] [-cat] [-ns] [-m max_offset] [-h] > out

Parameters:
   src_file         One or more WP 34S program source files. Conventionally, "wp34s"
                    is used as the filename extension.
   -cat             Generate a LBL catalogue.
   -ns              Suppress generation of step numbers.
   -m max_offset    Change the maximum offset limit.  [default: $DEFAULT_MAX_JMP_OFFSET]
   -v3              Enable v3 mode enhancements.
   -renum           Renumber step lines sequentially. Only has meaning in v3 mode.
   -xrom            Run in XROM mode. NOTE: This is normally only used by the internal
                    tool-chain. Not intended for use by the end-user!
   -h               This help script.

Examples:
  \$ $script_name vector_sym.wp34s > vector_pp.wp34s
  - Preprocess a source file to produce a new source listing with all branches and
    symbolic targets resolved, and all strings encoded to the appropriate [alpha]
    instruction.

  \$ $script_name vector_sym.wp34s gc_sym.wp34s > src.wp34s
  - Preprocess multiple source files.

  \$ $script_name vector_sym.wp34s -v3 > src.wp34s
  - Enable v3 mode.

Notes:
  1) All symbolic labels must contain only alphanumerics and/or underscores
     and must be terminated by a double colon. They must be at least 2 characters
     long and must not be only 2 decimal digits ('00::' is illegal but '_00::' is
     acceptable).
  2) You can use JMP instead of BACK, SKIP, or GTO and GSB instead of BSRB, BSRF, or XEQ.
     The tool will decide which direction the target label is and will substitute the
     appropriate instruction if the target offset is with the maximum of $DEFAULT_MAX_JMP_OFFSET
     steps. If the target offset if greater than $DEFAULT_MAX_JMP_OFFSET steps, the tool
     will insert a symbolic LBL and will GTO or XEQ to that label. There is no limit to
     the text-length of the labels. Alpha text strings can be entered within double quotes.

     Example program (modified excerpt from 'library/vectors.wp34s'):
          Vector::  LBL'VEC'
                    SSIZE8
                    CL[alpha]
                    "Vector"  // Use double quotes to enter alpha strings
                    [alpha]VIEW
          Done::    STOP
                    JMP Done  // Use JMP instead of BACK and/or SKIP

                    LBL A     // Execute absolute the subroutine
                    XEQ Vabs
                    JMP Done

          /* VAB
            Vector absolute value (modulus).  Vector in Y,Z,T.  Returns
            the length in X (dropping the vector).  The original vector
            is saved in L,I,J.
          */
          Vabs::    LBL'VAB'
                    XEQ Vsub  // Reference a purely symbolic subroutine name
                    x[<->] T
                    STO J
                    DROP
                    [cmplx]STO L
                    [cmplx]DROP
                    RTN

          Vsub::    ENTER[^] // Tool will automatically inject a LBL here
                    x[^2]
                    RCL Z
                    RCL[times] T
                    +
                    RCL T
                    RCL[times] T
                    +
                    [sqrt]
                    RTN
EOM

#######################################################################

get_options();

if ($override_step_digits) {
  $step_digits = $override_step_digits;
} elsif ($v3_mode) {
  $step_digits = 4;
}

debug_msg(this_function((caller(0))[3]), "MAX_JMP_OFFSET = $MAX_JMP_OFFSET") if $debug;

my (@src, @src_db);
foreach my $file (@files) {
  push @src, load_cleaned_source($file);
}

my @end_groups; # Groups of lines delimited by EDN statements.
if ($v3_mode and not $xrom_mode) {
  # Split the lines into an array of array references based on END.
  @end_groups = split_END(@src);
} else {
  # If v3 mode is not on or we are in XROM mode, fake it by pushing the reference
  # to the single, unified source onto the group. This way only one source will be
  # processed and it will behave as it did originally.
  #
  # For XROM mode, are looking to avoid forcing an END be added to the file. However,
  # the limitation is that we can only ever process a single file in this mode. Not
  # an issue for XROM.
  @end_groups = ();
  push @end_groups, \@src;
}


print "// $script_name: Command line: ", join ("  ", @cmd_args), "\n";
print "// $script_name: Source file(s): ", join (", ", @files), "\n";
print "// $script_name: Preprocessor revision: $SVN_Current_Revision \n";

# Process each END-delimited program in its own "space". This allows the reuse of local LBLs.
foreach (@end_groups) {
  my @src_lines = @{$_};
  my $length = scalar @src_lines; # Debug use only.
  debug_msg(this_function((caller(0))[3]), "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++") if $debug and $v3_mode;
  debug_msg(this_function((caller(0))[3]), "Processing new END group. Contains $length lines.") if $debug and $v3_mode;
  initialize_tables(@src_lines);
  preprocessor();
  display_steps("");
  display_max_branch_offsets();
  show_LBLs() if $show_catalogue;
  show_targets() if $show_targets;
  show_src_db() if exists $ENV{WP34S_PP} and ($ENV{WP34S_PP} =~ /SSDB/i);
}


# If we are in XROM mode, write a header file with any XLBL labels we find. If
# no XLBL labels are found, write the header file anyway since some other C-file is
# probably trying to include it. Also write a C GTO/XEQ offset file as well.
if ($xrom_mode) {
  my $str = "";

  # Print out the XROM C header file.
  open XLBL, "> $xlbl_file" or die_msg(this_function((caller(0))[3]), "Cannot open XLBL header file '$xlbl_file' for writing: $!");
  print XLBL "\n";
  print XLBL "// XROM XLBL source step addresses.\n";
  print XLBL "\n";
  print XLBL "#ifndef XLBL_LABELS_H\n";
  print XLBL "#define XLBL_LABELS_H\n\n";
  my $longest_xlabel = 0;
  for my $xlabel (sort keys %xlbl) {
    $longest_xlabel = length $xlabel if length($xlabel) > $longest_xlabel;
  }
  for my $xlabel (sort keys %xlbl) {
    print XLBL "#define XROM_${xlabel}" . " " x ($longest_xlabel - length($xlabel) + 3) . "${xlbl{$xlabel}}\n";
  }
  print XLBL "\n";
  print XLBL "#endif // XLBL_LABELS_H\n";
  close XLBL;

  # Print out the XROM C offset file.
  open XOFFSET, "> $xoffset_file" or die_msg(this_function((caller(0))[3]), "Cannot open XROM oofset file '$xoffset_file' for writing: $!");
  print XOFFSET "\n";
  print XOFFSET "// XROM virtual LBL target step addresses.\n";
  print XOFFSET "\n";
  print XOFFSET "const unsigned short int xrom_targets[] = {\n";

  # Dump it to an array so we can more easily suppress the last comma.
  my @offsets = ();
  for my $label (sort keys %LBLs) {
    my $content = get_line($ENUM_STEP, $LBLs{$label});
    push @offsets, $content;
  }
  if (scalar @offsets > 1) {
    for (my $k = 0; $k < (scalar @offsets) - 1; $k++) {
      $str = sprintf "%0s%0s, // virtual LBL %02d", $xoffset_leader, $offsets[$k], $k;
      print XOFFSET "$str\n";
    }
  }
  $str = sprintf "%0s%0s  // virtual LBL %02d", $xoffset_leader, $offsets[-1], (scalar @offsets) - 1;
  print XOFFSET "$str\n";
  print XOFFSET "}; // xrom_targets[]\n";
  print XOFFSET "\n";
  close XOFFSET;
}




#######################################################################
#
# Start of subroutine suite.
#
#######################################################################
#
# Execute the preprocessor steps on the lines in the global @lines array.
#
sub preprocessor {
  display_steps("// ") if $debug; # Initial debug image
  process_double_quotes();
  build_step_numbers();
  if ($xrom_mode) {
    extract_xlbls();
    build_step_numbers();
  }
  process_expressions();
  extract_labels();           # Look for existing "LBL \d{$label_digits_range}" or "LBL [A-Z]"opcodes
  extract_targets();          # Look for "SomeLabel::"
  extract_branches();         # Look for "(BACK|SKIP|JMP|BSRB|BSRF|GSB) SomeLabel"
  insert_synthetic_labels();  # Add synthetic symbolic label for any %LBLs without a counterpart in %targets
  build_step_numbers();
  check_consistency();
  populate_branch_array();
  extract_LBLd_opcodes();  # Look for "(GTO|XEQ|SLV|etc) SomeLabel"
  preprocess_synthetic_targets();
  return;
} # preprocessor


#######################################################################
#
# Split the source into groups of arrays based on the END opcode. Return
# a reference to the array of array references. If the last statement is not
# an END, one will be synthetically added there.
#
sub split_END {
  my @src = @_;
  my @END_groups;
  local $_;

  # Make sure there is an END as the last instruction. If not, add one.
  unless( $src[-1] =~ /(^|\s+)END($|\s+)/) {
    push @src, "END";
    warn_msg(this_function((caller(0))[3]), "Missing terminal \"END\". Appending after last statement in source.");
  }

  # Scan through the source cutting it up into groups delimited by "END".
  my @group = ();
  my $line;
  my $line_num = 1; # Debug use only
  while ($line = shift @src) {
    push @group, $line;

    # Detect an END instruction and split off source group.
    if( $line =~ /(^|\s+)END($|\s+)/ ) {
      my @this_group = @group;
      push @END_groups, \@this_group;
      @group = ();
      debug_msg(this_function((caller(0))[3]), "Found and split off an END group at line $line_num.") if $debug;
    }
    $line_num++;
  }
  return @END_groups;
} # split_END


#######################################################################
#
# Setup the tables for a new run.
#
sub initialize_tables {
  my @src_lines = @_;
  %LBLs = ();
  %targets = ();
  %branches = ();
  @branches = ();
  @steps = ();
  @src_db = ();
  src2src_db(@src_lines);
  return;
} # initialize_tables


#######################################################################
#
# Build the step numbers depending on op-code sizes
#
sub build_step_numbers {
  my $step_num = 1;

  for (my $step = 1; $step <= scalar(@src_db); $step++) {
    my $opsize = get_line($ENUM_NUM_WORDS, $step);

    # Update database with modified step number
    put_line($ENUM_STEP, $step, $step_num);
    $step_num += $opsize;
  }
  return;
} # build_step_numbers


#######################################################################
#
# Process double quoted statements into sufficient occurances of the 2
# alpha mechanisms. For example, given "Vector01", decompose this into:
#
# [alpha]'Vec'
# [alpha]'tor'
# [alpha] 0
# [alpha] 1
#
# Recall that escaped alphas (eg: [sigma]) count for only a single character.
#
sub process_double_quotes {
  local $_;
  debug_msg(this_function((caller(0))[3]), "**** Starting function") if $debug;
  my ($substring, $num_chars, $string, $line, $new_line, $label);
  my $step = 1;
  while ($step <= scalar(@src_db)) {
    $line = get_line($ENUM_OP, $step);
    $line =~ s/^\s*\d{0,3}:{0,1}//; # Remove any line numbers.
    if( $line =~ /^\s*([$label_spec]{2,}:{$NUM_TARGET_LABEL_COLONS})/ ) {
      $label = $1 . " ";
      $line =~ s/^\s*([$label_spec]{2,}):{$NUM_TARGET_LABEL_COLONS}//;
    } else {
      $label = "";
    }
    # By not being greedy with this search, we can support embedded double quotes.
    if ($line =~ /^\s*\"(.+)\"/) {
      my $string = $1;
      my $is_first = 1;
      while ($string) {
        my $fmt_step = format_step($step);
        ($substring, $num_chars, $string) = extract_substring($string, $step);
        debug_msg(this_function((caller(0))[3]), "Extracted $num_chars characters composed of substring '$substring' from line '$line'") if $debug;
        if ($num_chars == 3) {
          $new_line = "${label}[alpha]'${substring}'";
        } elsif ($num_chars == 2) {
          if ($debug) {
            print "ERROR: " . this_function((caller(0))[3]) . ": Why did we return a 2-character string at step $fmt_step! String: '$string'\n";
            show_state(__LINE__);
            die_msg(this_function((caller(0))[3]), "Cannot continue...");
          } else {
            die_msg(this_function((caller(0))[3]), "Why did we return a 2-character string at step $fmt_step! String: '$string'");
          }
        } else {
          $new_line = "${label}[alpha] ${substring}";
        }

        # Replace the line we are processing if this is the first time through. Thereafter
        # insert anymore lines.
        if ($is_first) {
          $is_first = 0;
          debug_msg(this_function((caller(0))[3]), "Replacing step '$fmt_step' with '$new_line'") if $debug;
          put_line($ENUM_OP, $step, $new_line . " // $line");
          adjust_words_per_step($step, $new_line);
          $label = "";
        } else {
          debug_msg(this_function((caller(0))[3]), "Inserting step '$fmt_step' with '$new_line'") if $debug;
          insert_step($new_line, $step);
        }
        $step++;
      }
    } else {
      $step++;
    }
  }
  debug_msg(this_function((caller(0))[3]), "Popped out at step '$step'.") if $debug;
  display_steps("// ") if $debug;
  return;
} # process_double_quotes


#######################################################################
#
# Process expressions within instructions. The expressions must be surrounded by an
# outer set of brackets. The inside expression will be evaluated using 'eval'.
#
# "xIN ((1 << 3) | 2)"  =>  "xIN 010 // (1 << 3) | 2"
#
sub process_expressions {
  local $_;
  debug_msg(this_function((caller(0))[3]), "**** Starting function") if $debug;
  my ($line, $new_line, $label);
  my $step = 1;
  while( $step <= scalar(@src_db) ) {
    $line = get_line($ENUM_OP, $step);
    $line =~ s/^\s*\d{0,3}:{0,1}//;
    if( $line =~ /^\s*([$label_spec]{2,}:{$NUM_TARGET_LABEL_COLONS})/ ) {
      $label = $1 . " ";
      $line =~ s/^\s*([$label_spec]{2,}):{$NUM_TARGET_LABEL_COLONS}//;
    } else {
      $label = "";
    }
    # We need to be greedy with this search!
    if( $line =~ /(^\s*[^\(]+)\s+\((.+)\)/ ) {

      my $the_rest = $1;
      my $expression = $2;
      my $evaluated = eval $expression;

      my $new_line = sprintf "%0s%0s %0d // %0s", $label, $the_rest, $evaluated, $expression;
      put_line($ENUM_OP, $step, $new_line);
      debug_msg(this_function((caller(0))[3]), "Evaluated '$expression' to get '$evaluated'") if $debug;
    }
    $step++;
  }
  display_steps("// ") if $debug;
  return;
} # process_expressions


#######################################################################
#
# Process branches and attempt to resolve them.
#
sub preprocess_synthetic_targets {
  debug_msg(this_function((caller(0))[3]), "**** Starting function") if $debug;
  local $_;
  my ($current_step);

  debug_msg(this_function((caller(0))[3]), "Prior to processing...") if $debug;
  show_state(__LINE__) if $debug;
  while( $current_step = shift @branches ) {
    debug_msg(this_function((caller(0))[3]), "Processing branch at step $current_step...") if $debug;
    my ($target_label, $target_step);
    my $fmt_cur_step = format_step($current_step);

    # See if the instruction is one of the LBL'd ones like XEQ, INT, etc.
    my $line = get_line($ENUM_OP, $current_step);
    my ($op, $label);

    $line =~ s/^\s*\d{0,3}:{0,1}//;
    $line =~ s/^\s+//;
    $line =~ s/\s+$//;

    if( $line =~ /^\s*[$label_spec]{2,}:{$NUM_TARGET_LABEL_COLONS}\s+(\S+)\s+(\S+)/ ) { # Line with target label
      $op = $1;
      $label = $2;
    } elsif( $line =~ /^\s*(\S+)\s+(\S+)/ ) { # Line without target label
      $op = $1;
      $label = $2;
    } else {
      if( $debug ) {
        print "ERROR: " . this_function((caller(0))[3]) . ": Cannot parse the line at step $current_step! Line: '$line'\n";
        show_state(__LINE__);
        die_msg(this_function((caller(0))[3]), "Cannot continue...");
      } else {
        die_msg(this_function((caller(0))[3]), "Cannot parse the line at step $current_step! Line: '$line'");
      }
    }

    # Need to exclude any square bracketing because this confuses the hash search. The
    # square brackets can't be in the hash because they are then treated as search sets
    # in regex's.
    my $tmp_op = $op;
    $tmp_op =~ s/\[//;
    $tmp_op =~ s/\]//;
    if( exists $LBLd_ops{$tmp_op} ) {
      $op =~ s/\\//g; # Remove any escape characters in the hash template.
      my $target_step = search_targets($label);
      my $target_LBL = search_LBLs($target_step);
      $line = replace_with_LBLd_target($target_LBL, $line, $op);
      put_line($ENUM_OP, $current_step, $line);
      print 1 if 0; # Breakpoint

    } else {
      # From the step number, get the label which is the target.
      if( exists $branches{$fmt_cur_step} ) {
        $target_label = $branches{$fmt_cur_step};
      } else {
        if( $debug ) {
          print "ERROR: " . this_function((caller(0))[3]) . ": Cannot locate current step (${fmt_cur_step}) in \%branches.\n";
          show_state(__LINE__);
          die_msg(this_function((caller(0))[3]), "Cannot continue...");
        } else {
          die_msg(this_function((caller(0))[3]), "Cannot locate current step (${fmt_cur_step}) in \%branches.");
        }
      }

      # From the target label, get the target step number.
      if( exists $targets{$target_label} ) {
        $target_step = $targets{$target_label};
      } else {
        if( $debug ) {
          print "ERROR: " . this_function((caller(0))[3]) . ": Cannot locate target label '${target_label}' in \%targets.\n";
          show_state(__LINE__);
          die_msg(this_function((caller(0))[3]), "Cannot continue...");
        } else {
          die_msg(this_function((caller(0))[3]), "Cannot locate target label '${target_label}' in \%targets.");
        }
      }

      my $offset = calculate_offset($target_step, $current_step);
      # The offset is outside the range so we can use a SKIP/BACK/BSRB/BSRF opcode. We need to use a local label.
      if( abs($offset) > $MAX_JMP_OFFSET ) {
        my ($new_label_num, $op_replacement);

        debug_msg(this_function((caller(0))[3]), "Offset exceeds maximum (abs(${offset}) > ${MAX_JMP_OFFSET}). target_step = $target_step, current_step = $current_step") if $debug;
        ($new_label_num, $current_step) = add_LBL_if_required($target_step, $offset, $current_step);
        if ($line =~ /JMP|GTO|SKIP|BACK/) {
          $op_replacement = "GTO";
        } elsif ($line =~ /GSB|XEQ|BSRB|BSRF/) {
          $op_replacement = "XEQ";
        } else {
          if( $debug ) {
            print "ERROR: " . this_function((caller(0))[3]) . ": Unrecognized branch instruction in line: '$line'.\n";
            show_state(__LINE__);
            die_msg(this_function((caller(0))[3]), "Cannot continue...");
          } else {
            die_msg(this_function((caller(0))[3]), "Unrecognized branch instruction in line: '$line'.");
          }
        }
        $line = replace_with_LBLd_target($new_label_num, $line, $op_replacement);
        put_line($ENUM_OP, $current_step, $line);
        print $_ if 0; # Breakpoint!
      } else {
        # At this moment, the offset is within range so we can use a SKIP/BACK/BSRB/BSRF opcode.
        # Note that this may change as a result of LBL injections so this may not always stay
        # within range.

        # The calculation is asymmetric in that the forward direction already accounts for
        # and offset of 1, so we must remove that offset.
        $offset-- if $offset > 0; # Adjust the SKIP offsets by 1.
        $line =~ s/\s+$//;
        $line = replace_with_branch($offset, $line);
        put_line($ENUM_OP, $current_step, $line);
      }
    }
    debug_msg(this_function((caller(0))[3]), "Next branch...") if $debug;
    show_state(__LINE__) if $debug;
  }
  debug_msg(this_function((caller(0))[3]), "Done...") if $debug;
  return;
} # assemble


#######################################################################
#
# Check if a LBL already exists satifying the criteria. If it does, simply return it.
# If a LBL does not exist at that location, get the next eligible LBL name and insert
# it at the target location. Fix up all tables that change as a result of this.
# Addiitonally, since an instruction has been inserted, we need to increment the curren
# step as well.
#
sub add_LBL_if_required {
  my $target_step = shift;  # Where the label is headed
  my $offset = shift;       # Direction (only used for SKIP fixups).
  my $current_step = shift; # (Only used for SKIP fixups, ie: if $offset is negative.)
  my ($new_label_num);

  # Is there already a LBL that we can take advantage of?
  $new_label_num = is_existing_label($target_step); # Returns <0 if no appropriate label already in existance.
  debug_msg(this_function((caller(0))[3]), "is_existing_label(${target_step}) => ${new_label_num}") if $debug;

  if ($xrom_mode) {
    if ($new_label_num < 0) {
      $new_label_num = get_next_label($target_step); # Reserve it for the next step.
    } else {
      debug_msg(this_function((caller(0))[3]), "Reusing virtual LBL '$new_label_num' at step '$target_step' in XROM mode") if $debug;
    }
  } if ($new_label_num < 0) {
    # We need to inject a label at the target.
    $new_label_num = get_next_label($target_step+1); # Reserve it for the next step.
    debug_msg(this_function((caller(0))[3]), "Adding new label '$new_label_num' at step '$target_step'.") if $debug;
    insert_step("LBL $new_label_num", $target_step);
    adjust_labels_used($target_step);
    adjust_targets($target_step);
    adjust_branches($target_step);
    adjust_branch_array($target_step);
    adjust_xlbls($target_step) if $xrom_mode;

    # If the label is required to be inserted before this step, the step is going to be incremented
    # to compensate for this.
    if( $offset < 0 ) {
      $current_step++;
    }
  } else {
    debug_msg(this_function((caller(0))[3]), "Label number '$new_label_num' already exists at step '$target_step' -- reusing it.") if $debug;
  }
  return ($new_label_num, $current_step);
} # add_LBL_if_required


#######################################################################
#
# Load the source. The result is a "clean" file with no comments or
# blank lines.
#
sub load_cleaned_source {
  my $file = shift;
  my @src = read_file($file);
  @src = redact_comments(@src);
  @src = remove_blank_lines(@src);
  return @src;
} # load_cleaned_source


#######################################################################
#
# Extract any numberic labels (could be expanded to quoted labels as well!) and
# save them along with the line they are "referring" to. The definition used here
# is that a LBL refers to first following line that is not a LBL. For example:
#
#   034      LBL 45
#   035      +
#
# Would result in the followin being recorded in the %LBLs hash:
#
#     45 => 035
#
sub extract_labels {
  debug_msg(this_function((caller(0))[3]), "**** Starting function") if $debug;
  for( my $step = 1; $step <= scalar(@src_db); $step++ ) {
    my $fmt_step = format_step($step+1); # Because we are pointing at the active step *after* the label.
    my $line = get_line($ENUM_OP, $step);
    if( $line =~ /LBL\s+(\d+)/ or $line =~ /LBL\s+([A-Z])([^A-Z]|$)/ ) {
      my $LBL = $1;
      my $fmt_LBL = format_LBL($LBL);
      if( not exists $LBLs{$fmt_LBL} ) {
        $LBLs{$fmt_LBL} = $fmt_step;
        debug_msg(this_function((caller(0))[3]), "Found user 'LBL $fmt_LBL' at step $fmt_step.") if $debug;
      } else {
        die_msg(this_function((caller(0))[3]), "Cannot support multiple labels with the same name. LBL $fmt_LBL, seen at step $fmt_step, was already seen at step ${LBLs{$fmt_LBL}}");
      }
    }
  }
  return;
} # extract_labels


#######################################################################
#
# Extract the label targets and the step number they are referring to. For
# example:
#
#   024:  DoItAgain5::   RCL 04
#   025:                 STO+ 23
#
# Would result in the following being recorded:
#
#     DoItAgain5 => 024
#
sub extract_targets {
  debug_msg(this_function((caller(0))[3]), "**** Starting function") if $debug;
  for( my $step = 1; $step <= scalar(@src_db); $step++ ) {
    if( get_line($ENUM_OP, $step) =~ /^\s*\d{0,3}:{0,1}\s*([$label_spec]{2,}):{$NUM_TARGET_LABEL_COLONS}\s+/ ) {
      my $label = $1;
      adjust_longest_label_length($label);
      if( not exists $targets{$label} ) {
        my $fmt_step = format_step($step);
        $targets{$label} = $fmt_step;
        debug_msg(this_function((caller(0))[3]), "Found target label ('$label') at step $fmt_step.") if $debug;
      } else {
        die_msg(this_function((caller(0))[3]), "Cannot support multiple target labels at different steps. Target label '$label' aleady seen at step ${targets{$label}}");
      }
    }
  }
  return;
} # extract_targets


#######################################################################
#
# Extract the branches and the step where they occur. For example:
#
#   001: Oomph::  +
#   002           [times]
#   003           JMP Label4
#   004 Label1::  RCL 00
#   005           x[>=]1?
#   006           BACK Label1
#   007           STO 98
#   008           ****LBL 98
#   009           SKIP Oomph
#
# Would result in the following being recorded:
#
#     003 => Label4
#     006 => Label1
#     009 => Oomph
#
# Note that the type of opcode (or pseudo opcode, in the case of JMP) does *not*
# need to be recorded as this is deduced by whether the offset was in the positive
# or negative direction. Hence, though the "SKIP Oomph" is shown in the wrong
# direction, it will eventually be resolved as a BACK opcode at the proper time.
#
sub extract_branches {
  debug_msg(this_function((caller(0))[3]), "**** Starting function") if $debug;
  for( my $step = 1; $step <= scalar(@src_db); $step++ ) {
    my $fmt_step = format_step($step);
    if( get_line($ENUM_OP, $step) =~ /(BACK|SKIP|JMP|BSRB|BSRF|GSB)\s+([$label_spec]{2,})/ ) {
      my $label = $2;
      if( $label =~ /^\d{$label_digits_range}$/ ) {
        # This is a "hard" target. We want to convert these to "soft" ones.
        debug_msg(this_function((caller(0))[3]), "Found 'hard' branch at step $fmt_step with label '$label'.") if $debug;
      } else {
        if( not exists $branches{$fmt_step} ) {
          $branches{$fmt_step} = $label;
          debug_msg(this_function((caller(0))[3]), "Found branch at step $fmt_step with label '$label'.") if $debug;
        } else {
          die_msg(this_function((caller(0))[3]), "Cannot support multiple target labels per step. Target label '$label' was already seen at step ${targets{$fmt_step}}");
        }
      }
    }
  }
  show_state(__LINE__) if $debug;
  return;
} # extract_branches


#######################################################################
#
# Extract existing GTO and XEQ instructions. For example:
#
#     010           XEQ Franktown
#     ...
#     023           LBL 98
#     024           GTO Smooch
#     025           -
#     026           f'(x) 98
#
# Would result in the following occuring:
#
#   1) For each LBL'd opcode, a search would be made of the %branches to
#
# being recorded to %LBLd_ops:
#
#
# Would result in the following being recorded in %branches:
#
#     003 => Label4
#     006 => Label1
#     009 => Oomph
#
#
#
sub extract_LBLd_opcodes {
  debug_msg(this_function((caller(0))[3]), "**** Starting function") if $debug;
  for( my $step = 1; $step <= scalar(@src_db); $step++ ) {
    my $line = get_line($ENUM_OP, $step);
    my $fmt_step = format_step($step);

    # Sort through any operation that can target a LBL. These include XEQ, INT, etc.
    for my $LBLd_op (sort keys %LBLd_ops) {
      # Does the opcode from the LBL'd type exist here?
      if( $line =~ /${LBLd_op}/ ) {
        # We have one of the LBL-eligible op-codes.
        if( $LBLd_ops{$LBLd_op} > 1 ) {
          $LBLd_op = "\\[${LBLd_op}\\]";
        }

        # Does it have an eligible label (ie: not a 'hard' label);
        if( $line =~ /${LBLd_op}\s+([$label_spec]{2,})/ or
            $line =~ /\[${LBLd_op}\]\s+([$label_spec]{2,})/ ) {
          # Extract the target label.
          my $label = $1;

          # Check out if the label is a hard label or symbolic one.
          if( $label =~ /^\d{$label_digits_range}$|^[A-Z]{1}$/ ) {
            debug_msg(this_function((caller(0))[3]), "Skipping 'hard' LBL'd opcode at step $fmt_step with LBL '$label': '$line'.") if $debug;

          # Does the target label exist already?
          } elsif( exists $targets{$label} ) {
            debug_msg(this_function((caller(0))[3]), "Found LBL'd opcode at step $fmt_step with label '$label': '$line'.") if $debug;

            # Get the target label.
            my $target_step = $targets{$label};
            my ($assigned_label);
            debug_msg(this_function((caller(0))[3]), "fmt_step = $fmt_step, targets{label} = $target_step") if $debug;

            # Search the existing LBLs for a match of the target step. If a LBL at the target step is found, we
            # can reuse it for this purposes of this LBL.
            my $found = 0;
            for my $LBL (sort keys %LBLs) {
              # Make sure the LBL is treated in the comtext of a number by adding 0 to it. This
              # will allow the use of the '==' instead of the numerically inexact 'eq'.
              if( eval($LBLs{$LBL}+0) == $target_step ) {
                $found = 1;
                $assigned_label = $LBL;
                last;
              }
            }

            # If a LBL at the target address does not exist, we need to insert one. This will have the ripple
            # effect of moving everything below it down one step.
            if( not $found ) {
              my $offset = calculate_offset($target_step, $step);
              debug_msg(this_function((caller(0))[3]), "Label not found at target step '$target_step'. Offset to target is $offset.") if $debug > 2;
              ($assigned_label, $step) = add_LBL_if_required($target_step, $offset, $step);
              $fmt_step = format_step($step);
            } else {
              debug_msg(this_function((caller(0))[3]), "Found label '$assigned_label' for tarteg step '$target_step'") if $debug > 2;
            }

            if( not exists $branches{$fmt_step} ) {
              if( $debug ) {
                debug_msg(this_function((caller(0))[3]), "Prior to inserting label '$label' into \%branches:");
                show_branches();
              }
              $branches{$fmt_step} = $label;
              if( $debug ) {
                debug_msg(this_function((caller(0))[3]), "After inserting label '$label' into \%branches:");
                show_branches();
                debug_msg(this_function((caller(0))[3]), "Prior to inserting step '$fmt_step' into \@branches:");
                show_branches_remaining();
              }
              insert_into_branch_array($step);
            } else {
              if(1) {
                if( $debug ) {
                  debug_msg(this_function((caller(0))[3]), "Locate step '$fmt_step' already in \%branches.");
                  show_state(__LINE__);
                }
              } else {
                if( $debug ) {
                  print "ERROR: " . this_function((caller(0))[3]) . ": Locate step '$fmt_step' already in \%branches.\n";
                  show_state(__LINE__);
                  die_msg(this_function((caller(0))[3]), "Cannot continue...");
                } else {
                  die_msg(this_function((caller(0))[3]), "Locate step '$fmt_step' already in \%branches.");
                }
              }
            }

          # The required target label does not exist so it is a user error in the source code.
          } else {
            if( $debug ) {
              print "ERROR: " . this_function((caller(0))[3]) . ": No target label exists for line '$line' at step '$fmt_step'\n";
              show_state(__LINE__);
              die_msg(this_function((caller(0))[3]), "Cannot continue...");
            } else {
              die_msg(this_function((caller(0))[3]), "No target label exists for line '$line' at step '$fmt_step");
            }
          }
          last;
        } else {
         debug_msg(this_function((caller(0))[3]), "Line is eligible for LBL '$LBLd_op' but does not have an eligible label: line '$line' at step '$fmt_step'") if $debug > 1;
        }
        next;
      } else {
        debug_msg(this_function((caller(0))[3]), "Line is not eligible for LBL '$LBLd_op': line '$line' at step '$fmt_step'") if $debug > 3;
      }
    }
  }
  show_state(__LINE__) if $debug;
  return;
} # extract_LBLd_opcodes


#######################################################################
#
# Add synthetic symbolic label for any %LBLs without a counterpart in %targets.
#
sub insert_synthetic_labels {
  debug_msg(this_function((caller(0))[3]), "**** Starting function") if $debug;
  # Process the existing LBLs.
  for my $LBL (sort keys %LBLs) {
    my $label_step = $LBLs{$LBL};
    search_and_insert_synthetic_label("LBL", $LBL, $label_step);
  }

  # Add synthetic labels for %branches with labels and fix up the orignal statement.
  for my $step (sort keys %branches) {
    my $fmt_step = format_step($step);
    my $target_label = $branches{$step};
    if( $target_label =~ /^\d{$label_digits_range}$/ ) {
      my $line = get_line($ENUM_OP, $step);
      if( $line =~ /BACK\s+(\d{$label_digits_range})/ ) {
        my $offset = $1;
        my $target_step = format_step($step - $offset);
        my $new_label = search_and_insert_synthetic_label("BACK", $offset, $target_step);
        debug_msg(this_function((caller(0))[3]), "Replacing: '$line'") if $debug;
        $line =~ s/BACK\s+(\d{$label_digits_range})/BACK ${new_label} \/\/ $1/;
        put_line($ENUM_OP, $step, $line);
        debug_msg(this_function((caller(0))[3]), "With:      '$line'") if $debug;
        replace_branch($step, $new_label);
      } elsif( $line =~ /SKIP\s+(\d{$label_digits_range})/ ) {
        my $offset = $1;
        my $target_step = format_step($step + $offset + 1);
        my $new_label = search_and_insert_synthetic_label("SKIP", $offset, $target_step);
        debug_msg(this_function((caller(0))[3]), "Replacing: '$line'") if $debug;
        $line =~ s/SKIP\s+(\d{$label_digits_range})/SKIP ${new_label} \/\/ $1/;
        put_line($ENUM_OP, $step, $line);
        debug_msg(this_function((caller(0))[3]), "With:      '$line'") if $debug;
        replace_branch($step, $new_label);
      } else {
        die_msg(this_function((caller(0))[3]), "Cannot parse line at step '$step': '$line'.");
      }
    }
  }
  debug_msg(this_function((caller(0))[3]), "Done.") if $debug;
  show_state(__LINE__) if $debug;
  return;
} # insert_synthetic_labels


#######################################################################
#
#
#
sub replace_branch {
  my $step = shift;
  my $new_label = shift;
  my $fmt_step = format_step($step);
  if( exists $branches{$fmt_step} ) {
    debug_msg(this_function((caller(0))[3]), "Replacing branch: '${branches{$fmt_step}}'") if $debug;
    $branches{$fmt_step} = $new_label;
    debug_msg(this_function((caller(0))[3]), "With:             '${branches{$fmt_step}}'") if $debug;
  } else {
    if( $debug ) {
      print "ERROR: " . this_function((caller(0))[3]) . ": Branch unexpectedly does not exist for step '$fmt_step'.\n";
      show_state(__LINE__);
      die_msg(this_function((caller(0))[3]), "Cannot continue...");
    } else {
      die_msg(this_function((caller(0))[3]), "Branch unexpectedly does not exist for step '$fmt_step'.");
    }
  }
  return;
} # replace_branch


#######################################################################
#
# Search for a label at specified step and add a synthetic symbolic label if none exists.
#
sub search_and_insert_synthetic_label {
  my $type = shift; # "LBL", "BACK", "SKIP", or anything from the %LBLd_ops hash.
  my $seed = shift; # Usually the LBL number of BACK/SKIP offset
  my $label_step = shift;
  my $fmt_step = format_step($label_step);
  my $assigned_label = "__oops__";
  my $found = 0;
  for my $label (sort keys %targets) {
    my $target_step = $targets{$label};
    if( eval($target_step + 0) == eval($label_step + 0) ) {
      $found = 1;
      $assigned_label = $label;
      last;
    }
  }
  if( not $found ) {
    $assigned_label = gen_unique_synthetic_label($type, $seed);
    $targets{$assigned_label} = $fmt_step;
    debug_msg(this_function((caller(0))[3]), "Inserting synthetic label ('$assigned_label') at step '$fmt_step' for '$type $seed'.") if $debug;

    # We have confirmed that no actual label exists so substitute it in.
    my $tmp_line = get_line($ENUM_OP, $label_step);
    my $msg = "Original line: '" . get_line($ENUM_OP, $label_step) . "'";
    debug_msg(this_function((caller(0))[3]), $msg) if $debug;
    if( $tmp_line =~ /^\s*\d{$step_digits}:{0,1}/ ) {
      $tmp_line =~ s/^(\s*\d{$step_digits}:{0,1})\s+(.+)/${1} ${assigned_label}:: $2/;
    } else {
      $tmp_line =~ s/^(\s*)(.+)/${1}${assigned_label}:: $2/;
    }
    $msg = "Modified line: '$tmp_line'";
    debug_msg(this_function((caller(0))[3]), $msg) if $debug;
    put_line($ENUM_OP, $label_step, $tmp_line);

  } else {
    debug_msg(this_function((caller(0))[3]), "Label ('$assigned_label') already exists at step '$fmt_step' for label '$type $seed'.") if $debug;
  }
  return ($assigned_label);
} # search_and_insert_synthetic_label


#######################################################################
#
# Generate a synthetic label name for an existing LBL or "hard" BACK/SKIP.
#
sub gen_unique_synthetic_label {
  my $type = shift;
  my $seed = shift;
  my $base_synthetic_label = "__${type}_${seed}";
  my $inc = "a";
  my $synthetic_label = $base_synthetic_label;
  adjust_longest_label_length($synthetic_label);
  while( exists $targets{$synthetic_label} ) {
    $synthetic_label = $base_synthetic_label . $inc;
    adjust_longest_label_length($synthetic_label);
    $inc++;
  }
  return $synthetic_label;
} # gen_unique_synthetic_label


#######################################################################
#
#
#
sub adjust_longest_label_length {
  my $label = shift;
  $longest_label = length($label) if length($label) > $longest_label;
  return;
} # adjust_longest_label_length


#######################################################################
#
# Check the tables for consistency.
#
sub check_consistency {
  debug_msg(this_function((caller(0))[3]), "**** Starting function") if $debug;
  debug_msg(this_function((caller(0))[3]), "Stubbed...") if $debug;
  show_state(__LINE__) if $debug;
  return;
} # check_consistency


#######################################################################
#
# Display the reconstructed steps.
#
sub display_steps {
  my $leader = shift;
  $leader = "// " if not defined $leader;
  local $_;
  foreach (reconstruct_steps($longest_label)) {
    if ($renumber_steps and /^\s*(\d{3,4})(:{0,1}.+)/ ) {
      my $line = $2;
      my $format = length($1);
      $_ = sprintf "%0${format}d%0s", $step++, $2;
    }
    if (/SKIP\s+(\d+)/) {
      $max_skip = $1 if $1 > $max_skip;
    }
    if (/BACK\s+(\d+)/) {
      $max_back = $1 if $1 > $max_back;
    }
    if (/BSRB\s+(\d+)/) {
      $max_bsrb = $1 if $1 > $max_bsrb;
    }
    if (/BSRF\s+(\d+)/) {
      $max_bsrf = $1 if $1 > $max_bsrf;
    }
    print "${leader}$_\n";
  }
  return;
} # display_steps


#######################################################################
#
# Display the reconstructed steps.
#
sub display_max_branch_offsets {
  print "// Max SKIP offset: $max_skip\n";
  print "// Max BACK offset: $max_back\n";
  print "// Max BSRB offset: $max_bsrb\n";
  print "// Max BSRF offset: $max_bsrf\n";
  return;
} # display_max_branch_offsets


#######################################################################
#
# Reconstruct the steps
#
sub reconstruct_steps {
  my $label_field_length = $longest_label+$NUM_TARGET_LABEL_COLONS;
  my ($label, $opcode, @reconstructed_steps, $step_num);

  for (my $step = 1; $step <= scalar(@src_db); $step++) {
    my $line = get_line($ENUM_OP, $step);
    my $opsize = get_line($ENUM_NUM_WORDS, $step);
    my $step_num = get_line($ENUM_STEP, $step);

    # Scrub any step number present. It will be recreated on the other end, unless otherwise requested.
    $line =~ s/^\s*\d{$step_digits}:{0,1}//;

    if ($opsize == 0) {
      $opcode = $line;
      debug_msg(this_function((caller(0))[3]), "$step:$step_num - Type 0: '$line', op-size=$opsize") if $debug > 3;
    } elsif ($line =~ /^\s*([$label_spec]{2,}:{$NUM_TARGET_LABEL_COLONS})\s+(\S+.*)/) { # Line with target label
      $label = ${1} . (" " x ($label_field_length - length($1)));
      $opcode = $2;
      debug_msg(this_function((caller(0))[3]), "$step:$step_num - Type 1: '$line', op-size=$opsize") if $debug > 3;
    } elsif ($line =~ /^\s*(\S+.*)/) { # Line without target label
      $label = " " x $label_field_length;
      $opcode = $1;
      debug_msg(this_function((caller(0))[3]), "$step:$step_num - Type 2: '$line', op-size=$opsize") if $debug > 3;
    } else {
      if ($debug) {
        print "ERROR: " . this_function((caller(0))[3]) . ": Cannot parse the line at step $step! Line: '$line'\n";
        show_state(__LINE__);
        die_msg(this_function((caller(0))[3]), "Cannot continue...");
      } else {
        die_msg(this_function((caller(0))[3]), "Cannot parse the line at step $step! Line: '$line'");
      }
    }

    # If the instruction is 0-length, it will be made a comment.
    # XXX This gets us XLBLs for free in the listing. It could be used to push the input
    #     comments through as well. Have to work on the redact_comments() function for this.
    if ($opsize == 0) {
      push @reconstructed_steps, "$line";
    # See if the user has turned off step number generation from the command line.
    } elsif ($prt_step_num) {
      push @reconstructed_steps, sprintf "%0${step_digits}d $OPEN_COMMENT %0s $CLOSE_COMMENT %0s", $step_num, $label, $opcode;
    } else {
      push @reconstructed_steps, sprintf "$OPEN_COMMENT %0s $CLOSE_COMMENT %0s", $label, $opcode;
    }
  }
  return @reconstructed_steps;
} # reconstruct_steps


#######################################################################
#
# Extract either a 3 character or a 1 character substring from the string.
# Note that a 2-character string would take up 4 bytes regardless of whether it
# was coded as "[alpha]'xy[null]'" or "[alpha] x, [alpha] y" so might as well make
# it easier on myself by leaving them as 2 single alpha steps. (Doesn't look
# as nice in the source but: too bad, so sad.)
#
sub extract_substring {
  my $string = shift;
  my $fmt_step = format_step(shift); # Used for error reporting only.
  my $num_chars = 0;
  my (@alphas);
  my $alpha = "";
  my $actual_alpha = "";
  my $org_string = $string;
  my $substring = "";
  debug_msg(this_function((caller(0))[3]), "Attempting to extract substring from '$string'") if $debug;
  while($string and ($num_chars < 3) ) {
    # Check for any escaped alphas since these need to be treated differently.
    if( $string =~ /^(\[.+?\])/ ) {
      $alpha = $1;
      $actual_alpha = $alpha;
    } elsif( $string =~ /^(.)/ ) {
      $alpha = $1;
      $actual_alpha = $alpha;

      # Substitue any spaces the user encoded into the escaped alpha version.
      if( $alpha eq " " ) {
        $alpha = "[space]";
      }
    } else {
      if( $debug ) {
        print "ERROR: " . this_function((caller(0))[3]) . ": Cannot parse the string string at step $fmt_step! String: '$org_string'\n";
        show_state(__LINE__);
        die_msg(this_function((caller(0))[3]), "Cannot continue...");
      } else {
        die_msg(this_function((caller(0))[3]), "Cannot parse the string string at step $fmt_step! String: '$org_string'");
      }
    }
    debug_msg(this_function((caller(0))[3]), "Found character '$actual_alpha' equated to '$alpha' in '$string'") if $debug;
    push @alphas, $alpha;

    # Use a different replacement function rather than a regex because the text
    # patttern for the substitution have regex control sequences in them and
    # that screws things up!
    $string = str_replace($actual_alpha, "", $string);

    $num_chars++;
  }

  # Put some back if we can't get a full 3 characters. In this case we only want to return
  # a single character.
  if( $num_chars != 3 ) {
    debug_msg(this_function((caller(0))[3]), "Could only collect $num_chars -- putting some back. Current string is '$string'.") if $debug;
    $substring = shift @alphas;
    $num_chars = 1;
    while( @alphas ) {
      $string .= shift @alphas;
    }
    debug_msg(this_function((caller(0))[3]), "Reconstituted string to '$string'.") if $debug;
  } else {
    $substring = join "", @alphas;
  }
  debug_msg(this_function((caller(0))[3]), "Extracted $num_chars character(s): '$substring'. Left with string of '$string'.") if $debug;
  return ($substring, $num_chars, $string);
} # extract_substring


#######################################################################
#
# Replace a string without using RegExp.
# See: http://www.bin-co.com/perl/scripts/str_replace.php
#
sub str_replace {
  my ($replace_this, $with_this, $string, $do_globally) = @_;

  my $len = length($replace_this);
  my $position = 0; # current position

  while( ($position = index($string, $replace_this, $position)) >= 0 ) {
    substr($string, $position, $len) = $with_this;
    if( not defined $do_globally or not $do_globally ) {
      last;
    }
  }
  return $string;
} # str_replace


#######################################################################
#
# Insert a step into an array at the location specified.
#
# Note that $step is offset-1 while arrays are offset-0, so we need to adjust
# the insertion point to account for this. In other words, if we insert at 1,
# the inserted step will become the first step. Hint: If you want to insert at
# the end, insert at location (scalar(@lines)+1).
#
# Don't worry about the performance issues with splice since our arrays are
# generally short: <500 lines.
#
sub insert_step {
  my $line = shift;
  my $location = shift;
  my $msg = "Inserting mnemonic '$line' at step " . format_step($location) . ".";
  debug_msg(this_function((caller(0))[3]), $msg) if $debug;
#  if( $debug ) {
#    debug_msg(this_function((caller(0))[3]), "Prior to inserting: '$line'") if $debug;
#    panl(@lines);
#    print "\n...done\n";
#  }
  my %entry = ();
  $entry{$ENUM_OP} = $line;
  $entry{$ENUM_NUM_WORDS} = step2words($line);
  show_src_db_step("Processed step: ", $location) if $debug > 3;

  splice @src_db, ($location-1), 0, \%entry;
  build_step_numbers();
#  if( $debug ) {
#    debug_msg(this_function((caller(0))[3]), "After inserting: '$line'") if $debug;
#    panl(@lines);
#    print "\n...done\n";
#  }
  return;
} # insert_step


#######################################################################
#
#
#
sub search_targets {
  my $label = shift;
  my $target_step = -1;
  if( exists $targets{$label} ) {
    $target_step = $targets{$label};
    debug_msg(this_function((caller(0))[3]), "Found target step '$target_step' with target label '$label' in \%targets.") if $debug > 3;
  } else {
    if( $debug ) {
      print "ERROR: " . this_function((caller(0))[3]) . ": Cannot find label '$label' in \%targets.\n";
      show_state(__LINE__);
      die_msg(this_function((caller(0))[3]), "Cannot continue...");
    } else {
      die_msg(this_function((caller(0))[3]), "Cannot find label '$label' in \%targets.");
    }
  }
  return $target_step;
} # search_targets


#######################################################################
#
#
#
sub search_LBLs {
  my $target_step = shift;
  my $LBL = "";
  my $found = 0;
  for my $tmp_LBL (sort keys %LBLs) {
    my $fmt_step = $LBLs{$tmp_LBL};
    if( $fmt_step eq $target_step ) {
      $found = 1;
      $LBL = $tmp_LBL;
      last;
    }
  }
  if( not $found ) {
    if( $debug ) {
      print "ERROR: " . this_function((caller(0))[3]) . ": Cannot find target step '$target_step' in \%LBLs.\n";
      show_state(__LINE__);
      die_msg(this_function((caller(0))[3]), "Cannot continue...");
    } else {
      die_msg(this_function((caller(0))[3]), "Cannot find target step '$target_step' in \%LBLs.");
    }
  } else {
    debug_msg(this_function((caller(0))[3]), "Found target step '$target_step' with LBL '$LBL' in \%LBLs.") if $debug > 3;
  }
  return $LBL;
} # search_LBLs


#######################################################################
#
# Format the step into a N-digit number. N will be 3 for sources earlier than v3 and
# 4 for V3 sources.
#
sub format_step {
  my $step = shift;
  return sprintf "%0${step_digits}d", $step;
} # format_step


#######################################################################
#
# Format the step into either a 2-digit number or a single upper case character.
#
sub format_LBL {
  my $LBL = shift;
  my $output = "";
  if( $LBL =~ /[A-Z]/ ) {
    $output = sprintf "%1s", $LBL;
  } else {
    $output = sprintf "%0${label_digits}d", $LBL;
  }
  return $output;
} # format_LBL


#######################################################################
#
# Format the step into either a {2,3}-digit number or a single upper case character.
#
sub format_branch {
  my $branch = shift;
  my $output = "";
  if( $branch =~ /[A-Z]/ ) {
    $output = sprintf "%1s", $branch;
  } else {
    $output = sprintf "%0${branch_digits}d", $branch;
  }
  return $output;
} # format_branch


#######################################################################
#
# Find an unused label. When it is found, reserve it for that step.
#
# The direction of the next label (+/-1) is contained in the $next_label_dir
# offset. XROM starts at 00 and increments while normal programs start at
# 99 and decrement.
#
# The reason the normal programs start at 99 and decrement is to keep them
# out of the way of the user. A very quick sample of users usages of LBL
# showed they most often started at 00 and and worked their way up. XROM,
# on the other hand, is used as a numerical offset into a table. So it needs
# to start at 00.
#
sub get_next_label {
  my $step = shift;
  my $label = $last_label_used + $next_label_dir;
  while( exists $LBLs{format_LBL($label)} ) {
    $label += $next_label_dir; # Point at next label in sequence.

    # Make sure we know which direction the next label is to be found (+/-1) and test
    # for overflow/underflow accordingly.
    if( (($next_label_dir == -1) and ($label < 0)) or (($next_label_dir == 1) and ($label > $MAX_LABEL_NUM) ) ) {
      my $msg = "Numeric label supply has been exhausted.\n Greater then " . $MAX_LABEL_NUM+1 . " numeric labels used.";
      die_msg(this_function((caller(0))[3]), $msg);
    }
  }
  $LBLs{format_LBL($label)} = format_step($step);
  return $label;
} # get_next_label


#######################################################################
#
#
#
sub replace_with_branch {
  my $offset = shift;
  my $line = shift;
  my ($step, $label, $spaces, $opcode);
  my ($mnemonic);

  if ($line =~ /JMP|BACK|SKIP/) {
    $mnemonic = ($offset < 0) ? "BACK" : "SKIP";
  } elsif ($line =~ /GSB|BSRB|BSRF/) {
    $mnemonic = ($offset < 0) ? "BSRB" : "BSRF";
  } else {
    if( $debug ) {
      print "ERROR: " . this_function((caller(0))[3]) . ": Unrecognized branch instruction in line: '$line'.\n";
      show_state(__LINE__);
      die_msg(this_function((caller(0))[3]), "Cannot continue...");
    } else {
      die_msg(this_function((caller(0))[3]), "Unrecognized branch instruction in line: '$line'.");
    }
  }
  $offset = format_branch(abs($offset));

  $line =~ s/^\s*\d{0,3}:{0,1}//;
  $line =~ s/^\s+//;
  $line =~ s/\s+$//;

  if( $line =~ /^([$label_spec]{2,}):{$NUM_TARGET_LABEL_COLONS}(\s+)(.+)/ ) {
    $label = $1;
    $spaces = $2;
    $opcode = $3;
    debug_msg(this_function((caller(0))[3]), "Replacing: '$line' (type 1)") if $debug;
    $line =~ s/^[$label_spec]{2,}:{$NUM_TARGET_LABEL_COLONS}\s+(.+)/${label}::${spaces}${mnemonic} $offset \/\/ $1/;
    debug_msg(this_function((caller(0))[3]), "With:      '$line' (type 1)") if $debug;
  } elsif( $line =~ /^(.+)/ ) {
    $opcode = $1;
    debug_msg(this_function((caller(0))[3]), "Replacing: '$line' (type 2)") if $debug;
    $line =~ s/(.+)/${mnemonic} $offset \/\/ $1/;
    debug_msg(this_function((caller(0))[3]), "With:      '$line' (type 2)") if $debug;
  } else {
    die_msg(this_function((caller(0))[3]), "Cannot parse the line: '$line' (offset $offset)");
  }
  return $line;
} # replace_with_branch


#######################################################################
#
# Replace instruction with the version targeting a LBL.
#
sub replace_with_LBLd_target {
  my $new_label_num = shift;
  my $line = shift;
  my $mnemonic = shift;
  my ($label, $spaces, $opcode, $target);

  $line =~ s/^\s*\d{0,3}:{0,1}//;
  $line =~ s/^\s+//;
  $line =~ s/\s+$//;

  my $fmt_label_num = format_LBL($new_label_num);

  if( $line =~ /^\s*([$label_spec]{2,}):{$NUM_TARGET_LABEL_COLONS}(\s+)(\S+)\s+(\S+)/ ) {
    $label = $1;
    $spaces = $2;
    $opcode = $3;
    $target = $4;
    debug_msg(this_function((caller(0))[3]), "Replacing: '$line' (type 1)") if $debug;
    $line = "${label}::${spaces}${mnemonic} $fmt_label_num // $opcode $target";
    debug_msg(this_function((caller(0))[3]), "With:      '$line' (type 1)") if $debug;
  } elsif( $line =~ /^\s*(\S+)\s+(\S+)/ ) {
    $opcode = $1;
    $target = $2;
    debug_msg(this_function((caller(0))[3]), "Replacing: '$line' (type 2)") if $debug;
    $line = "${mnemonic} $fmt_label_num // $opcode $target";
    debug_msg(this_function((caller(0))[3]), "With:      '$line' (type 2)") if $debug;
  } else {
    die_msg(this_function((caller(0))[3]), "Cannot parse the line: '$line'");
  }
  return $line;
} # replace_with_LBLd_target


#######################################################################
#
# Increment any step associated with an existing label if that step is past the
# point at which a step was inserted.
#
sub adjust_labels_used {
  my $step = shift;
  debug_msg(this_function((caller(0))[3]), "Step '$step'.") if $debug;
  for my $label (sort keys %LBLs) {
    my $target = $LBLs{$label};
    if( $target > ($step + 1) ) {
      debug_msg(this_function((caller(0))[3]), "Incrementing target ('$target') for label '$label' because it is at or past step '$step'.") if $debug;
      $LBLs{$label} = format_step($target+1);
    }
  }
  return;
} # adjust_labels_used


#######################################################################
#
# Increment any step associated with an existing target if that step is past the
# point at which a new step was inserted.
#
sub adjust_targets {
  my $step = shift;
  for my $label (sort keys %targets) {
    my $target = $targets{$label};
    if( eval($target+0) >= eval($step+0) ) {
      debug_msg(this_function((caller(0))[3]), "Incrementing step ('$target') for label '$label' because it is at or past step '$step'.") if $debug;
      $targets{$label}++;
    }
  }
  return;
} # adjust_targets


#######################################################################
#
# Increment any step associated with an existing label if that step is past the
# point at which a new step was inserted. This requires new keys and to avoid colliding
# with existing keys, just recreate the hash.
#
sub adjust_branches {
  my $step = shift;
  my (%new_branches);
  for my $branch_step (sort keys %branches) {
    my $label = $branches{$branch_step};
    my $eval_branch_step = eval($branch_step + 0);
    if( $eval_branch_step >= eval($step + 0) ) {
      $new_branches{format_step($eval_branch_step+1)} = $label;
      debug_msg(this_function((caller(0))[3]), "Incrementing branch step ('$branch_step') with label '$label' because it is at or past step '$step'.") if $debug;
    } else {
      if( exists $new_branches{$branch_step} ) {
        die_msg(this_function((caller(0))[3]), "Existing branch already present for branch step ('$branch_step') with label '$label' prior to step '$step'.");
      }
      $new_branches{$branch_step} = $label;
      debug_msg(this_function((caller(0))[3]), "Leaving branch step ('$branch_step') with label '$label' untouched because it is prior to step '$step'.") if $debug > 1;
    }
  }
  %branches = %new_branches; # Replace the branches hash.
  return;
} # adjust_branches


#######################################################################
#
# Increment any step associated with an existing label if that step is past the
# point at which a new step was inserted.
#
sub adjust_branch_array {
  my $step = shift;
  show_branches_remaining() if $debug;
  for( my $k = 0; $k < scalar(@branches); $k++ ) {
    if( $branches[$k] >= $step ) {
      debug_msg(this_function((caller(0))[3]), "Incrementing branch array step ('${branches[$k]}').") if $debug;
      $branches[$k] = format_step($branches[$k] + 1);
    }
  }
  show_branches_remaining() if $debug;
  return;
} # adjust_branch_array


#######################################################################
#
# Increment any step associated with an existing XLBL if that step is past the
# point at which a new step was inserted.
#
sub adjust_xlbls {
  my $step = shift;
  $step = get_line($ENUM_STEP, $step);
  for my $xlabel (sort keys %xlbl) {
    if ($xlbl{$xlabel} > $step) {
      $xlbl{$xlabel}++;
      debug_msg(this_function((caller(0))[3]), "Incrementing xlabel $xlabel to $xlbl{$xlabel} because it is at or past step '$step'.") if $debug;
    }
  }
  return;
} # adjust_xlbls


#######################################################################
#
# Find out if the requested label exists. Return either label or -1.
#
sub is_existing_label {
  my $step = shift;
  my $label_spec = -1; # Flag that no label exists.
  for my $label_num (sort keys %LBLs) {
    my $target_step = $LBLs{$label_num};
    if( format_step($step) eq $target_step ) {
      $label_spec = $label_num;
      last;
    }
  }
  return $label_spec;
} # is_existing_label


#######################################################################
#
# Read in a file.
#
sub read_file {
  my $file = shift;
  local $_;
  my (@lines);
  open SRC, $file or die_msg(this_function((caller(0))[3]), "Cannot open input file '$file' for reading: $!");
  while( <SRC> ) {
    chomp; chomp; chomp;
    push @lines, $_;
  }
  close SRC;
  return @lines;
} # read_file


#######################################################################
#
# Redact any commented sections.
#
# Currently this will handle "//", "/* ... */" on a single line, "/*" alone
# on a line up to and including the next occurence of "*/", plus more than one
# occurence of "/* ... */ ... /* ... */" on a single line. There may be some
# weird cases that it does not handle.
#
sub redact_comments {
  my @lines = @_;
  local $_;

  # Oh no!! Slanted toothpick syndrome! :-)
  my $slc  = "\/\/"; # Single line comment marker.
  my $mlcs = "/\\*"; # Multiline comment start marker.
  my $mlce = "\\*/"; # Multiline comment end marker.
  my $in_mlc = 0;    # In-multiline-comment flag.
  my (@redacted_lines);

  foreach (@lines) {
    chomp; chomp;

    # Detect if we are currently in a multiline comment and we see the end of a
    # multiline comment. If so, remove up to and including the trailing multiline
    # comment marker.
    if( $in_mlc and m<${mlce}> ) {
      $in_mlc = 0;
      s/^.*?\*\///;
    }

    # Remove any single line comment sections from the line.
    s/${slc}.*$//;

    # Detect complete multiline comments ...and remove them. (don't be greedy)
    while( m/${mlcs}.+?${mlce}/ ) {
      s/${mlcs}.*?${mlce}//;
    }

    # If we still have a start of a multiline comment, set the flag on (possibly again).
    # It means we don't have a terminated comment set so delete to the end of the line.
    if( m<${mlcs}> ) {
      $in_mlc = 1;
      s/${mlcs}.*$//;
    }

    # If we are still in a multiline comment and we get here, scrub the entire line.
    if( $in_mlc ) {
      $_ = "";
    }

    # Store anything that is left.
    push @redacted_lines, $_;
  }
  return @redacted_lines;
} # redact_comments


#######################################################################
#
# Remove any array elements that are blank lines.
#
sub remove_blank_lines {
  my @lines = @_;
  my (@new_lines);
  local $_;
  foreach (@lines) {
    next if /^\s*$/;
    push @new_lines, $_;
  }
  return @new_lines;
} #remove_blank_lines


#######################################################################
#
# Extract and record any XLBL pseudo op-codes. Insert into a global hash
# using the label as the key and the 1-based line number as the data.
# After they are processed, scrub them from the source since they are
# treated the same as comments after that.
#
sub extract_xlbls {
  my (@new_lines);
  my $line = 1;
  local $_;
  for (my $k = 0; $k < scalar @src_db; $k++ ) {
    my $line = $src_db[$k]->{$ENUM_OP};
    if ($line =~ /^\s*XLBL\"(.+)\"(\s+|$)/) {
      my $xlabel = $1;
      debug_msg(this_function((caller(0))[3]), "Found XLBL ($xlabel) at line $line.") if $debug or (exists $ENV{WP34S_PP} and ($ENV{WP34S_PP} =~ /XLBL/i));
      $src_db[$k]->{$ENUM_NUM_WORDS} = 0; # Effectively remove this from further processing by setting the op size to 0.
      $src_db[$k]->{$ENUM_OP} = "// $src_db[$k]->{$ENUM_OP}";
      if (exists $xlbl{$xlabel}) {
        if( $debug ) {
          print "ERROR: " . this_function((caller(0))[3]) . ": Duplicate XLBL ($xlabel) at line $line. First seen at line ${xlbl{$xlabel}}.\n";
          show_state(__LINE__);
          die_msg(this_function((caller(0))[3]), "Cannot continue...");
        } else {
          die_msg(this_function((caller(0))[3]), "Duplicate XLBL ($xlabel) at line $line. First seen at line ${xlbl{$xlabel}}.");
        }
      } else {
        $xlbl{$xlabel} = $src_db[$k]->{$ENUM_STEP};
      }
      next;
    }
  }
  return;
} # extract_xlbls


#######################################################################
#
#
#
sub populate_branch_array {
  debug_msg(this_function((caller(0))[3]), "**** Starting function") if $debug;
  debug_msg(this_function((caller(0))[3]), "Extracting sorted branch steps....") if $debug;
  @branches = sort keys %branches;
  show_state(__LINE__) if $debug;
  return;
} # populate_branch_array


#######################################################################
#
# Insert a new step into the branch array.
#
sub insert_into_branch_array {
  my $fmt_step = format_step(shift);
  debug_msg(this_function((caller(0))[3]), "Inserting step '$fmt_step' into \@branches if required.") if $debug;
  my $found = 0;
  for my $existing_step (@branches) {
    if( $existing_step eq $fmt_step ) {
      $found = 1;
      debug_msg(this_function((caller(0))[3]), "Step '$fmt_step' already exists in \@branches. Abandoning insert.") if $debug;
      last;
    }
  }
  if( not $found ) {
    push @branches, $fmt_step;
    @branches = sort {eval($a+0) <=> eval($b+0)} @branches;
    if( $debug ) {
      debug_msg(this_function((caller(0))[3]), "After inserting step '$fmt_step' into \@branches:") if $debug;
      show_branches_remaining();
    }
  }
  return;
} # insert_into_branch_array


#######################################################################
#
# Convert the array of scrubbed source lines to the database containing
# an array of hashes. Each hash has multiple keys to record various attributes
# about the line.
#
# XXX In future we will put comments in other keys in this database.
#
sub src2src_db {
  my @src = @_;
  local $_;
  my $step_num = 1;
  foreach (@src) {
    my %entry = ();
    my $words = step2words($_);
    $entry{$ENUM_OP} = $_;
    $entry{$ENUM_NUM_WORDS} = $words;
    $entry{$ENUM_STEP} = $step_num;
    push @src_db, \%entry;
    $step_num += $words;
  }
  show_src_db() if $debug;
  return @src_db;
} # src2src_db


#######################################################################
#
# Adjust the words/step attribute. Usually called after a change has been
# made to the line or a line has been inserted.
#
sub adjust_words_per_step {
  my $step = shift;
  my $line = shift;
  my $index = $step - 1;
  $src_db[$index]->{$ENUM_NUM_WORDS} = step2words($line);
  return;
} # adjust_words_per_step


#######################################################################
#
# Based on the mode (XROM or not) and the content of the line, the instructions
# might have 1 or 2 (or 0!) words per step. (A 0 is a special case indicating that the
# line contains probably only a comment).
#
sub step2words {
  my $src_line = shift;
  my $num_words = 1;
  if ($xrom_mode) {
    debug_msg(this_function((caller(0))[3]), "XROM mode: ($src_line)") if ($debug > 3) or (exists $ENV{WP34S_PP} and ($ENV{WP34S_PP} =~ /S2W/i));
    if ($src_line !~ /${DOUBLE_QUOTE}.+${DOUBLE_QUOTE}/) {
      debug_msg(this_function((caller(0))[3]), "No double quotes: ($src_line)") if ($debug > 3) or (exists $ENV{WP34S_PP} and ($ENV{WP34S_PP} =~ /S2W/i));
      if ($src_line =~ /\[alpha\]'/) {
        $num_words = 2;
        debug_msg(this_function((caller(0))[3]), "Instruction has escaped alpha single quote resulting in 2 words ($src_line)") if ($debug > 3) or (exists $ENV{WP34S_PP} and ($ENV{WP34S_PP} =~ /S2W/i));
      } elsif ($src_line !~ /\[alpha\]/) {
        debug_msg(this_function((caller(0))[3]), "No escaped alpha: ($src_line)") if ($debug > 3) or (exists $ENV{WP34S_PP} and ($ENV{WP34S_PP} =~ /S2W/i));
        if ($src_line =~ /\'.+\'/) {
          $num_words = 2;
          debug_msg(this_function((caller(0))[3]), "Instruction has quoted label resulting in 2 words ($src_line)") if ($debug > 3) or (exists $ENV{WP34S_PP} and ($ENV{WP34S_PP} =~ /S2W/i));
        } else {
          debug_msg(this_function((caller(0))[3]), "No single quotes: ($src_line)") if ($debug > 3) or (exists $ENV{WP34S_PP} and ($ENV{WP34S_PP} =~ /S2W/i));
        }
      } else {
        debug_msg(this_function((caller(0))[3]), "Has escaped alpha: ($src_line)") if ($debug > 3) or (exists $ENV{WP34S_PP} and ($ENV{WP34S_PP} =~ /S2W/i));
      }
    } else {
      debug_msg(this_function((caller(0))[3]), "Has double quotes: ($src_line)") if ($debug > 3) or (exists $ENV{WP34S_PP} and ($ENV{WP34S_PP} =~ /S2W/i));
      $num_words = 0 if ($src_line =~ /XLBL${DOUBLE_QUOTE}/);
    }
  } else {
    $num_words = 1;
  }
  return $num_words;
} # steps2words


#######################################################################
#
# Get the asked for attribute from the
#
sub get_line {
  my $field = shift;
  my $step_num = shift;
  my $val = "";
  my $index = $step_num-1;
  $val = $src_db[$index]->{$field};
  return $val;
} # get_line


#######################################################################
#
#
#
sub put_line {
  my $field = shift;
  my $step_num = shift;
  my $val = shift;
  my $index = $step_num-1;
  $src_db[$index]->{$field} = $val;
  return;
} # put_line


#######################################################################
#
#
#
sub calculate_offset {
  my $target_step = shift;
  my $current_step = shift;
  my $offset = 0;
  my $target_index = $target_step - 1;    # Adjust the steps to be indexes (ie: -1).
  my $current_index = $current_step - 1;
  my ($max, $min);
  if ($target_index > $current_index) {
    ($max, $min) = ($target_index, $current_index);
    debug_msg(this_function((caller(0))[3]), "Noninverted offset calculation: max=$max, min=$min") if $debug;
  } else {
    ($max, $min) = ($current_index, $target_index);
    debug_msg(this_function((caller(0))[3]), "Inverted offset calculation: max=$max, min=$min") if $debug;
  }
  if ($debug > 2) {
    show_src_db_step("Current step: ", $current_step);
    show_src_db_step("Target step:  ", $target_step);
  }

  for (my $k = $min; $k < $max; $k++) {
    $offset += $src_db[$k]->{$ENUM_NUM_WORDS} if exists $src_db[$k]->{$ENUM_NUM_WORDS};
  }

  if ($target_index < $current_index) {
    $offset = -$offset;
  }
  return $offset;
} # calculate_offset


#######################################################################
#
# Perl debugger helpers: Print array joined with \n.
#
# Print A with New Line (panl)
#
sub panl { # Print array joined with \n.
  print "// ", join "\n", @_;
}

sub show_LBLd_ops { # Dump the %LBLd_ops hash
  dump_hash("LBLd_ops", \%LBLd_ops);
  return;
}

sub show_targets { # Dump the %targets hash
  dump_hash("targets", \%targets, " Label    Step");
  return;
}

sub show_branches { # Dump the %branches hash
  dump_hash("branches", \%branches, " Step   Target");
  return;
}

sub show_LBLs { # Dump the %LBLs hash
  my $ref_hash = \%LBLs;
  my $count = 0;
  print "// Label Step\n";
  for my $key (sort keys %{$ref_hash}) {
    my $content = get_line($ENUM_STEP, $ref_hash->{$key});
    print "//  $key => $content\n";
    $count++;
  }
  print "// $count LBLs used.\n";
  return;
}

sub show_branches_remaining { # Dump the @branches array
  print "// \@branches:\n";
  print "// ", join ":", @branches;
  print "\n";
  my $count = scalar @branches;
  print "// Total of $count elements\n";
  return;
}

sub dump_hash { # Dump the specified hash
  my $name = shift;
  my $ref_hash = shift;
  my $second_line = shift;
  my $count = 0;
  print "// \%${name}:\n";
  print "// $second_line\n" if defined $second_line;
  for my $key (sort keys %{$ref_hash}) {
    my $content = $ref_hash->{$key};
    print "//  $key => $content\n";
    $count++;
  }
  print "// Total of $count elements\n";
  return;
} # show_LBLs

sub show_lines {
  display_steps("// ");
  return;
} # show_lines

sub show_tables {
  show_LBLs();
  show_targets();
  show_branches();
  show_branches_remaining();
  return;
} # show_tables

sub show_state {
  my $line_num = shift;
  print "// ==========================================================\n";
  print "// State as of line $line_num:\n";
  show_lines();
  show_tables();
  return;
} # show_state

# Show SRC DataBase -- quickie debugger version
sub ssdb {
  show_src_db();
  return;
} # ssdb

sub show_src_db {
  local $_;
  for (my $step = 1; $step < (scalar @src_db)+1; $step++) {
    show_src_db_step("", $step);
  }
  return;
} # show_src_db

# Show a single step
sub show_src_db_step {
  my $leader = shift;
  my $step = shift;
  my $index = $step - 1;
  print "// ${leader}Step: $step\n";
  print "//  STEP:         ", $src_db[$index]->{$ENUM_STEP}, "\n"         if exists $src_db[$index]->{$ENUM_STEP};
  print "//  OP:           ", $src_db[$index]->{$ENUM_OP}, "\n"           if exists $src_db[$index]->{$ENUM_OP};
  print "//  WORDS:        ", $src_db[$index]->{$ENUM_NUM_WORDS}, "\n"    if exists $src_db[$index]->{$ENUM_NUM_WORDS};
  print "//  POST-COMMENT: ", $src_db[$index]->{$ENUM_PRECOMMENT}, "\n"   if exists $src_db[$index]->{$ENUM_PRECOMMENT};
  print "//  PRE-COMMENT:  ", $src_db[$index]->{$ENUM_POSTCOMMENT}, "\n"  if exists $src_db[$index]->{$ENUM_POSTCOMMENT};
  return;
} # show_src_db_step


#######################################################################
#
#
#
sub extract_svn_version {
  my $svn_rev = "";
  if( $SVN_Current_Revision =~ /Rev.+?(\d+)\s*\$/ ) {
    $svn_rev = $1;
  }
  return $svn_rev;
} # extract_svn_version


#######################################################################
#
# Format a fatal message.
#
sub die_msg {
  my $func_name = shift;
  my $text = shift;
  my $msg = "";

  $msg = "$ansi_red_bg" if $use_ansi_colour;
  $msg .= "ERROR: $func_name:";
  $msg .= "$ansi_normal " if $use_ansi_colour;
  $msg .= " $text";
  if ($stderr2stdout) {
    print "$msg\n";
    die "\n";
  } else {
    die "$msg\n";
  }
} # die_msg


#######################################################################
#
# Format a warning message.
#
sub warn_msg {
  my $func_name = shift;
  my $text = shift;
  my $msg = "";

  $msg = "$ansi_rev_red_bg" if $use_ansi_colour;
  $msg .= "WARNING: $func_name:";
  $msg .= "$ansi_normal " if $use_ansi_colour;
  $msg .= " $text";
  if ($stderr2stdout) {
    print "$msg\n";
  } else {
    warn "$msg\n";
  }
} # warn_msg


#######################################################################
#
# Format a debug message.
#
sub debug_msg {
  my $func_name = shift;
  my $text = shift;
  my $msg = "";

  $msg = "$ansi_green_bg" if $use_ansi_colour;
  $msg .= "// DEBUG: $func_name:";
  $msg .= "$ansi_normal " if $use_ansi_colour;
  $msg .= " $text";
  print "$msg\n";
} # debug_msg


#######################################################################
#
# Swap the main:: for the actual script name.
#
sub this_function {
  my $this_function = shift;
  $this_function = "main" if not defined $this_function or ($this_function eq "");
  $this_function =~ s/main/$script_name/;
  return $this_function;
} # this_function


#######################################################################
#######################################################################
#
# Process the command line option list.
#
sub get_options {
  my ($arg);
  @cmd_args = @ARGV;
  while ($arg = shift(@ARGV)) {

    # See if help is asked for
    if( $arg eq "-h" ) {
      print "$usage\n";
      die "\n";
    }

    elsif( ($arg eq "--version") or ($arg eq "-V") ) {
      print "$script\n";
      my $svn_rev = extract_svn_version();
      print "SVN version: $svn_rev\n" if $svn_rev;
      die "\n";
    }

    elsif( $arg eq "-d" ) {
      $debug = shift(@ARGV);
    }

    elsif( ($arg eq "-v3") or ($arg eq "-end") ) {
      $v3_mode = 1;
      $label_digits = 2;
      $branch_digits = 3;
      $label_digits_range = "2,3"
    }

    elsif( $arg eq "-no_v3" ) {
      $v3_mode = 0;
    }

    # Step the maximum BACK/SKIP offset limit.
    elsif( $arg eq "-m" ) {
      $MAX_JMP_OFFSET = shift(@ARGV);
    }

    elsif( $arg eq "-ns" ) {
      $prt_step_num = 0;
    }

    elsif( $arg eq "-cat" ) {
      $show_catalogue = 1;
    }

    elsif( $arg eq "-xlbl_file" ) {
      $xlbl_file = shift(@ARGV);
    }

    elsif( $arg eq "-targets" ) {
      $show_targets = 1;
    }

    elsif( $arg eq "-renum" ) {
      $renumber_steps = 1;
    }

    elsif( $arg eq "-no_renum" ) {
      $renumber_steps = 0;
    }

    elsif( ($arg eq "-xrom") or ($arg eq "-x") ) {
      $xrom_mode = 1;
    }

    elsif( ($arg eq "-step_digits") or ($arg eq "-sd") ) {
      $override_step_digits = shift(@ARGV);
    }

    elsif( ($arg eq "-e2so") or ($arg eq "-stderr2stdout")) {
      $stderr2stdout = 1;
    }

    elsif( $arg eq "-nc" ) {
      $use_ansi_colour = 0;
    }

    elsif( ($arg eq "-ac") or ($arg eq "-colour") or ($arg eq "-color") ) {
      $use_ansi_colour = 1;
    }

    elsif ($arg eq "-colour_mode") {
      $use_ansi_colour = shift(@ARGV);
    }

    # Might behave badly if files have already been entered.
    # XXX Leave undocumented for now.
    elsif( $arg eq "--" ) {
      @files = ();      # Scrub any files that have already been entered.
      push @files, "-"; # Use STDIN instead of a file.
    }

    else {
      push @files, $arg;
    }
  }

  #----------------------------------------------
  # Check consistency of the options.
  unless( @files ) {
    die_msg(this_function((caller(0))[3]), "Must enter at least one file to process.\n Enter '$script_name -h' for help.");
  }

  if ((not $v3_mode and ($MAX_JMP_OFFSET > 99) or ($MAX_JMP_OFFSET < 1))
    or ($v3_mode and ($MAX_JMP_OFFSET > 255) or ($MAX_JMP_OFFSET < 1))) {
    my $max = ($v3_mode) ? 255 : 99;
    my $MAX_JMP_OFFSET = ($v3_mode) ? 240 : 90;
    print "// WARNING: Maximum BACK/SKIP offset limit (-m) must be between 5 and $max. Resetting to $MAX_JMP_OFFSET.\n";
  }
  debug_msg(this_function((caller(0))[3]), "Debug level set to: $debug") if $debug;

  # XROM mode increments the virtual LBLs from the 00 instead of decrementing them from MAX.
  # Change the sense of the next-offset and the starting location of the first virtual LBL.
  if ($xrom_mode) {
    $next_label_dir = +1;  # Next virtual LBL offset direction
    $last_label_used = -1; # Initial virtual LBL (it will be incremented before being used).
  }

  return;
} # get_options

