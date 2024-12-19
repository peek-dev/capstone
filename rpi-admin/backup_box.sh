#!/usr/bin/env bash
TIMESTAMP="$(date -Iseconds)"
echo "Beginning backup: ${TIMESTAMP}"
sudo rm -f /backup.tar.gz
sudo tar -cp -f /backup.tar.gz -I "pigz -4" --exclude=/backup.tar.gz --exclude=/dev/* --exclude=/proc/* --exclude=/sys/* --exclude=/tmp/* --exclude=/run/* --exclude=/mnt/* --exclude=/media/* --exclude=/lost+found/ /
pigz -l /backup.tar.gz
sudo rclone -P copyto /backup.tar.gz "box:/capstone-backups/rachel-${TIMESTAMP}.tar.gz" >> /home/jeb/backup_box.progress-log
#sudo rm -f /backup.tar.gz
echo "Backup complete!"
echo
