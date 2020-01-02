NVidia fan control service
==

## Installation

These dependencies must be present before building:
 - `meson`
 - `valac`
 - `libx11-dev`
 - `libxext-dev`

Use the following command to install the dependencies on elementary OS:
```shell
sudo apt install meson valac libx11-dev libxext-dev
```

## Building

```
git clone https://github.com/msmaldi/nvidia-fan-service.git
cd nvidia-fan-service
meson build --prefix=/usr
cd build
ninja
```

To install, use ninja install, then execute with nvidia-fan-service

```
sudo ninja install
nvidia-fan-service
```

## License
This project is licensed under the GPL-3.0 License - see the [COPYING](COPYING) file for details.
