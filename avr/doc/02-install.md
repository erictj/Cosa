# How to install Cosa ?

## Via GIT

```shell
cd <custom_path>/sketchbook
mkdir hardware # Create the hardware folder if missing
cd hardware
git clone git@github.com:mikaelpatel/Cosa.git
```

Then you can update the library by using:

```shell
git pull origin master
```

## Via Archives

### Step 1

* Download the [Cosa zip file](https://github.com/mikaelpatel/Cosa/archive/master.zip)
* Create the hardware folder if missing (<custom_path>/sketchbook/hardware)
* Unzip the files to your hardware folder
* Rename `Cosa-master` to `Cosa`

### Step 2 (For Arduino 1.5.x only)    

* Create an outer `Cosa` folder (<custom_path>/sketchbook/hardware/Cosa).
* Unzip the files to your `hardware/Cosa` folder
* Rename inner `Cosa` folder (<custom_path>/sketchbook/hardware/Cosa/Cosa-master) to `avr` (<custom_path>/sketchbook/hardware/Cosa/avr)

### Step 3 (For Attiny users on Windows version only)

* Install [this patch](https://github.com/TCWORLD/ATTinyCore/tree/master/PCREL%20Patch%20for%20GCC)

Do not forget to program the ATtiny device with the bootloader, i.e.,
set the fuse bits, before using the device for the first time.

### Step 4

* Restart the Arduino IDE and Cosa will show up as a number of boards and example sketches.  