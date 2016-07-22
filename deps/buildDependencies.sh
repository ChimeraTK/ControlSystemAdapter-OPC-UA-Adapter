#!/bin/bash

CMAKE_PROJECT_DIRECTORY="$1"

## Directory configs
PROJECT_DEPENDENCIES_DIR="$CMAKE_PROJECT_DIRECTORY/deps"

## Subproject targets
SUBPROJECTS=2

SUBPROJECTS_NAMES[0]="open62541"
SUBPROJECTS_VCS[0]="git"
SUBPROJECTS_URI[0]="https://github.com/open62541/open62541.git"
SUBPROJECTS_CMAKEOPTIONS[0]="-DUA_ENABLE_AMALGAMATION=On -DBUILD_SHARED_LIBS=On -DUA_ENABLE_GENERATE_NAMESPACE0=On -DUA_ENABLE_METHODCALLS=ON -DUA_ENABLE_NODEMANAGEMENT=ON -DUA_ENABLE_SUBSCRIPTIONS=ON"

SUBPROJECTS_NAMES[1]="ControlSystemAdapter"
SUBPROJECTS_VCS[1]="git"
SUBPROJECTS_URI[1]="https://github.com/ChimeraTK/ControlSystemAdapter"
SUBPROJECTS_CMAKEOPTIONS[1]=""

function vcs_getProject() {
    VCS="$1"
    URI="$2"
    TGTDIR="$3"
    
    if [ $VCS == "git" -o $VCS="GIT" ]; then
        git $OPTIONS clone "$URI" "$TGTDIR"
    elif  [ $VCS == "svn" -o $VCS="SVN" ]; then
        svn $OPTIONS checkout "$URI" "$TGTDIR"
    fi
}

function vcs_updateProject() {
    VCS="$1"
    URI="$2"
    TGTDIR="$3"
    
    if [ $VCS == "git" -o $VCS="GIT" ]; then
        git -C "$TGTDIR" fetch 
    elif  [ $VCS == "svn" -o $VCS="SVN" ]; then
        svn $OPTIONS update "$TGTDIR"
    fi
}

function log() {
    echo $@
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

## Stage 1: Clone/Fetch all subprojects
I=0
while [ $I -lt $SUBPROJECTS ]; do
    log "Fetching/Updating subproject $I ${SUBPROJECTS_NAMES[$I]}"
    # Decide whether to clone or pull 
    if [ -d "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}" ]; then
        vcs_updateProject "${SUBPROJECTS_VCS[$I]}" "${SUBPROJECTS_URI[$I]}" "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}"
    else
        vcs_getProject "${SUBPROJECTS_VCS[$I]}" "${SUBPROJECTS_URI[$I]}" "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}"
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
    if [ ! -f "$PROJECT_DEPENDENCIES_DIR/setup_${SUBPROJECTS_NAMES[$I]}.sh" ]; then
        log "Installscript does not exist... creating template"
        echo -e "#!/bin/bash\nCMAKE_PROJECT_DIRECTORY=\"\$1\"\nSUBPROJECT_BUILD_DIRERCTORY=\"\$2\"\n\nfunction log() {\n    echo \$@\n}\n\n## Check Options\nif [ \$# -lt 2 ];then\n  exit 1\nfi\n\n\nexit 0" >  "$PROJECT_DEPENDENCIES_DIR/setup_${SUBPROJECTS_NAMES[$I]}.sh"
    fi
    
    /bin/bash "$PROJECT_DEPENDENCIES_DIR/setup_${SUBPROJECTS_NAMES[$I]}.sh" "$CMAKE_PROJECT_DIRECTORY" "$PROJECT_DEPENDENCIES_DIR/${SUBPROJECTS_NAMES[$I]}/build"
    
    I=$((I+1))
done

exit 0;