#!/bin/bash
#	= ^ . ^ =

# Entrypoint for 'python:3.9-slim-bullseye', and also before_script for GitLab CI/CD
# This fine «NEEDS» to be named `entrypoint.sh`, otherwise the CI/CD pipeline breaks

# Linux console without GUI
export TERM=linux

# https://manpages.debian.org/bullseye/debconf-doc/debconf.7.en.html
export DEBIAN_FRONTEND=noninteractive
export DEBIAN_PRIORITY=critical

APT_CONF_LOCAL="/etc/apt/apt.conf.d/99-local"

# Set this variable to the empty string if you want pip to return "the warning" about running as root:
# WARNING: Running pip as the 'root' user can result in broken permissions and conflicting behaviour with the system package manager.
# It is recommended to use a virtual environment instead: https://pip.pypa.io/warnings/venv
# PIP_INSTALL_OPTS=""
PIP_INSTALL_OPTS="--root-user-action=ignore"

set -exuo pipefail

export PATH="${PATH}:${HOME}/.local/bin"

date

pwd

truncate --size=0 "${APT_CONF_LOCAL}"

cat >> "${APT_CONF_LOCAL}" << EOF
quiet "2";
APT::Get::Assume-Yes "1";
APT::Install-Recommends "0";
APT::Install-Suggests "0";
APT::Color "0";
Dpkg::Progress "0";
Dpkg::Progress-Fancy "0";
EOF

apt -qq update
apt -qq install apt-utils >/dev/null
apt -qq install make git  >/dev/null

pip3 config --global set global.progress_bar off
python3 -m pip install --quiet --upgrade pip

make install PIP_INSTALL_OPTS="${PIP_INSTALL_OPTS}"

# mkdocs serve --verbose --dev-addr 0.0.0.0:1313
