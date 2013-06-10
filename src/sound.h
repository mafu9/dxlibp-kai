#include "dxlibp.h"
#include <pspmp3.h>
#include <psputility.h>
#include <psptypes.h>
#include <pspaudio.h>
#include <tremor/ivorbisfile.h>

#define DXP_BUILDOPTION_SOUNDHANDLE_MAX 32

#define DXP_SOUNDCMD_NONE 0
#define DXP_SOUNDCMD_PLAY 1
#define DXP_SOUNDCMD_STOP 2
#define DXP_SOUNDCMD_EXIT 3

#define DXP_SOUNDFMT_MP3 1
#define DXP_SOUNDFMT_OGG 2
#define DXP_SOUNDFMT_AT3 3
#define DXP_SOUNDFMT_WAV 4

/* Wave formats */
#define WAVE_FORMAT_PCM			0x0001
#define WAVE_FORMAT_ATRAC3		0x0270
#define WAVE_FORMAT_ATRAC3PLUS	0xFFFE

/* At3 */
#define AT3_TYPE_ATRAC3			0
#define AT3_TYPE_ATRAC3PLUS		1

extern const int At3_Sample_Per_Frame[2];

#define SHND2PTR(HNDLE,PTR) {if(!dxpSoundData.init)return -1; if(HNDLE < 0 || HNDLE >= DXP_BUILDOPTION_SOUNDHANDLE_MAX)return -1;PTR = dxpSoundArray + HNDLE;if(!PTR->used)return -1;}

//unsigned long * 65 byteのバッファ
typedef struct _DXPAVCODEC_BUFFER {
	//予約領域
	u32 reserved0[6];
	//デコード前の圧縮されたデータのバッファへのアドレス
	u8* datIn;
	//圧縮されたデータのバッファのバイト長
	u32 frameSize0;
	//デコード後のPCMデータのバッファへのアドレス
	u32* pcmOut;
	//デコード後のPCMデータのバッファのバイト長
	u32 decodeByte;//set 4608 (= 1152[sample per frame] * 2[byte per sample] * 2[channel])
	//frameSize0のダミー？
	u32 frameSize1;
	//予約領域
	u32 reserved1[54];
} DXPAVCODEC_BUFFER ;

typedef struct _DXPAVCONTEXT_MP3 {
	DXPAVCODEC_BUFFER *avBuf;
	int id3v1Pos;
	int id3v2Pos;
	u8 *mp3Buf;
	u32 mp3BufSize;
} DXPAVCONTEXT_MP3 ;

typedef struct _DXPAVCONTEXT_OGG {
	OggVorbis_File *file;
} DXPAVCONTEXT_OGG ;

typedef struct _DXPAVCONTEXT_AT3 {
	/* デコード用のバッファ */
	unsigned long *codecBuf;
	/* AT3のタイプ(基本ATRAC3PLUS) */
	u8 type;
	union {
		/* AT3 */
		u8 at3_channel_mode;
		/* AT3PLUS */
		u8 at3plus_flagdata[2];
	};
	/* チャンネル数 */
	u16 data_align;
	u16 samples_per_frame;
	/* 波形データ */
	u32 dataPos;
	u32 dataSize;
	u8  dataHeader;
	/* 一時バッファ */
	u8* dataBuf;
	u32 dataBufSize;
} DXPAVCONTEXT_AT3 ;

typedef struct _DXPAVCONTEXT_WAV {
	u32 bloack_size;
	u32 dataPos;
	u32 dataSize;
	u16 blockalign;
	u8  dataHeader;
} DXPAVCONTEXT_WAV ;

typedef struct DXPAVCONTEXT {
	//Uファイルハンドル
	int fileHandle;
	//Uファイルサイズ
	int fileSize;
	//U出力先
	u32 *pcmOut;
	
	//D1サンプルのサイズ
	u8 sampleSize;
	//Dチャンネル数
	u8 channels;
	//Dサンプリングレート
	int sampleRate;
	//Dデコーダが次にデコードするサンプル位置
	int nextPos;
	//Dデコーダが必要とする出力先バッファサイズ（サンプル数）
	int outSampleNum;
	//D総サンプル数
	int totalSampleNum;
	//Dフォーマット
	u8 format;

	union {
		DXPAVCONTEXT_MP3 mp3;
		DXPAVCONTEXT_OGG ogg;
		DXPAVCONTEXT_AT3 at3;
		DXPAVCONTEXT_WAV wav;
	};
} DXPAVCONTEXT ;

