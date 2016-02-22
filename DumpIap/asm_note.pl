#!/usr/bin/perl

#Assembly listing note copier. This program works on two files with addresses
#on the left end of each line. The first file is a (new) assembly listing, 
#which is to have notes attached to it. The second one is the file with the
#notes, delimited by a double hash ## . This was chosen because semicolon
#is used for automatically generated comments which do not need to be copied,
#and single hash is used to mark literal values in ARM assembly language.
#The two files are assumed to have roughly the same address coverage. For each
#line of the note file, there are several possible cases: 
# 1) No notes and no address: This line is ignored.
# 2) No notes, and an address. The assembly file can be printed up to and
#    including this line, from wherever we had previously left off
# 3) Notes, but no address. This line is remembered, and all such lines are
#    printed when the next line in the note file with an address is found.
# 4) Notes and address. The assembly file is printed up to this address, and the
#    line for this address has the note appended. If the length of the line
#    in the assembly is less than the start position of the note in the note 
#    file, then the line is padded (so as to try to get comments to line up).
#    If not, the not is appended directly, so as to not truncate the assembly
#    line.
# When all assembly lines are processed, we are done.
#
# Assembly file comes from stdin. Note file is named on command line. Output is 
# the annotated assembly, printed to stdout.

use strict;
use Text::Tabs;

my $debug=0;
my $noten=$ARGV[0];
open(NOTEF,"<$noten");
my ($noteAddress, $asmAddress, $note, $asm, $comment,$noteNoAddress);
my $done;
$noteNoAddress="";

sub printAsm {
    $done=0;
    while(!$done) {
      $asm=<STDIN>;
      chomp $asm;
      $asm=expand($asm);
      print "asm line: $asm\n" if $debug;
      if($asm=~/(^[ 0-9a-fA-F]{7}[0-9a-fA-F]):.*/) {  #Don't match lines with addresses and labels, just addresses
        $asmAddress=$1;
        if($asmAddress ge $noteAddress) {
          $done=1;
          print "This asm address matches, asmAddress $asmAddress eq noteAddress $noteAddress. Appending note to asm\n" if $debug;
          print "asm:     $asm\n" if $debug;
          print "comment: $comment\n" if $debug;
          print $noteNoAddress; #this is supposed to not have \n. If the noteNoAddress is blank, then we print nothing. If it's not blank, the \n was already added
          $noteNoAddress="";
          $asm=sprintf("%-80s %s",$asm,$comment);
        } else {
          print "This asm address doesn't match, asmAddress $asmAddress ne noteAddress $noteAddress\n" if $debug;
        }
      } else {
        print "This asm line doesn't have an address\n" if $debug;
      }
 #     $asm =~ s/\s+$//; #rtrim
      print $asm."\n";
    }
#Now read in the note file up to the asm address. If we are already there, great, this will skip the loop completely. If we miss any notes, too bad. The result is that the line only gets the first note
  print "Catching up note file to asm file, asmAddress=$asmAddress, noteAddress=$noteAddress\n" if $debug;
  while($asmAddress gt $noteAddress) {
    $note=<NOTEF>;
    chomp $note;
    $note = expand($note);
    print "catch up note line:    $note\n" if $debug;
    if($note=~/(^[ 0-9a-fA-F]{7}[0-9a-fA-F]):.*/) {
      $noteAddress=$1;
      $comment=$2;
    }
    print "Catching up note file to asm file, asmAddress=$asmAddress, noteAddress=$noteAddress\n" if $debug;
  }
}


while(<NOTEF>) {
  $note=$_;
  chomp $note;
  $note = expand($note);
  print "note line:    $note\n" if $debug;
  if($note=~/(^[ 0-9a-fA-F]{7}[0-9a-fA-F])[^:]*:.*(##.*)/) {
    $noteAddress=$1;
    $comment=$2;
    print "Found an address with a note\n" if $debug;
    print "note address: $noteAddress\n" if $debug;
    print "note comment: $comment\n" if $debug;
    printAsm;
  } elsif($note=~/^.*(##.*)/) {
    print "Found a note with no address\n" if $debug;
     $noteNoAddress=$noteNoAddress.$1."\n";
  } elsif($note=~/^([ 0-9a-fA-F]{7}[0-9a-fA-F]):.*/) {
    print "Found an address with no note\n" if $debug;
    $noteAddress=$1;
    $comment="";
    printAsm;
  } else {
    print "Found no address and no note\n" if $debug;
  }
}

while(<STDIN>) {
  print $_;
}

      
