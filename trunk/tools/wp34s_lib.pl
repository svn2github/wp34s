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
my $Description = "WP 34S library manager.";
#
#-----------------------------------------------------------------------
#
# Language:         Perl script
#
my $SVN_Current_Revision  =  '$Revision$';
#
#-----------------------------------------------------------------------

use strict;
use POSIX;
use File::Basename;

# ---------------------------------------------------------------------

my $debug = 0;
my $quiet = 0;

my $in_libfile = "";
my $out_libfile = "";
my $gen_cat = 0;
my $state_file = 0;
my %state_data = ();
my $chk_crc = 0;
my $conv_state2flash = 0;

my $FLASH_MODE = "-lib";
my $STATE_MODE = "";

my $CRC_INITIALIZER = 0x5aa5;

my $DEFAULT_ASM = "wp34s_asm.pl";
my $asm_script = $DEFAULT_ASM;

my $disasm_options = (exists $ENV{WP34S_LIB_DISASM_OPTIONS})
                   ? $ENV{WP34S_LIB_DISASM_OPTIONS} : "-s 2 -ns";
my $asm_options    = (exists $ENV{WP34S_LIB_ASM_OPTIONS})
                   ? $ENV{WP34S_LIB_ASM_OPTIONS} : "";

my $die_on_existing_duplicate = 1;

my $use_pp = "-pp";
my @new_srcs = ();
my @rm_progs = ();

my $lib_cat_dump_basefile = (exists $ENV{WP34S_LIB_CAT_DUMP} and ($ENV{WP34S_LIB_CAT_DUMP} ne ""))
                          ? $ENV{WP34S_LIB_CAT_DUMP}
                          : "wp34s_lib_cat_dump.wp34s";

my $lib_cat_dump_final_file = (exists $ENV{WP34S_LIB_FINAL_SRC} and ($ENV{WP34S_LIB_FINAL_SRC} ne ""))
                          ? $ENV{WP34S_LIB_FINAL_SRC}
                          : "wp34s_lib_final_dump.wp34s";

# ANSI colour codes.
my $ansi_normal           = "\e[0m";
my $ansi_red_bg           = "\e[41;33;1m";
my $ansi_green_bg         = "\e[42;33;1m";
my $ansi_rev_green_bg     = "\e[42;1;7;33;1m";
my $ansi_rev_red_bg       = "\e[41;1;7;33;1m";
my $ansi_rev_blue_bg      = "\e[47;1;7;34;1m";
my $ansi_rev_cyan_bg      = "\e[30;46m";

my $DEFAULT_USE_ANSI_COLOUR = (exists $ENV{WP34S_LIB_COLOUR})
                            ? $ENV{WP34S_LIB_COLOUR}
                            : 1;
my $use_ansi_colour       = $DEFAULT_USE_ANSI_COLOUR;

# ---------------------------------------------------------------------

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
  }
} else {
  # If we get here, the set file system type function has NOT been run and we may not
  # be able to locate "relative" file.
  debug_msg($0, "Unrecognized O/S ($^O). Some features may not work as expected.");
}

# Automatically extract the name of the executable that was used. Also extract
# the directory so we can potentially source other information from this location.
my $script_executable = $0;
my ($script_name, $script_dir, $script_suffix) = fileparse($script_executable);

if (exists $ENV{WP34S_LIB_OS_DBG} and ($ENV{WP34S_LIB_OS_DBG} == 1)) {
  debug_msg($script_name, "script_executable = '$script_executable'");
  debug_msg($script_name, "script_name       = '$script_name'");
  debug_msg($script_name, "script_dir        = '$script_dir'");
  debug_msg($script_name, "script_suffix     = '$script_suffix'");
}

my $script  = "$script_name  - $Description";
my $usage = <<EOM;

$script

Usage:
   $script_name [src_file [src_file2 [src_file3]]] [-rm PRG [-rm PG2]|-rm "PRG PG2 [...]"] \\
                [-conv] -ilib org_lib.dat [-olib new_lib.dat|-f] \\
                [-cat] [-nc|-colour|-color]

