#!/usr/bin/env bash

if [ "$#" -lt "1" ]; then
    echo "Incorrect invocation: Should be change_submodule_branch.sh branch git-url"
    echo "where:"
    echo "  branch is the new branch required"
    echo "  (optional) git-url is the url of the submodule git repository required"
    echo "  examples:"
    echo "  change_submodule_branch main"
    echo "  change_submodule_branch main https://github.com/OpenCPN/opencpn-libs.git"
    exit
fi
BRANCH=$1
REPO=""
if [ "$#" -eq "2" ]; then
    echo "Using $2 instead of https://github.com/OpenCPN/opencpn-libs"
    REPO=$2
else
    echo "Using devault repository: https://github.com/OpenCPN/opencpn-libs"
   REPO="https://github.com/OpenCPN/opencpn-libs"
fi

git submodule deinit -f opencpn-libs
git rm --cached opencpn-libs
rm -rf .git/modules/opencpn-libs  #(on Windows, simply delete the target directory ".git/modules/opencpn-libs" )
rm -rf opencpn-libs  #(on Windows, simply delete the target directory "opencpn-libs" )
git config -f .gitmodules --remove-section submodule.opencpn-libs
git add .gitmodules
git commit -m "Remove opencpn-libs submodule."

git submodule add -b $BRANCH $REPO
git commit -m "Adding $REPO submodule branch $BRANCH"

echo "This will now need to be pushed to make it available to circleci builds in the cloud"
