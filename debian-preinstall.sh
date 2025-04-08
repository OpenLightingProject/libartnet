echo "Installing autoconf and libtool"
apt update && apt install -y autoconf libtool
echo "Running autoreconf"
autoreconf -fi
echo "Running configure"
./configure
echo "Compiling libartnet"
make -j18
echo "Run make install to install library on system"