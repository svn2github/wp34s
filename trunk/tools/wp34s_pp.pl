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
my (%LBLs, %targets, %branches, @branches, @steps);

my $DEFAULT_MAX_JMP_OFFSET = 99;
my $MAX_JMP_OFFSET = $DEFAULT_MAX_JMP_OFFSET; # Maximum offset for a BACK/SKIP statement.

my $MAX_LABEL_NUM = 99;  # Maximum label number
my $last_label_used = $MAX_LABEL_NUM+1;

# The labels used as targets for program branches are only allowed to be made from
# the following characters. Additionally, they are always terminated by a double colon.
my $label_spec = "A-Za-z0-9_";
my $longest_label = 0; # Recorded for pretty print purposes.

my $NUM_TARGET_LABEL_COLONS = 2;

my @files;

my $prt_step_num = 1;
my $show_catalogue = 0;

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
   -h               This help script.

Examples:
  \$ $script_name vector_sym.wp34s > vector_pp.wp34s
  - Preprocess a source file to produce a new source listing with all branches and
    symbolic targets resolved, and all strings encoded to the appropriate [alpha]
    instruction.

  \$ $script_name vector_sym.wp34s gc_sym.wp34s > src.wp34s
  - Preprocess multiple source files.

Notes:
  1) All symbolic labels must contain only alphanumerics and/or underscores
     and must be terminated by a double colon. They must be at least 2 characters
     long and must not be only 2 decimal digits ('00::' is illegal but '_00::' is
     acceptable).
  2) You can use JMP instead of BACK and/or SKIP. The tool will decide which
     direction the target label is and will substitute a BACK or SKIP for the
     JMP instruction if the target offset is with the maximum of $DEFAULT_MAX_JMP_OFFSET steps.
     If the target offset if greater than $DEFAULT_MAX_JMP_OFFSET steps, the tool will insert a
     symbolic LBL and will GTO that label. There is no limit to the text-length
     of the labels. Alpha text strings can be entered within double quotes.

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

print "// DEBUG: main: MAX_JMP_OFFSET = $MAX_JMP_OFFSET\n" if $debug;

my (@src, @lines);
foreach my $file (@files) {
  push @src, load_cleaned_source($file);
}

# Split the lines into an array of array references based on END.
my @end_groups = split_END(@src);

print "// $script_name: Source file(s): ", join (", ", @files), "\n";
print "// $script_name: Preprocessor revision: $SVN_Current_Revision \n";

foreach (@end_groups) {
  @lines = @{$_}; # Cast the array element into an array.
  my $length = scalar @lines; # Debug use only.
  print "// DEBUG: main: ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" if $debug;
  print "// DEBUG: main: Processing new END group. Contains $length lines.\n" if $debug;
  initialize_tables();
  preprocessor();
  display_steps("");
  show_LBLs() if $show_catalogue;
}

#######################################################################

