#include "../general.h"
#include "../input.h"

int ProcessMessage()
{
	if(dxpGeneralData.homebutton_pushed)return -1;
	
	dxpInputRenew();
	return 0;
}