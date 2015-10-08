#!/usr/bin/perl
use strict;
use File::Basename;

#Make a treemap from a GCC linker map output file. This is specialized to how
#I make Loginator binaries, but may be usable or extensable to other projects.
#It shows the sizes of all the code and read-only data in the binary, and the
#RAM used when the binary is installed and running.
#The treemap has the following levels:
# * Section in the output ELF
#     This is things like .text, .rodata, etc. I have more sections in my 
#     binaries than are strictly needed, to see how big the libraries are for 
#     instance
#   * Library 
#       This is the library folder for my code, or the static archive for 
#       code provided by GCC, not my own code. 
#    * Object file
#      * Function/variable
#        This is the only part which has a size. For my code, each routine and
#        variable is in its own section in the object file, so it is easy to see
#        how big each one is. In the library code, several functions/variables
#        might be in the same object section, so you have to use the offsets to
#        figure the size.


#Open the file
open(my $inf, "<", $ARGV[0]);

#Skip the header stuff and get to the meat. As it happens in my map files, the
#meat is after lines that start with any of LOAD/START GROUP/END GROUP . 
while (my $line = <$inf>) { #Skip the lines which DON'T start with these words
  last if $line =~ /(LOAD|START GROUP|END GROUP).*/;
}
while (my $line = <$inf>) { #Skip the lines which DO start with them
  last if $line !~ /(LOAD|START GROUP|END GROUP).*/;
}

my $outSection;
my $address;
my $outSecAddress;
my $outSecLength;
my $inSection;
my $inSecAddress;
my $inSecLength;
my $lastSymbol;
my $obj;
my $symbol;
my $inOutputSection=0;
my $inInputSection=0;
my $hasLastSymbol=0;
my %symbolLength;
my %symbolInSec;
my %symbolOutSec;
my %symbolObj;
my %symbolAddr;
my %symbolType;

print '<html>'."\n";
print '  <head>'."\n";
print '    <script type="text/javascript" src="https://www.google.com/jsapi"></script>'."\n";
print '    <script type="text/javascript">'."\n";
print '      google.load("visualization", "1", {packages:["treemap"]});'."\n";
print '      google.setOnLoadCallback(drawChart);'."\n";
print 'function drawChart() {'."\n";                                      
print '        var data = google.visualization.arrayToDataTable(['."\n";
print "          ['Id', 'Parent', 'Size (bytes)','Type'],\n";
print "['Yukari3',null,0,null],\n";
#Parse each line of the meat, which ends with the first line that starts .stab  
while (my $line = <$inf>) {
  last if $line =~ /.stab/;
#  print $line;
#Each section in the output file is designated by a name which starts with a 
#dot in the first character of the line. For instance, we have .text as the
#first section of the file. Keep track of this as we go along. An output section
#is described by a name which starts with a period in the first column, then
#an address in the output image, followed by a length.
  if(substr($line,0,1) eq '.') {
#    print("if1\n");
    if($hasLastSymbol) {
      $symbolLength{$lastSymbol}=$inSecLength-($symbolAddr{$lastSymbol}-$inSecAddress);
#      printf("Symbol2 %s: length %d (0x%04x)\n",$lastSymbol,$symbolLength{$lastSymbol},$symbolLength{$lastSymbol});
    } else {
      printf("<!-- No symbols in object %s, section %s, so making one called %s with length %d -->\n",$obj,$inSection,$obj."/".$inSection,$inSecLength);
      $symbol=$inSection,$obj."/".$inSection;
      $symbolAddr{$symbol}=$inSecAddress;
      $symbolInSec{$symbol}=$inSection;
      $symbolOutSec{$symbol}=$outSection;
      $symbolObj{$symbol}=$obj;
      $symbolLength{$lastSymbol}=$inSecLength;
    }
    if($line=~/(\S+)\s+(0x[0-9a-fA-F]{16})\s+(0x[0-9a-fA-F]+)/) {
      $outSection=$1;
      $outSecAddress=hex($2);
      $outSecLength=hex($3);
      $inOutputSection=0;
    } else {
      $outSection=$line;
      $outSection =~ s/^\s+|\s+$//g;
      $inOutputSection=1;
    } 
      
    $hasLastSymbol=0;
  } elsif($inOutputSection) {
 #   print("if2\n");
    if($line=~/\s+(0x[0-9a-fA-F]{16})\s+(0x[0-9a-fA-F]+)\s+/) {
      $outSecAddress=hex($1);
      $outSecLength=hex($2);
      $inOutputSection=0;
    } 
    $hasLastSymbol=0;
#Each output section is built up of one or more input sections. Each input 
#section is described by a name that starts with a period in the second column,
#then an address in the output image, a length, and an input object file name
  } elsif(substr($line,0,2) eq ' .') {
    if($hasLastSymbol) {
      $symbolLength{$lastSymbol}=$inSecLength-($symbolAddr{$lastSymbol}-$inSecAddress);
#      printf("Symbol2 %s: length %d (0x%04x)\n",$lastSymbol,$symbolLength{$lastSymbol},$symbolLength{$lastSymbol});
    } else {
      printf("<!-- No symbols in object %s, so making one called %s with length %d -->\n",$obj,$inSection,$inSecLength);
      $symbol=$inSection;
      $symbolAddr{$symbol}=$inSecAddress;
      $symbolInSec{$symbol}=$inSection;
      $symbolOutSec{$symbol}=$outSection;
      $symbolObj{$symbol}=$obj;
      $symbolLength{$lastSymbol}=$inSecLength;
    }
  #  print("if3\n");
    if($line=~/ (\S+)\s+(0x[0-9a-fA-F]{16})\s+(0x[0-9a-fA-F]+)\s+(\S+)/) {
      $inSection=$1;
      $inSecAddress=hex($2);
      $inSecLength=hex($3);
      $obj=basename($4);
      my $path=dirname($4);
      if(substr($path,0,length('../libraries/')) eq '../libraries/') {
        $path=substr($path,length('../libraries/')).'/';
      } else {
        $path=basename($path);
      }
      $obj=$path.$obj;
      $inInputSection=0;
    } else {
      $inSection=substr($line,1);
      $inSection =~ s/^\s+|\s+$//g;
      $inInputSection=1;
#      printf("here4a %d\n",$inInputSection);
    } 
    $hasLastSymbol=0;
  } elsif($inInputSection) {
   # printf("if4\n");
    if($line=~/\s+(0x[0-9a-fA-F]{16})\s+(0x[0-9a-fA-F]+)\s+(\S+)/) {
      $inSecAddress=hex($1);
      $inSecLength=hex($2);
      $obj=basename($3);
      my $path=dirname($3);
      if(substr($path,0,length('../libraries/')) eq '../libraries/') {
        $path=substr($path,length('../libraries/')).'/';
      } else {
        $path=basename($path);
      }
      $obj=$path.$obj;
      $inInputSection=0;
    } 
    $hasLastSymbol=0;
  } elsif($line=~/\s+(0x[0-9a-fA-F]{16})\s+(.+)$/) {
    $symbol=$2;
    $symbolAddr{$symbol}=hex($1);
    $symbolInSec{$symbol}=$inSection;
    $symbolOutSec{$symbol}=$outSection;
    $symbolObj{$symbol}=$obj;
    if(substr($inSection,0,4) eq '.tex') {
      $symbolType{$symbol}=0;
    } elsif(substr($inSection,0,4) eq '.bss') {
      $symbolType{$symbol}=1;
    } elsif(substr($inSection,0,3) eq '.ro') {
      $symbolType{$symbol}=2;
    } elsif(substr($inSection,0,3) eq '.so') {
      $symbolType{$symbol}=3;
    } else {
      $symbolType{$symbol}=4;
    } 
    if($hasLastSymbol) {
      $symbolLength{$lastSymbol}=$symbolAddr{$symbol}-$symbolAddr{$lastSymbol};
#      printf("Symbol2 %s: length %d (0x%04x)\n",$lastSymbol,$symbolLength{$lastSymbol},$symbolLength{$lastSymbol});
    } else {
    }
    $hasLastSymbol=1;
    $lastSymbol=$symbol;    
#    printf("Found symbol: %s\n",$symbol);
  }
}

