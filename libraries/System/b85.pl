#!/usr/bin/perl

sub decode {
	my ($in) = @_;
	for ($in) {
		tr[ \t\r\n\f][]d;
		s/z/!!!!!/g;
		s/y/+<VdL/g;
	}

	my $padding = -length($in) % 5;
	$in .= 'u' x $padding;
	my $out = '';

	for my $n (unpack '(a5)*', $in) {
		my $tmp = 0;
		for my $i (unpack 'C*', $n) {
			$tmp *= 85;
			$tmp += $i - 33;
		}
		$out .= pack 'N', $tmp;
	}

	substr $out, 0, length($out) - $padding
}

my $line;

{
    local( $/ ) ;
    $line = <>;
}

print decode($line);

