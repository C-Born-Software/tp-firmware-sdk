#!/bin/sh
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019-2022 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   set_weston_config.sh
#
# Brief:    setup weston configuration
#
# Author:   Ralf Gliese: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#


eeprom_device="/sys/bus/i2c/devices/1-0054/eeprom"
hdmi=0
weston_ini_file=/etc/xdg/weston/weston

#pio1 pio2 retain fix
/etc/script/retain_pio2.sh

# setup weston.ini
set_weston_ini () {
	### get devconf from eeprom
	devconf=$(hexdump -v -s 0x00001fe -n 2 $eeprom_device | head -n 1 | cut  -f 2 -d ' ')
	if [ "$devconf" == "1000" ] ; then
		addchar_xres="_480_272"
	elif [ "$devconf" == "1001" ] ; then
		addchar_xres="_640_480"
	elif [ "$devconf" == "1002" ] ; then
		addchar_xres="_800_480"
	elif [ "$devconf" == "1003" ] ; then
		addchar_xres="_1280_800"
	elif [ "$devconf" == "1008" ] ; then
		addchar_xres="_1920_1080"
	elif [ "$devconf" == "1009" ] ; then
		addchar_xres="_1920_1080"
	elif [ "$devconf" == "1010" ] ; then
		hdmi="1"
		addchar_xres="_1920_1080"
	else
		echo "screen resolution touch not recognized set default 800x480"
		addchar_xres="_800_480"
	fi

	### get touch type
	if [ "$devconf" = "1000" ]; then
		cap="0"
	elif [ "$devconf" = "1001" ]; then
		cap="0"
	elif [ "$devconf" = "1002" ]; then
		(cat /proc/bus/input/devices | grep "PIXCIR HID Touch Panel") &>/dev/null
		if [ $? -eq 0 ]; then
			cap="1"
		else
			cap="0"
		fi
	elif [ "$devconf" = "1003" ]; then
		(cat /proc/bus/input/devices | grep "PIXCIR HID Touch Panel") &>/dev/null
		if [ $? -eq 0 ]; then
			cap="1"
		else
			cap="0"
		fi
	elif [ "$devconf" = "1008" ]; then
		(cat /proc/bus/input/devices | grep "eGalax Inc. eGalaxTouch P80H84") &>/dev/null
		if [ $? -eq 0 ]; then
			cap="1"
			echo 6 > /sys/bus/i2c/devices/2-001b/nthr_value
		else
			cap="0"
		fi
	elif [ "$devconf" = "1009" ]; then
		(cat /proc/bus/input/devices | grep "eGalax Inc. eGalaxTouch P80H84") &>/dev/null
		if [ $? -eq 0 ]; then
			cap="1"
			echo 6 > /sys/bus/i2c/devices/2-001b/nthr_value
		else
			cap="0"
		fi
	elif [ "$devconf" = "1010" ]; then
		hdmi="1"
	else
		echo "no valid display resolution in eeprom found, use default resolution 800x480"
		cap="0"
	fi

	if [ "$cap" == "1" ] ; then
		echo "cap touch found"
		addchar="_cap"
		weston_ini_file=$weston_ini_file$addchar
		#echo 15 > /sys/bus/i2c/devices/2-001b/NTHR_VALUE
	elif [ "$cap" == "0" ] ; then
		echo "res touch found"
		#echo 10 > /sys/bus/i2c/devices/2-001b/NTHR_VALUE
	elif [ "$hdmi" == "1" ] ; then
		echo "hdmi found"
		addchar="_hdmi"
		weston_ini_file=$weston_ini_file$addchar
		#echo 10 > /sys/bus/i2c/devices/2-001b/NTHR_VALUE
	else
		echo "cap/res touch not recognized"
	fi

	weston_ini_file=$weston_ini_file$addchar_xres

	### get screen orientation
	orientation=$(hexdump -v -s 0x00001f8 -n 2 $eeprom_device | head -n 1 | cut  -f 2 -d ' ')
	orientation=$(echo ${orientation:3})
	if [ "$orientation" == 0 ] ; then
		addchar=".ini"
		weston_ini_file=$weston_ini_file$addchar
	elif [ "$orientation" == 1 ] ; then
		addchar="_CW.ini"
		weston_ini_file=$weston_ini_file$addchar
	elif [ "$orientation" == 2 ] ; then
		addchar="_UD.ini"
		weston_ini_file=$weston_ini_file$addchar
	elif [ "$orientation" == 3 ] ; then
		addchar="_CCW.ini"
		weston_ini_file=$weston_ini_file$addchar
	else
		echo "screen orientation touch not recognized"
		addchar=".ini"
		weston_ini_file=$weston_ini_file$addchar
	fi

	if [ -e "$weston_ini_file" ] ; then
		rm -f /etc/xdg/weston/weston.ini
		ln -s $weston_ini_file /etc/xdg/weston/weston.ini
		#echo "#### weston_ini_file=$weston_ini_file"
	else
		echo "weston.ini not found ${weston_ini_file}"
	fi

	###remove resistive touch configuration if captouch found
	if [ "$cap" == "1" ] ; then
		rm -f /lib/udev/rules.d/65-weston-res-touchscreen.rules
	fi
}

set_weston_ini

# create java cache directory
mkdir -p /tmp/.java
mkdir -p /tmp/.java/deployment
mkdir -p /tmp/.java/deployment/cache
chmod 755 /tmp/.java/deployment/cache

# allow user www to read brightness
brightness_file=/sys/class/backlight/backlight/brightness
if [ -e "$brightness_file" ]; then
chmod 666 "$brightness_file"
fi

# allow user www to set / read display orientation
chmod 666 $eeprom_device
if [[ ! -d "/sys/class/gpio/gpio23" ]]; then
	echo 23 > /sys/class/gpio/export
	echo out > /sys/class/gpio/gpio23/direction
	chmod 666 /sys/class/gpio/gpio23/value
fi

exit 0

