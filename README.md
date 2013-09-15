dxlibp-kai
==========


### DX Library Portable for PSP


This library is based on

* [dxlibp](https://github.com/yreeen/dxlibp) -- by [yreeen](https://github.com/yreeen) <br>
* DX Library Portable Kai Ver 3.5 Fixed -- by Dadrfy


For more details of based libraries or Japanese README, please see the doc directory.


### Differences from based libraries

* OGG audio format support (if DXP_BUILDOPTION_USE_LIBOGG is defined)
* malloc/free and FileRead_* functions are thread-safe
* Grayscale Jpeg support
* DX_SOUNDDATATYPE_MEMPRESS support
* Mutex support
* Support for opening more than 9 files

To use this library,

* libvorbisidec and libogg need to be linked if DXP_BUILDOPTION_USE_LIBOGG is defined

```
LIBS += dxlibp.a -lvorbisidec -logg -ljpeg -lpng -lpspgum -lpspgu -lz -lm -lpsprtc -lpspaudio -lpspaudiocodec -lpsputility -lpspvalloc -lpsppower
```

* Set LDFLAGS

```
LDFLAGS += -Wl,--wrap,malloc -Wl,--wrap,realloc -Wl,--wrap,calloc -Wl,--wrap,memalign -Wl,--wrap,free
```

* The following copyright needs to be added if DXP_BUILDOPTION_USE_LIBOGG is defined

```
libogg and libvorbisidec

Copyright (c) 2002, Xiph.org Foundation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

- Neither the name of the Xiph.org Foundation nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

