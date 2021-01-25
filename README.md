# Mini Hypervisor

Mini Hypervisor is a type 1 (bare-metal lightweight hypervisor) made for learning purpouses.
It supports now only boot via PXE booting and the target machine must be installed Legacy (no UEFI support)

## Introduction

This native lightweight hypervisor was made only for learning purpouses (and was tested on a small ammount of machines), so is not recommaned to test it on yours.
In case you want to test it, just copy the *.bin file building to your PXE folder.

No need to install/bring additional dependencies. It has all ACPICA source files integrated and ready to build.

For logging it supports serial logging (most useful one) and VGA logging.

## License

```
MIT License

Copyright (c) 2018 Vieriu Denis-Gabriel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
