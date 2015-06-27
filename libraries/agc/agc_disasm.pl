#!/usr/local/bin/perl

#The index to this is the concatenation of the opcode field, the quartercode field, and the peripheral code field, in other words the high 6 bits
#The second half of the table is in effect when the extracode flag is set
@opcode=(
    "TC",   "TC",   "TC",   "TC",   "TC",   "TC",   "TC",   "TC",     #0000-0007  All of the below are non-extracodes
    "CCS",  "CCS",  "TCF",  "TCF",  "TCF",  "TCF",  "TCF",  "TCF",    #0010-0017
    "DAS",  "DAS",  "LXCH", "LXCH", "INCR", "INCR", "ADS",  "ADS",    #0020-0027
    "CA",   "CA",   "CA",   "CA",   "CA",   "CA",   "CA",   "CA",     #0030-0037
    "CS",   "CS",   "CS",   "CS",   "CS",   "CS",   "CS",   "CS",     #0040-0047
    "INDEX","INDEX","DXCH", "DXCH", "TS",   "TS",   "XCH",  "XCH",    #0050-0057
    "AD",   "AD",   "AD",   "AD",   "AD",   "AD",   "AD",   "AD",     #0060-0067
    "MASK", "MASK", "MASK", "MASK", "MASK", "MASK", "MASK", "MASK",   #0070-0077
    "READ", "WRITE","RAND", "WAND", "ROR",  "WOR",  "RXOR", "EDRUPT", #0100-0107  All of the below are Extracodes
    "DV",   "DV",   "BZF",  "BZF",  "BZF",  "BZF",  "BZF",  "BZF",    #0110-0117
    "MSU",  "MSU",  "QXCH", "QXCH", "AUG",  "AUG",  "DIM",  "DIM",    #0120-0127
    "DCA",  "DCA",  "DCA",  "DCA",  "DCA",  "DCA",  "DCA",  "DCA",    #0130-0137
    "DCS",  "DCS",  "DCS",  "DCS",  "DCS",  "DCS",  "DCS",  "DCS",    #0140-0147
    "INDEX","INDEX","INDEX","INDEX","INDEX","INDEX","INDEX","INDEX",  #0150-0157
    "SU",   "SU",   "BZMF", "BZMF", "BZMF", "BZMF", "BZMF", "BZMF",   #0160-0167
    "MP",   "MP",   "MP",   "MP",   "MP",   "MP",   "MP",   "MP"      #0170-0177
);

if($ARGV[1] =~ /\s*([0-9A-Fa-f]+)/) {
  $addr=hex($1);
} else {
  $addr=0;
}

open(INF,$ARGV[0]);
binmode(INF);
#open(INFSYM,"$ARGV[0].sym");
#%sym=();

#while($sl=<INFSYM>) {
#  if($sl=~/(\w*)\W*\w*\W*(\w*)/) {
#    %sym=(%sym,"0".$2=>$1);
#  }
#}  

#print "; Loaded symbols";

$imm=0;
$add=0;
$nolit=0;
#$trysym=undef;
$origin=0;
for(my $i_bank=0;$i_bank<044;$i_bank++) {
  read(INF,$bank,02000*2) or die sprintf("Couldn't read bank 0%2o",$i_bank);
  $extracode=0;
  for($i_word=0;$i_word<02000;$i_word++) {
    $word=vec($bank,$i_word,16);
#swap endian
    $word=(($word>>8) & 0xff) | (($word << 8) & 0xff00);
    
    $opc   =($word >> 12) & ((1<< 3)-1);
    $qc    =($word >> 10) & ((1<< 2)-1);
    $pc    =($word >>  9) & ((1<< 3)-1);
    $opindex=$extracode<<6 | ($word >> 9) & ((1<< 6)-1);
    $addr12=($word >>  0) & ((1<<12)-1);
    $addr10=($word >>  0) & ((1<<10)-1);
    $addr09=($word >>  0) & ((1<< 9)-1);
    $use_qc=(($extracode==0) && ($opc==1 || $opc==2 || $opc==5)) ||
            (($extracode==1) && ($opc==1 || $opc==2 || $opc==6));
    $mn=$opcode[$opindex];
    $use_pc=($extracode && ($opc==0));
    $old_extracode=$extracode;
    $extracode=0;
    if($old_extracode==0 && $opc==0) { #TC special cases
      if($addr12==3) {
        $mn="RELINT";
      } elsif($addr12==4) {
        $mn="INHINT";
      } elsif($addr12==6) {
        $mn="EXTEND";
        $extracode=1;
      }
    }
    if($addr>0) {
      print sprintf(" %05x",$addr);
    }
    if(($i_bank==2) || ($i_bank==3)) {
      print sprintf("    %04o",$i_word+04000+($i_bank-2)*02000);
    } else {
      print sprintf(" %02o,%04o",$i_bank,$i_word+02000);
    }
    if($use_pc) {
      print sprintf(" %05o %03b %03b %09b %-7s %04o",$word,$opc,$pc,$addr09,$mn,$addr09);
    } elsif($use_qc) {
      print sprintf(" %05o %03b %02b %010b %-7s %04o",$word,$opc,$qc,$addr10,$mn,$addr10);
    } else {
      print sprintf(" %05o %03b  %012b %-7s %04o",$word,$opc,$addr12,$mn,$addr12);
    } 
    print "\n";
    if($addr>0) {
      $addr+=2;
    }
  }
}


