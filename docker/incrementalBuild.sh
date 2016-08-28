#!/bin/bash

# incrementalBuild.sh - Script to build opendlv.core.
# Copyright (C) 2016 Christian Berger
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

BUILD_AS=$1

# Adding user for building.
groupadd $BUILD_AS
useradd $BUILD_AS -g $BUILD_AS

cat <<EOF > /opt/opendlv.core.build/build.sh
#!/bin/bash
cd /opt/opendlv.core.build

echo "[opendlv.core Docker builder] Incremental build."

mkdir -p build.system && cd build.system
PATH=/opt/od4/bin:$PATH cmake -D CXXTEST_INCLUDE_DIR=/opt/opendlv.core.sources/thirdparty/cxxtest -D OPENDAVINCI_DIR=/opt/od4 -D CMAKE_INSTALL_PREFIX=/opt/opendlv.core /opt/opendlv.core.sources/code/core/system

make -j4 && make test && make install
EOF

chmod 755 /opt/opendlv.core.build/build.sh
chown $BUILD_AS:$BUILD_AS /opt/opendlv.core.build/build.sh

su -m `getent passwd 1000|cut -f1 -d":"` -c /opt/opendlv.core.build/build.sh

