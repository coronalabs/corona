# How to Build Corona's Android Software


## Building apps and templates

#### How to build an app:
- Simplest way to build an app based on local source is `./mkapk app-source-directory`

- For more options use `./build_app.sh` script. For example, from platform/android folder run
./build_app.sh /Applications/CoronaSDK/SampleCode/Animation/HorseAnimation/
This would produce /Applications/CoronaSDK/SampleCode/Animation/HorseAnimation/HorseAnimation.apk which you can install on your android device for testing.

- See usage inside script for more details.

#### How to build "Corona Android app template":
- Simplest way to build Android template is `./mks3upload`

- For more options building the template.apk that is used for Android apps built through the Corona Simulator, use the build.template.sh script.

- See usage inside script for more details.

#### How to build template for CoronaCards:
- To build the package distributed for CoronaCards on Android, use the build_template_cards.sh script

- See usage inside script for more details.

#### How to build Corona Enterprise/Cards Android Documentation:
- To examine just the Java documentation provided for Corona Enterprise and CoronaCards on Android, use the build_enterprise_docs.sh and build_cards_docs.sh scripts respectively.




## Development Environment Setup

This differs from the current Corona Native setup as that uses Android Studio while build the SDK uses Ant.

### Short Guide:
This is short guide on how to get started on macOS

