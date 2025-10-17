path=$(cd "$(dirname "$0")" && pwd)

XCODE_LOG_FILTERS="^    export |clang -x |libtool -static |^    cd $CWD"
FULL_LOG_FILE="mac-build-xcodebuild.log"
if [ "$WORKSPACE" != "" ]
then
    FULL_LOG_FILE="$WORKSPACE/$FULL_LOG_FILE"
fi

echo "Building CoronaCards For Mac"

xcodebuild \
  SYMROOT="$path/build" \
  -project "${path}/ratatouille.xcodeproj" \
  -target CoronaCards \
  -configuration Release \
  ARCHS="arm64 x86_64" \
  ONLY_ACTIVE_ARCH=NO \
  2>&1 | tee -a "$FULL_LOG_FILE" | egrep -v "$XCODE_LOG_FILTERS"

