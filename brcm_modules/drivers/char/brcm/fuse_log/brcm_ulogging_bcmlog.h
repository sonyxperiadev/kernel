	// Definations used for  Unified Logging
	extern int bcmlog_mtt_on;
	extern unsigned short bcmlog_log_ulogging_id;

	int brcm_retrive_early_printk();

	bcmlog_log_ulogging_id = BCMLOG_LOG_ULOGGING_ID;
	BCMLOG_EnableLogId( bcmlog_log_ulogging_id, 1 ) ;
	
	bcmlog_mtt_on = 1;

	ret= brcm_retrive_early_printk();
	if (ret < 0)
	    printk("\n Printk->Mtt: Couldn't get early printk \n");