Parameters:
   src_file         One or more source files to add or replace within an existing library (when
                    -ilib provided), or to add to a newly created library (when no -ilib provided).
   -rm "PRG"        Name of program(s) to remove from the library. This switch can appear multiple
                    times to name multiple programs, or a single instance of the switch can name
                    multiple programs. In the latter case, the list of program names must be
                    surrounded by quotes. If the program name contains escaped-alphas, these must
                    be surrounded by quotes.
   -state           Indicates that the source library is a 'state' file, as opposed to a flash library.
   -conv            Convert an existing state library file to a flash library format. This can be
                    done with no modifications to the library content or can be used with any of the
                    other editting features such as adding, removing, or replacing programs.
   -ilib libfile    Original library file to add or replace programs in. If requesting a flash-style
                    output, this input file is not strictly required. In this case a flash library will
                    be constructed from scratch and will be named as per the '-olib' switch.
   -olib outfile    Output produced by the tool. This switch is required unless the -f switch is
                    used or no modifications are being requested to the input library (eg: simply
                    a '-cat' command is used). If input library modifications are being requested
                    (eg: add, replace, or remove programs), this must be supplied unless the '-f'
                    override switch is used.
   -f               Force the -ilib name to be used as the output library name as well. Note that
                    setting the same file name using both the -ilib and -olib switches results in
                    the same action. If the '-f' switch is used, it MUST follow the '-ilib' switch
                    on the command line!
   -cat             Display catalogue of input and/or output libraries.

   -pp              Use the preprocessor when assembling the sources. [default]
   -no_pp           Don\'t use the preprocessor when assembling the sources.
   -nc              Turn off ANSI colour codes in warning and error messages. [default for non-MSDOS O/S]
   -colour          Turn on ANSI colour codes in warning and error messages.  [default for all other O/S's]
   -color           Turn on ANSI color codes in warning and error messages. Same as previous (provides
                    for American color/spelling palette :-).
   -h               This help script.

Examples:
  \$ $script_name great_circle.wp34s -olib wp34s-lib.dat -ilib wp34s-lib.dat -cat
  - Assembles the named WP34s program source file(s) and either replaces existing versions
    in the named flash library or adds them to it. In this case the output and input libraries
    have the same name, so the original library file will be overwritten. Provides a catalogue
    of both the input and output libraries.

  \$ $script_name great_circle.wp34s floating_point.wp34s -olib wp34s-1.dat -ilib wp34s-1.dat
  - Assembles the named WP34s program source files and either replaces existing versions in the
    output flash library or adds them to it. In this case the input and output libraries have
    the different names, so the original input library file will be untouched.

  \$ $script_name -ilib wp34s-lib.dat -f -rm ABC -rm BCD -rm "A[times][beta]"
  \$ $script_name -ilib wp34s-lib.dat -olib wp34s-lib.dat -rm "ABC BCD A[times]B"
  - These 2 invocations have the identical results. They remove the 3 programs 'ABC', 'BCD',
    and 'A[times][beta]' from the named input library and write the resulting output library
    back to the a file of the same name. The first invocation 'forces' the input library to
    be overwritten. The second explicitly names the input and output libraries to the same
    name. Notice that the escaped-alpha program name must be surrounded by quotes at all times.

  \$ $script_name great_circle.wp34s floating_point.wp34s -ilib wp34s.dat -f -state -rm XYZ
  - Assembles the named WP34s program source files and either replaces existing versions in the
    state library or adds them to it. Removes a program called XYZ. In this case the input
    library name is reused for the output as well.

  \$ $script_name -ilib wp34s-lib.dat -cat
  - Provides a catalogue of the input lib.

  \$ $script_name -ilib wp34s.dat -olib wp34s-lib.dat -conv -cat
  - Convert a state file to a flash library file. Provides a catalogue of the library.

  \$ $script_name -ilib wp34s.dat -olib wp34s-lib.dat -conv -rm ABC myprog.wp34s
  - Convert a state file to a flash library file. The progra, 'ABC' will be removed from the
    flash copy. Whatever programs are in 'myprog.wp34s' will be added or replaced, as the case
    may be, from the flash copy.

  \$ $script_name -olib wp34s-lib.dat great_circle.wp34s mod.wp34s matrix.wp34s
  - Assembles the named WP34s program source file(s) and creates a brand new output
    flash library (ie: no existing input library was present).

Notes:
  1) The library binary format is different from state format. However, though these images
     are different, they can both still be disassembled using the assembler's disassembler
     function using the standard command format. For example:

     \$ wp34s_asm.pl -dis some_lib.dat

  2) A flash output library can be created without an existing input library. However, this is
     not possible for the state file as its format is much more involved. Therefore, an '-ilib'
     switch is required whenever the '-state' or '-conv' switches are used.

  3) There is no order or position requirement of any command line switches EXCEPT '-f'. If
     used, there MUST be a preceeding '-ilib' switch.

EOM

#######################################################################

get_options();

my (@lib_src, %lib_cat, %new_progs, @status, $initial_lib);

# Only process an input library if one is given, otherwise the output will be create
# "fresh" from the new sources. Note that we (currently) cannot create a state file
# from scratch. However, this situation will have been trapped at the get_options()
# function.
if ($in_libfile) {
  if ($state_file or $conv_state2flash) {
    # Read in and decompose the state file.
    %state_data = %{parse_state_file($in_libfile)};

    @lib_src = @{$state_data{SRC}};
    $initial_lib = $in_libfile;

  } else {
    # Disassemble the library.
    @lib_src = disassemble_binary($in_libfile);
    $initial_lib = $in_libfile;
  }

  if ($gen_cat) {
    my $lib_format = "";
    if ($state_file or $conv_state2flash) {
      $lib_format = "State file";
    } else {
      $lib_format = "flash file";
    }
    print "Initial library catalogue (format: $lib_format)\n";
    show_catalogue("  ", $initial_lib, @lib_src);
  }

  # Catalogue the initial library.
  %lib_cat = %{catalogue_binary($initial_lib, @lib_src)};


  dbg_show_cat("after initial in_lib processing", "lib_cat", \%lib_cat)
      if $debug or (exists $ENV{WP34S_LIB_CAT_DBG} and ($ENV{WP34S_LIB_CAT_DBG} == 1));
  dbg_dump_cat("after processing org lib", "org_lib_${lib_cat_dump_basefile}", "lib_cat", \%lib_cat)
      if ($debug > 1) or (exists $ENV{WP34S_LIB_CAT_DUMP} and ($ENV{WP34S_LIB_CAT_DUMP} ne ""));
}

