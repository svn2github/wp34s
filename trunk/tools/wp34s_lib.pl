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

#use strict;
use POSIX;
use File::Basename;

# ---------------------------------------------------------------------

my $debug = 0;
my $quite = 0;

my $in_libfile = "";
my $out_libfile = "";
my $force_lib_overwrite = 0;
my $gen_cat = 0;

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
my $ansi_rev_blue_bg      = "\e[47;1;7;34;1m";
my $ansi_rev_cyan_bg      = "\e[30;46m";

my $DEFAULT_USE_ANSI_COLOUR = 0;
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
   $script_name [src_file [src_file2 [src_file3]]] -ilib org_lib.dat [-olib new_lib.dat|-f] \\
                  [-rm "'PRG'" [-rm "'PG2'"]|-rm "'PRG' 'PG2' [...]"] [-cat]

Parameters:
   src_file         One or more source files to add or replace within an existing library.
   -ilib libfile    The original library file to add or replace programs in. Unless this has the same
                    name as outfile, this file will not be modified.
   -olib outfile    Output produced by the tool. This is required and will be the binary flash image.
                    Libraries extensions are conventionally ".dat". Must be supplied unless the '-f'
                    override switch is used.
   -f               Force the ilib to be used as the olib name as well.
   -pp              Use the preprocessor when assembling the sources. [default]
   -no_pp           Don\'t use the preprocessor when assembling the sources.
   -rm "'PRG'"      Name of program(s) to remove from the library. This switch can appear multiple
                    times to name multiple programs, or a single instance of the switch can name
                    multiple programs.
                    NOTE: Currently, program names MUST be enclosed in outer double quotes AND single
                          inner quotes!
   -cat             Display catalogue of initial and final library.
   -h               This help script.

Examples:
  \$ $script_name great_circle.wp34s -olib wp34s-lib.dat -ilib wp34s-lib.dat
  - Assembles the named WP34s program source file producing and either replaces an existing
    version in the named library or adds it. In this case the output and named library have
    the same name, so the original library file will be overwritten.

  \$ $script_name  great_circle.wp34s floating_point.wp34s -olib wp34s-1.dat -ilib wp34s-1.dat
  - Assembles the named WP34s program source files producing and either replaces existing
    versions in the named library or adds them. In this case the output and named library have
    the different names, so the original library file will be untouched.

  \$ $script_name -ilib wp34s-lib.dat -f -rm ABC -rm BCD
  \$ $script_name -ilib wp34s-lib.dat -ilib wp34s-lib.dat -rm "ABC BCD"
  - These 2 invocations have the identical result. They remove the 2 programs 'ABC'
    and 'BCD' from the named library and write the resulting library back to the
    sam named library. The first invocation 'forces' the input library to be overwritten.

Notes:
  1)
EOM

#######################################################################

get_options();

# Disassemble the library.
my @lib_src = disassemble_binary($in_libfile);

if ($gen_cat) {
  print "Initial library catalogue:\n";
  show_catalogue("  ", "$in_libfile", @lib_src);
}


# Catalogue the initial library.
my %lib_cat = %{catalogue_binary($in_libfile, @lib_src)};

dbg_show_cat("after initial in_lib processing", "lib_cat", \%lib_cat)
    if $debug or (exists $ENV{WP34S_LIB_CAT_DBG} and ($ENV{WP34S_LIB_CAT_DBG} == 1));
dbg_dump_cat("after processing org lib", "org_lib_${lib_cat_dump_basefile}", "lib_cat", \%lib_cat)
    if $debug or (exists $ENV{WP34S_LIB_CAT_DUMP} and ($ENV{WP34S_LIB_CAT_DUMP} ne ""));

