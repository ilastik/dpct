cd build

set CONFIGURATION=Release

REM set LINKER_FLAGS="-L${PREFIX}/lib"
REM set DYLIB="dylib"
REM if [ `uname` != "Darwin" ]; then
    REM LINKER_FLAGS="-Wl,-rpath-link,${PREFIX}/lib ${LINKER_FLAGS}"
    REM set DYLIB="so"
REM fi

cmake .. -G "NMake Makefiles" ^
         -DCMAKE_BUILD_TYPE=%CONFIGURATION% ^
         -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" ^
         -DCMAKE_INSTALL_PREFIX="%LIBRARY_PREFIX%" ^
         -DPYTHON_EXECUTABLE="%PYTHON%" ^
         -DWITH_LOG="OFF" ^
         -DWITH_PYTHON="OFF" ^
         -DBoost_NO_BOOST_CMAKE=ON


if errorlevel 1 exit 1

REM cmake .. \
    REM -DCMAKE_C_COMPILER=${PREFIX}/bin/gcc \
    REM -DCMAKE_CXX_COMPILER=${PREFIX}/bin/g++ \
    REM -DCMAKE_OSX_DEPLOYMENT_TARGET=10.7 \
    REM -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    REM -DCMAKE_BUILD_TYPE=Release \
    REM -DPYTHON_EXECUTABLE=${PYTHON} \
    REM -DPYTHON_LIBRARY=${PREFIX}/lib/libpython2.7.${DYLIB} \
    REM -DPYTHON_INCLUDE_DIR=${PREFIX}/include/python2.7 \
    REM -DPYTHON_INCLUDE_DIR2=${PREFIX}/include/python2.7 \
    REM -DWITH_LOG=OFF

nmake all
if errorlevel 1 exit 1

nmake install
if errorlevel 1 exit 1