# Only execute the modification section if we have been requested to do something.
if (@new_srcs or @rm_progs) {
  # Only run this section if there are programs that need to be added.
  if (@new_srcs) {
    # Prepare the new source files.
    %new_progs = %{prepare_new_srcs(@new_srcs)};

    dbg_show_cat("after new programs preparation", "new_progs", \%new_progs)
        if $debug or (exists $ENV{WP34S_LIB_CAT_DBG} and ($ENV{WP34S_LIB_CAT_DBG} == 1));
    dbg_dump_cat("after processing new files", "new_src_${lib_cat_dump_basefile}", "new_progs", \%new_progs)
        if ($debug > 1) or (exists $ENV{WP34S_LIB_CAT_DUMP} and ($ENV{WP34S_LIB_CAT_DUMP} ne ""));


    # Replace any existing programs with the new programs, or add the new programs
    # if the program is not currently found in the library.
    for my $new_prog_name (sort keys %new_progs) {
      my $new_prog_src_ref = $new_progs{$new_prog_name};  # a source array
      if (exists $lib_cat{$new_prog_name}) {
        printf "Replacing program: \"%0s\", old program steps: %0d, new program steps: %0d\n",
                $new_prog_name, scalar @{$lib_cat{$new_prog_name}}, scalar @{$new_prog_src_ref} unless $quiet;

        %lib_cat = %{replace_prog(\%lib_cat, $new_prog_name, $new_prog_src_ref)};

        dbg_show_cat("after replacing \"$new_prog_name\"", "lib_cat", \%lib_cat)
            if $debug or (exists $ENV{WP34S_LIB_CAT_DBG} and ($ENV{WP34S_LIB_CAT_DBG} == 1));
        dbg_dump_cat("after processing replacement of $new_prog_name",
                    "replace_${new_prog_name}_${lib_cat_dump_basefile}", "lib_cat", \%lib_cat)
            if ($debug > 1) or (exists $ENV{WP34S_LIB_CAT_DUMP} and ($ENV{WP34S_LIB_CAT_DUMP} ne ""));
      } else {
        printf "Adding program: \"%0s\", new program steps: %0d\n",
                $new_prog_name, scalar @{$new_prog_src_ref} unless $quiet;

        %lib_cat = %{add_prog(\%lib_cat, $new_prog_name, $new_prog_src_ref)};

        dbg_show_cat("after adding \"$new_prog_name\"", "lib_cat", \%lib_cat)
            if $debug or (exists $ENV{WP34S_LIB_CAT_DBG} and ($ENV{WP34S_LIB_CAT_DBG} == 1));
        dbg_dump_cat("after processing addition of $new_prog_name",
                    "add_${new_prog_name}_${lib_cat_dump_basefile}", "lib_cat", \%lib_cat)
            if ($debug > 1) or (exists $ENV{WP34S_LIB_CAT_DUMP} and ($ENV{WP34S_LIB_CAT_DUMP} ne ""));
      }
    }
  }

  # Remove any programs as requested.
  foreach my $rm_prog_name (@rm_progs) {
    if (exists $lib_cat{$rm_prog_name}) {
      printf "Removing program: \"%0s\", old program steps: %0d\n",
              $rm_prog_name, scalar @{$lib_cat{$rm_prog_name}} unless $quiet;
      %lib_cat = %{remove_prog(\%lib_cat, $rm_prog_name)};
      dbg_show_cat("after removing \"$rm_prog_name\"", "lib_cat", \%lib_cat) if $debug or (exists $ENV{WP34S_LIB_CAT_DBG} and ($ENV{WP34S_LIB_CAT_DBG} == 1));

      dbg_dump_cat("after processing removal of $rm_prog_name",
          "remove_${rm_prog_name}_${lib_cat_dump_basefile}", "lib_cat", \%lib_cat)
          if ($debug > 1) or (exists $ENV{WP34S_LIB_CAT_DUMP} and ($ENV{WP34S_LIB_CAT_DUMP} ne ""));
    } else {
      warn_msg(this_function_script((caller(0))[3]), "Program to remove (\"$rm_prog_name\") does not exist in the library.");
    }
  }

  dbg_dump_cat("after all processing", $lib_cat_dump_final_file, "lib_cat", \%lib_cat)
      if ($debug > 1) or (exists $ENV{WP34S_LIB_FINAL_SRC} and ($ENV{WP34S_LIB_FINAL_SRC} ne ""));
}

if ($state_file and not $conv_state2flash) {
  # Create a new binary of the final program set and inject the program tokens into a newly
  # constituted state file. The old calculator state data will be transfered directly from
  # the orignal copy to the new state file. A CRC will be recalculated over the entire file
  # and will be injected into the new state file.
  my $tmp_file = gen_random_writeable_filename();
  @status = reassemble_output(\%lib_cat, $tmp_file, $STATE_MODE);
  rebuild_state_file(\%state_data, $tmp_file, $out_libfile);
  unlink $tmp_file;

} else {
  # Reassemble the new library. In the case of a run where we have not been asked to do anything, this
  # will be the original disassembled source library.
  @status = reassemble_output(\%lib_cat, $out_libfile, $FLASH_MODE);
}

# Display a final catalogue if requested.
if ($gen_cat and (@new_srcs or @rm_progs or $conv_state2flash)) {
  @lib_src = disassemble_binary($out_libfile);
  my $lib_format = "";
  if ($state_file and not $conv_state2flash) {
    $lib_format = "State file";
  } else {
    $lib_format = "flash file";
  }
  print "Modified library catalogue (format: $lib_format)\n";
  show_catalogue("  ", "$out_libfile", @lib_src);
}

