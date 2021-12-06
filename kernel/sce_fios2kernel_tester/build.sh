if [ ! -d extra ]; then
  mkdir extra
  vita-libs-gen -c import.yml extra
  cd extra
  cmake .
  make
  cd ..
fi

if [ ! -d build ]; then
  mkdir build
fi

cd build
cmake ../
make
cd ..

