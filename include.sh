#!/usr/bin/env bash

WPL_PATH_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/" && pwd )"

source $WPL_PATH_ROOT"/conf/conf.sh.dist"

if [ -f $WPL_PATH_ROOT"/conf/conf.sh" ]; then
    source $WPL_PATH_ROOT"/conf/conf.sh"
fi
