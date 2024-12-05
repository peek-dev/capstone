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
