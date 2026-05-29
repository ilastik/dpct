set WITH_TESTS=ON
set MULTI_STAGE_BUILD=ON

call %RECIPE_DIR%\build.bat

make test
