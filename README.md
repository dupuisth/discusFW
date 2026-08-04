# discusFW

## Windows

### Attach the USB device to WSL

```powershell
usbipd list
```
Find your ESP device, then:

```powershell
usbipd bind --busid <BUSID>
usbipd attach --wsl --busid <BUSID>
```

### Configure your WSL .bashrc

Then, open WSL and identify the serial device

```sh
lsusb
```

or 

```
ls -l /dev/ttyUSB* /dev/ttyACM*
```

Once the serial device found, update your ~/.bashrc to add the following:
```sh
export ESP_PORT=/dev/<YOUR_DEVICE>
```

Reload it:
```sh
source ~/.bashrc
```

### Open the project inside WSL

```sh
cd discusFW
code .
```