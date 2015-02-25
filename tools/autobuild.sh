
# check arguments
if [[ ( "$1" != "nightly") && "$1" != "build" ]]; then
	echo -e "Usage:\n\t$0 nightly -> preserve build results"
	echo -e "or\n\t$0 build   -> delete build results\n"
    exit 1
fi

BUILD_TYPE=$1
BUILD_BRANCH=${BUILD_TYPE}-$(date +%Y%m%d%H%M%S)
PULL_BRANCH=Master
BINDIR=exe

# prepare a fresh branch for build
cd WyLight && \
git checkout ${PULL_BRANCH} && \
git pull && \
git checkout -b ${BUILD_BRANCH} && \
./configure
if [ $? -ne 0 ]; then
	echo "prepare repository and branch for build failed"
	exit 1
fi

# run tests
# TODO replace this with 'make all' when supported on raspi
make firmware_test && \
make cli
#make firmware_pic

if [ $? -ne 0 ]; then
	echo "build failed!"
	exit $?
fi

# save nightly build results only
if [ "${BUILD_TYPE}" == "nightly" ]; then
    git add -f ${BINDIR}/*.bin && \
    git commit -m "${BUILD_BRANCH} was successful"
else
    git checkout .
    git checkout ${PULL_BRANCH}
    git branch -D ${BUILD_BRANCH}
fi
