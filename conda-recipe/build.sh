mkdir build
cd build

if [ $(uname) == "Darwin" ]; then
    LINKER_FLAGS="-L${PREFIX}/lib"
else
    LINKER_FLAGS="-Wl,-rpath-link,${PREFIX}/lib -L${PREFIX}/lib"
fi

cmake .. \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    -DCMAKE_BUILD_TYPE=Release \
    -DPYTHON_EXECUTABLE=${PYTHON} \
    -DPYTHON_INCLUDE_DIRS=${PREFIX}/include/python${CONDA_PY} \
    -DWITH_LOG=OFF \

make -j${CPU_COUNT}
make install
