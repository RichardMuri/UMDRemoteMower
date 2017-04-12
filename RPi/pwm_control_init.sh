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

echo "This is before the case statement\n"

SCRIPT=pwm_control
RUNAS=root

PIDFILE=/var/run/pwm_control.pid
LOGFILE=/var/log/pwm_control.log

start(){
	if [ -f "$PIDFILE" ] && kill -0 $(cat "$PIDFILE"); then
    echo 'Service already running' >&2
    return 1
  fi
  echo 'Starting service…' >&2
  local CMD="$SCRIPT &> \"$LOGFILE\" & echo \$!"
  su -c "$CMD" $RUNAS > "$PIDFILE"
  echo 'Service started' >&2
}

stop() {
  if [ ! -f "$PIDFILE" ] || ! kill -0 $(cat "$PIDFILE"); then
    echo 'Service not running' >&2
    return 1
  fi
  echo 'Stopping service…' >&2
  kill -15 $(cat "$PIDFILE") && rm -f "$PIDFILE"
  echo 'Service stopped' >&2
}

uninstall() {
  echo -n "Are you really sure you want to uninstall this service? That cannot be undone. [yes|No] "
  local SURE
  read SURE
  if [ "$SURE" = "yes" ]; then
    stop
    rm -f "$PIDFILE"
    echo "Notice: log file is not be removed: '$LOGFILE'" >&2
    update-rc.d -f <NAME> remove
    rm -fv "$0"
  fi
}

case "$1" in
  start)
    start
    ;;
  stop)
    stop
    ;;
  uninstall)
    uninstall
    ;;
  restart)
    stop
    start
    ;;
  *)
    echo "Usage: $0 {start|stop|restart|uninstall}"
esac

exit 0