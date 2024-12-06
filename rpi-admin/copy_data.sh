#!/usr/bin/env bash

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
