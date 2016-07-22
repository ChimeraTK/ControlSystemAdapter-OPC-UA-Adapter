#!/bin/bash
CMAKE_PROJECT_DIRECTORY="$1"
SUBPROJECT_BUILD_DIRERCTORY="$2"

function log() {
    echo $@
}

## Check Options
if [ $# -lt 2 ];then
  exit 1
fi

if [ ! -d "$CMAKE_PROJECT_DIRECTORY/include/open62541" ]; then
    mkdir "$CMAKE_PROJECT_DIRECTORY/include/open62541"
fi

log "Installing headers"
cp "$SUBPROJECT_BUILD_DIRERCTORY"/open62541.h "$CMAKE_PROJECT_DIRECTORY/include/open62541"
log "Installing libraries"
cp "$SUBPROJECT_BUILD_DIRERCTORY"/libopen62541* "$CMAKE_PROJECT_DIRECTORY/lib"

cp -r "$SUBPROJECT_BUILD_DIRERCTORY/../tools/pyUANamespace" "$CMAKE_PROJECT_DIRECTORY/tools"
rm "$CMAKE_PROJECT_DIRECTORY"/tools/pyUANamespace/*txt

exit 0