# Display the output of the last assembly run to show the user how big everything is.
unless ($quiet) {
  print "Library details:\n";
  print join "\n", @status, "\n";
}

# If we haven't been asked to do anything, a dummy output library name will have been generated.
# We need to blow it away now.
if (not @new_srcs and not @rm_progs and not $conv_state2flash) {
  unlink $out_libfile;
}


#######################################################################
# Start of subroutine suite
#######################################################################
#
# Disassemble a binary image and return the listing as an array of lines.
# All commented and blank lines will be removed.
#
sub disassemble_binary {
  my $file = shift;
  debug_msg(this_function_script((caller(0))[3]), "Processing library binary for '$file'") if $debug;
  local $_;
  my @raw_src = ();
  my @src = ();
  my $cmd = $asm_script;
  my $cmd_line = "-dis $file $disasm_options";
  @raw_src = run_prog($cmd, $cmd_line);
  @src = clean_array_of_whitespace_and_comments(@raw_src);
  return @src;
} # disassemble_binary


#######################################################################
#
# Split a disassembled listing into program segments bounded by END
# statements. Parses these program segments into a hash of arrays references
# using the program name as the key. If the program name is single quoted,
# the quotes are part of the key name. Returns a reference to a hash.
#
sub catalogue_binary {
  my $src_file = shift;
  my @src = @_; # Array of 'cleaned' program lines.
  debug_msg(this_function_script((caller(0))[3]), "Parsing listing catalogue...") if $debug;
  local $_;
  my %cat = ();

  my @segment_refs = split_END($src_file, @src);
  foreach my $segment_ref (@segment_refs) {
    my @this_prog_src = ();

    # Recast the reference back into an array.
    my @segment = @{$segment_ref};

    my $prog_name = "";
    my $first_line = shift @segment; # Get 1st line of source and find the program name.

    # XXX This (and many other lines) cannot deal with a program that starts with one of
    #     the hotkey labels.
    if ($first_line =~ /^\s*\*{0,}LBL\'(.+)\'/) {
      $prog_name = $1;

    # This will effectively scrub any non-program from the catalogue. This would
    # include NULL programs (ie: a program consisting of only an 'END' statement).
    } else {
      warn_msg(this_function_script((caller(0))[3]), "Removing single line program: '$first_line'") if $debug or not $quiet;
      next;
    }

    debug_msg(this_function_script((caller(0))[3]), "  Found program: \"$prog_name\"") if $debug;
    push @this_prog_src, $first_line;

    # Make sure we don't already have a duplicate. This would indicate a badly built
    # library.
    if (exists $cat{$prog_name} and $die_on_existing_duplicate) {
      die_msg(this_function_script((caller(0))[3]), "Invalid library. Has duplicate label: \"$prog_name\".");
    }

    # Slurp up the rest of the program segment.
    while (@segment) {
      my $line = shift @segment;

      # XXX Shitty little (temporary?) workaround for a long-standing bug in the PP!
      #     It turns out that the PP cannot read a bare source line that consists of
      #     only a 0! Will have to fix thix as some time.
      if ($line =~ /^0+$/) {
        $line .= " // dummy comment to workaround long standing wp34s_pp.pl bug!";
      }
      push @this_prog_src, $line;
    }
    $cat{$prog_name} = \@this_prog_src;
  }
  return \%cat;
} # catalogue_binary


#######################################################################
#
# Read in and decompose the state file into its components.
#
sub parse_state_file {
  my $file = shift;
  debug_msg(this_function_script((caller(0))[3]), "Processing state file '$file'") if $debug > 1;
  local $_;
  my %state = ();
  my @state_file_array = read_bin($file);

  $state{MAX_SIZE_AVAIL} = $state_file_array[0];
  $state{LAST_WORD_USED} = $state_file_array[1];
  $state{ORG_CRC} = $state_file_array[-1];

  $state{STATE_START} = $state{MAX_SIZE_AVAIL} + 2;
  $state{STATE_END} = (scalar @state_file_array) - 2;

  # Does NOT include the CRC!
  my @state_array = @state_file_array[$state{STATE_START} .. $state{STATE_END}];
  $state{STATE} = \@state_array;

  if ($chk_crc or $debug) {
    my $recalc_crc = calc_crc16(@state_file_array[0 .. $state{STATE_END}]);
    if ($recalc_crc != $state{ORG_CRC}) {
      my $msg = "Original CRC (0x" . dec2hex4($state{ORG_CRC}) . ") not equal to recalculated CRC (0x" . dec2hex4($recalc_crc) . ").";
      die_msg(this_function_script((caller(0))[3]), $msg);
    } else {
      my $msg = "Original CRC (0x" . dec2hex4($state{ORG_CRC}) . ") matches recalculated CRC (0x" . dec2hex4($recalc_crc) . ").";
      debug_msg(this_function_script((caller(0))[3]), $msg);
    }
  }

  # Use 0 for the initial dummy CRC. The disassembler doesn't care.
  my @src_bin = (0, $state{LAST_WORD_USED}, @state_file_array[2 .. ($state{LAST_WORD_USED}+1)]);
  my $tmp_file = gen_random_writeable_filename();
  write_bin($tmp_file, @src_bin);
  my $cmd = $asm_script;
  my $cmd_line = "-dis $tmp_file $disasm_options";
  my @raw_src = run_prog($cmd, $cmd_line);
  my @tmp_array = clean_array_of_whitespace_and_comments(@raw_src);
  $state{SRC} = \@tmp_array;
  unlink $tmp_file;
  dbg_show_state("after being parsed:", "\%state", \%state) if $debug;
  return \%state;
} # parse_state_file


