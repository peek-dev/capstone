#!/usr/bin/env bash

#
# Copyright (C) 2024 John E. Berberian, Jr.
#
# copy_data.sh: a small shell script to be run by systemd whenever a 
# generic USB storage device (e.g., a flash drive) is mounted to copy 
# C.H.E.S.S.B.O.A.R.D. stored game data onto the device, then clear it from 
# the Raspberry Pi's filesystem.
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>
#

LOGFILE="/root/copy_data.log"
MOUNTPT="/media/chessboard"
SRCDIR="/opt/chessboard"
(
flock -s 200
mkdir -p "${MOUNTPT}"
echo "Got partition: $1"
TARGET_DEV="$1"
if [[ -b "${TARGET_DEV}" ]]; then
  echo "Mounting ${TARGET_DEV}"
  mount -w "${TARGET_DEV}" "${MOUNTPT}"
  echo "Copying files."
  cp "${SRCDIR}/"* "${MOUNTPT}/"
  echo "Copying done."
  sync
  umount "${MOUNTPT}"
else
  echo "${TARGET_DEV} does not exist, skipping."
fi


) 200>/root/copy_data.lock >> "${LOGFILE}" 2>> "${LOGFILE}.err"

tail -n100 "$LOGFILE" | sponge "$LOGFILE"
