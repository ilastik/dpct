export WITH_TESTS=ON
export MULTI_STAGE_BUILD=ON

source build.sh

make test
