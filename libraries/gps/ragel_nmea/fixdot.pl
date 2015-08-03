sub fixchr {
  $n=shift(@_);
  if($n lt 32) {
    return sprintf("0x%02x",$n);
  } elsif ($n eq 32) {
    return "Space";
  } else {
    return chr($n);
  }
}

while(<>) {
  if($_=~/(.*label = ")([0-9]+)(..)([0-9]+)(.*)/) {
    $firstChar=fixchr($2);
    $secondChar=fixchr($4);
    print $1.$firstChar.$3.$secondChar.$5."\n";
  } elsif($_=~/(.*label = ")([0-9]+)(.*)/) {
    $firstChar=fixchr($2);
    $secondChar=fixchr($4);
    print $1.$firstChar.$3."\n";
  } else {
    print $_;
  }
}

