//******************************************************************************
//
// Function Name:	ConfigGenericCmds
//
// Description:		Generate generic commands to command queue
//
// Notes:
//
//******************************************************************************

#include "string.h"
#include "chip_version.h"
#include "shared.h"
#include "sharedmem_comms.h"
#include "sharedmem.h"
#include "patch.h"
#include "ripcmdq.h"

#define CL_DSP_PATCH_OVERLAY	91293


Patch_Flash_t *p_flash_patch_data;

PatchData_t   *p_patch_data;
extern UInt32 dsp_changelist;

void PATCH_SetBaseAddr(				// Set the system parameter base address
	UInt32 patch_base_addr			// Base address
	)
{
	p_flash_patch_data = (Patch_Flash_t *)patch_base_addr;
}

Boolean PATCH_Check(void)
{
	const char signature[]=SIGNATURE;
    UInt16 len, i, check_sum;
	UInt16* p;

	if (p_flash_patch_data == NULL)
	{
		return FALSE;
	}
	
    if( dsp_changelist == p_flash_patch_data->patch1.dspchangelist)
	{
		p_patch_data = &p_flash_patch_data->patch1;
	}
#ifndef DSP_FEATURE_2133A2_TWO_LAYER_OVERLAY_PATCH
	else if( dsp_changelist == p_flash_patch_data->patch2.dspchangelist)
	{
  		p_patch_data = &p_flash_patch_data->patch2;
	}
#endif
	else
		return FALSE;
// check signature
	len=strlen(signature);
	for (i=0; i<len; i++)
	{
		if (signature[i]!=p_patch_data->signature[i])
		{
			p_patch_data = NULL;
			return FALSE;
		}
	}
// check checksum
	p = (UInt16*)p_patch_data;
	check_sum = 0;
	len = sizeof(PatchData_t)/2;
	for (i=0; i<len; i++)
	{
		check_sum ^= *p++;
	}
	if (check_sum==0)
	{
		return TRUE;
	}
	else
	{
		p_patch_data = NULL;
		return FALSE;
	}
}

void PATCH_Config(void )	// Generate generic commands to command queue
{

}

UInt8* PATCH_GetDate( void )
{
	return p_patch_data->date;
}

UInt8* PATCH_GetRevision( void )
{
	
	return p_patch_data->revision;
	
}

UInt8* PATCH_GetChangelist( void )
{
	return p_patch_data->changelist;
}

UInt16 PATCH_GetEnabled( void )
{
	if ( p_patch_data != NULL )
		return p_patch_data->enabled;
	else
		return 0;
}

UInt16 PATCH_GetWriteInitCmdSize( void )
{
	return p_patch_data->write_init_cmd_size;
}

UInt16 PATCH_GetPramWriteCmdSize( void )
{
	return p_patch_data->pram_write_cmd_size;
}

#ifndef DSP_FEATURE_2133A2_TWO_LAYER_OVERLAY_PATCH

UInt16 PATCH_GetPramPatchCommandEnable( void )
{
	Patch_GenericCmd_t *ptr;
	ptr = &p_patch_data->pram_write_cmd[ 1 ];

	return ptr->value;
}

UInt16 PATCH_GetPramPatchCommandDisable( void )
{
	Patch_GenericCmd_t *ptr;
	ptr = &p_patch_data->pram_write_cmd[ 2 ];

	return ptr->value;
}

#endif

