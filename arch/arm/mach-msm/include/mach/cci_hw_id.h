#ifndef __CCI_HW_ID_H
#define __CCI_HW_ID_H

/* Implement CCI HW ID/PROJECT ID */

#define CCI_HWID_MASK 		(0x000000FF)
#define CCI_PROJECTID_MASK (0x0000FF00)
#define CCI_SIMID_MASK 	(0x00FF0000)
#define CCI_BANDID_MASK     (0xFF000000)

#define CCI_HWID_OFFSET 		0
#define CCI_PROJECTID_OFFSET 	8
#define CCI_SIMID_OFFSET	 	16
#define CCI_BANDID_OFFSET     24

#define SET_CCI_HWID_VALUE(value) 		((value & CCI_HWID_MASK) << CCI_HWID_OFFSET)
#define SET_CCI_PROJECTID_VALUE(value) ((value & CCI_HWID_MASK) << CCI_PROJECTID_OFFSET)
#define SET_CCI_SIMID_VALUE(value) 	((value & CCI_HWID_MASK) << CCI_SIMID_OFFSET)
#define SET_CCI_BANDID_VALUE(value)    ((value & CCI_HWID_MASK) << CCI_BANDID_OFFSET)

#define GET_CCI_HWID_VALUE(value) 		(value & CCI_HWID_MASK)
#define GET_CCI_PROJECTID_VALUE(value) ((value & CCI_PROJECTID_MASK) >> CCI_PROJECTID_OFFSET)
#define GET_CCI_SIMID_VALUE(value) 	((value & CCI_SIMID_MASK) >> CCI_SIMID_OFFSET)
#define GET_CCI_BANDID_VALUE(value)    ((value & CCI_BANDID_MASK) >> CCI_BANDID_OFFSET)

#define CCI_HWID_TYPE_STRLEN 20

#define GPIO_CCI_PROJECT_ID_1							 2
#define GPIO_CCI_PROJECT_ID_2							 3
#define GPIO_CCI_SIM_ID								13
#define GPIO_CCI_BAND_ID_1								12
#define GPIO_CCI_BAND_ID_2							   114
#define GPIO_CCI_HW_ID_1								50
#define GPIO_CCI_HW_ID_2								 1
#define GPIO_CCI_HW_ID_3							   116
#define GPIO_CCI_CUSTOMER_ID							31

#define GPIO_CCI_WATERPROOF_ID							34
#define NUM_CCI_HWID_GPIO								 9

enum cci_project_id_type {
	CCI_PROJECTID_VY52_53 = 0,
	CCI_PROJECTID_VY55_56 = 1,
	CCI_PROJECTID_VY58_59 = 2,
	CCI_PROJECTID_INVALID
};

enum cci_band_id_type {
	CCI_BANDID_EU = 0,
	CCI_BANDID_US = 1,
	CCI_BANDID_INVALID
};

enum cci_sim_id_type {
	CCI_SIMID_SS = 0, 		/* Single SIM */
	CCI_SIMID_DS = 1, 		/* Dual SIM */
	CCI_SIMID_INVALID
};

enum cci_hw_id_type {
	CCI_HWID_EVT 	 = 0,
	CCI_HWID_DVT1	 = 1,
	CCI_HWID_DVT1_1 = 2,
	CCI_HWID_DVT2 	 = 3,
	CCI_HWID_DVT3 	 = 4,
	CCI_HWID_PVT 	 = 5,
	CCI_HWID_MP 	 = 6,
	CCI_HWID_INVALID
};

enum cci_customer_id_type {
	CCI_CUSTOMERID_CUSTOMER = 0,
	CCI_CUSTOMERID_CCI = 1,
	CCI_CUSTOMERID_INVALID
};

extern int get_cci_hw_id(void); 
extern int get_cci_project_id(void);
extern int get_cci_customer_id(void);
extern int get_cci_band_id(void);
extern int get_cci_sim_id(void);
int cci_hw_sim_type_read( char *page, char **start, off_t off, int count, int *eof, void *data );
int cci_hw_board_type_read( char *page, char **start, off_t off, int count, int *eof, void *data );
int cci_hw_band_type_read( char *page, char **start, off_t off, int count, int *eof, void *data );
int cci_hw_proj_name_read( char *page, char **start, off_t off, int count, int *eof, void *data );
int cci_hw_customer_type_read( char *page, char **start, off_t off, int count, int *eof, void *data );
int cci_hwid_info_read( char *page, char **start, off_t off, int count, int *eof, void *data );

#endif /* __CCI_HW_ID_H */
