
strip_invalid_archs() {
  binary="$1"
  # Get architectures for current target binary
  binary_archs="$(lipo -info "$binary" | rev | cut -d ':' -f1 | awk '{$1=$1;print}' | rev)"

  for arch in $binary_archs; do
    echo "$arch"
  done

  # Intersect them with the architectures we are building for
  intersected_archs=["x86_64"]

  # If there are no archs supported by this binary then warn the user
  if [[ -z "$intersected_archs" ]]; then
    echo "warning: [CP] Vendored binary '$binary' contains architectures ($binary_archs) none of which match the current build architectures ($ARCHS)."
    STRIP_BINARY_RETVAL=0
    return
  fi
  stripped=""
  for arch in $binary_archs; do
    if ! [[ "${ARCHS}" == *"$arch"* ]]; then
      # Strip non-valid architectures in-place
      lipo -remove "$arch" -output "$binary" "$binary"

      stripped="$stripped $arch"
    fi
  done
  if [[ "$stripped" ]]; then
    echo "Stripped $binary of architectures:$stripped"
  fi
  STRIP_BINARY_RETVAL=1
}

DESTNATION="${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}/ElastosCarrier.framework/Frameworks"
CARRIERLIBS="crystal elacarrier elasession elafiletrans"

if [[ "$CONFIGURATION" == "Release" ]]; then
for dylib in ${CARRIERLIBS}; do
strip_invalid_archs "${DESTNATION}/lib${dylib}.dylib"
done
fi

if [ -n "${EXPANDED_CODE_SIGN_IDENTITY}" -a "${CODE_SIGNING_REQUIRED}" != "NO" -a "${CODE_SIGNING_ALLOWED}" != "NO" ]; then
for dylib in ${CARRIERLIBS}; do
/usr/bin/codesign --force --sign ${EXPANDED_CODE_SIGN_IDENTITY} ${OTHER_CODE_SIGN_FLAGS}  --preserve-metadata=identifier,entitlements "${DESTNATION}/lib${dylib}.dylib"
done
fi

