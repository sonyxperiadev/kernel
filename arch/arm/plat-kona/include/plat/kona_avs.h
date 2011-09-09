
#ifndef __KONA_AVS___
#define __KONA_AVS___


enum
{
    SILICON_TYPE_SLOW,
    SILICON_TYPE_TYPICAL,
    SILICON_TYPE_FAST
};

enum
{
    AVS_TYPE_OPEN,
    AVS_TYPE_BOOT
};

struct kona_avs_pdata
{
    u32 avs_type;
    void (*silicon_type_notify)(u32 silicon_type);

    u32 nmos_bin_size;
    u32 pmos_bin_size;

    u32* svt_pmos_bin;
    u32* svt_nmos_bin;

    u32* lvt_pmos_bin;
    u32* lvt_nmos_bin;

    u32* svt_silicon_type_lut;
    u32* lvt_silicon_type_lut;

    u8** volt_table;

    u32 otp_row;

};


u32 kona_avs_get_solicon_type(void);
u8* kona_avs_get_volt_table(void);

#endif /*__KONA_AVS___*/
