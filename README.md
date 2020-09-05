# SunDesktop

| Platform | Build Status |
|-|-|
| Linux | ![build](https://github.com/zhenghaoz/dynamic-desktop/workflows/build/badge.svg?branch=master) |
| Windows | ![build](https://github.com/zhenghaoz/dynamic-desktop/workflows/build/badge.svg?branch=master) |

SunDesktop is a solar-aware dynamic wallpaper engine.

## Build

The build process requires:


### Linux

```bash
# Install dependencies
sudo apt-get install qt5-default qttools5-dev libboost-all-dev

# Clone repo
git clone https://github.com/zhenghaoz/sundesktop.git

# Create build folder
cd sundesktop
mkdir build
cd build

# Build
cmake .. && make -j 4
```

### Windows

## Usage

## Acknowledgments

- This project is inspired by *[macOS Mojave 的动态桌面，可不只是「定时换壁纸」这么简单](https://sspai.com/post/47390)*
- Thanks for awesome dynamic wallpapers from *[Dynamic Wallpaper Club](https://dynamicwallpaper.club/)*
