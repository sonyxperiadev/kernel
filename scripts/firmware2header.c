#/************************************************************************/
/*                                                                      */
/*  Copyright 2013  Broadcom Corporation                                */
/*                                                                      */
/* Unless you and Broadcom execute a separate written software license  */
/* agreement governing use of this software, this software is licensed  */
/* to you under the terms of the GNU General Public License version 2   */
/* (the GPL), available at						*/
/*                                                                      */
/*          http://www.broadcom.com/licenses/GPLv2.php                  */
/*                                                                      */
/*  with the following added to such license:                           */
/*                                                                      */
/*  As a special exception, the copyright holders of this software give */
/*  you permission to link this software with independent modules, and  */
/*  to copy and distribute the resulting executable under terms of your */
/*  choice, provided that you also meet, for each linked independent    */
/*  module, the terms and conditions of the license of that module. An  */
/*  independent module is a module which is not derived from this       */
/*  software.  The special   exception does not apply to any            */
/*  modifications of the software.					*/
/*									*/
/*  Notwithstanding the above, under no circumstances may you combine	*/
/*  this software in any way with any other Broadcom software provided	*/
/*  under a license other than the GPL, without Broadcom's express	*/
/*  prior written consent.						*/
/*									*/
/*  Author: Arun KS <arun.ks@broadcom.com>				*/
/*									*/
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#define err_exit(fmt, arg...)   \
	{ printf(fmt, ##arg);	\
	  exit(0);		\
	}

void usage(void)
{
	err_exit("./firmware2header <firmware file name>\n");
}

int main(int argc, char **argv)
{
	FILE *f_ip, *f_op;
	unsigned char byte;
	int ret, i = 1, count = 0;
	char op_filename[50];

	if (argc < 2)
		usage();

	f_ip = fopen(argv[1], "r");
	if (!f_ip)
		err_exit("failed to open %s\n", argv[1]);

	sprintf(op_filename, "%s.i", argv[1]);
	printf("%s\n", op_filename);

	f_op = fopen(op_filename, "w");
	if (!f_op)
		err_exit("failed to open output file\n");

	do {
		ret = fread(&byte, 1, 1, f_ip);
		if (ret < 0)
			exit(0);
		fprintf(f_op, "0x%02x,", byte);
		i++;
		/* New line once in every 10 elements */
		if (i%10 == 0) {
			i = 1;
			fprintf(f_op, "\n");
		}
		count++;
	} while (ret == 1);

	fclose(f_ip);
	fclose(f_op);

	printf("count = %d\n", count);
	printf("file created at %s\n", op_filename);
}
