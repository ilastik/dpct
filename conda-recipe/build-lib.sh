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
    -DWITH_LOG=OFF \
    -DCMAKE_CXX_LINK_FLAGS="${LDFLAGS}" \
    -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}" \
    -DBoost_INCLUDE_DIR=${PREFIX}/include \
    -DBoost_LIBRARY_DIRS=${PREFIX}/lib \
    -DBoost_PYTHON_LIBRARY=${PREFIX}/lib/libboost_python${CONDA_PY}${SHLIB_EXT} \
    -DBoost_NO_BOOST_CMAKE=ON \
    -DWITH_PYTHON=OFF \
    "${CMAKE_PLATFORM_FLAGS[@]}" \

make -j${CPU_COUNT}
make install
