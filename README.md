# hue_monitor
Monitors status of Panasonic HUE lights

simply `make` to build the executable.
requires C++ Poco library (libPocoFoundation, libPocoJSON, and libPocoNet)
if Poco library is not installed in standard include and lib dirs, edit Makefile (see Makefile.nonstandard)

run hue_status executable with no arguments

config.json must contain ipaddr, port, and user, and can optionally contain sleeptime
