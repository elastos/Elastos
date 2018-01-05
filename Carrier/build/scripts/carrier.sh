#!/bin/sh

# argv1: HOST
# argv2: DIST_DIR
postInstallProduct() {
    case $1 in
       "Android")
            cd $2/lib && ${AR} -M <$2/../../../scripts/libcarrier.mri
            ;;
       *)
            ;;
    esac
}

if [ x"$1" = x"postInstall" ]; then
    postInstallProduct $2 $3
fi

# argv1: HOST
# argv2: ARCH
# argv3: BUILD
# argv4: DIST_DIR
packDevDist() {
    FILES="include/ela_carrier.h "
    for header in ext session; do
        FILES="${FILES} include/ela_${header}.h "
    done

    case $1 in
        "Linux" | "Raspbian")
            for lib in common carrier session; do
                FILES="${FILES} lib/libela${lib}.so "
            done
            ;;
        "Android")
            FILES="${FILES} $(ls lib/libelacarrier.a)"
            ;;
        "Darwin")
            for lib in common carrier session; do
                FILES="${FILES} lib/libela${lib}.dylib "
            done
            ;;
        "iOS")
            for lib in sodium pj pjlib-util pjnath pjmedia elacommon elacarrier elasession; do
                FILES="${FILES} lib/lib${lib}.a " 
            done
            ;;
        *)
            echo "Unsupported Host: Arch($1:$2)."
            ;;
    esac

    if [ x"$(uname -s)" = x"Darwin" ]; then
        export COPYFILE_DISABLE=1
    fi

    tar -cjvf carrier-$1-$2-$3.tar.bz2 ${FILES}
    tar -czvf carrier-$1-$2-$3.tar.gz  ${FILES}
}

if [ x"$1" = x"packDevDist" ]; then
    cd $5 && packDevDist $2 $3 $4
fi
