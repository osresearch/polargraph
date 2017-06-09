/*
 * Pulley for the polargraph motors.
 * You will need two.
 */

circumference = 120;
height = 10;
shank_height = 8;
radius = circumference / (2 * PI);

render() difference()
{

union()
{
	// the two pieces of the pulley
	{
		translate([0,0,height*2/3])
		cylinder(r1=radius, r2=radius+height*0.5, h=height/3, $fn=90);

		translate([0,0,height/3])
		cylinder(r=radius, h=height/3, $fn=90);

		cylinder(r2=radius, r1=radius+height*0.5, h=height/3, $fn=90);
	}

	// the shank
	rotate([0,0,-10]) translate([0,0,height]) cylinder(r=8, h=shank_height, $fn=7);
}

// shaft for the 5mm nema23 stepper motor
cylinder(r=5.0/2 + 0.25, h=30, $fn=30);
cylinder(r1=5.0/2 + 1, r2=5.0/2 + 0.25, h=2, $fn=30);

translate([0,-5.0/2+1.2,height+shank_height/2])
rotate([90,0,0]) {
	// m3 mounting screw
	cylinder(r=3.0/2 + 0.2, h=30, $fn=30);
	// m3 nut to capture it
	rotate([0,0,30]) cylinder(r=6.0/2+0.5, h=2, $fn=6);
	translate([0,3.5,0.5]) cube([5.5,10,2],center=true);
}

// string hole
translate([radius,0,height*2/3]) cylinder(r=1, h=height+2, $fn=12);
}

