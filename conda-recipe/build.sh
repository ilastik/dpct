mkdir build
cd build

if [ $(uname) == "Darwin" ]; then
    LDFLAGS="-undefined dynamic_lookup -L${PREFIX}/lib ${LDFLAGS}"
else
    LDFLAGS="-Wl,-rpath-link,${PREFIX}/lib -L${PREFIX}/lib ${LDFLAGS}"
fi

WITH_PYTHON=${WITH_PYTHON:-OFF}
MULTI_STAGE_BUILD=${MULTI_STAGE_BUILD:-OFF}
WITH_TESTS=${WITH_TESTS:-OFF}

cmake .. \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET}" \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    -DCMAKE_BUILD_TYPE=Release \
    -DPython_EXECUTABLE=${PYTHON} \
    -DWITH_LOG=OFF \
    -DWITH_PYTHON=${WITH_PYTHON} \
    -DMULTI_STAGE_BUILD=${MULTI_STAGE_BUILD} \
    -DWITH_BIN=OFF \
    -DWITH_TESTS=${WITH_TESTS} \
    -DCMAKE_CXX_LINK_FLAGS="${LDFLAGS}" \
    -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}" \
    "${CMAKE_PLATFORM_FLAGS[@]}" \

make -j${CPU_COUNT}


# This is for the conda recipe really - the tests don't define install targets
# so if these are run in isolation, we want to skip.
if [[ "${WITH_TESTS}" == "OFF" ]]; then
    make install
fi