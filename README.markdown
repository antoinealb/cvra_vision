# OpenCV based computer vision for Eurobot2014

## Requirements

* OpenCV

## Compilation

```sh
mkdir build
cd build
cmake .. -DOn_Robot=On
make
cd ..
```

# Notes

* Please use the name 'build' for the folder where you invoke cmake
  since the python wrapper has the destination for the generated
  dynamic library hard-coded.
* If you want to compile the version with debug output on your computer
  you can leave out the '-DOn_Robot=On' part of the cmake command, since
  it is off by default.
* Cmake caches some arguments, if you change the command line option but see
  no change in behaviour clean the whole build directory and recompile.
