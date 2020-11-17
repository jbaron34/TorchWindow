# TorchWindow

TorchWindow is a Python library written mostly in C that enables viewing of PyTorch Cuda Tensors on screen directly from GPU memory (No copying back and forth between GPU and CPU) via Vulkan-Cuda interop.

## Build requirements
Currently TorchWindow is only targeting 64-bit platforms. 32-bit may be added in future.

### Windows
Visual Studio  
CUDA SDK  
Vulkan SDK  
CMake

### Linux
Linux is currently not supported but will be in the next release.

### MacOS
MacOS is not supported but may be at some point in the future.

## Install from source

In the project root directory:
```
mkdir build
cd build
cmake ..
cmake --build .
cd ..
pip install .
```

## Install from binaries

```
pip install torchwindow
```

## Use
To create a window:
```
from torchwindow import Window
window = Window(640, 480, name="Torch Window")
```
To display an image tensor in the window
```
window.draw(image_tensor)
```
Note that the image tensor must have 3 dimensions, specifically (width, height, channels) in that order.
This layout restriction will be relaxed in a future release.