dxlibp-kai
by mafu <mafu9mafu@gmail.com>



--�T�v
PSP�p��DX���C�u�����ł��B

���O���x�[�X���C�u�����́uDX Library Portable kai�v�Ɣ���Ă܂����A
���ɂ������O���v�����Ȃ��̂ō��̂Ƃ���C���\��͂Ȃ��ł��c�c�B

���̃��C�u�����͈ȉ��̃\�[�X�R�[�h���x�[�X�ɂ��Ă��܂��B
  * DX Library Portable -- by �J��
  * DX Library Portable Kai Ver 3.5 Fixed -- by Dadrfy
�ڂ����͂� doc �f�B���N�g������
  * DX Library Portable_readme.txt
  * DX Library Portable kai v3.5 Fixed_ReadMe.txt
��ǂ�ł��������B



--��ȕύX�_(���ɂ����قǍŋ߂̕ύX�_�ł�)
  * OGG Vorbis �`���̉����t�@�C���ɑΉ�
  * malloc/free �n�֐������b�p�[����
    ���̃��C�u�����������N����S�Ă� malloc/free ���X���b�h�Z�[�t��
  * �������̑��e�ʁA���󂫗e�ʂ��擾�\��
  * �O���C�X�P�[��Jpeg �ɑΉ�
  * DX_SOUNDDATATYPE_MEMPRESS �ɑΉ�
  * �t�@�C���ǂݍ��݌n�֐�(FileRead_***)���X���b�h�Z�[�t��
  * �~���[�e�b�N�X��ǉ�
  * FileRead_open �Ŏ����t�@�C����9�܂ł����J���Ȃ������o�O���C��
  �ڂ����� GitHub �̃R�~�b�g���O(https://github.com/mafu9/dxlibp-kai/commits/)�Ȃǂ�ǂ�ł��������B


--�g����
1, dxlibp-kai/src �f�B���N�g���� ($PSPSDK)/bin/make.exe �����s���ă��C�u�������R���p�C�����Ă��������B
   �R���p�C���ɂ� PSPSDK ���K�v�ł��B(��҂� MinPSPW �ŊJ�����Ă��܂�)
2, dxlibp.h �Ɛ������ꂽ dxlibp.a ��K�X�z�u���Ă��������A
   ($PSPSDK)/psp/include/dxlibp.h
   ($PSPSDK)/psp/lib/dxlibp.a
   ��L�̂悤�ɔz�u����̂� Makefile �ɗ]�v�Ȑݒ�����Ȃ��Ă����̂Ōl�I�ɃI�X�X���B
3. ���C�u�������g�p����Ƃ��� Makefile �� LIBS �Ɉȉ��̂悤�Ƀ��C�u������ǉ����Ă��������B
     LIBS += dxlibp.a -lvorbisidec -logg -ljpeg -lpng -lpspgum -lpspgu -lz -lm -lpsprtc -lpspaudio -lpspaudiocodec -lpsputility -lpspvalloc -lpsppower
4, Makefile �� LDFLAGS �Ɉȉ��̂悤�Ƀt���O��ǉ����Ă��������B
     LDFLAGS += -Wl,--wrap,malloc -Wl,--wrap,realloc -Wl,--wrap,calloc -Wl,--wrap,memalign -Wl,--wrap,free



--���쌠�\���K�{����
���̃��C�u�������g�p�����\�[�X�R�[�h����s�t�@�C����z�z����ꍇ�A
�ȉ��̒��쌠�\����K���܂߂Ă��������B


DX���C�u����Portable
DX Library Portable Copyright (C) 2008-2010 Kawai Yuichi.
�� �C��


zlib
Copyright (c) 1995-2004 Jean-loup Gailly and Mark Adler.
�� ������ dxlibp.h �Łu#define DXP_BUILDOPTION_USE_LIBPNG�v���R�����g�A�E�g����
   ���̃��C�u�������R���p�C�������ꍇ�͖����ő��v�ł��B


libpng
Copyright (c) 2004 Glenn Randers-Pehrson
distributed according to the same disclaimer and license as libpng-1.2.5
with the following individual added to the list of Contributing Authors
��1 ���쌠�\���̓C���X�g�[�����Ă���libpng�̃o�[�W�����ɍ��킹�ĉ������B
��2 �������ȉ��̂����ꂩ�̏ꍇ�͂Ȃ��Ă����v�ł��B
      * dxlibp.h �Łu#define DXP_BUILDOPTION_USE_LIBPNG�v���R�����g�A�E�g����
        ���̃��C�u�������R���p�C�������ꍇ
      * SaveGraph�ASaveScreen ���̉摜�ۑ��֐����g���Ă��Ȃ��ꍇ


jpeglib
this software is based in part on the work of the Independent JPEG Group.
��1 ���쌠�\���̓C���X�g�[�����Ă��� jpeglib �̃o�[�W�����ɍ��킹�ĉ������B
��2 ������ dxlibp.h �Łu#define DXP_BUILDOPTION_USE_LIBJPEG�v���R�����g�A�E�g����
    ���̃��C�u�������R���p�C�������ꍇ�͖����ő��v�ł��B


intraFont
Uses intraFont by BenHur
�� ������ DrawString ���̕�����`��֐����g���Ă��Ȃ��ꍇ�͖����ő��v�ł��B


liblzr
Uses liblzr by BenHur


�S�p�����\�����C�u����
���̃\�t�g�E�F�A�� mediumgauge ���쐬�̑S�p�����\�����C�u�������g�p���Ă��܂��B
�� ������ printfDx ���̊ȈՕ�����`��֐����g���Ă��Ȃ��ꍇ�͖����ő��v�ł��B


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

�� ������ dxlibp.h �Łu#define DXP_BUILDOPTION_USE_LIBOGG�v���R�����g�A�E�g����
   ���̃��C�u�������R���p�C�������ꍇ�͖����ő��v�ł��B



--���̑�
�o�O��/�C����^��W���ł��I
���[����GitHub�A�^�f���ȂǍ�҂��������ȂƂ���Ȃ�ǂ��ł�OK�ł��B
�� �K�������Ή�/�ԐM�ł���Ƃ͌���܂���


