if [[ $(dpkg-query -s python3-venv 2>&1) == *'is not installed'* ]]; then
    printf "\nPackage python3-venv is missing.\nOn Debian-like distros, run:\n\napt install python3-venv\n\n"
    return
fi

if [[ $(cat /etc/udev/rules.d/20-hw1.rules) == *'ATTRS{idVendor}=="2c97", ATTRS{idProduct}=="0004"'* ]]; then
    printf "\nMissing udev rules. Please refer to https://support.ledger.com/hc/en-us/articles/115005165269-Fix-connection-issues\n\n"
    return
fi


if [ ! -d dev-env ]; then
    mkdir dev-env
    mkdir dev-env/SDK
    mkdir dev-env/CC
    mkdir dev-env/CC/others
    mkdir dev-env/CC/nanox

    wget https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q1-update/+download/gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2
    tar xf gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2
    rm gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2
    cp -r gcc-arm-none-eabi-5_3-2016q1 dev-env/CC/nanox/gcc-arm-none-eabi-5_3-2016q1
    mv gcc-arm-none-eabi-5_3-2016q1 dev-env/CC/others/gcc-arm-none-eabi-5_3-2016q1

    wget http://releases.llvm.org/4.0.0/clang+llvm-4.0.0-x86_64-linux-gnu-ubuntu-16.10.tar.xz -O clang+llvm.tar.xz
    tar xf clang+llvm.tar.xz
    rm clang+llvm.tar.xz
    mv clang+llvm* dev-env/CC/others/clang-arm-fropi

    wget http://releases.llvm.org/7.0.0/clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz -O clang+llvm.tar.xz
    tar xf clang+llvm.tar.xz
    rm clang+llvm.tar.xz
    mv clang+llvm* dev-env/CC/nanox/clang-arm-fropi

    wget https://github.com/LedgerHQ/blue-secure-sdk/archive/blue-r21.1.tar.gz -O blue-secure-sdk.tar.gz
    tar xf blue-secure-sdk.tar.gz
    rm blue-secure-sdk.tar.gz
    mv blue-secure-sdk* dev-env/SDK/blue-secure-sdk

    wget https://github.com/LedgerHQ/nanos-secure-sdk/archive/nanos-160.tar.gz -O nanos-secure-sdk.tar.gz
    tar xf nanos-secure-sdk.tar.gz
    rm nanos-secure-sdk.tar.gz
    mv nanos-secure-sdk* dev-env/SDK/nanos-secure-sdk

    python3 -m venv dev-env/ledger_py3
    source dev-env/ledger_py3/bin/activate
    pip install wheel
    pip install ledgerblue
    sudo bash ./udev.sh
fi


source dev-env/ledger_py3/bin/activate

export BOLOS_SDK=$(pwd)/dev-env/SDK/nanos-secure-sdk
export BOLOS_ENV=$(pwd)/dev-env/CC/others