1. Install Java (1.8 is good)
2. use brew to install Ant `brew install ant`.
3. Download archive with Android SDK and NDK [here](https://www.dropbox.com/s/a0jkh5mivzkm7xx/Android.zip?dl=0)
and unpack it to the home directory. You should have `~/Android/android-ndk-r10d/` and
`~/Android/android-sdk-mac_86/` folders after this
4. In your `~/.bash_profile` or other shell config (I got zsh, so it's `~/.zshrc`) add this lines:

        export ANDROID_SDK=~/Android/android-sdk-mac_86
        export ANDROID_NDK=~/Android/android-ndk-r10d
        export PATH=${PATH}:$ANDROID_SDK/tools:$ANDROID_SDK/platform-tools

    Last line is optional if you want to use command line tools to install apps to the device and view logs (and don't have it already set up)
5. Restart your terminal, go to folder where this file is located and build apps using `./mkapk <path-to-your-corona-app>`


------------------------------

_TODO:_ Update rest of the guide!


### Installing Java

Android developement is done using the Java programming language.

* Download the current version of Java from: http://www.oracle.com/technetwork/java/javase/downloads/index.html

Follow the link, click the download button and follow the install instructions to have Java properly set up.

### Installing the Android Tools

If you’ve already been building Android apps for Corona SDK, you may have done some of this before, but it won’t hurt to repeat the process and get the latest updates from Google. Essentially, you need to install Google’s Android SDK and its tools. You can get these [here](https://developer.android.com/sdk/index.html#Other). After opening the link, click on the **Android SDK package for Mac OS X** link. On the next page, agree to the terms and click the download button. This will download a file with a name like `android-sdk_rXX.X-macosx.zip` where `XX.X` represents the version of the Android SDK.

If your system does not automatically unzip this for you, double-click the file to unpack the archive. Rename this folder to just `sdk`. Move this folder from your Downloads directory to the directory: `~/Library/Android`. Since the Library directory is hidden, and you cannot easily access it from the Finder. So, from the Terminal, type in this command:

	mv ~/Downloads/sdk ~/Library/Android/sdk

* **NOTE:** While you could move the Android SDK to anywhere you want on your machine, we put it in this directory specifically to be compatible IDEs commonly used with native Android development, such as [Android Studio](http://developer.android.com/tools/studio/index.html).

This folder contains some files that you’ll want to access from the command line without typing the entire path each time. To facilitate this, you can tell your computer where to find them by creating a file in your home directory called `.bash_profile`. Like the Library folder, the `.bash_profile` file is hidden, so we can interact with it through the Terminal application again. In the Terminal, type in these commands to tell you computer where the Android SDK is:

	echo "export ANDROID_SDK=${HOME}/Library/Android/sdk" >> ~/.bash_profile
	echo "export PATH=${PATH}:${ANDROID_SDK}/tools:${ANDROID_SDK}/tools" >> ~/.bash_profile

The first of these commands make a new environement variable called `ANDROID_SDK` and assign it to the path to the Android SDK on your computer. Corona Enterprise's build scripts rely on the existence of an `ANDROID_SDK` environment variable. The second of which appends your new environment variable to the `PATH` environment variable. This is the step that add the location of the Android SDK to what paths your computer is aware of.

Press `cmd + Q` to quit the Terminal application and then reopen it. This will allow the changes you made to `.bash_profile` to take effect and your computer will now know where the Android tools are.

Run the `android` command in the Terminal. This should load the Android SDK Manager which looks similar to this:

![](https://coronalabs.com/wp-content/uploads/2014/03/android_sdk_manager.png "Android SDK Manager")

From here, you’ll want to install several things including:

* Android SDK Tools
* Android SDK Build Tools (latest version)
* Several of the Android n.n.n (API X) entries

Which Android SDK entries you need to install varies based on what you’re building. You should install Android 7.1.1 (API 25) since this is the current version that Corona SDK targets. You should also install the minimum SDK version that you want to support — for Corona SDK, this is Android 4.4 (API 19). Finally, you need to install the target and minimum SDK versions for any library that you plan to use. If you can't find all the SDK versions you need, checking the `Obsolete` checkbox will reveal all SDK versions in the SDK Manager. While it may seem easier to just install everything, these SDKs take up disk space and they also take time to download/unpack. If you download them all, expect a long wait.

Once you have the necessary items selected, click on the Install [n] packages… button. On the next screen, accept the license agreement(s) and sit back while everything installs. When the process is finished, close the Android SDK Manager and return to the Terminal command line.

With all our desired API Levels installed, now we need to tell our computer where to find the build tools for the target SDK version (API 25). We do this to access the `zipalign` tool which we'll need to use when uploading to the [Google Play Developer Console](https://play.google.com/apps/publish/) as well as other Android debugging utilities. In the Terminal, type this command:

	echo "export PATH=${PATH}:${ANDROID_SDK}/build-tools/25.0.2" >> ~/.bash_profile

### Installing Ant

For building Android projects, Corona Enterprise uses Apache Ant to compile the code and gather all necessary resources. Installing Ant on OS X is best done through Homebrew.

If you haven't installed Homebrew, visit http://brew.sh and copy the installation command under "Install Homebrew" into your Terminal window. Run the command and Homebrew should install.

With Homebrew installed, type the following into the Terminal to install Ant:

	brew update
	brew install ant

Once this has finished, you should now have Ant installed at `/usr/local/Cellar/ant/X.X.X` where `X.X.X` is again the version number of Ant. To run Corona Enterprise's build scripts properly, you'll need to tell your computer where to find the Ant installation. In a similar fashion to the Android tools, type in these commands, replacing the X's with your version number for Ant:

	echo "export ANT_HOME=/usr/local/Cellar/ant/X.X.X" >> ~/.bash_profile
	echo "export PATH=${PATH}:${ANT_HOME}/bin" >> ~/.bash_profile

* **NOTE:** At this point, your `.bash_profile` may be a little messy. Feel free to open it in your favorite text editor and clean it up however you wish. Just be sure that you don't change the name of the `ANDROID_SDK` environment variable as Corona Enterprise's build scripts assume that environment variable exists.

### Installing the Android NDK

- Install Android NDK version r10: https://developer.android.com/ndk/downloads/older_releases.html#ndk-10e-downloads

- Add environment variables to your Android SDK and NDK installations with names (ANDROID_SDK and ANDROID_NDK) respectively.