#######################################################################
#
#
#
sub prepare_new_srcs {
  my @srcs = @_;
  my $src_list = join " ", @srcs;

  my $dbg_msg = "Preparing new source(s): " . join ", ", @srcs;
  debug_msg(this_function_script((caller(0))[3]), "$dbg_msg") if $debug;

  my $tmp_file = gen_random_writeable_filename();
  my $cmd = $asm_script;
  my $cmd_line = "$use_pp $src_list -o $tmp_file $asm_options -lib";
  my @result = run_prog($cmd, $cmd_line);
  my @new_src = disassemble_binary($tmp_file);
  my %news_src_cat = %{catalogue_binary($tmp_file, @new_src)};
  unlink $tmp_file unless (exists $ENV{WP34S_LIB_KEEP_TEMP} and ($ENV{WP34S_LIB_KEEP_TEMP} == 1));
  return \%news_src_cat;
} # prepare_new_srcs


#######################################################################
#
# Split the source into groups of arrays based on the END opcode. Return
# a reference to the array of array references. If the last statement is not
# an END, one will be synthetically added there.
#
# Returns an array of references to arrays containing the source lines of
# each program segment.
#
sub split_END {
  my $src_file = shift;
  my @src = @_;
  my @org = @src;
  my @END_segments = ();
  my $dbg_msg = "";
  local $_;

  unless (@src) {
    die_msg(this_function_script((caller(0))[3]), "Likely the assembler run failed. Halting $script_name.");
  }

  # Make sure there is an END as the last step.
  unless ($src[-1] =~ /(^|\s+)END($|\s+)/) {
    die_msg(this_function_script((caller(0))[3]), "Invalid library ($src_file). Missing END statement as last line.");
  }

  # Scan through the source cutting it up into segments delimited by "END".
  my @segment = ();
  my $ln = 1; # Debug use only
  $dbg_msg = "Starting out with " . scalar @src . " lines.";
  debug_msg(this_function_script((caller(0))[3]), $dbg_msg) if $debug > 3;
  while (@src) {
    my $l = shift @src;
    next if $l =~ /^\s*$/;
    next if $l =~ /^\s*\/\//;

    debug_msg(this_function_script((caller(0))[3]), "New line: '$l' at line number '$ln'.") if $debug > 3;
    push @segment, $l;

    # Detect an END instruction and split off source group.
    if ($l =~ /(^|\s+)END($|\s+)/) {
      my @this_segment = @segment;
      push @END_segments, \@this_segment;
      @segment = ();
      debug_msg(this_function_script((caller(0))[3]), "Found and split off an END segment at line $ln.") if $debug > 3;
      $dbg_msg = "Currently there are " . scalar @END_segments . " segments.";
      debug_msg(this_function_script((caller(0))[3]), $dbg_msg) if $debug > 3;
    }
    $ln++; # Debug use only
  }
  $dbg_msg = "Ended up with " . scalar @END_segments . " segments.";
  debug_msg(this_function_script((caller(0))[3]), $dbg_msg) if $debug > 3;
  return @END_segments;
} # split_END


#######################################################################
#
#
#
sub replace_prog {
  my $cat_ref = shift;
  my $prog_name = shift;
  my $prog_src_ref = shift;
  if (not exists $cat_ref->{$prog_name}) {
    die_msg(this_function_script((caller(0))[3]), "Internal error: Program \"$prog_name\" does not exist in current library.");
  }
  debug_msg(this_function_script((caller(0))[3]), "Replacing program: \"$prog_name\"") if $debug;
  $cat_ref->{$prog_name} = $prog_src_ref;
  return $cat_ref;
} # replace_prog


#######################################################################
#
#
#
sub add_prog {
  my $cat_ref = shift;
  my $prog_name = shift;
  my $prog_src_ref = shift;
  if (exists $cat_ref->{$prog_name}) {
    die_msg(this_function_script((caller(0))[3]), "Internal error: Program \"$prog_name\" already exists in current library.");
  }
  debug_msg(this_function_script((caller(0))[3]), "Adding program: \"$prog_name\"") if $debug > 0;
  $cat_ref->{$prog_name} = $prog_src_ref;
  return $cat_ref;
} # add_prog


