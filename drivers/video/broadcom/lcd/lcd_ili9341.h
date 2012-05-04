
//Haipeng, I could not open the excel spreadsheet. So, please fill the following two arrays.

DISPCTRL_REC_T ili9341_Init[] = {

	{DISPCTRL_WR_CMND         , 0xCF                            , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0xE3    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0xFF    },     
	
	{DISPCTRL_WR_CMND         , 0xED                            , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x64    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x03    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x12    },  
	{DISPCTRL_WR_DATA         , 0x00                            , 0x81    },  

	{DISPCTRL_WR_CMND         , 0xE8                            , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x85    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x60    },  

	{DISPCTRL_WR_CMND         , 0xCB                            , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x39    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x2D    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },  
	{DISPCTRL_WR_DATA         , 0x00                            , 0x34    }, 
	{DISPCTRL_WR_DATA         , 0x00                            , 0x02    },  
	
    {DISPCTRL_WR_CMND_DATA    , 0xF7                            , 0x20       },
		
	
	{DISPCTRL_WR_CMND         , 0xEA                            , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0xC3    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },            

	{DISPCTRL_WR_CMND_DATA    , 0xC0                            , 0x15       },
	{DISPCTRL_WR_CMND_DATA    , 0xC1                            , 0x11       },

	{DISPCTRL_WR_CMND         , 0xC5                            , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x1e    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x4b    },  
	
	//Initializing sequence
	{DISPCTRL_WR_CMND_DATA    , 0x35                           , 0x00       },
	{DISPCTRL_WR_CMND_DATA    , 0x36                           , 0x08       },
	{DISPCTRL_WR_CMND_DATA    , 0x3A                           , 0x06       }, //RGB565, RGB666:0x06h
	{DISPCTRL_WR_CMND_DATA    , 0x26                           , 0x01       },

	{DISPCTRL_WR_CMND         , 0xB1                            , 0x0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x19    }, 
		
	{DISPCTRL_WR_CMND         , 0xB4                            , 0x0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x02    },            
  
	{DISPCTRL_WR_CMND         , 0xB6                           , 0x0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x0A    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0xC2    },  

	{DISPCTRL_WR_CMND         , 0xF6                            , 0x0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x01    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x30    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x00    },


	//Gama setting
	{DISPCTRL_WR_CMND_DATA    , 0xF2                           ,  0x0       },

	{DISPCTRL_WR_CMND         , 0xE0                            , 0x0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x0A    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x13    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x18    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x05    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x0C    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x07    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x44    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0xC8    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x36    },	
    {DISPCTRL_WR_DATA         , 0x00                            , 0x09    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x10    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x02    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x0F    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x0D    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x0D    },
	
	{DISPCTRL_WR_CMND         , 0xE1                            , 0x0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x05    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x18    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x22    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x00    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x10    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x07    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x33    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x47    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x45    },	
    {DISPCTRL_WR_DATA         , 0x00                            , 0x04    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x09    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x0D    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x30    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x33    },
	{DISPCTRL_WR_DATA         , 0x00                            , 0x0D    },

	//Sleep out
	{DISPCTRL_WR_CMND         , 0x11                           , 0       },   
	{DISPCTRL_SLEEP_MS        , 0                              , 120      },
	
	{DISPCTRL_WR_CMND         , 0x13                           , 0       }, 

	{DISPCTRL_WR_CMND         , 0x29                           , 0       },   
	{DISPCTRL_SLEEP_MS        , 0                              , 40      },
	{DISPCTRL_WR_CMND         , 0x2C                           , 0       }, 
	{DISPCTRL_WR_CMND         , 0x3C                           , 0       }, 

    //--- END OF COMMAND LIST -----------------------
    {DISPCTRL_LIST_END       , 0                                , 0       }
};

DISPCTRL_REC_T ili9341_Sleep[] = {
    {DISPCTRL_SLEEP_MS        , 0                               , 10      },
	{DISPCTRL_LIST_END       , 0                                , 0       }
};
