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
  log "Installing headers"
  cp -rLf "$SUBPROJECT_BUILD_DIRERCTORY"/../include/ControlSystemAdapter "$CMAKE_PROJECT_DIRECTORY"/include 
  log "Installing libraries"
  cp "$SUBPROJECT_BUILD_DIRERCTORY"/libControlSystemAdapter* "$CMAKE_PROJECT_DIRECTORY/lib"
  ;;
uninstall)
  rm -r ControlSystemAdapter "$CMAKE_PROJECT_DIRECTORY"/include/ControlSystemAdapter
  rm -r ControlSystemAdapter "$CMAKE_PROJECT_DIRECTORY"/lib/libControlSystemAdapter*
;;
*) 
  echo "Invalid script command \"$1\""
  usage
  exit 1
;;
esac

exit 0
