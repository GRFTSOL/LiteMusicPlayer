
PYTHON="C:\Program Files (x86)\Microsoft Visual Studio\Shared\Python39_64"

%PYTHON% TinyJS/build-script/build.py

mkdir build
cd build && cmake -G "Visual Studio 17 2022" .. || cd.. && exit /b 1
cd ..

cd LocalServer\www
REM cmd /C "node_modules\.bin\quasar build release"
cd .. && cd ..

set path=%path%;"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE"

%PYTHON% build.py
