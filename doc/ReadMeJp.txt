dxlibp-kai
by mafu <mafu9mafu@gmail.com>



--概要
PSP用のDXライブラリです。

名前がベースライブラリの「DX Library Portable kai」と被ってますが、
特にいい名前が思いつかないので今のところ修正予定はないです……。

このライブラリは以下のソースコードをベースにしています。
  * DX Library Portable -- by 憂煉
  * DX Library Portable Kai Ver 3.5 Fixed -- by Dadrfy
詳しくはは doc ディレクトリ内の
  * DX Library Portable_readme.txt
  * DX Library Portable kai v3.5 Fixed_ReadMe.txt
を読んでください。



--主な変更点(下にいくほど最近の変更点です)
  * OGG Vorbis 形式の音声ファイルに対応
  * malloc/free 系関数をラッパーして
    このライブラリをリンクする全ての malloc/free がスレッドセーフに
  * メモリの総容量、総空き容量を取得可能に
  * グレイスケールJpeg に対応
  * DX_SOUNDDATATYPE_MEMPRESS に対応
  * ファイル読み込み系関数(FileRead_***)をスレッドセーフに
  * ミューテックスを追加
  * FileRead_open で実質ファイルが9個までしか開けなかったバグを修正
  詳しくは GitHub のコミットログ(https://github.com/mafu9/dxlibp-kai/commits/)などを読んでください。


--使い方
1, dxlibp-kai/src ディレクトリで ($PSPSDK)/bin/make.exe を実行してライブラリをコンパイルしてください。
   コンパイルには PSPSDK が必要です。(作者は MinPSPW で開発しています)
2, dxlibp.h と生成された dxlibp.a を適宜配置してください、
   ($PSPSDK)/psp/include/dxlibp.h
   ($PSPSDK)/psp/lib/dxlibp.a
   上記のように配置するのが Makefile に余計な設定をしなくていいので個人的にオススメ。
3. ライブラリを使用するときは Makefile の LIBS に以下のようにライブラリを追加してください。
     LIBS += dxlibp.a -lvorbisidec -logg -ljpeg -lpng -lpspgum -lpspgu -lz -lm -lpsprtc -lpspaudio -lpspaudiocodec -lpsputility -lpspvalloc -lpsppower
4, Makefile の LDFLAGS に以下のようにフラグを追加してください。
     LDFLAGS += -Wl,--wrap,malloc -Wl,--wrap,realloc -Wl,--wrap,calloc -Wl,--wrap,memalign -Wl,--wrap,free



--著作権表示必須項目
このライブラリを使用したソースコードや実行ファイルを配布する場合、
以下の著作権表示を必ず含めてください。


DXライブラリPortable
DX Library Portable Copyright (C) 2008-2010 Kawai Yuichi.
※ 任意


zlib
Copyright (c) 1995-2004 Jean-loup Gailly and Mark Adler.
※ ただし dxlibp.h で「#define DXP_BUILDOPTION_USE_LIBPNG」をコメントアウトして
   このライブラリをコンパイルした場合は無しで大丈夫です。


libpng
Copyright (c) 2004 Glenn Randers-Pehrson
distributed according to the same disclaimer and license as libpng-1.2.5
with the following individual added to the list of Contributing Authors
※1 著作権表示はインストールしているlibpngのバージョンに合わせて下さい。
※2 ただし以下のいずれかの場合はなくても大丈夫です。
      * dxlibp.h で「#define DXP_BUILDOPTION_USE_LIBPNG」をコメントアウトして
        このライブラリをコンパイルした場合
      * SaveGraph、SaveScreen 等の画像保存関数を使っていない場合


jpeglib
this software is based in part on the work of the Independent JPEG Group.
※1 著作権表示はインストールしている jpeglib のバージョンに合わせて下さい。
※2 ただし dxlibp.h で「#define DXP_BUILDOPTION_USE_LIBJPEG」をコメントアウトして
    このライブラリをコンパイルした場合は無しで大丈夫です。


intraFont
Uses intraFont by BenHur
※ ただし DrawString 等の文字列描画関数を使っていない場合は無しで大丈夫です。


liblzr
Uses liblzr by BenHur


全角文字表示ライブラリ
このソフトウェアは mediumgauge 氏作成の全角文字表示ライブラリを使用しています。
※ ただし printfDx 等の簡易文字列描画関数を使っていない場合は無しで大丈夫です。


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

※ ただし dxlibp.h で「#define DXP_BUILDOPTION_USE_LIBOGG」をコメントアウトして
   このライブラリをコンパイルした場合は無しで大丈夫です。



--その他
バグ報告/修正絶賛募集中です！
メールやGitHub、某掲示板など作者が見そうなところならどこでもOKです。
※ 必ずしも対応/返信できるとは限りません