#######################################################################
#
#
#
sub remove_prog {
  my $cat_ref = shift;
  my $prog_name = shift;
  if (not exists $cat_ref->{$prog_name}) {
    die_msg(this_function_script((caller(0))[3]), "Internal error: Program \"$prog_name\" does not exist in current library.");
  }
  debug_msg(this_function_script((caller(0))[3]), "Removing program: \"$prog_name\"") if $debug > 0;
  delete $cat_ref->{$prog_name};
  return $cat_ref;
} # remove_prog


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
#
#
sub reassemble_output {
  my $cat_ref = shift;
  my $output_file = shift;
  my $mode = shift; # '-lib' for a flash library, and '' for a state file.

  # Create a temporary intermediate file holding the raw sources concatenated together.
  my $tmp_file = gen_random_writeable_filename();
  open TMP, "> $tmp_file" or die_msg(this_function_script((caller(0))[3]), "Cannot open temp file '$tmp_file' for writing: $!");

#  debug_msg(this_function_script((caller(0))[3]), "Assembling final catalogue into \"$output_file\"") if $debug > 0;

  # Rebuild the file source file.
  my %cat = %{$cat_ref};
  for my $prog (sort keys %cat) {
    local $_;
    my @src = @{$cat{$prog}};
    foreach (@src) {
      print TMP "$_\n";
    }
  }
  close TMP;

  my $cmd = $asm_script;
  my $cmd_line = "$use_pp $tmp_file $mode -o $output_file";
  my @result = run_prog($cmd, $cmd_line);
  unlink $tmp_file unless (exists $ENV{WP34S_LIB_KEEP_TEMP} and ($ENV{WP34S_LIB_KEEP_TEMP} == 1));

  return @result;
} # reassemble_output


#######################################################################
#
#
#
sub rebuild_state_file {
  my $state_ref = shift;
  my $bin_file = shift;
  my $output_file = shift;
  local $_;

  my @bin_array = read_bin( $bin_file );

  # Create a blank array of the required size. By create a static size, we will ensure that the
  # state will be injected at the correct location. By blanking it, we will make it easier to
  # look around in side. The actual content of the unused areas is irrelevent but 0's are easier
  # to deal with for humans.
  my @new_state_words = ();
  foreach (0 .. ($state_ref->{STATE_START}-1)) {
    push @new_state_words, 0;
  }

  # Set the LWU to the value found in the newly assembled state file program suite.
  $state_ref->{LAST_WORD_USED} = $bin_array[1];

  if ($state_ref->{LAST_WORD_USED} > $state_ref->{MAX_SIZE_AVAIL}) {
    my $msg = "Program(s) size of final state file exceeds maximum allowed in configuration: 0x"
            . dec2hex4($state_ref->{LAST_WORD_USED}) . " > 0x" . dec2hex4($state_ref->{MAX_SIZE_AVAIL}) . ".";
    die_msg(this_function_script((caller(0))[3]), $msg);
  }

  # Copy the assembled output to the new array. We need to add 2 extra words because we need
  # to account for the MaxSizeAvail and LastWordUsed.
  for (my $k = 0; $k < $state_ref->{LAST_WORD_USED}+2; $k++) {
    $new_state_words[$k] = $bin_array[$k];
  }

  # Copy the actual values back into the array.
  $new_state_words[0] = $state_ref->{MAX_SIZE_AVAIL};
  $new_state_words[1] = $state_ref->{LAST_WORD_USED};

  # Copy the original state.
  my @old_state = @{$state_ref->{STATE}};
  foreach (@old_state) {
    push @new_state_words, $_;
  }

  # Calculate and insert the final CRC. This one covers the entire state area.
  my $crc = calc_crc16(@new_state_words);
  push @new_state_words, $crc;

  write_bin($output_file, @new_state_words);

#  debug_msg(this_function_script((caller(0))[3]), "Assembling final catalogue into \"$output_file\"") if $debug > 0;
  return;
} # rebuild_state_file


#######################################################################
#
# Run the requested program and command line. The program is split from the
# command line so we can attempt to locate it.
#
sub run_prog {
  my $prog = shift;
  my $cmd_line = shift;
  my @output = ();

  my ($location, $cmd);

  # Look in the current directory.
  if (-e "${prog}") {
    $location = ".";
    $cmd = "$location/$prog $cmd_line"

  # Look in the same location as the script that is executing.
  } elsif ($script_dir and -e "${script_dir}${prog}") {
    $location = "$script_dir";
    $cmd = "$location/$prog $cmd_line"

  } else {
    die_msg(this_function_script((caller(0))[3]), "Cannot locate daughter script '$prog' in current directory or '$script_dir'.");
  }
  debug_msg(this_function_script((caller(0))[3]), "Spawning command line: '$cmd'")
    if ($debug > 1) or (exists $ENV{WP34S_LIB_SPAWN_DBG} and ($ENV{WP34S_LIB_SPAWN_DBG} == 1));

  @output = `$cmd 2>&1`; # Make sure to slurp up the STDERR to STDOUT so we can see any errors.
  my @cleaned = clean_array_of_eol(@output);
  if (has_errors(@cleaned)) {
    warn join "\n", @cleaned, "\n";
    die_msg(this_function_script((caller(0))[3]), "Error occured executing '$cmd'");
  }
  my $dbg_msg = "Output of spawned program:\n" . join "\n", @cleaned;
  debug_msg(this_function_script((caller(0))[3]), $dbg_msg) if $debug > 1;

  return @cleaned;
} # run_prog


#######################################################################
#
# Remove any end-of-lines from an array of lines.
#
sub clean_array_of_eol {
  my @raw = @_;
  my @cleaned = ();
  local $_;
  foreach (@raw) {
    chomp; chomp;
    push @cleaned, $_;
  }
  return @cleaned;
} # clean_array_of_eol


#######################################################################
#
# Remove any whitespace and comments.
#
sub clean_array_of_whitespace_and_comments {
  my @raw = @_;
  my @cleaned = ();
  local $_;
  foreach (@raw) {
    chomp; chomp;
    next if /^\s*\/\//;
    next if /^\s*$/;
    push @cleaned, $_;
  }
  return @cleaned;
} # clean_array_of_whitespace_and_comments


