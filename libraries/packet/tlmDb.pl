#!/usr/bin/perl
use strict;
use warnings;
use Switch;
 
use Text::CSV;
 
my $oldApid=0; 
my $wrapRobot=0;
my $writeExtract=0;
my $hasTC=0;
my %fill=('fp'=>'fp',
          'int8_t'=>'',
          'int16_t'=>'16',
          'int32_t'=>'32',
          'uint8_t'=>'',
          'uint16_t'=>'16',
          'uint32_t'=>'32',
          'char[]'=>'');
my %ntoh=('fp'=>'ntohf',
          'int8_t'=>'',
          'int16_t'=>'ntohs',
          'int32_t'=>'ntohl',
          'uint8_t'=>'',
          'uint16_t'=>'ntohs',
          'uint32_t'=>'ntohl',
          'char[]'=>'');
my %format=('fp'=>'%f',
          'int8_t'=>'%d',
          'int16_t'=>'%d',
          'int32_t'=>'%d',
          'uint8_t'=>'%u',
          'uint16_t'=>'%u',
          'uint32_t'=>'%u',
          'char[]'=>'%s');
my @shortNames;
my @fields;
my @types;
my $csv = Text::CSV->new ( { binary => 1 } )  # should set binary attribute.
                or die "Cannot use CSV: ".Text::CSV->error_diag ();
 
open my $fh, "<:encoding(utf8)", $ARGV[0] or die "$ARGV[0]: $!";
open my $oufExtractStr, ">","extract_str.inc"; 
open my $oufExtractVars, ">","extract_vars.inc"; 
open my $oufExtractBody, ">","extract_body.inc"; 
my $oufRobot;
my $oufReverse;
my $shortName;
my $succFile=0;
my $extractor;
my $pretype;
my $array;
my $field;
my $type;
sub finishApid() {
    if($succFile>0) {
      #Finish up old apid before we read anything new about the new apid
      if($wrapRobot) {
        print $oufRobot "ccsds.finish($oldApid);\n"
      }
      if($writeExtract) {
        if($succFile>0) {
          print $oufExtractStr "};\n";
          if($extractor eq "dump") {
            print $oufExtractBody "  }\n";
            print $oufExtractBody "  fwrite($shortName->".$fields[@fields-1].",1,ccsds->length+7-($shortName->".$fields[@fields-1]."-buf),f$shortName);\n";
            if($hasTC ne '') {
              print $oufExtractBody "  #include \"reverse_packet_$shortName.inc\"\n";
              print $oufExtractBody "  static unsigned int lastTC;\n";
              print $oufExtractBody "  static unsigned int min;\n";
              print $oufExtractBody "  if($shortName->TC<lastTC) min++;\n";
              print $oufExtractBody "  lastTC=$shortName->TC;\n"; 
              print $oufReverse "$shortName->TC=ntohl($shortName->TC);\n";
            }
          } elsif($extractor eq "csv") {
            print $oufExtractBody "    fprintf(f$shortName,\"";
            if($hasTC) {
              print $oufExtractBody "TC,t,";
            }
            for(my $i=0;$i<@fields;$i++) {
              $field=$fields[$i];
              if($i!=0) {
                print $oufExtractBody ",";
              }
              print $oufExtractBody $field;
            }
            print $oufExtractBody "\\n\");\n";
            print $oufExtractBody "  }\n";
            if($hasTC ne '') {
              print $oufExtractBody "  #include \"reverse_packet_$shortName.inc\"\n";
              print $oufExtractBody "  static unsigned int lastTC;\n";
              print $oufExtractBody "  static unsigned int min;\n";
              print $oufExtractBody "  if($shortName->TC<lastTC) min++;\n";
              print $oufExtractBody "  lastTC=$shortName->TC;\n"; 
              print $oufReverse "$shortName->TC=ntohl($shortName->TC);\n";
            }
            for(my $i=0;$i<@fields;$i++) {
              $field=$fields[$i];
              $type=$types[$i];
              my $this_ntoh=$ntoh{$type};
              if($this_ntoh ne '') {
                print $oufReverse "  $shortName->$field=$ntoh{$type}($shortName->$field);\n";
              }
            } 
            if($types[@types-1] eq 'char[]') {
              #Special case - if the last field is a string, add a null-terminator
              print $oufExtractBody "  $shortName->".$fields[@types-1]."[ccsds->length-6]=0;\n";
            }
            print $oufExtractBody "  fprintf(f$shortName,\"";
            if($hasTC) {
              print $oufExtractBody "%10u,%f,";
            }
            for(my $i=0;$i<@fields;$i++) {
              $field=$fields[$i];
              $type=$types[$i];
              if($i!=0) {
                print $oufExtractBody ",";
              }
              print $oufExtractBody $format{$type};
            }
            print $oufExtractBody "\\n\",\n";
            if($hasTC) {
              print $oufExtractBody "    $shortName->TC,(double)(min*60)+(double)($shortName->TC)/60/1000000,\n";
            }
            for(my $i=0;$i<@fields;$i++) {
              $field=$fields[$i];
              $type=$types[$i];
              print $oufExtractBody "    $shortName->$field";
              if($i!=@fields-1) {
                print $oufExtractBody ",\n";
              } else {
                print $oufExtractBody "\n";
              }
            }
            print $oufExtractBody "  );\n";
            close $oufRobot;
            close $oufReverse;
          }    
          print $oufExtractBody "};\n";
        }
      }
    }
}

