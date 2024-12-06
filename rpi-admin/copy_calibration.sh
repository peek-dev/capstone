#!/usr/bin/env bash
LOGFILE="/home/jeb/copy_calibration.log"
date | tr -d '\n' >> "${LOGFILE}"
echo -n " : " >> "${LOGFILE}"
COPYFROM="/home/paul/capstone/wrapper/tests/calibration_data.txt"
COPYTO="/mnt/calibration/calibration_data4.txt"
LASTUPDATED_TRACKINGFILE="/home/jeb/.config/calibration-last-updated"
if [ -f "${COPYFROM}" ]; then
  if [ ! "${COPYFROM}" -ot "${LASTUPDATED_TRACKINGFILE}" ]; then
    touch "${LASTUPDATED_TRACKINGFILE}"
    sudo cp "${COPYFROM}" "${COPYTO}"
    echo "Copied file." >> "${LOGFILE}"
  else
    echo "Not old enough, skipping." >> "${LOGFILE}"
  fi
else
  echo "File does not exist, skipping." >> "${LOGFILE}"
fi

tail -n100 "${LOGFILE}" | sponge "${LOGFILE}"