#######################################################################
#
# Generate a random file name that can be used and thrown away.
# It is thrown away by the consumer, not in here!
#
sub gen_random_writeable_filename {
  my $filename = rand();
  $filename = ".__${filename}.tmp";
  my $attempts_left = 10;
  while( -e "$filename" ) {
    $filename = rand();
    $filename = ".__${filename}.tmp";
    $attempts_left--;
    if( $attempts_left < 0 ) {
      die_msg(this_function_script((caller(0))[3]), "Could not succeed in creating a temporary file: $!");
    }
  }
  return $filename;
} # gen_random_writeable_filename


#######################################################################
#
# Scan through the listing gleening the names of the programs.
#
sub show_catalogue {
  my $leader = shift;
  my $src_id = shift;
  my @src = @_;
  local $_;
  my $ln = 1;
  my $l = shift @src;
  print_prog_name($leader, $src_id, $l, $ln);
  while (@src) {
    next if $l =~ /^\s*$/;
    next if $l =~ /^\s*\/\//;

    # Detect an END instruction and split off source group.
    if ($l =~ /(^|\s+)END($|\s+)/) {
      $l = shift @src;
      $ln++;
      print_prog_name($leader, $src_id, $l, $ln);
    } else {
      $l = shift @src;
      $ln++;
    }
  }
  return;
} # show_catalogue


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
sub write_bin {
  my $file = shift;
  my @bin_array = @_;
  local $_;
  open OUT, "> $file" or die_msg(this_function_script((caller(0))[3]), "Cannot open file '$file' for writing: $!");
  binmode OUT;
  foreach (@bin_array) {
    my $bin_lo = $_ & 0xFF;
    my $bin_hi = $_ >> 8;
    print OUT chr($bin_lo), chr($bin_hi);
  }
  close OUT;
  return;
} # write_bin


#######################################################################
#
#
#
sub read_bin {
  my $file = shift;
  open BIN, $file or die_msg(this_function_script((caller(0))[3]), "Cannot open file '$file' for reading: $!");
  # This trick will read in the binary image in 16-bit words of the correct endian.
  local $/;
  my @bin_array = unpack("S*", <BIN>);
  return @bin_array;
} # read_bin


#######################################################################
#
# Extract a print a program name by examining the LBL.
#
sub dbg_show_cat {
  my $epoch = shift; # "When" this dump is occuring in the flow of processing.
  my $hash_name = shift;
  my $cat_ref = shift;
  debug_msg(this_function_script((caller(0))[3]), "Current contents of \%${hash_name} $epoch:");
  for my $prog_name (sort keys %{$cat_ref}) {
    my $steps = scalar @{$cat_ref->{$prog_name}};
    debug_msg(this_function_script((caller(0))[3]), "  Program: \"$prog_name\", steps: $steps");
  }
  return;
} # dbg_show_cat


#######################################################################
#
# Dump a catalogue hash to a file.
#
sub dbg_dump_cat {
  my $epoch = shift; # "When" this dump is occuring in the flow of processing.
  my $file = shift;
  my $hash_name = shift;
  my $cat_ref = shift;
  debug_msg(this_function_script((caller(0))[3]), "Dumping contents of \%${hash_name} $epoch to '$file'");
  open DUMP, "> $file" or die_msg(this_function_script((caller(0))[3]), "Cannot open cat dump file '$file' for writing: $!");
  for my $prog_name (sort keys %{$cat_ref}) {
    local $_;
    foreach (@{$cat_ref->{$prog_name}}) {
      print DUMP "$_\n";
    }
  }
  close DUMP;
  return;
} # dbg_dump_cat


#######################################################################
#
# Dump a summary of the state hash.
#
sub dbg_show_state {
  my $epoch = shift; # "When" this dump is occuring in the flow of processing.
  my $hash_name = shift;
  my $hash_ref = shift;
  my $msg = "";
  debug_msg(this_function_script((caller(0))[3]), "Contents of ${hash_name} $epoch");

  $msg = "  Max size available: " . $hash_ref->{MAX_SIZE_AVAIL} . " (0x" . dec2hex4($hash_ref->{MAX_SIZE_AVAIL}) . ")";
  debug_msg(this_function_script((caller(0))[3]), $msg);

  $msg = "  Last word used:     " . $hash_ref->{LAST_WORD_USED} . " (0x" . dec2hex4($hash_ref->{LAST_WORD_USED}) . ")";
  debug_msg(this_function_script((caller(0))[3]), $msg);

  $msg = "  Original CRC:       " . $hash_ref->{ORG_CRC} . " (0x" . dec2hex4($hash_ref->{ORG_CRC}) . ")";
  debug_msg(this_function_script((caller(0))[3]), $msg);

  $msg = "  State start loc:    " . $hash_ref->{STATE_START} . " (0x" . dec2hex4($hash_ref->{STATE_START}) . ")";
  debug_msg(this_function_script((caller(0))[3]), $msg);

  $msg = "  State end loc:      " . $hash_ref->{STATE_END} . " (0x" . dec2hex4($hash_ref->{STATE_END}) . ")";
  debug_msg(this_function_script((caller(0))[3]), $msg);

  $msg = "  Source length:      " . scalar @{$hash_ref->{SRC}};
  debug_msg(this_function_script((caller(0))[3]), $msg);
  if ($debug > 2) {
    print join "\n", @{$hash_ref->{SRC}};
  }

  return;
} # dbg_show_state


