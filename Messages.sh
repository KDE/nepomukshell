#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/pimoshell.pot