#######################################################################
#
# Execute the preprocessor steps on the lines in the global @lines array.
#
sub preprocessor {
  display_steps("// ") if $debug;
  process_double_quotes();
  display_steps("// ") if $debug;
  extract_labels();           # Look for existing "LBL \d{2}" or "LBL [A-Z]"opcodes
  extract_targets();          # Look for "SomeLabel::"
  extract_branches();         # Look for "(BACK|SKIP|JMP) SomeLabel"
  show_state(__LINE__) if $debug;
  insert_synthetic_labels();  # Add synthetic symbolic label for any %LBLs without a counterpart in %targets
  show_state(__LINE__) if $debug;
  check_consistency();
  show_state(__LINE__) if $debug;
  populate_branch_array();
  show_state(__LINE__) if $debug;
  extract_LBLd_opcodes();  # Look for "(GTO|XEQ|SLV|etc) SomeLabel"
  show_state(__LINE__) if $debug;
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
    print "// WARNING: Missing terminal \"END\". Appending after last statement in source.\n";
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
      print "// DEBUG: split_END: Found and split off an END group at line $line_num.\n" if $debug;
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
  %LBLs = ();
  %targets = ();
  %branches = ();
  @branches = ();
  @steps = ();
  return;
} # initialize_tables


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
  my ($substring, $num_chars, $string, $line, $new_line, $label);
  my $step = 1;
  while( $step <= scalar(@lines) ) {
    $line = $lines[$step-1];
    $line =~ s/^\s*\d{0,3}:{0,1}//;
    if( $line =~ /^\s*([$label_spec]{2,}:{$NUM_TARGET_LABEL_COLONS})/ ) {
      $label = $1 . " ";
      $line =~ s/^\s*([$label_spec]{2,}):{$NUM_TARGET_LABEL_COLONS}//;
    } else {
      $label = "";
    }
    # By not being greedy with this search, we can support embedded double quotes.
    if( $line =~ /^\s*\"(.+)\"/ ) {
      my $string = $1;
      my $is_first = 1;
      while( $string ) {
        my $fmt_step = format_step($step);
        ($substring, $num_chars, $string) = extract_substring($string, $step);
        print "// DEBUG: process_double_quotes: Extracted $num_chars characters composed of substring '$substring' from line '$line'\n" if $debug;
        if( $num_chars == 3 ) {
          $new_line = "${label}[alpha]'${substring}'";
        } elsif( $num_chars == 2 ) {
          if( $debug ) {
            print "ERROR: process_double_quotes: Why did we return a 2-character string at step $fmt_step! String: '$string'\n";
            show_state(__LINE__);
            die;
          } else {
            die "ERROR: process_double_quotes: Why did we return a 2-character string at step $fmt_step! String: '$string'\n";
          }
        } else {
          $new_line = "${label}[alpha] ${substring}";
        }

        # Replace the line we are processing if this is the first time through. Thereafter
        # insert anymore lines.
        if( $is_first ) {
          $is_first = 0;
          print "// DEBUG: process_double_quotes: Replacing step '$fmt_step' with '$new_line'\n" if $debug;
          $lines[$step-1] = $new_line . " // $line";
          $label = "";
        } else {
          print "// DEBUG: process_double_quotes: Inserting step '$fmt_step' with '$new_line'\n" if $debug;
          @lines = insert_step($new_line, $step, @lines);
        }
        $step++;
      }
    } else {
      $step++;
    }
  }
  print "// DEBUG: process_double_quotes: Popped out at step '$step'.\n" if $debug;
  return;
} # process_double_quotes


