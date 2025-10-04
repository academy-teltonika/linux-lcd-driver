# Linux kernel LCD screen driver

# THIS PROJECT IS UNFINISHED

## Build instructions
Set your *target architecture*, *toolchain* and *kernel directory* in the provided *Makefile*.

**Run:** <br>
`make`

## Set up your device tree
Add the appropriate LCD device into your device tree.
The device must have one GPIO pin and it's *compatible string* needs to be ***lcd-device***

## Usage
Insert with `insmod lcd-driver.ko`

## TODO
- [ ] Fix LCD screen initialization problem, where the screen is not getting set to 4-bit mode correctly. 
- [ ] Provide CLI interface to set the displayed text.

## Current state (broken):
![demo](https://github.com/user-attachments/assets/3227aae0-eba9-4edc-9a63-3f6161e86784)
