# IntrusionGame

There are two versions of the simulator available, a slow python version that only requires python >= 3.8 and the libraries installed and the C++ version which is only supported in Linux. The C++, as expected, is about 50 times faster than the python version (with the `--hidden` option set).

---
## Python version
Just run the main files as normal.  

**Note:** After the path finding and wall simulation, this version is very outdated, using the C++ version is highly recommended.
```
python main.py
```
---
## C++ version
To run the C++ version, make sure the libraries are installed:

```
sudo apt-get install cmake libsdl2-2.0-0 libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdlnlohmann-json-dev
```
Pull the necessary git submodules:
```
git submodule update
```
Build:
```
mkdir build && cd build
cmake ..
make
```
The `intrusion_game` executable is then created.

---
## Simulator options
The simulator can be used with a JSON configuration file like the one below, all options need to be present, there are no "default" values.  
  
Some of the parameters can be overridden with the command line. These options are available in both versions and can be accessed with the `--help` option.

## "Automatic" testing
To make running multiple simulations with different parameters, the `run.sh` file is provided. This simple bash script gives an example of how multiple simulations with different parameters can be run.  

Currently only one parameter of the config file can be updated, if more changes are needed multiple config files can be used.