#######################################################################
#
# Extract a print a program name by examining the LBL.
#
sub print_prog_name {
  my $leader = shift;
  my $src_id = shift;
  my $prog_line = shift;
  my $ln = shift;
  my $prog_name = "";
  if ($prog_line =~ /^\s*\*{0,}LBL\'(.+)\'/) {
    $prog_name = $1;
    print "${leader}Source: $src_id, Program name: $prog_name, Line number: $ln\n";
  } elsif ($prog_line =~ /^\s*END(\s+|$)/) {
    if ($debug or $quiet) {
      print "${leader}Source: $src_id, Program name: --Bare END--, Line number: $ln\n";
    } else {
      warn_msg(this_function_script((caller(0))[3]), "Appears to be a NULL program. First line of propram was 'END'.") unless $quiet and not $debug;
    }
  } else {
    die_msg(this_function_script((caller(0))[3]), "First line of propram was not a correctly formatted LBL: '$prog_line'");
  }
  return;
} # print_prog_name


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
  die "$msg\n";
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
  warn "$msg\n";
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
# Scan the array for "ERROR".
#
sub has_errors {
  my @raw = @_;
  local $_;
  my $result = 0;
  foreach (@raw) {
    if (/ERROR\:/) {
      $result = 1;
      last;
    }
  }
  return $result;
} # has_errors


#######################################################################
#
# Swap the main:: for the actual script name.
#
sub this_function_script {
  my $this_function = shift;
  $this_function = "main" if not defined $this_function or ($this_function eq "");
  $this_function =~ s/main/$script_name/;
  return $this_function;
} # this_function_script


#######################################################################
#
#
#
sub print_version {
  print "$script\n";
  my $svn_rev = extract_svn_version();
  print "SVN version: $svn_rev\n" if $svn_rev;
  return;
} # print_version


#######################################################################
#######################################################################
#
# Process the command line option list.
#
sub get_options {
  my ($arg);
  while ($arg = shift(@ARGV)) {

    # See if help is asked for
    if( ($arg eq "-h") or ($arg eq "-help") or ($arg eq "--help") or ($arg eq "-?")) {
      print "$usage\n";
      die "\n";
    }

    elsif( ($arg eq "--version") or ($arg eq "-V") ) {
      print_version();
      die "\n";
    }

    elsif( $arg eq "-d" ) {
      $debug = shift(@ARGV);
    }

    elsif( $arg eq "-rm" ) {
      my $rm_tmp = shift(@ARGV);
      my @rm_list = split /\s+/, $rm_tmp;
      push @rm_progs, @rm_list;
    }

    elsif( ($arg eq "-cat") or ($arg eq "-c") ) {
      $gen_cat = 1;
    }

    elsif( ($arg eq "-ilib") or ($arg eq "-i") ) {
      $in_libfile = shift(@ARGV);
    }

    elsif( ($arg eq "-olib") or ($arg eq "-o") ) {
      $out_libfile = shift(@ARGV);
    }

    elsif( $arg eq "-f" ) {
      $out_libfile = $in_libfile;
    }

    elsif( ($arg eq "-state") or ($arg eq "-s") ) {
      $state_file = 1;
    }

    elsif( ($arg eq "-conv") or ($arg eq "-s2f") ) {
      $conv_state2flash = 1;
    }

    elsif( ($arg eq "-check_crc") or ($arg eq "-cc") ) {
      $chk_crc = 1;
    }

    elsif( $arg eq "-q" ) {
      $quiet = 1;
      $debug = 0;
    }

    elsif( $arg eq "-pp" ) {
      $use_pp = "-pp";
    }

    elsif( $arg eq "-no_pp" ) {
      $use_pp = "";
    }

    elsif( $arg eq "-nc" ) {
      $use_ansi_colour = 0;
    }

    elsif( ($arg eq "-ac") or ($arg eq "-colour") or ($arg eq "-color") ) {
      $use_ansi_colour = 1;
    }

    else {
      push @new_srcs, $arg;
    }
  }

  #----------------------------------------------
  # Check the sanity of the command line arguments.

  if ($state_file and not $in_libfile) {
    warn "ERROR: Cannot create a state file/library from scratch. Can only modify one.\n";
    die  "       Enter '$script_name -h' for help.\n";
  }

  # See if nothing is being asked to be done. This is not really an error, so no loud
  # admonishments, just a helpful indication.
  if ((not @new_srcs and not @rm_progs and not $out_libfile and not $in_libfile)
   or (not @new_srcs and not @rm_progs and not $out_libfile and $in_libfile and not $gen_cat)) {
    print_version();
    die "Enter '$script_name -h' for help.\n";
  }

  unless ($out_libfile or (not @new_srcs and not @rm_progs)) {
    warn "ERROR: Must enter an output file name in for recieving binary library (-olib SomeFileName).\n";
    warn "       or use the force-override switch (-f).\n";
    die  "       Enter '$script_name -h' for help.\n";
  }

  # We have only an input library and no actions.
  if (not @new_srcs and not @rm_progs and not $out_libfile and $in_libfile) {
    $out_libfile = gen_random_writeable_filename();
  }

  # We have only an output library and no input library.
  if ($out_libfile and not $in_libfile) {
    print "Creating olib '$out_libfile' from scratch.\n";
  }

  return;
} # get_options
