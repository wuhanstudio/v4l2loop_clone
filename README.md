# v4l2loop_clone

This program clones the video stream from one v4l2 device to multiple v4l2loopback devices. (Virtual Camera)

For example:

```
$ sudo modprobe v4l2loopback video_nr="40,41"
$ v4l2-ctl --list-devices
Dummy video device (0x0000) (platform:v4l2loopback-000):
	/dev/video40

Dummy video device (0x0001) (platform:v4l2loopback-001):
	/dev/video41

HP HD Camera: HP HD Camera (usb-0000:00:14.0-9):
	/dev/video0
	/dev/video1
```

We can clone the video from HD Camera (/dev/video0) to two virtual cameras (/dev/video40 and /dev/video41):

```
$ make
$ ./v4l2loop_clone /dev/video0 /dev/video40 /dev/video41
```

Related Projects:

- https://bitbucket.org/OscarAcena/ocv-virtual-cam/
- https://github.com/wuhanstudio/capturev4l2
