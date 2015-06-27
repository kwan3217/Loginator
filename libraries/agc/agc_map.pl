#!/usr/local/bin/perl
use File::Basename;

#Get a list of all the *.agc.html files and parse each one in turn

chdir $ARGV[0];

@files=glob("*.agc.html");

%lines=();
%origtxts=();
$longfn=0;
foreach $infn (@files) {
  $origtxtfn=basename($infn,'.agc.html').".agc";
  @this_origtxts=`cat $origtxtfn`;
  if(length($origtxtfn)>$longfn) {
    $longfn=length($origtxtfn);
  }
#  print $infn."\n";
  open($INF, '<', $infn) or die "Can't open $infn";
  while($line=<$INF>) {
    if($line=~/([0-9]{6}),([0-9]{6}):\s+(([0-7]{2}),)?([0-7]{4})\s+([0-7]{5})/) {
      $all_line=$1+0;
      $this_line=$2+0;
      $addr=oct($5);
      if($addr>=06000) {
        $bank=3;
      } elsif ($addr>=04000) {
        $bank=2;
      } else {
        $bank=oct($4);
      }
      $data=oct($6);
      $bankaddr=sprintf("%02o,%04o",$bank,$addr);
      $filelinenum=basename($infn,'.agc.html').":".$this_line;
      $origtxt=$this_origtxts[$this_line-1];
#      print $origtxt;
#      print $bankaddr.":   ".$fileline."  ".scalar(keys %lines)."\n";
      $lines{$bankaddr}=$filelinenum;
      $origtxts{$bankaddr}=$origtxt;
#      print "All line: $all_line, This line: $this_line, Bank: $bank, Address: ".sprintf("%04o",$addr).", Data: $data\n";
    }
  }
}

@key=sort keys %lines;
$lastfile='';
$firstln=0;
$firstad='';
$lastln=0;
$lastad='';
$firstbank=0;
$lastbank=0;
foreach $thisad (@key) {
  $thisline=$lines{$thisad};
  if($thisline=~/([^:]*):(.*)/) {
    $thisfile=$1;
    $thisln=$2;
  }
  $thisad=~/([0-7]{2}),([0-7]{4})/;
  $thisbank=oct($1);
  if($thisfile ne $lastfile || $thisbank ne $firstbank) {
    if($lastfile ne '') {
      if($firstln eq $lastln) {
        print sprintf("%7s         (    1) - %s:%d\n",$firstad,$lastfile,$firstln);
      } else {
        $firstad=~/([0-7]{2}),([0-7]{4})/;
        $firstadd=oct($2);
        $lastad=~/([0-7]{2}),([0-7]{4})/;
        $lastadd=oct($2);
        $size=$lastadd-$firstadd+1;
        print sprintf("%7s-%7s (%5d) - %s:%d-%d\n",$firstad,$lastad,$size,$lastfile,$firstln,$lastln);
      }
    }
    $firstad=$thisad;
    $lastad=$thisad;
    $firstln=$thisln;
    $lastln=$thisln;
    $lastfile=$thisfile;
    $firstbank=$thisbank;
    $lastbank=$thisbank;
  } else {
    $lastad=$thisad;
    $lastln=$thisln;
  }
}

if($lastfile ne '') {
  if($firstln eq $lastln) {
    print sprintf("%7s         (    1) - %s:%d\n",$firstad,$lastfile,$firstln);
  } else {
    $firstad=~/([0-7]{2}),([0-7]{4})/;
    $firstadd=oct($2);
    $lastad=~/([0-7]{2}),([0-7]{4})/;
    $lastadd=oct($2);
    $size=$lastadd-$firstadd+1;
    print sprintf("%7s-%7s (%5d) - %s:%d-%d\n",$firstad,$lastad,$size,$lastfile,$firstln,$lastln);
  }
}

$pat=sprintf("%%7s %%-%ds %%s",$longfn+4);
print $pat."\n";

for($bank=0;$bank<044;$bank++) {
  for($addr=0;$addr<02000;$addr++) {
    if($bank==2) {
      $thisad=sprintf("%02o,%04o",$bank,$addr+04000);
    } elsif ($bank==3) {
      $thisad=sprintf("%02o,%04o",$bank,$addr+06000);
    } else {
      $thisad=sprintf("%02o,%04o",$bank,$addr+02000);
    }
    if($lines{$thisad}) {
      $thisline=$lines{$thisad};
      $thisorig=$origtxts{$thisad};
    } else {
      $thisline="";
      $thisorig="\n";
    }
    print STDERR sprintf($pat,$thisad,$thisline,$thisorig);
  }
}

