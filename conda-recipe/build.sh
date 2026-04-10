mkdir build
cd build

if [ $(uname) == "Darwin" ]; then
    LDFLAGS="-undefined dynamic_lookup -L${PREFIX}/lib ${LDFLAGS}"
else
    LDFLAGS="-Wl,-rpath-link,${PREFIX}/lib -L${PREFIX}/lib ${LDFLAGS}"
fi

cmake .. \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET}" \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    -DCMAKE_BUILD_TYPE=Release \
    -DPython_EXECUTABLE=${PYTHON} \
    -DPYTHON_INCLUDE_DIRS=${PREFIX}/include/python${CONDA_PY} \
    -DWITH_LOG=OFF \
    -DWITH_PYTHON=ON \
    -DWITH_BIN=ON \
    -DCMAKE_CXX_LINK_FLAGS="${LDFLAGS}" \
    -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}" \
    "${CMAKE_PLATFORM_FLAGS[@]}" \

make -j${CPU_COUNT}
make install
