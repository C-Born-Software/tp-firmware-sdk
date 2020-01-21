#!/bin/bash
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
# Script:   start_targetvisu.sh
#
# Brief:    starting up codesys targetvisu
#
# Author:   Wolfgang Rückl: elrest Automationssysteme GmbH
#
#-----------------------------------------------------------------------------#


#CP / VP / WP demomode
#BOOTAPP="$(/etc/config-tools/get_eruntime bootapp)"
#if [ "$BOOTAPP" == "yes" ]; then
  PIDV3=`pidof codesys3`
  if [ -z $PIDV3 ]; then
  /etc/init.d/runtime start 3
  fi
  echo close > /dev/webenginebrowser
#fi
