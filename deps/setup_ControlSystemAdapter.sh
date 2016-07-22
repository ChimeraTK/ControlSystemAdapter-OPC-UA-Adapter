#!/bin/bash
CMAKE_PROJECT_DIRECTORY="$1"
SUBPROJECT_BUILD_DIRERCTORY="$2"

function log() {
    echo $@
}

## Check Options
if [ $# -lt 1 ];then
  exit 1
fi


log "Installing headers"
cp -rLf "$SUBPROJECT_BUILD_DIRERCTORY"/../include/ControlSystemAdapter "$CMAKE_PROJECT_DIRECTORY"/include 
log "Installing libraries"
cp "$SUBPROJECT_BUILD_DIRERCTORY"/libControlSystemAdapter* "$CMAKE_PROJECT_DIRECTORY/lib"

exit 0
