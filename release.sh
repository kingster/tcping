#!/bin/bash

VERSION=$(date +%s)
fpm --force \
  --input-type dir --output-type deb\
  --version "0.2.$VERSION" --name tcping\
  --architecture amd64\
  --prefix /\
  --description 'tcping'\
  --url "https://github.com/kingster/tcping"\
  --maintainer "me@kinshuk.in" \
  --no-deb-systemd-restart-after-upgrade \
  --package tcping.deb \
  ./tcping=/usr/sbin/tcping

