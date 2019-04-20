# T-Stick Conversion Box - Building recipe - Rpi 2B and 3B+

Under Raspbian Lite

## Prepare the OS

- Flash the latest Prynth image (https://prynth.github.io/create/downloads.html)
- Add ssh file to Boot partition: `touch ssh`
- Add wpa_supplicant.conf to file to Boot partition: `vim wpa_supplicant.conf`. File contents:

```bash
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=US

network={
     ssid="Your network name/SSID"
     psk="Your WPA/WPA2 security key"
     key_mgmt=WPA-PSK
}
```

- Open Prynth software settings at `http://raspberrypi.local:3000/system` and change hostname to match the converted T-Stick  
- SSH, change password, hostname, set country, expand file system (raspi-config)

## Update

```bash
sudo apt-get -y update
sudo apt-get -y dist-upgrade
```

## Install liblo

```bash
sudo apt-get remove -y liblo-dev
mkdir ~/sources
cd ~/sources
git clone https://github.com/radarsat1/liblo.git
cd liblo
sudo apt-get install -y vim autoconf libtool automake
./autogen.sh
make
sudo make install
```

## Install Libmapper

```bash
sudo apt-get install python-dev swig
cd ~/sources
git clone https://github.com/libmapper/libmapper.git
cd libmapper
./autogen.sh
echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib' >> ~/.bashrc
make
sudo make install
```

## Install Webmapper

```bash
pip install netifaces
cd ~/sources
git clone https://github.com/libmapper/webmapper.git
cp ~/sources/libmapper/swig/_mapper.so ~/sources/webmapper
cp ~/sources/libmapper/swig/mapper.py ~/sources/webmapper
cd webmapper/
```

- To run Webmapper: ./webmapper.py


## Create python application to read serial port and send OSC messages

- Install packages:

```bash
cd ~
sudo apt-get install -y python3-pip python-pip setserial
sudo pip3 install python-osc pyserial apscheduler bitstring netifaces
pip install pyserial apscheduler bitstring pyOSC netifaces oscpy
mkdir tstick
```

Old, unnecessary packages are removed after the system update with the following commands:

```bash
sudo apt-get autoremove
sudo apt-get autoclean
```
