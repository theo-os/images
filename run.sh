#!/bin/bash
set -eo pipefail

podman build -t localhost/timdows/base -f Dockerfile.base .
kit

qemu-system-x86_64 -bios /usr/share/ovmf/x64/OVMF.fd -cdrom build/os.iso -hda build/rootfs.img -m 4G \
    -serial stdio
