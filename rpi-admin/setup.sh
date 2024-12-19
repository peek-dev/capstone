#
# Copyright (C) <year>  <name of author>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>
#

# Must be run as root
apt -y install vim git

# Add unique chessboard group, modify existing user(s) to have access, 
# then create data directory with access for root and chessboard group.
addgroup chessboard
usermod -a -G chessboard paul
mkdir -p -m 0775 /opt/chessboard && chown root:chessboard /opt/chessboard


mkdir -m 0755 /home/paul/sf-env && chown paul:paul /home/paul/sf-env
python3 -m venv /home/paul/sf-env
echo -e '\nsource /home/paul/sf-env/bin/activate' >> /home/paul/.bashrc
