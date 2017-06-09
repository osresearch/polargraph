/*
 * Pulley for the polargraph motors.
 * You will need two.
 */

circumference = 100;
height = 6;
radius = circumference / (2 * PI);

render() difference()
{

union()
{
	// the two pieces of the pulley
	translate([0,0,height/2])
	{
		cylinder(r1=radius, r2=radius+height*0.6, h=height/2);

		rotate([180,0,0])
		cylinder(r1=radius, r2=radius+height*0.6, h=height/2);
	}

	// the shank
	translate([0,0,height]) cylinder(r=8, h=height);
}

// shaft for the 5mm nema23 stepper motor
cylinder(r=5.0/2 + 0.1, h=30, $fn=30);

translate([0,-5.0/2+1.25,height+height/2])
rotate([90,0,0]) {
	// m3 mounting screw
	cylinder(r=3.0/2 + 0.2, h=30, $fn=30);
	// m3 nut to capture it
	rotate([0,0,30]) cylinder(r=6.0/2, h=1, $fn=6);
	translate([0,5,0.5]) cube([5,10,1],center=true);
}

// string hole
translate([radius,0,height/2]) cylinder(r=0.5, h=height+2, $fn=12);
}

