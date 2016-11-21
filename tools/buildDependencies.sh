#!/bin/bash

## Subprojects and dependencies installation script
## This script will fetch, compile and install any dependencies required to compile this project
## 
## usage: buildDependencies.sh <CMAKE_PROJECT_DIRECTORY> [uninstall]
##
## The script must be passed the CMake's main project directory. An optional additional parameter
## "uninstall" may be passed, which will remove any files generated from the dependencies from 
## the project.
##
## Note the installing and uninstalling the subprojects is left up to the individual setup scripts.
## They can be found alongside this script and are called setup_subproject.sh

#  Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
#  Chair for Process Systems Engineering
#  Technical University of Dresden
#  
#  Permission is hereby granted, free of charge, to any person
#  obtaining a copy of this software and associated documentation
#  files (the "Software"), to deal in the Software without
#  restriction, including without limitation the rights to use,
#  copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following
#  conditions:
# 
#  The above copyright notice and this permission notice shall be
#  included in all copies or substantial portions of the Software.
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#  OTHER DEALINGS IN THE SOFTWARE.

CMAKE_PROJECT_DIRECTORY="$1"

## Directory configs
PROJECT_DEPENDENCIES_DIR="$CMAKE_PROJECT_DIRECTORY/tools/dependencies"

if [  ! -d "$PROJECT_DEPENDENCIES_DIR" ]; then
    mkdir -p "$PROJECT_DEPENDENCIES_DIR"
fi

## Subproject targets
SUBPROJECTS=1

SUBPROJECTS_NAMES[0]="open62541"
SUBPROJECTS_VCS[0]="git"
SUBPROJECTS_URI[0]="https://github.com/open62541/open62541.git"
SUBPROJECTS_BRANCH[0]=""
SUBPROJECTS_CMAKEOPTIONS[0]="-DUA_ENABLE_AMALGAMATION=On -DBUILD_SHARED_LIBS=On -DUA_ENABLE_GENERATE_NAMESPACE0=On -DUA_ENABLE_METHODCALLS=ON -DUA_ENABLE_NODEMANAGEMENT=ON -DUA_ENABLE_SUBSCRIPTIONS=ON"

#SUBPROJECTS_NAMES[1]="ControlSystemAdapter"
#SUBPROJECTS_VCS[1]="git"
#SUBPROJECTS_URI[1]="https://github.com/ChimeraTK/ControlSystemAdapter"
#SUBPROJECTS_CMAKEOPTIONS[1]=""

function vcs_getProject() {
    VCS="$1"
    URI="$2"
		BRANCH="$3"
    TGTDIR="$4"
    
    if [ $VCS == "git" -o $VCS="GIT" ]; then
			if [$BRANCH == ""]; then
        git $OPTIONS clone "$URI" "$TGTDIR"
			else
				git $OPTIONS clone -b $BRANCH "$URI" "$TGTDIR"
			fi
    elif  [ $VCS == "svn" -o $VCS="SVN" ]; then
        svn $OPTIONS checkout "$URI" "$TGTDIR"
    fi
}

function vcs_updateProject() {
    VCS="$1"
    URI="$2"
		BRANCH="$3"
    TGTDIR="$4"
    
    if [ $VCS == "git" -o $VCS="GIT" ]; then
        git -C "$TGTDIR" fetch
    elif  [ $VCS == "svn" -o $VCS="SVN" ]; then
        svn $OPTIONS update "$TGTDIR"
    fi
}

function log() {
    echo $@
}

function installAll() {
  ## Stage 1: Clone/Fetch all subprojects
  I=0
  while [ $I -lt $SUBPROJECTS ]; do
    log "Fetching/Updating subproject $I ${SUBPROJECTS_NAMES[$I]}"
    # Decide whether to clone or pull 
    if [ -d "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}" ]; then
        vcs_updateProject "${SUBPROJECTS_VCS[$I]}" "${SUBPROJECTS_URI[$I]}" "${SUBPROJECTS_BRANCH[$I]}" "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}"
    else
        vcs_getProject "${SUBPROJECTS_VCS[$I]}" "${SUBPROJECTS_URI[$I]}" "${SUBPROJECTS_BRANCH[$I]}" "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}"
    fi
    
    I=$((I+1))
  done

  ## Stage 2: Patch all subprojects

  ## Stage 3: Build all subprojects
  I=0
  while [ $I -lt $SUBPROJECTS ]; do
    log "Building subproject ${SUBPROJECTS_NAMES[$I]}"
    # Decide whether to clone or pull 
    if [ ! -d "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}/build" ]; then
        mkdir "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}/build"
    fi
    
    OLDD=`pwd`
    cd "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}/build"
    (cmake ${SUBPROJECTS_CMAKEOPTIONS[$I]} .. && make) || exit 1
    cd $OLDD
    
    log "Installing subproject"
    if [ ! -f "$CMAKE_PROJECT_DIRECTORY/tools/setup_${SUBPROJECTS_NAMES[$I]}.sh" ]; then
        log "Installscript does not exist... creating template"
        echo -e "#!/bin/bash\nCMAKE_PROJECT_DIRECTORY=\"\$1\"\nSUBPROJECT_BUILD_DIRERCTORY=\"\$2\"\n\nfunction log() {\n    echo \$@\n}\n\n## Check Options\nif [ \$# -lt 2 ];then\n  exit 1\nfi\n\n\nexit 0" >  "$PROJECT_DEPENDENCIES_DIR/setup_${SUBPROJECTS_NAMES[$I]}.sh"
    fi
    
    /bin/bash "$CMAKE_PROJECT_DIRECTORY/tools/setup_${SUBPROJECTS_NAMES[$I]}.sh" install "$CMAKE_PROJECT_DIRECTORY" "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}/build"
    
    I=$((I+1))
  done
}

function uninstallAll() {
  ## Call setup script to uninstall all installed files
  I=0
  while [ $I -lt $SUBPROJECTS ]; do
    log "Unistalling subproject $I ${SUBPROJECTS_NAMES[$I]}"
    
    /bin/bash "$CMAKE_PROJECT_DIRECTORY/tools/setup_${SUBPROJECTS_NAMES[$I]}.sh" uninstall "$CMAKE_PROJECT_DIRECTORY" "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}/build"
    
    I=$((I+1))
  done
  
  log "Removing dependencies build directory"
  rm -rf "$PROJECT_DEPENDENCIES_DIR"
}

## Check Options
if [ $# -lt 1 ]; then
    log "Invalid number of arguments"
    exit 1
fi

if [ ! -d $CMAKE_PROJECT_DIRECTORY -o ! -d $PROJECT_DEPENDENCIES_DIR ]; then
    log "Invalid Project folder"
    exit 1
fi

if [ ! -z "$2" -a "$2"="uninstall" ]; then
  uninstallAll
else
  installAll
fi

exit 0;
