# Polargraph plotting program
![Gosper curve](https://farm2.static.flickr.com/1531/25435298340_7cefb92d62.jpg)

This is a total hack that was built and written in just a few hours.
It is not robust, generic, usable, etc.  But perhaps it could be
improved!

More info: https://trmm.net/Polargraph


TSV-6
118.5" diagonal to zero == 3010 mm
165" side to side == 4191 mm

./polargraph --scale 1.25 -F 6000 -w 4191 -l 3010 --offset-x 500 --offset-y 400 < paris-crop.pjl > paris-crop.gcode 2>/tmp/p

TSV lobby
3m high
7m wide
diagonal 4609.7722
./polargraph -F 6000 -f 4000 -w 7000 -l 4609 --offset-x 0 --offset-y 0