# Only execute the modification section if we have been requested to do something.
if (@new_srcs or @rm_progs) {
  # Only run this section if there are programs that need to be added.
  if (@new_srcs) {
    # Prepare the new source files.
    my %new_progs = %{prepare_new_srcs(@new_srcs)};

    dbg_show_cat("after new programs preparation", "new_progs", \%new_progs)
        if $debug or (exists $ENV{WP34S_LIB_CAT_DBG} and ($ENV{WP34S_LIB_CAT_DBG} == 1));
    dbg_dump_cat("after processing new files", "new_src_${lib_cat_dump_basefile}", "new_progs", \%new_progs)
        if $debug or (exists $ENV{WP34S_LIB_CAT_DUMP} and ($ENV{WP34S_LIB_CAT_DUMP} ne ""));


    # Replace any existing programs with the new programs, or add the new programs
    # if the program is not currently found in the library.
    for my $new_prog_name (sort keys %new_progs) {
      my $new_prog_src_ref = $new_progs{$new_prog_name};  # a source array
      if (exists $lib_cat{$new_prog_name}) {
        printf "Replacing program: \"%0s\", old program steps: %0d, new program steps: %0d\n",
                $new_prog_name, scalar @{$lib_cat{$new_prog_name}}, scalar @{$new_prog_src_ref} unless $quite;

        %lib_cat = %{replace_prog(\%lib_cat, $new_prog_name, $new_prog_src_ref)};

        dbg_show_cat("after replacing \"$new_prog_name\"", "lib_cat", \%lib_cat)
            if $debug or (exists $ENV{WP34S_LIB_CAT_DBG} and ($ENV{WP34S_LIB_CAT_DBG} == 1));
        dbg_dump_cat("after processing replacement of $new_prog_name",
                    "replace_${new_prog_name}_${lib_cat_dump_basefile}", "lib_cat", \%lib_cat)
            if $debug or (exists $ENV{WP34S_LIB_CAT_DUMP} and ($ENV{WP34S_LIB_CAT_DUMP} ne ""));
      } else {
        printf "Adding program: \"%0s\", new program steps: %0d\n",
                $new_prog_name, scalar @{$new_prog_src_ref} unless $quite;

        %lib_cat = %{add_prog(\%lib_cat, $new_prog_name, $new_prog_src_ref)};

        dbg_show_cat("after adding \"$new_prog_name\"", "lib_cat", \%lib_cat)
            if $debug or (exists $ENV{WP34S_LIB_CAT_DBG} and ($ENV{WP34S_LIB_CAT_DBG} == 1));
        dbg_dump_cat("after processing addition of $new_prog_name",
                    "add_${new_prog_name}_${lib_cat_dump_basefile}", "lib_cat", \%lib_cat)
            if $debug or (exists $ENV{WP34S_LIB_CAT_DUMP} and ($ENV{WP34S_LIB_CAT_DUMP} ne ""));
      }
    }
  }

  # Remove any programs as requested.
  foreach my $rm_prog_name (@rm_progs) {
    if (exists $lib_cat{$rm_prog_name}) {
      printf "Removing program: \"%0s\", old program steps: %0d\n",
              $rm_prog_name, scalar @{$lib_cat{$rm_prog_name}} unless $quite;
      %lib_cat = %{remove_prog(\%lib_cat, $rm_prog_name)};
      dbg_show_cat("after removing \"$rm_prog_name\"", "lib_cat", \%lib_cat) if $debug or (exists $ENV{WP34S_LIB_CAT_DBG} and ($ENV{WP34S_LIB_CAT_DBG} == 1));

      dbg_dump_cat("after processing removal of $rm_prog_name",
          "remove_${rm_prog_name}_${lib_cat_dump_basefile}", "lib_cat", \%lib_cat)
          if $debug or (exists $ENV{WP34S_LIB_CAT_DUMP} and ($ENV{WP34S_LIB_CAT_DUMP} ne ""));
    } else {
      warning_msg(this_function_script((caller(0))[3]), "Program to remove (\"$rm_prog_name\") does not exist in the library.");
    }
  }

  dbg_dump_cat("after all processing", $lib_cat_dump_final_file, "lib_cat", \%lib_cat)
      if $debug or (exists $ENV{WP34S_LIB_FINAL_SRC} and ($ENV{WP34S_LIB_FINAL_SRC} ne ""));
}

# Reassemble the new library. In the case of a run where we have not been asked to do anything, this
# will be the original disassembled source library.
my @status = resassemble_lib(\%lib_cat, $out_libfile);

# Display a final catalogue if requested.
if ($gen_cat and (@new_srcs or @rm_progs)) {
  @lib_src = disassemble_binary($out_libfile);
  print "Modified library catalogue:\n";
  show_catalogue("  ", "$out_libfile", @lib_src);
}

# Display the output of the last assembly run to show the user how big everything is.
unless ($quite) {
  print "Library details:\n";
  print join "\n", @status, "\n";
}