my %objUsed;

#Figure out the tree from the data we have in the symbols
foreach $symbol (sort(keys %symbolAddr)) {
  if($symbolLength{$symbol}>0) {
    $objUsed{$symbolObj{$symbol}}=1;
  }
}

#Print out the top-level things (output sections)
foreach my $obj (sort(keys %objUsed)) {
  printf("  ['%s','Yukari3',null,null],\n",$obj);
}

foreach $symbol (sort(keys %symbolAddr)) {
  if($symbolLength{$symbol}>0) {
    printf("  ['%s','%s',%d,%d],\n", $symbol,$symbolObj{$symbol},$symbolLength{$symbol},$symbolType{$symbol});
  }
}
#Close the file 
print "        ]);\n";
print "\n";
print "        tree = new google.visualization.TreeMap(document.getElementById('chart_div'));\n";
print "\n";
print "        tree.draw(data, {\n";
print "        maxDepth: 2,\n";
print "        maxPostDepth: 2,\n";
print "       minHighlightColor: '#8c6bb1',\n";
print "        midHighlightColor: '#9ebcda',\n";
print "        maxHighlightColor: '#edf8fb',\n";
print "        minColor: '#009688',\n";
print "        midColor: '#f7f7f7',\n";
print "        maxColor: '#ee8100',\n";
print "          headerHeight: 10,\n";
print "          fontColor: 'black',\n";
print "          showScale: true,\n";
print "          generateTooltip: showFullTooltip\n";
print "        });\n";
print "  function showFullTooltip(row, size, value) {\n";
print "    return '<div style=\"background:#fd9; padding:10px; border-style:solid\">' +\n";
print "           '<span style=\"font-family:Courier\"><b>' + data.getValue(row, 0) +\n";
print "           '</b>, ' + data.getValue(row, 1) + ', ' + data.getValue(row, 2) +\n";
print "           ', ' + data.getValue(row, 3) + '</span><br>' +\n";
print "           'Datatable row: ' + row + '<br>' +\n";
print "	   data.getColumnLabel(2) +\n";
print "           ' (total value of this cell and its children): ' + size + '<br>' +\n";
print "	   data.getColumnLabel(3) + ': ' + value + ' </div>';\n";
print "}\n";
print "\n";
print "      }\n";
print "    </script>\n";
print "  </head>\n";
print "  <body>\n";
print '    <div id="chart_div" style="width: 100%; height: 100%;"></div>'."\n";
print "  </body>\n";
print "</html>\n";