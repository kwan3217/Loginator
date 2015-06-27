#!/usr/local/bin/perl

open(INF,$ARGV[0]);
binmode(INF);
binmode(STDOUT);
@banks=();

for(my $i_bank=0;$i_bank<044;$i_bank++) {
  read(INF,$bank,02000*2) or die sprintf("Couldn't read bank 0%2o",$i_bank);
  for($i_word=0;$i_word<02000;$i_word++) {
    $word=vec($bank,$i_word,16)/2;
#    print sprintf("%04x ",$word);
    $word=(($word>>8) & 0xff) | (($word << 8) & 0xff00);
#    print sprintf("%04x\n",$word);
    vec($bank,$i_word,16)=$word;
  }
  if($i_bank==0 || $i_bank==1) {
    $print_bank=$i_bank+2;
  } elsif ($i_bank==2 || $i_bank==3) {
    $print_bank=$i_bank-2;
  } else {
    $print_bank=$i_bank;
  }
  $banks[$print_bank]=$bank; 
}

for(my $i_bank=0;$i_bank<044;$i_bank++) {
  print $banks[$i_bank];
}