typedef struct DXPSOUNDHANDLE
{
	//ハンドルステータス
	unsigned used : 1;
	int soundDataType;
	//ユーザーから指定する情報
	int cmd;
	int loopResumePos;
	u8 volume;
	int pan;
	int playing;

	union {
		//DX_SOUNDDATATYPE_MEMPRESSとDX_SOUNDDATATYPE_FILEで使用される
		struct {
			int threadId;
			int gotoPos;
			int loop;
			//DX_SOUNDDATATYPE_MEMPRESSでのみ使用
			void *buffer;
		} file;

		//DX_SOUNDDATATYPE_MEMNOPRESSで使用
		struct {
			int length;
			u32 *pcmBuf;
			int cmdplaytype;
		} memnopress;
	};

	DXPAVCONTEXT avContext;
} DXPSOUNDHANDLE;

typedef struct DXPSOUNDDATA
{
	u8 init;
	u8 createSoundDataType;
}DXPSOUNDDATA;

extern DXPSOUNDHANDLE dxpSoundArray[];
extern DXPSOUNDDATA dxpSoundData;

extern int memnopress_handle[PSP_AUDIO_CHANNEL_MAX];
extern int memnopress_pos[PSP_AUDIO_CHANNEL_MAX];
extern int memnopress_playtype[PSP_AUDIO_CHANNEL_MAX];

int dxpSoundInit();
int dxpSoundTerm();
int dxpSoundReserveHandle();
int dxpSoundReleaseHandle(int handle);

int dxpSoundAudioChReserve(DXPAVCONTEXT *av);
int dxpSoundCalcBufferSize(DXPAVCONTEXT *av, int sample);
int dxpSoundGetNextSampleNum(DXPAVCONTEXT *av);

int dxpSoundMp3Init(DXPAVCONTEXT *av);
int dxpSoundMp3GetSampleLength(DXPAVCONTEXT *av);
int dxpSoundMp3Seek(DXPAVCONTEXT *av, int sample);
int dxpSoundMp3Decode(DXPAVCONTEXT *av);
int dxpSoundMp3End(DXPAVCONTEXT *av);

int dxpSoundOggInit(DXPAVCONTEXT *av);
int dxpSoundOggGetSampleLength(DXPAVCONTEXT *av);
int dxpSoundOggSeek(DXPAVCONTEXT *av, int sample);
int dxpSoundOggDecode(DXPAVCONTEXT *av);
int dxpSoundOggEnd(DXPAVCONTEXT *av);

int dxpSoundAt3Init(DXPAVCONTEXT *av);
int dxpSoundAt3GetSampleLength(DXPAVCONTEXT *av);
int dxpSoundAt3Seek(DXPAVCONTEXT *av, int sample);
int dxpSoundAt3Decode(DXPAVCONTEXT *av);
int dxpSoundAt3End(DXPAVCONTEXT *av);

int dxpSoundWavInit(DXPAVCONTEXT *av);
int dxpSoundWavGetSampleLength(DXPAVCONTEXT *av);
int dxpSoundWavSeek(DXPAVCONTEXT *av, int sample);
int dxpSoundWavDecode(DXPAVCONTEXT *av);
int dxpSoundWavAllDecode(DXPAVCONTEXT *av);
int dxpSoundWavEnd(DXPAVCONTEXT *av);

int dxpSoundCodecInit(DXPSOUNDHANDLE *pHnd);
int dxpSoundCodecGetSampleLength(DXPSOUNDHANDLE *pHnd);
int dxpSoundCodecSeek(DXPSOUNDHANDLE *pHnd,int sample);
int dxpSoundCodecDecode(DXPSOUNDHANDLE *pHnd);
int dxpSoundCodecEnd(DXPSOUNDHANDLE *pHnd);

int dxpSoundThreadFunc_file(SceSize size,void* argp);
int dxpSoundThreadFunc_memnopress(SceSize size,void* argp);