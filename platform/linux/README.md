## How To Build For Native Linux

If you have not done it already you will need to clone the git repository

```
cd ~
git clone --recursive https://github.com/coronalabs/corona.git 
```

Run the provided script to install required dependencies.
```
cd ~/corona/platform/linux
sudo sh ./setup_dev.sh  
```

For those who like IDE's the source has been setup for use with CodeLite it can be installed using.
```
sudo apt-get install codelite -y
```

If you have installed CodeLite the workspace file is named "Solar2D.workspace" and is located at ~/corona/platform/linux



To build everything run. This will build linux simulator and template and install Solar2D in /usr/local/bin

```
mkdir build
cd build
cmake ..
make -j8
sudo make install
```

You would also need Raspbian binaries to build cross-compiled template for Raspbian Pi. You can download them [here](https://drive.google.com/file/d/1ZysxJdDg-XgU3-jshxUPSewTqBYeA7Qq/view?usp=sharing).

