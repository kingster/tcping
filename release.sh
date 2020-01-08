#!/bin/bash

fpm --force \
  --input-type dir --output-type deb\
  --version "0.2" --name tcping\
  --architecture amd64\
  --prefix /\
  --description 'tcping'\
  --url "https://github.com/kingster/tcping"\
  --no-deb-systemd-restart-after-upgrade \
  --package tcping.deb \
  ./tcping=/usr/bin

