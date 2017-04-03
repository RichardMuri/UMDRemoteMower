#!/bin/sh
# /etc/init.d/pwm_control
#
### BEGIN INIT INFO
# Provides:          pwm_control
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Initscript for pwm_control
# Description:       Initscript for pwm_control;
# 					 Reads serial pins using I2C and adjusts
#					 PWM on pins 12 and 18 to control motors
### END INIT INFO

# Author: Richard Muri <muri.engineering@gmail.com>
# This file needs to be placed in /etc/init.d/ to 
# run on start up for the Raspberry Pi

case "$1" in 
    start)
        echo "Starting pwm_control\n"
		/home/pwm_control
        ;;
    stop)
        echo "Stopping pwm_control"
        killall pwm_control
        ;;
    *)
        echo "Usage: /etc/init.d/lawnmower_control start|stop"
        exit 1
        ;;
esac

exit 0