#######################################################################
#
# Process branches and attempt to resolve them.
#
sub preprocess_synthetic_targets {
  local $_;
  my ($current_step);

  print "// DEBUG: preprocess_synthetic_targets: Prior to processing...\n" if $debug;
  show_state(__LINE__) if $debug;
  while( $current_step = shift @branches ) {
    print "// DEBUG: preprocess_synthetic_targets: Processing branch at step $current_step...\n" if $debug;
    my ($target_label, $target_step);
    my $fmt_cur_step = format_step($current_step);

    # See if the instruction is one of the LBL'd ones like XEQ, INT, etc.
    my $line = $lines[$current_step-1];
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
        print "ERROR: preprocess_synthetic_targets: Cannot parse the line at step $current_step! Line: '$line'\n";
        show_state(__LINE__);
        die;
      } else {
        die "ERROR: preprocess_synthetic_targets: Cannot parse the line at step $current_step! Line: '$line'\n";
      }
    }

    # Need to exclude any square bracketing because this confuses the hash search. The
    # square brackets can't be in the hash becuase they are then treated as search sets
    # in regex's.
    my $tmp_op = $op;
    $tmp_op =~ s/\[//;
    $tmp_op =~ s/\]//;
    if( exists $LBLd_ops{$tmp_op} ) {
      $op =~ s<\\><>g; # Remove any escape characters in the hash template.
      my $target_step = search_targets($label);
      my $target_LBL = search_LBLs($target_step);
      $line = replace_with_LBLd_target($target_LBL, $line, $op);
      $lines[$current_step-1] = $line;
      print 1 if 0; # Breakpoint

    } else {
      # From the step number, get the label which is the target.
      if( exists $branches{$fmt_cur_step} ) {
        $target_label = $branches{$fmt_cur_step};
      } else {
        if( $debug ) {
          print "ERROR: preprocess_synthetic_targets: Cannot locate current step (${fmt_cur_step}) in \%branches.\n";
          show_state(__LINE__);
          die;
        } else {
          die "ERROR: preprocess_synthetic_targets: Cannot locate current step (${fmt_cur_step}) in \%branches.\n";
        }
      }

      # From the target label, get the target step number.
      if( exists $targets{$target_label} ) {
        $target_step = $targets{$target_label};
      } else {
        if( $debug ) {
          print "ERROR: preprocess_synthetic_targets: Cannot locate target label '${target_label}' in \%targets.\n";
          show_state(__LINE__);
          die;
        } else {
          die "ERROR: preprocess_synthetic_targets: Cannot locate target label '${target_label}' in \%targets.\n";
        }
      }

      my $offset = $target_step - $current_step;
      if( abs($offset) > $MAX_JMP_OFFSET ) {
        my ($new_label_num);

        print "// DEBUG: preprocess_synthetic_targets: Offset exceeds maximum (abs(${offset}) > ${MAX_JMP_OFFSET}). target_step = $target_step, current_step = $current_step.\n" if $debug;
        ($new_label_num, $current_step) = add_LBL_if_required($target_step, $offset, $current_step);
        $line = replace_with_LBLd_target($new_label_num, $line, "GTO");
        $lines[$current_step-1] = $line;
        print $_ if 0; # Breakpoint!
      } else {
        # At this moment, the offset is within range so we can use a SKIP/BACK opcode.
        # Note that this may change as a result of LBL injections so this may not always stay
        # within range.
        $offset-- if $offset > 0; # Adjust the SKIP offsets by 1.
        $line =~ s/\s+$//;
        $line = replace_with_branch($offset, $line);
        $lines[$current_step-1] = $line;
      }
    }
    print "// DEBUG: preprocess_synthetic_targets: Next branch...\n" if $debug;
    show_state(__LINE__) if $debug;
  }
  print "// DEBUG: preprocess_synthetic_targets: Done...\n" if $debug;
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
  print "// DEBUG: add_LBL_if_required: is_existing_label(${target_step}) => ${new_label_num}\n" if $debug;

  if( $new_label_num < 0 ) {
    # We need to inject a label at the target.
    $new_label_num = get_next_label($target_step+1); # Reserve it for the next step.
    print "// DEBUG: add_LBL_if_required: Adding new label '$new_label_num' at step '$target_step'.\n" if $debug;
    @lines = insert_step("LBL $new_label_num", $target_step, @lines);
    adjust_labels_used($target_step);
    adjust_targets($target_step);
    adjust_branches($target_step);
    adjust_branch_array($target_step);

    # If the label is required to be inserted before this step, the step is going to be incremented
    # to compensate for this.
    if( $offset < 0 ) {
      $current_step++;
    }
  } else {
    print "// DEBUG: add_LBL_if_required: Label number '$new_label_num' already exists at step '$target_step' -- reusing it.\n" if $debug;
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
  my @lines = read_file($file);
  @lines = redact_comments(@lines);
  @lines = remove_blank_lines(@lines);
#  @lines = strip_strip_numbers(@lines);
  return @lines;
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
  for( my $step = 1; $step <= scalar(@lines); $step++ ) {
    my $fmt_step = format_step($step+1); # Because we are pointing at the active step *after* the label.
    my $line = $lines[$step-1];
    if( $line =~ /LBL\s+(\d+)/ or $line =~ /LBL\s+([A-Z])([^A-Z]|$)/ ) {
      my $LBL = $1;
      my $fmt_LBL = format_LBL($LBL);
      if( not exists $LBLs{$fmt_LBL} ) {
        $LBLs{$fmt_LBL} = $fmt_step;
        print "// DEBUG: extract_labels: Found user 'LBL $fmt_LBL' at step $fmt_step.\n" if $debug;
      } else {
        die "ERROR: extract_labels: Cannot support multiple labels with the same name. LBL $fmt_LBL, seen at step $fmt_step, was already seen at step ", $LBLs{$fmt_LBL}, "\n";
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
  for( my $step = 1; $step <= scalar(@lines); $step++ ) {
    if( $lines[$step-1] =~ /^\s*\d{0,3}:{0,1}\s*([$label_spec]{2,}):{$NUM_TARGET_LABEL_COLONS}\s+/ ) {
      my $label = $1;
      adjust_longest_label_length($label);
      if( not exists $targets{$label} ) {
        my $fmt_step = format_step($step);
        $targets{$label} = $fmt_step;
        print "// DEBUG: extract_targets: Found target label ('$label') at step $fmt_step.\n" if $debug;
      } else {
        die "ERROR: Cannot support multiple target labels at different steps. Target label '$label' aleady seen at step ${targets{$label}}\n";
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
  for( my $step = 1; $step <= scalar(@lines); $step++ ) {
    my $fmt_step = format_step($step);
    if( $lines[$step-1] =~ /(BACK|SKIP|JMP)\s+([$label_spec]{2,})/ ) {
      my $label = $2;
      if( $label =~ /^\d{2}$/ ) {
        # This is a "hard" target. We want to convert these to "soft" ones.
        print "// DEBUG: extract_branches: Found 'hard' branch at step $fmt_step with label '$label'.\n" if $debug;
      } else {
        if( not exists $branches{$fmt_step} ) {
          $branches{$fmt_step} = $label;
          print "// DEBUG: extract_branches: Found branch at step $fmt_step with label '$label'.\n" if $debug;
        } else {
          die "ERROR: Cannot support multiple target labels per step. Target label '$label' was already seen at step ${targets{$fmt_step}}\n";
        }
      }
    }
  }
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
  for( my $step = 1; $step <= scalar(@lines); $step++ ) {
    my $line = $lines[$step-1];
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
          if( $label =~ /^\d{2}$|^[A-Z]{1}$/ ) {
            print "// DEBUG: extract_LBLd_opcodes: Skipping 'hard' LBL'd opcode at step $fmt_step with LBL '$label': '$line'.\n" if $debug;

          # Does the target label exist already?
          } elsif( exists $targets{$label} ) {
            print "// DEBUG: extract_LBLd_opcodes: Found LBL'd opcode at step $fmt_step with label '$label': '$line'.\n" if $debug;

            # Get the target label.
            my $target_step = $targets{$label};
            my ($assigned_label);
            print "// DEBUG: extract_LBLd_opcodes: fmt_step = $fmt_step, targets{label} = $target_step\n" if $debug > 1;

            # Search the existing LBLs for a match of the target step. If a LBL at the target step is found, we
            # can reuse it for this purposes of this LBL.
            my $found = 0;
            for my $LBL (sort keys %LBLs) {
              if( eval($LBLs{$LBL}+0) == $target_step ) {
                $found = 1;
                $assigned_label = $LBL;
                last;
              }
            }

            # If a LBL at the target address does not exist, we need to insert one. This will have the ripple
            # effect of moving everything below it down one step.
            if( not $found ) {
              my $offset = $target_step - $step;
              print "// DEBUG: extract_LBLd_opcodes: Label not found at target step '$target_step'. Offset to target is $offset.\n" if $debug > 2;
              ($assigned_label, $step) = add_LBL_if_required($target_step, $offset, $step);
              $fmt_step = format_step($step);
            } else {
              print "// DEBUG: extract_LBLd_opcodes: Found label '$assigned_label' for tarteg step '$target_step'\n" if $debug > 2;
            }

            if( not exists $branches{$fmt_step} ) {
              if( $debug ) {
                print "// DEBUG: extract_LBLd_opcodes: Prior to inserting label '$label' into \%branches:\n";
                show_branches();
              }
              $branches{$fmt_step} = $label;
              if( $debug ) {
                print "// DEBUG: extract_LBLd_opcodes: After inserting label '$label' into \%branches:\n";
                show_branches();
                print "// DEBUG: extract_LBLd_opcodes: Prior to inserting step '$fmt_step' into \@branches:\n";
                show_branches_remaining();
              }
              insert_into_branch_array($step);
            } else {
              if(1) {
                if( $debug ) {
                  print "// DEBUG: extract_LBLd_opcodes: Locate step '$fmt_step' already in \%branches.\n";
                  show_state(__LINE__);
                }
              } else {
                if( $debug ) {
                  print "ERROR: extract_LBLd_opcodes: Locate step '$fmt_step' already in \%branches.\n";
                  show_state(__LINE__);
                  die;
                } else {
                  die "ERROR: extract_LBLd_opcodes: Locate step '$fmt_step' already in \%branches.\n";
                }
              }
            }

          # The required target label does not exist so it is a user error in the source code.
          } else {
            if( $debug ) {
              print "ERROR: extract_LBLd_opcodes: No target label exists for line '$line' at step '$fmt_step'\n";
              show_state(__LINE__);
              die;
            } else {
              die "ERROR: extract_LBLd_opcodes: No target label exists for line '$line' at step '$fmt_step\n";
            }
          }
          last;
        } else {
          print "// DEBUG: extract_LBLd_opcodes: Line is eligible for LBL '$LBLd_op' but does not have an eligible label: line '$line' at step '$fmt_step'\n" if $debug > 1;
        }
        next;
      } else {
        print "// DEBUG: extract_LBLd_opcodes: Line is not eligible for LBL '$LBLd_op': line '$line' at step '$fmt_step'\n" if $debug > 3;
      }
    }
  }
  return;
} # extract_LBLd_opcodes


#######################################################################
#
# Add synthetic symbolic label for any %LBLs without a counterpart in %targets.
#
sub insert_synthetic_labels {
  # Process the existing LBLs.
  for my $LBL (sort keys %LBLs) {
    my $label_step = $LBLs{$LBL};
    search_and_insert_synthetic_label("LBL", $LBL, $label_step);
  }

  # Add synthetic labels for %branches with %02d labels and fix up the orignal statement.
  for my $step (sort keys %branches) {
    my $fmt_step = format_step($step);
    my $target_label = $branches{$step};
    if( $target_label =~ /^\d{2}$/ ) {
      my $line = $lines[$step-1];
      if( $line =~ /BACK\s+(\d{2})/ ) {
        my $offset = $1;
        my $target_step = format_step($step - $offset);
        my $new_label = search_and_insert_synthetic_label("BACK", $offset, $target_step);
        print "// DEBUG: insert_synthetic_labels: Replacing: '$line'\n" if $debug;
        $line =~ s/BACK\s+(\d{2})/BACK ${new_label} \/\/ $1/;
        $lines[$step-1] = $line;
        print "// DEBUG: insert_synthetic_labels: With:      '$line'\n" if $debug;
        replace_branch($step, $new_label);
      } elsif( $line =~ /SKIP\s+(\d{2})/ ) {
        my $offset = $1;
        my $target_step = format_step($step + $offset + 1);
        my $new_label = search_and_insert_synthetic_label("SKIP", $offset, $target_step);
        print "// DEBUG: insert_synthetic_labels: Replacing: '$line'\n" if $debug;
        $line =~ s/SKIP\s+(\d{2})/SKIP ${new_label} \/\/ $1/;
        $lines[$step-1] = $line;
        print "// DEBUG: insert_synthetic_labels: With:      '$line'\n" if $debug;
        replace_branch($step, $new_label);
      } else {
        die "ERROR: insert_synthetic_labels: Cannot parse line at step '$step': '$line'.\n";
      }
    }
  }
  print "// DEBUG: insert_synthetic_labels: Done.\n" if $debug;
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
    print "// DEBUG: replace_branch: Replacing branch: '${branches{$fmt_step}}'\n" if $debug;
    $branches{$fmt_step} = $new_label;
    print "// DEBUG: replace_branch: With:             '${branches{$fmt_step}}'\n" if $debug;
  } else {
    if( $debug ) {
      print "ERROR: replace_branch: Branch unexpectedly does not exist for step '$fmt_step'.\n";
      show_state(__LINE__);
      die;
    } else {
      die "ERROR: replace_branch: Branch unexpectedly does not exist for step '$fmt_step'.\n";
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
    print "// DEBUG: search_and_insert_synthetic_label: Inserting synthetic label ('$assigned_label') at step '$fmt_step' for '$type $seed'.\n" if $debug;

    # We have confirmed that no actual label exists so substitute it in.
    print "// DEBUG: search_and_insert_synthetic_label: Original line: '", $lines[$label_step-1], "'\n" if $debug > 1;
    if( $lines[$label_step-1] =~ /^\s*\d{3}:{0,1}/ ) {
      $lines[$label_step-1] =~ s/^(\s*\d{3}:{0,1})\s+(.+)/${1} ${assigned_label}:: $2/;
    } else {
      $lines[$label_step-1] =~ s/^(\s*)(.+)/${1}${assigned_label}:: $2/;
    }
    print "// DEBUG: search_and_insert_synthetic_label: Modified line: '", $lines[$label_step-1], "'\n" if $debug > 1;

  } else {
    print "// DEBUG: search_and_insert_synthetic_label: Label ('$assigned_label') already exists at step '$fmt_step' for label '$type $seed'.\n" if $debug;
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
  print "// DEBUG: check_consistency: Stubbed....\n" if $debug;
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
    print "${leader}$_\n";
  }
  return;
} # display_steps


#######################################################################
#
# Reconstruct the steps
#
sub reconstruct_steps {
  my $label_field_length = $longest_label+$NUM_TARGET_LABEL_COLONS;
  my ($label, $opcode, @reconstructed_steps);

  for( my $step = 1; $step <= scalar(@lines); $step++ ) {
    my $line = $lines[$step-1];

    # Scrub any step number present. It will be recreated on the other end, unless otherwise requested.
    $line =~ s/^\s*\d{3}:{0,1}//;

    if( $line =~ /^\s*([$label_spec]{2,}:{$NUM_TARGET_LABEL_COLONS})\s+(\S+.*)/ ) { # Line with target label
      $label = ${1} . (" " x ($label_field_length - length($1)));
      $opcode = $2;
      print "// DEBUG: reconstruct_steps: Type 1: '$line'\n" if $debug > 3;
    } elsif( $line =~ /^\s*(\S+.*)/ ) { # Line without target label
      $label = " " x $label_field_length;
      $opcode = $1;
      print "// DEBUG: reconstruct_steps: Type 2: '$line'\n" if $debug > 3;
    } else {
      if( $debug ) {
        print "ERROR: reconstruct_steps: Cannot parse the line at step $step! Line: '$line'\n";
        show_state(__LINE__);
        die;
      } else {
        die "ERROR: reconstruct_steps: Cannot parse the line at step $step! Line: '$line'\n";
      }
    }

    # See if the user has turned off step number generation from the command line.
    if( $prt_step_num ) {
      push @reconstructed_steps, sprintf "%03d /* %0s */ %0s", $step, $label, $opcode;
    } else {
      push @reconstructed_steps, sprintf "/* %0s */ %0s", $label, $opcode;
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
  print "// DEBUG: extract_substring: Attempting to extract substring from '$string'\n" if $debug;
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
        print "ERROR: extract_substring: Cannot parse the string string at step $fmt_step! String: '$org_string'\n";
        show_state(__LINE__);
        die;
      } else {
        die "ERROR: extract_substring: Cannot parse the string string at step $fmt_step! String: '$org_string'\n";
      }
    }
    print "// DEBUG: extract_substring: Found character '$actual_alpha' equated to '$alpha' in '$string'\n" if $debug;
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
    print "// DEBUG: extract_substring: Could only collect $num_chars -- putting some back. Current string is '$string'.\n" if $debug;
    $substring = shift @alphas;
    $num_chars = 1;
    while( @alphas ) {
      $string .= shift @alphas;
    }
    print "// DEBUG: extract_substring: Reconstituted string to '$string'.\n" if $debug;
  } else {
    $substring = join "", @alphas;
  }
  print "// DEBUG: extract_substring: Extracted $num_chars character(s): '$substring'. Left with string of '$string'.\n" if $debug;
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
  my @lines = @_;
  print "// DEBUG: insert_step: Inserting mnemonic '$line' at step ", format_step($location), ".\n" if $debug;
#  if( $debug ) {
#    print "// DEBUG: insert_step: Prior to inserting: '$line'\n";
#    panl(@lines);
#    print "\n...done\n";
#  }
  splice @lines, ($location-1), 0, $line;
#  if( $debug ) {
#    print "// DEBUG: insert_step: After inserting: '$line'\n";
#    panl(@lines);
#    print "\n...done\n";
#  }
  return @lines;
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
    print "// DEBUG: search_targets: Found target step '$target_step' with target label '$label' in \%targets.\n" if $debug > 3;
  } else {
    if( $debug ) {
      print "ERROR: search_targets: Cannot find label '$label' in \%targets.\n";
      show_state(__LINE__);
      die;
    } else {
      die "ERROR: search_targets: Cannot find label '$label' in \%targets.\n";
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
      print "ERROR: search_LBLs: Cannot find target step '$target_step' in \%LBLs.\n";
      show_state(__LINE__);
      die;
    } else {
      die "ERROR: search_LBLs: Cannot find target step '$target_step' in \%LBLs.\n";
    }
  } else {
    print "// DEBUG: search_LBLs: Found target step '$target_step' with LBL '$LBL' in \%LBLs.\n" if $debug > 3;
  }
  return $LBL;
} # search_LBLs


#######################################################################
#
# Format the step into a 3-digit number.
#
sub format_step {
  my $step = shift;
  return sprintf "%03d", $step;
} # format_step


#######################################################################
#
# Format the step into either a 3-digit number or a single upper case character.
#
sub format_LBL {
  my $LBL = shift;
  my $output = "";
  if( $LBL =~ /[A-Z]/ ) {
    $output = sprintf "%1s", $LBL;
  } else {
    $output = sprintf "%02d", $LBL;
  }
  return $output;
} # format_LBL


#######################################################################
#
# Find an unused label. When it is found, reserve it for that step.
#
sub get_next_label {
  my $step = shift;
  my $label = $last_label_used - 1;
  while( exists $LBLs{format_LBL($label)} ) {
    $label--;
    if( $label < 0 ) {
      warn "ERROR: get_next_label: Numeric label supply has been exhausted.\n";
      die  "                       Greater then ", $MAX_LABEL_NUM+1, " numeric labels used.\n";
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
  my $mnemonic = ($offset < 0) ? "BACK" : "SKIP";
  $offset = format_LBL(abs($offset));

  $line =~ s/^\s*\d{0,3}:{0,1}//;
  $line =~ s/^\s+//;
  $line =~ s/\s+$//;

  if( $line =~ /^([$label_spec]{2,}):{$NUM_TARGET_LABEL_COLONS}(\s+)(.+)/ ) {
    $label = $1;
    $spaces = $2;
    $opcode = $3;
    print "// DEBUG: replace_with_branch: Replacing: '$line' (type 1)\n" if $debug;
    $line =~ s/^[$label_spec]{2,}:{$NUM_TARGET_LABEL_COLONS}\s+(.+)/${label}::${spaces}${mnemonic} $offset \/\/ $1/;
    print "// DEBUG: replace_with_branch: With:      '$line' (type 1)\n" if $debug;
  } elsif( $line =~ /^(.+)/ ) {
    $opcode = $1;
    print "// DEBUG: replace_with_branch: Replacing: '$line' (type 2)\n" if $debug;
    $line =~ s/(.+)/${mnemonic} $offset \/\/ $1/;
    print "// DEBUG: replace_with_branch: With:      '$line' (type 2)\n" if $debug;
  } else {
    die "ERROR: replace_with_branch: Cannot parse the line: '$line' (offset $offset)\n";
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
    print "// DEBUG: replace_with_LBLd_target: Replacing: '$line' (type 1)\n" if $debug;
    $line = "${label}::${spaces}${mnemonic} $fmt_label_num // $opcode $target";
    print "// DEBUG: replace_with_LBLd_target: With:      '$line' (type 1)\n" if $debug;
  } elsif( $line =~ /^\s*(\S+)\s+(\S+)/ ) {
    $opcode = $1;
    $target = $2;
    print "// DEBUG: replace_with_LBLd_target: Replacing: '$line' (type 2)\n" if $debug;
    $line = "${mnemonic} $fmt_label_num // $opcode $target";
    print "// DEBUG: replace_with_LBLd_target: With:      '$line' (type 2)\n" if $debug;
  } else {
    die "ERROR: replace_with_LBLd_target: Cannot parse the line: '$line'\n";
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
  for my $label (sort keys %LBLs) {
    my $target = $LBLs{$label};
    if( $target > ($step + 1) ) {
      print "// DEBUG: adjust_labels_used: Incrementing target ('$target') for label '$label' because it is at or past step '$step'.\n" if $debug;
      $LBLs{$label} = format_step($target+1);
    }
  }
  return;
} # adjust_labels_used


#######################################################################
#
# Increment any step associated with an existing target if that step is past the
# point at which a step was inserted.
#
sub adjust_targets {
  my $step = shift;
  for my $label (sort keys %targets) {
    my $target = $targets{$label};
    if( eval($target+0) >= eval($step+0) ) {
      print "// DEBUG: adjust_targets: Incrementing step ('$target') for label '$label' because it is at or past step '$step'.\n" if $debug;
      $targets{$label}++;
    }
  }
  return;
} # adjust_targets


#######################################################################
#
# Increment any step associated with an existing label if that step is past the
# point at which a step was inserted. This requries new keys and to avoid colliding
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
      print "// DEBUG: adjust_branches: Incrementing branch step ('$branch_step') with label '$label' because it is at or past step '$step'.\n" if $debug;
    } else {
      if( exists $new_branches{$branch_step} ) {
        die "ERROR: adjust_branches: Existing branch already present for branch step ('$branch_step') with label '$label' prior to step '$step'.\n";
      }
      $new_branches{$branch_step} = $label;
      print "// DEBUG: adjust_branches: Leaving branch step ('$branch_step') with label '$label' untouched because it is prior to step '$step'.\n" if $debug > 1;
    }
  }
  %branches = %new_branches; # Replace the branches hash.
  return;
} # adjust_branches


#######################################################################
#
# Increment any step associated with an existing label if that step is past the
# point at which a step was inserted. This requries new keys and to avoid colliding
# with existing keys, just recreate the hash.
#
sub adjust_branch_array {
  my $step = shift;
  show_branches_remaining() if $debug;
  for( my $k = 0; $k < scalar(@branches); $k++ ) {
    if( $branches[$k] >= $step ) {
      print "// DEBUG: adjust_branch_array: Incrementing branch array step ('${branches[$k]}').\n" if $debug;
      $branches[$k] = format_step($branches[$k] + 1);
    }
  }
  show_branches_remaining() if $debug;
  return;
} # adjust_branch_array


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
  open SRC, $file or die "ERROR: Cannot open input file '$file' for reading: $!\n";
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
# on a line up to and including the next occurance of "*/", plus more than one
# occurance of "/* ... */ ... /* ... */" on a single line. There may be some
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
    push @redacted_lines, $_;
  }
  return @redacted_lines;
} # redact_comments


