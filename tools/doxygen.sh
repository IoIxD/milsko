#!/bin/sh
# $Id$
doxygen
rm -rf /var/www/milsko-doxygen
cp -rf doxygen/html /var/www/milsko-doxygen
