# phalanx

My first Vulkan renderer, based on the [Vulkan tutorial](https://vulkan-tutorial.com/).  I've tacked on some additional features for fun:
* dynamic shader recompilation when the R key is pressed (Windows only)

## Setup
### macOS
1. `brew install glfw`
1. `brew install glm`
1. Install the Vulkan SDK
1. `make run`

### Windows
Follow the directions at the [Vulkan tutorial environment setup for Windows](https://vulkan-tutorial.com/Development_environment#page_Windows).  This repo includes my personal VS 2019 project setup, but it may not work on your local machine, so it's probably best to just make a new one.

## Namesake
I named this after the Phalanx CIWS, for no particular reason.

## Topics for further investigation
* [texels](https://en.wikipedia.org/wiki/Texel_(graphics))
* [best practice validation](https://vulkan.lunarg.com/doc/view/1.1.126.0/windows/best_practices.html)
* what even is a descriptor set?  or even a descriptor?
* what is the color attachment?  it's the swap chain image?
