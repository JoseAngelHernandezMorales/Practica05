#!/usr/bin/make -f
#	= ^ . ^ =
SHELL=/bin/bash

PRE_COMMIT=pre-commit
MKDOCS=mkdocs

# Inherit the arguments override for "pip install"
# PIP_INSTALL_OPTS?=--user
PIP_INSTALL_OPTS?=

default:	pre-commit serve
test:	build

pre-commit:
	which ${PRE_COMMIT}
	${PRE_COMMIT} install --color=never
	${PRE_COMMIT} run --all-files --color=never

install:
	pip3 config --user set global.progress_bar off
	-pip3 install ${PIP_INSTALL_OPTS} --upgrade pip
	which ${MKDOCS} || \
	pip3 install ${PIP_INSTALL_OPTS} --quiet --requirement requirements.txt

build:	install
	${MKDOCS} $@ --strict

serve:	install
	NO_COLOR=1 ${MKDOCS} $@ --strict --verbose

grade:
	pwd
	find ./docs/ -mindepth 4 -maxdepth 5 -name Makefile -exec dirname {} + | \
	  xargs -r -t -n 1 make -C
