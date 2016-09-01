#!/bin/bash
SETUP_COMMAND="$1"
CMAKE_PROJECT_DIRECTORY="$2"
SUBPROJECT_BUILD_DIRERCTORY="$3"

function usage() {
    echo "setup script usage: setup_<depName> [install|uninstall] CMAKE_PROJECT_DIRECTORY SUBPROJECT_BUILD_DIRERCTORY"
    echo ""
}

function log() {
    echo $@
}

## Check Options
if [ $# -lt 3 ];then
  exit 1
fi

case $SETUP_COMMAND in
install)
  if [ ! -d "$CMAKE_PROJECT_DIRECTORY/include/open62541" ]; then
      mkdir "$CMAKE_PROJECT_DIRECTORY/include/open62541"
  fi
  mkdir -p "$CMAKE_PROJECT_DIRECTORY/lib"
  
  log "Installing headers"
  cp "$SUBPROJECT_BUILD_DIRERCTORY"/open62541.h "$CMAKE_PROJECT_DIRECTORY/include/open62541"
  log "Installing libraries"
  cp "$SUBPROJECT_BUILD_DIRERCTORY"/libopen62541* "$CMAKE_PROJECT_DIRECTORY/lib"

  cp -r "$SUBPROJECT_BUILD_DIRERCTORY/../tools/pyUANamespace" "$CMAKE_PROJECT_DIRECTORY/tools"
  rm "$CMAKE_PROJECT_DIRECTORY"/tools/pyUANamespace/*txt
  ;;
uninstall)
  rm -r "$CMAKE_PROJECT_DIRECTORY/include/open62541"
  rm -r "$CMAKE_PROJECT_DIRECTORY/lib"/libopen62541*
  rm -r "$CMAKE_PROJECT_DIRECTORY/tools/pyUANamespace"
;;
*) 
  echo "Invalid script command \"$1\""
  usage
  exit 1
;;
esac



exit 0
