# profi++
A lightwight C++ API based on p-net to allow devices to communicate via profinet.

## Dependencies on/Relationship with other projects
This software is based/uses the following projects:
- osal: All files in the osal/ subdirectory. The project osal is an OS abstraction layer. It is licenced under the BSD 3-Clause License, see osal/LICENSE and osal/README.md. The original repository can be found under https://github.com/rtlabs-com/osal
- p-net: All files in the pnet/ subdirectory. The project p-net provides the profinet stack for which the profi++ project mainly provides a convenient C++ API. The project p-net depends itself on the project osal. It is licenced under a dual licence, see pnet/LICENSE.md and pnet/README.md. Of the two licences, the current project uses the GPL version 3. The original repository can be found at https://github.com/rtlabs-com/p-net . The p-net source code files which can be found in this repository were slightly reorganized and/or modified as compared to the original repository to allow for a smooth integration. These reorganizations/modifications included, besides others: placing all files necessary for the C++ API into the include directory, deleting several parts not needed for the current project, and modifying the cmake files. None of these changes are considered to be likely to impact the logic/functioning of the p-net profinet stack in any way, but no guarantees and/or warranties are given. 
- Besides the files under pnet/, some parts of the source code of the profi++ C++ API (i.e. files under profipp/) are also based on different code sniplets of the original pnet project. This mainly stems from the fact that the examples in the original repository of how to use p-net were used as a starting point for the development of the C++ API. This is specifically true for the files profipp/src/ProfinetInternal.h and profipp/src/ProfinetInternal.cpp , as they establish the interface between the profi++ C++ API and the p-net profinet stack.
- pugixml: The GSDML export of profi++ uses pugixml for XML export. The pugixml source is located at profipp/src/pugixml/ . pugixml is licensed under the MIT License. The original project can be found at https://pugixml.or . No changes were done to the pugixml source code.

## Licence
profi++ itself is licenced under the GPL version 3. See LICENSE for details. 

## Installation (short version)
Run:
```
./build_and_install_release.sh 
```

## Installation (long version)
Needed: 
- Raspberry Pi 3 Model B+
- or Raspberry Pi 4
- or Banana Pi M2 Zero or higher
- or similar
Steps:
- Install fresh Raspberry Pi OS (32bit, full)
- Connect Raspberry Pi to screen, mouse and keyboard (ssh over ethernet can make problems, since profinet can change IP settings)
- Enable wifi. If you want to connect to eduroam, see https://www.elektronik-kompendium.de/sites/raspberry-pi/2205191.htm . It might be necessary to run the following after rebooting the Raspberry Pi:
	```
	sudo rm /var/run/wpa_supplicant/wlan0
	sudo killall wpa_supplicant
	sudo wpa_supplicant -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
	```
- Enable SSH: Start->Preferences->Raspberry Pi Configuration ->Interfaces -> Check "SSH" and "Serial Console"
- Disable DHCP client daemon to adjust network interface settings (interferes with Profinet control of the interfaces): Enter `sudo nano /etc/dhcpcd.conf`, then add line `denyinterfaces eth*`. Then, reboot (`sudo reboot`)
- Install cmake (via snap, since version from apt too old as of 2023-04-20):
  ```
  sudo apt update
  sudo apt install snapd
  sudo reboot
  sudo snap install core
  sudo snap install cmake --classic
  cmake --version
  ```
  The last command is for verifying that the correct version is installed. Now, you should test if you can also run cmake under sudo rights:
  ```
  sudo cmake --version
  ```
  If sudo cannot find cmake, the following might help:
  ```
  sudo ln -s /snap/bin/cmake /usr/local/bin/cmake
  ```
- Install git:
  ```
  sudo apt install git
  ```
