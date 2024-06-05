WORKDIR=$(pwd)
echo $WORKDIR
cd $WORKDIR/submodule/DTP
git apply $WORKDIR/submodule_patches/0001-DTP-build-add-stable-Cargo.lock.patch &

cd $WORKDIR/submodule/quiche
git apply $WORKDIR/submodule_patches/0001-quiche-build-add-stable-Cargo.lock.patch &