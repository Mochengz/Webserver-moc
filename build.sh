#!/bin/sh


set -e

rm -rf `pwd`/build/*
cd `pwd`/build &&
	cmake .. &&
	make
cd ..