#######################################################################
#
#
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
#
#
sub populate_branch_array {
  print "// DEBUG: populate_branch_array: Extracting sorted branch steps...\n" if $debug;
  @branches = sort keys %branches;
  return;
} # populate_branch_array


#######################################################################
#
# Insert a new step into the branch array.
#
sub insert_into_branch_array {
  my $fmt_step = format_step(shift);
  print "// DEBUG: insert_into_branch_array: Inserting step '$fmt_step' into \@branches if required.\n" if $debug;
  my $found = 0;
  for my $existing_step (@branches) {
    if( $existing_step eq $fmt_step ) {
      $found = 1;
      print "// DEBUG: insert_into_branch_array: Step '$fmt_step' already exists in \@branches. Abandoning insert.\n" if $debug;
      last;
    }
  }
  if( not $found ) {
    push @branches, $fmt_step;
    @branches = sort {eval($a+0) <=> eval($b+0)} @branches;
    if( $debug ) {
      print "// DEBUG: insert_into_branch_array: After inserting step '$fmt_step' into \@branches:\n";
      show_branches_remaining();
    }
  }
  return;
} # insert_into_branch_array


#######################################################################
#
# Perl debugger helpers: Print array joined with \n.
#
sub panl { # Print array joined with \n.
  print "// ", join "\n", @_;
}

