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
cd ~/corona
rm -rf build
mkdir build
cd build
cmake .. \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_C_FLAGS="-Wno-error=incompatible-pointer-types -Wno-error=implicit-function-declaration"
make -j$(nproc)
sudo make install
```

You would also need Raspbian binaries to build cross-compiled template for Raspbian Pi. You can download them [here](https://drive.google.com/file/d/1ZysxJdDg-XgU3-jshxUPSewTqBYeA7Qq/view?usp=sharing).


## Generate Corona.aar and android-template.zip for Android build

```
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
export ANDROID_HOME=~/Android/SDK
export PATH="$JAVA_HOME/bin:$PATH"

cd ~/corona/platform/android
# clean
./gradlew clean
rm -rf sdk/.cxx sdk/build app/build
# build and copy Corona.aar
./gradlew installAppTemplateAndAARToSim \
  -PcoronaResourcesDir=~/corona/build/Resources \
  -PcoronaNativeOutputDir=~/corona/build/Resources/Native/Corona
# build and copy android-template.zip
./gradlew installAppTemplateToSim \
  -PcoronaResourcesDir=~/corona/build/Resources \
  -PcoronaNativeOutputDir=~/corona/build/Resources/Native/Corona
```

Files will be installed in the following places:
~/corona/build/Resources/Native/Corona/android/lib/gradle/Corona.aar
~/corona/build/Resources/Native/Corona/android/resource/android-template.zip


## Run app via simulator

```
cd ~/corona/build
./Solar2DSimulator /path/to/your/app
```

Then the app can be built for Android by menu option File > Build > Android
