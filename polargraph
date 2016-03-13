#!/usr/bin/perl
# Translate the vectors from a PJL file into gcode for the polargraph.
# There is probably a better way to do this.
use warnings;
use strict;

# origin is at l1=800 l2=800 
# xy = 600,500 or so
# separation between the two is 1120

my $w = 1120;
my $default_feed = 4000;
my $step_size = 40;

my $ox = 300;
my $oy = 200;
my $scale = 0.06;

while(<>)
{
	chomp;
	my @points = split /,/;

	for(my $i = 0 ; $i < @points ; $i+=2)
	{
		my ($x,$y) = @points[$i,$i+1];
		my $feed = $default_feed;
		if ($x =~ s/^PU//)
		{
			$feed = 10000;
		}

		$x =~ s/^PD//;

		goxy($x,$y, $feed);
	}
}


sub goxy_step
{
	my ($x0,$y0,$x1,$y1) = @_;

	# iterate from x0,y0 to x1,y1
	my $dx = $x1 - $x0;
	my $dy = $y1 - $y0;
	my $dist = sqrt($dx*$dx+$dy*$dy);

	my $steps = int($dist/$step_size);

	for(my $i = 0 ; $i < $steps ; $i++)
	{
		$x0 += $dx / $steps;
		$y0 += $dy / $steps;
		goxy($x0,$y0);
		#sleep(1);
	}

	goxy($x1,$y1);
}


sub goxy
{
	# convert from pts to something
	my $x = shift;
	my $y = shift;
	my $feed = shift || 1000;

	$x = $x * $scale + $ox;
	$y = $y * $scale + $oy;
	
	my $l1 = sqrt(($w-$x)*($w-$x) + $y*$y);
	my $l2 = sqrt($x*$x + $y*$y);

	printf STDERR "%.3f %.3f -> %.3f %.3f\n", $x, $y, $l1, $l2;
	printf "G1 X%.3f Y%.3f F%d\n", $l1, $l2, $feed;
}