while ( my $row = $csv->getline( $fh ) ) {
  my $apid=$row->[0];
  if($apid eq '') {
    $apid=$oldApid;
  }
  $apid =~ m/0x[0-9A-Fa-f][0-9A-Fa-f]/ or next; # Apid field has to be in the form of hex
  if($apid ne $oldApid) {
    finishApid;
    $shortName=$row->[1];
    $wrapRobot=($row->[2] eq 'y');
    $writeExtract=$row->[3];
    $hasTC=$row->[4];
    push @shortNames,$shortName;
    open $oufRobot, ">",sprintf("write_packet_%s.inc",$shortName);
    open $oufReverse, ">",sprintf("reverse_packet_%s.inc",$shortName);
    if($wrapRobot) {
      print $oufRobot "ccsds.start(sdStore,$apid";
      if($hasTC ne '') {
        print $oufRobot ",$hasTC";
      }
      print $oufRobot ");\n";
    }
    @fields=();
    @types=();
    $extractor=$row->[5];
    if($writeExtract ne '') {
      print $oufExtractStr "struct $shortName {\n"; 
      print $oufExtractStr "  struct ccsdsHeader head;\n"; 
      if($hasTC ne '') {
        print $oufExtractStr "  uint32_t TC __attribute__((packed));\n"; 
      }
      print $oufExtractBody "if(ccsds->apid==$apid) {\n";
      print $oufExtractBody "  static FILE* f$shortName=NULL;\n";
      print $oufExtractBody "  if(f$shortName==NULL) {\n";
      if($shortName eq $writeExtract) {
        print $oufExtractBody "    sprintf(oufn,\"%s.$shortName\",base);\n";
      } else {
        print $oufExtractBody "    sprintf(oufn,\"%s.$shortName.$writeExtract\",base);\n";
      }
      print $oufExtractBody "    f$shortName=fopen(oufn,\"wb\");\n";
    }
    $oldApid=$apid;
    $succFile=1;
  }
  my $source=$row->[6];
  $field=$row->[7];
  push @fields,$field;
  $type=$row->[8];
  push @types,$type;
  if($type =~ /([^[]+)([[^\]]*])/) {
    $pretype=$1;
    $array=$2;
  } else {
    $pretype=$type;
    $array='';
  }
  my $unit=$row->[9];
  my $desc=$row->[10];
  print $oufRobot "ccsds.fill".$fill{$type}."(".$source."); //".$desc."\n";
  if($writeExtract) {
    print $oufExtractStr "  $pretype $field$array __attribute__((packed));\n";
  }
}
finishApid;
foreach $shortName (@shortNames) {
  print $oufExtractVars "struct $shortName * $shortName=(struct $shortName *)buf;\n";
}
close $oufExtractStr;
close $oufExtractVars;
close $oufExtractBody;