- Clone repository. First, to not be asked everytime you use git to provide your password, type
  ```
  git config --global credential.helper store
  ```
  Now, you will be only asked for your password the first time. Ask the repo owner for the password, or, better, the "personal access token" (github is not supporting normal password authetification anymore, you instead type in your access token instead of the password). If you are the repo owner: On the repository webpage, click on the top right on your user icon -> settings -> left, at the bottom on "Developper Settings" -> Personal access token. Create a new one. Activate checkbox(es) "repo".
  Now, you can install the repo:
  ```
  cd ~
  git clone --recurse-submodules https://github.com/langmo/profipp.git
  ```
- Run:
  ```
  ./build_and_install_release.sh 
  ```
- Now, profi++ is completely installed. To learn on how to use it from another project, see examples/profiecho/
## Programming via VS Code from Windows
- To program on the Raspberry Pi from a Windows laptop over a directly connected Ethernet cable:
  - On the Raspberry:
    ```
	sudo ip address flush label eth0
    sudo ip address add 10.0.0.8/255.0.0.0 dev eth0
	sudo ip link set dev eth0 up
    ```
  - On the laptop:
    Start -> Settings -> Network and Internet -> Ethernet -> Modify Adapter Options -> Double click on respective ethernet card -> Settings -> Mark "Internet protocol version 4 (TCP/IP)" -> Settings -> Check "Use following IP address". Set IP-Address=10.0.0.6 and subnet mask 255.0.0.0. Leave standard gateway and DNS server free. Confirm.
  - Install Visual Studio Code on the laptop:
    Click left on the "extensions" (icon looks like 4 boxes). Install "Remote - SSH" extension. Click on new icon "Remote Explorer" (looks like screen). Press on "Add new" (looks like plus sign). Enter `ssh [username]@10.0.0.8` and confirm. Save whereever. Then double click on "Connect to host in new window" (icon right next to new connection with a plus sign). You might be asked for a password to connect, so make sure the console is visible (otherwise nothing will happen).
  - The connection takes a while to establish. When connected, install additional packages on the remote machine. In VS Code on the laptop, in the new window (remote access), add "C++ Extension Pack", "CMake" and "CMake Tools (Extension)" and "C++ class creator"
  - Then, in VS Code, press File -> Open Folder. Now, select the Rasperry folder where you installed the repo. You might be asked for the password again, so make sure the terminal is showing. You are asked "Would you like to configure project "proficontrol"->Press "Yes".
  
## Debugging Memory
For debugging, we use valgrind. Do not(!) use
```
 sudo apt install valgrind
```
As of the time writing (2023-02-20) this would install a pretty outdated version of valgrind (3.7.0) which will throw hundreds of false positive results; so many, that the output is practically unusable.

Instead: 
- go to https://valgrind.org/downloads/current.html#current , and download latest release
- unpack
- go to folder, and run 
```
sudo ./configure
sudo make
sudo make install
```
If everything works, 
```
valgrind --version
```
should result in valgrind-3.20.0 or newer.

Now, build the debug from VS Code, go to debug and run valgrind:
```
cd ~/profipp/debug/examples/profiecho/
sudo valgrind --leak-check=yes ./profiecho -s
```

## Creating images
You should regularly create an image of your SD card in case the hardware fails or (more likely) you accidentially modify OS or profi++ settings in a not (easily) revertible way.

On windows, you can e.g. use WIN32 DISK IMAGER ( https://win32diskimager.org/ ). It can both create images of an SD card ("read") or write an existing image to a SD card ("write").

In case you switch SD cards, it might happen that the new SD card is too small for the existing image. In this case, you can try PiShrink ( https://github.com/Drewsif/PiShrink ) to reduce the image size (the shrunken image is automatically extended to the maximal size the new SD card allows for upon first booting of the image).

## Banana Pi M2 Zero
The Raspbian image of the banana pi is pretty old. In case you need a new gcc compiler (for C++17), see
https://solarianprogrammer.com/2017/12/08/raspberry-pi-raspbian-install-gcc-compile-cpp-17-programs/
Important: The description is for gcc10.1, but you should do everything for gcc9.1 which is in the same repo. The banana pi uses Raspbian stretch, only the gcc9.1 version works there
