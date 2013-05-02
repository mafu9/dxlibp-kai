#include "../general.h"

DXPGENERALDATA dxpGeneralData =
{
	.initialized = 0,

	.homebutton_callback_initialized = 0,
	.homebutton_pushed = 0,
	.exit_called = 0,
	.homebutton_callback_threadid = -1,

	.randmode = 0,
	.mt19937context = NULL,

	.charset = DXP_CP_SJIS
};
