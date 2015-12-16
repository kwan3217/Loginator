#!/usr/bin/perl
use warnings;
use strict;

use XML::Simple;

my $file = $ARGV[0];
print "#$ARGV[0] $ARGV[1]\n";
my $kml=XMLin($file);

my $placemarks=$kml->{Document}->{Folder}->{Placemark};
my %placemarks=%$placemarks;

my $placemark=$placemarks{$ARGV[1]};
my %placemark=%$placemark;

my $coordsS=$placemark{"LineString"}->{coordinates};
#$coords=$$coords;
$coordsS=~s/^\s+|\s+$//g;
my $PI=3.14159265358972;
my @coords=split(/\s+/,$coordsS);

my $initcoords=$coords[0];
my ($lon0,$lat0,$alt0)=split(/\s*,\s*/,$initcoords);
my @wp=(0,0);
for(my $i=1;$i<scalar @coords;$i++) {
  my ($lon,$lat,$alt)=split(/\s*,\s*/,$coords[$i]);
  my $x=($lon-$lon0)*1e7*cos(40*$PI/180);
  my $y=($lat-$lat0)*1e7;
  push(@wp,sprintf("%.0f",$x));
  push(@wp,sprintf("%.0f",$y));
}
print "wp ".join(' ',@wp)."\n"; 
