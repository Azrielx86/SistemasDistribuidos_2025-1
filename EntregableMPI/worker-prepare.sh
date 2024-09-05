#!/bin/bash
if ! [ $(id -u) = 0 ]; then
    echo "This script must be run as root!"
exit 1
fi

if [[ -d "/opt/fimpi" ]]; then
  echo "MPI Projects directory already exists."
else
  echo "MPI Projects directory does not exists! Creating one..."
  mkdir /opt/fimpi
  chown -R $(whoami) /opt/fimpi
fi

cat /proc/mounts | grep -E -q "manager|nfs"
if [[ $? -ne 0 ]]; then
  echo "Mounting manager NFS server."
  mount -t nfs manager:/opt/fimpi /opt/fimpi
  echo "NFS directory mounted!"
else
  echo "NFS directory is already mounted"
fi