sub show_LBLs { # Dump the %LBLs hash
  dump_hash("LBLs", \%LBLs, " Label Step");
  return;
}

sub show_targets { # Dump the %ytargets hash
  dump_hash("targets", \%targets, " Label    Step");
  return;
}

sub show_branches { # Dump the %branches hash
  dump_hash("branches", \%branches, " Step   Target");
  return;
}

sub show_LBLd_ops { # Dump the %LBLd_opss hash
  dump_hash("LBLd_ops", \%LBLd_ops);
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
  print "// $second_line\n" if defined $second_line and $second_line;
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

#    elsif( ($arg eq "-internal") or ($arg eq "-i") ) {
#      push @files, $USE_INTERNAL_SRC;
#    }

    # Step the maximum BACK/SKIP offset limit.
    elsif( $arg eq "-m" ) {
      $MAX_JMP_OFFSET = shift(@ARGV);
      if( ($MAX_JMP_OFFSET > 99) or ($MAX_JMP_OFFSET < 4) ) {
        print "// WARNING: Maximum BACK/SKIP offset limit (-m) must be between 5 and 99. Resetting to 90.\n";
        $MAX_JMP_OFFSET = 90;
      }
    }

    elsif( $arg eq "-ns" ) {
      $prt_step_num = 0;
    }

    elsif( $arg eq "-cat" ) {
      $show_catalogue = 1;
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
    warn "ERROR: Must enter at least one file to process.\n";
    die  "       Enter '$script_name -h' for help.\n";
  }

  return;
} # get_options

