#/bin/sh

ARCH=$1
wget http://ftp.debian.org/debian/pool/main/p/pugixml/libpugixml1v5_1.9-3_$ARCH.deb 
wget http://ftp.debian.org/debian/pool/main/p/pugixml/libpugixml-dev_1.9-3_$ARCH.deb
dpkg -i libpugixml1v5_1.9-3_$ARCH.deb libpugixml-dev_1.9-3_$ARCH.deb