# If we haven't been asked to do anything, a dummy output library name will have been generated.
# We need to blow it away now.
if (not @new_srcs and not @rm_progs) {
  unlink $out_libfile;
}


#######################################################################
# Start of subrountine suite
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
  my @src = @_;
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

    if ($first_line =~ /^\s*\*{0,}LBL\'(.+)\'/) {
      $prog_name = $1;
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

      # XXX Shitty little (temporary) workaround for a long-standing bug in the assembler!
      #     It turns out that the assembler cannot read a bare source line that consists of
      #     only a 0! Will have to fix thix as some time.
      if ($line =~ /^0+$/) {
        $line .= " // dummy comment to workaround long standing wp34s_asm.pl bug!";
      }
      push @this_prog_src, $line;
    }
    $cat{$prog_name} = \@this_prog_src;
  }
  return \%cat;
} # catalogue_binary


#######################################################################
#
#
#
sub prepare_new_srcs {
  #MvC
  #my $new_src_lib = shift;
  my @srcs = @_;
  my $src_list = join " ", @srcs;

  my $dbg_msg = "Preparing new source(s): " . join ", ", @srcs;
  debug_msg(this_function_script((caller(0))[3]), "$dbg_msg") if $debug;

  my $tmp_file = gen_random_writeable_filename();
  my $cmd = $asm_script;
  my $cmd_line = "$use_pp $src_list -o $tmp_file $asm_options";
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
sub resassemble_lib {
  my $cat_ref = shift;
  my $output_file = shift;

  # Create a temporary intermediate file holding the raw sources concatenated together.
  my $tmp_file = gen_random_writeable_filename();
  open TMP, "> $tmp_file" or die_msg(this_function_script((caller(0))[3]), "Cannot open temp file '$tmp_file' for writing: $!");

  debug_msg(this_function_script((caller(0))[3]), "Assembling final catalogue into \"$output_file\"") if $debug > 0;

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
  my $cmd_line = "$use_pp $tmp_file -o $output_file";
  my @result = run_prog($cmd, $cmd_line);
  unlink $tmp_file unless (exists $ENV{WP34S_LIB_KEEP_TEMP} and ($ENV{WP34S_LIB_KEEP_TEMP} == 1));

  return @result;
} # resassemble_lib


#######################################################################
#
#
#
sub run_prog {
  my $prog = shift;
  my $cmd_line = shift;
  my @output = ();

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
# Remove any
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
# Extract a print a program name by examining the LBL.
#
sub dbg_show_cat {
  my $epoch = shift;
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
  my $epoch = shift;
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
  } else {
    die_msg(this_function_script((caller(0))[3]), "First line of propram was not a correctly formatted LBL: '$prog_line'");
  }
  return;
} # print_prog_name


#######################################################################
#
#
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
#
#
sub warn_msg {
  my $func_name = shift;
  my $text = shift;
  my $msg = "";

  $msg = "$ansi_rev_green_bg" if $use_ansi_colour;
  $msg .= "WARNING: $func_name:";
  $msg .= "$ansi_normal " if $use_ansi_colour;
  $msg .= " $text";
  warn "$msg\n";
} # warn_msg


#######################################################################
#
#
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
# Scan the array for ERROR.
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
  my $this_function = shift( @_ );
  $this_function =~ s/main/$script_name/;
  return $this_function;
} # this_function_script


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

    elsif( ($arg eq "--version") or ($arg eq "-V") ) {
      print "$script\n";
      my $svn_rev = extract_svn_version();
      print "SVN version: $svn_rev\n" if $svn_rev;
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

    elsif( $arg eq "-nc" ) {
      $use_ansi_colour = 0;
    }

    elsif( $arg eq "-f" ) {
      $force_lib_overwrite = 1;
    }

    elsif( $arg eq "-q" ) {
      $quite = 1;
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

    else {
      push @new_srcs, $arg;
    }
  }

  #----------------------------------------------
  # Verify we have sufficient parameters.

  unless ($out_libfile and not $force_lib_overwrite or (not @new_srcs and not @rm_progs)) {
    warn "ERROR: Must enter an output file name in for recieving binary library (-olib SomeFileName).\n";
    warn "       or use the force-override switch (-f).\n";
    die  "       Enter '$script_name -h' for help.\n";
  }

  if (not @new_srcs and not @rm_progs and not $out_libfile) {
    $out_libfile = gen_random_writeable_filename();
  }

  return;
} # get_options
