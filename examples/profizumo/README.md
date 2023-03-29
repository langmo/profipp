# profizumo
A profinet interface for the Pololu Zumo robot.

## Licence
profizumo itself is licenced under the GPL version 3. See LICENSE for details. 

## Installation on Raspberry Pi
Needed: 
- Raspberry Pi 3 Model B+ or higher
- Pololu Zumo 32U4. The Arduino version should also work, but the 32U4 is assumed in the following.
Steps:
- Install profi++
- Install the Arduino IDE on the Raspberry: https://www.arduino.cc
- In the Arduino IDE, install the board driver of the 32U4:
  - Open File->Preferences. Enter https://files.pololu.com/arduino/package_pololu_index.json in the "Additional Board Manager URLs" text box. Press OK.
  - In Tools->Boards->Boards Manager, search and install "Pololu A-Star Boards"
  - In Tools->Board, select "Pololu A-Star 32U4"
- In the Arduino IDE, install the library of the 32U4:
  - Select Sketch->Include library->Manage Libraries
  - Search and "Zumo32U4"
  - If you are asked to install missing dependencies: install them all!
- In the Arduino IDE, upload the Arduino program (~/profipp/examples/profizumo/resources/profizumo/
- Configure to connect to iWLAN on startup:
  - In the terminal, type ``sudo nano /etc/network/interfaces``. Add/modify the following lines:
    ``` 
	allow-hotplug wlan0
	iface wlan0 inet static
	address 192.168.0.40 # or whatever IP the Raspberry should have
	netmask 255.255.255.0 # Or whatever subnet mask it should have
	gateway 192.168.1.1 # or whatever is your default gateway
	wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf 
	```
  - In the terminal, type ``sudo nano /etc/wpa_supplicant/wpa_supplicant.conf``. Add/modify the following lines:
    ```
	network={
         ssid="F5.29_wlan" # your wifi's name
         psk="password123" # your wifi's password
         key_mgmt=WPA-PSK
    }
	```
- Compile profizumo
- Generate the GSDML file:
  - Run ``./profizumo -e ./``
  - Copy the GSDML (e.g. via a USB stick) to the computer of the PLC.
  - Start TIA Portal, install the GSDML and add it to the installed devices.
- Configure to autostart profizumo on startup
  - Type ``sudo nano /lib/systemd/system/profizumo.service`` into the console.
  - Add the following content to the file:
    ```
	[Unit]
	Description=profizumo
	After=network-online.target
	
	[Service]
	Type=idle
	# Wait 10s upon reboot to ensure wifi is up and running
	ExecStartPre=/bin/sleep 10
	# replace path to whereever you installed profizumo
	ExecStart=/usr/bin/sudo /home/username/profipp/scripts/start_profizumo.sh
	
	[Install]
	WantedBy=network-online.target
	```
  - Change access permissions: ``sudo chmod 644 /lib/systemd/system/profizumo.service`` and ``sudo chmod 644 //home/username/profipp/scripts/start_profizumo.sh``
  - Tell systemd to run this service:
    ```
    sudo systemctl daemon-reload
    sudo systemctl enable profizumo
	```
  - Test the service: ``sudo systemctl start profizumo``. Then, wait more than 10s and check the status: ``sudo systemctl status profizumo``. 
  - If everything works fine: reboot (``sudo reboot``), wait a few seconds and test status again: ``sudo systemctl status profizumo``
  