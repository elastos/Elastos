#!/bin/sh

#argv1: SRC_DIR
#argv2: HOST
#argv3: ARCH

SRC_DIR=$1
shift
HOST=$1
shift
ARCH=$1

PJ_CONFIG_SITE_H=${SRC_DIR}/pjlib/include/pj/config_site.h

sed -ie "s/^#[[:space:]]*define[[:space:]]*PJ_STUN_SOCK_PKT_LEN[[:space:]]*2000/#   define PJ_STUN_SOCK_PKT_LEN                2176/" "${SRC_DIR}/pjnath/include/pjnath/config.h"

case "${HOST},${ARCH}" in
    "iOS",*)
        echo "#define PJ_CONFIG_IPHONE 1" > ${PJ_CONFIG_SITE_H}
        echo "#include <pj/config_site_sample.h>" >> ${PJ_CONFIG_SITE_H}
        sed -ie "s%PJMEDIA_AUDIO_DEV_HAS_COREAUDIO[[:space:]]*1%PJMEDIA_AUDIO_DEV_HAS_COREAUDIO 0%" ${SRC_DIR}/pjlib/include/pj/config_site_sample.h
        sed -ie "s%PJMEDIA_HAS_ILBC_CODEC[[:space:]]*1%PJMEDIA_HAS_ILBC_CODEC 0%" ${SRC_DIR}/pjlib/include/pj/config_site_sample.h
        sed -ie "s%-framework AudioToolbox -framework Foundation%-framework AudioToolbox -framework AVFoundation -framework CFNetwork -framework Foundation%" ${SRC_DIR}/configure-iphone
        ;;
    "Android",*)
        echo "#define PJ_CONFIG_ANDROID 1" > ${PJ_CONFIG_SITE_H}
        echo "#include <pj/config_site_sample.h>" >> ${PJ_CONFIG_SITE_H}
        ;;
    *)
        ;;
esac

exit 0

