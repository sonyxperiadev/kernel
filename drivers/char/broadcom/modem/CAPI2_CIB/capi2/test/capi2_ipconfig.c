/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   capi2_ipconfig.c
*
*   @brief  This file provides utility functions to create/parse ipconfig options
*
****************************************************************************/


#ifndef UNDER_LINUX
#include <string.h>
#include <stdlib.h>
#else
#include <linux/random.h>
#define rand get_random_int
#endif

// orig headers
#if 0
#include "mobcom_types.h"
#include "resultcode.h"
#include "common_defs.h"
#ifndef UNDER_LINUX
#include "crypto_def.h"
#include "crypto_api.h"
#endif
#include "uelbs_api.h"
#include "taskmsgs.h"
#include "ms_database_def.h"
#include "common_sim.h"
#include "sim_def.h"
//#include "pch_def.h"
//#include "capi2_stk_ds.h"
//#include "capi2_pch_msg.h"
//#include "capi2_gen_msg.h"
#include "capi2_reqrep.h"
#include "capi2_ss_api.h"
#include "capi2_phonectrl_api.h"
#include "capi2_reqrep.h"
#include "capi2_pch_api.h"
#endif 

#if 1
#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"

#include "xdr_porting_layer.h"
#include "xdr.h"

#include "common_defs.h"
#include "uelbs_api.h"
#include "ms_database_def.h"
#include "common_sim.h"
#include "sim_def.h"
#include "assert.h"
#include "sysparm.h"

#include "engmode_api.h"

///
#include "i2c_drv.h"
#include "ecdc.h"
#include "uelbs_api.h"
#include "common_sim.h"
#include "sim_def.h"
#include "stk_def.h"
#include "mti_trace.h"
#include "logapi.h"
#include "log.h"
#include "tones_def.h"
#include "phonebk_def.h"
#include "phonectrl_def.h"
#include "phonectrl_api.h"
#include "rtc.h"
#include "netreg_def.h"
#include "ms_database_old.h"
#include "ms_database_api.h"
#include "netreg_util.h"
#include "netreg_api_old.h"
#include "netreg_api.h"
#include "common_sim.h"
#include "sim_def.h"
#include "stk_def.h"
#include "ostypes.h"
#include "ss_def.h"
#include "sim_api.h"
#include "phonectrl_def.h"
#include "isim_def.h"
#include "pch_def.h"
#include "pchex_def.h"
#include "hal_em_battmgr.h"
#include "cc_def.h"
#include "rtem_events.h"
#include "rtc.h"
#include "engmode_def.h"
#include "sms_def.h"
#include "simlock_def.h"
#include "isim_def.h"
#include "pch_def.h"
#include "pchex_def.h"

#include "engmode_api.h"
#include "ms_database_old.h"
#include "ms_database_api.h"
#include "ss_api.h"
#include "sms_api_atc.h"
#include "sms_api_old.h"
#include "sms_api.h"
#include "cc_api_old.h"
#include "sim_api_old.h"
#include "sim_api.h"
#include "phonebk_api_old.h"
#include "phonebk_api.h"
#include "phonectrl_api.h"
#include "isim_api_old.h"
#include "isim_api.h"

#include "util_api.h"
#include "dialstr_api.h"
#include "stk_api_old.h"
#include "stk_api.h"

#include "pch_api_old.h"
#include "pch_api.h"
#include "pchex_api.h"
#include "ss_api_old.h"
#include "lcs_cplane_rrlp_api.h"
#include "cc_api.h"
#include "netreg_util.h"
#include "netreg_api.h"
#include "lcs_ftt_api.h"
#include "lcs_cplane_rrc_api.h"
#include "lcs_cplane_shared_def.h"

#include "capi2_mstruct.h"
#include "capi2_sim_api.h"
#include "capi2_phonectrl_api.h"
#include "capi2_sms_api.h"
#include "capi2_cc_api.h"
#include "capi2_lcs_cplane_api.h"
#include "capi2_ss_api.h"
#include "capi2_phonebk_api.h"
#include "capi2_cmd_resp.h"
#include "capi2_phonectrl_api.h"
//#include "capi2_gen_api.h"	

#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

#include "capi2_global.h"
#include "capi2_mstruct.h"
#include "capi2_cc_ds.h"
#include "capi2_cc_msg.h"
#include "capi2_msnu.h"
#include "ss_api_old.h"
#include "ss_lcs_def.h"
#include "capi2_ss_msg.h"
#include "capi2_cp_socket.h"
#include "capi2_cp_msg.h"
#include "capi2_pch_msg.h"
#include "capi2_sms_msg.h"
#include "capi2_phonectrl_api.h"
#include "capi2_phonectrl_msg.h"
#include "capi2_isim_msg.h"
#include "capi2_sim_msg.h"
#include "capi2_ss_msg.h"
#include "capi2_stk_ds.h"
#include "capi2_stk_msg.h"
#include "lcs_cplane_api.h"
#include "capi2_lcs_msg.h"
#include "lcs_cplane_rrc_api.h"
#include "lcs_cplane_shared_def.h"
#include "lcs_ftt_api.h"
#include "capi2_phonebk_msg.h"
#include "capi2_lcs_cplane_msg.h"
#define MAX_CC_GET_ELEM_SIZE  64
#include "capi2_gen_msg.h"

#include "capi2_reqrep.h"
#endif

#include "capi2_ipconfig.h"

#include "md5.h"


//******************************************************************************
//	 			Static functions
//******************************************************************************
static void GenChapRequest(CHAP_ChallengeOptions_t* cc, char* chal_name);

static void GenChapResponse(CHAP_ChallengeOptions_t*cc, 
					 CHAP_ResponseOptions_t *cr, 
					 char* secret, 
					 char* chal_name);

static void BuildIpConfigOptions(PCHProtConfig_t *ip_cnfg,
						  IPConfigAuthType_t authType,
						  CHAP_ChallengeOptions_t *cc,
						  CHAP_ResponseOptions_t *cr,
						  PAP_CnfgOptions_t *po);

static void UPAPGenWapAuth(PAP_CnfgOptions_t *po, char * user,  char * password);

static UInt8 gChid=0;



/******************************************************************************
	 			Capi2BuildIpConfigOptions

	The purpose of the protocol configuration options information element is to 
	transfer external network protocol options associated with a PDP context activation. 
	( ex LCP, CHAP, PAP, and IPCP )
	The username and password chap/pap MD5 digest needs to be set in the options.
	The DNS primary address request etc.
	Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
	- section 10.5.6.3 ( Mobile radio interface layer 3 specification )

******************************************************************************/
Boolean Capi2BuildIpConfigOptions(PCHProtConfig_t *cie,
										char* username,
										char* password,
										IPConfigAuthType_t authType)
{

	gChid++;
	
	if(authType == REQUIRE_CHAP)
	{
		CHAP_ChallengeOptions_t cc;
		CHAP_ResponseOptions_t	cr;
		
		cc.flag = 1;
		cr.flag = 1;

		//Build chap request ( generates random number )
		GenChapRequest(&cc, (username)?username:"");
		//Build chap response with MD5 digest of random number + secret/passwaord
		GenChapResponse(&cc, &cr, (username)?username:"", (password)?password:"");

		//Add chap options + ipconfig options + 
		BuildIpConfigOptions(cie, REQUIRE_CHAP, &cc, &cr, NULL);
	}
	else if(authType == REQUIRE_PAP)
	{
		PAP_CnfgOptions_t po;

		UPAPGenWapAuth(&po, (username)?username:"", (password)?password:"");
		BuildIpConfigOptions(cie, REQUIRE_PAP, NULL, NULL, &po);
	}

	return TRUE;
}

//******************************************************************************
//	 			BuildIpConfigOptions
//******************************************************************************
void BuildIpConfigOptions(PCHProtConfig_t *ip_cnfg,
						  IPConfigAuthType_t authType,
						  CHAP_ChallengeOptions_t *cc,
						  CHAP_ResponseOptions_t *cr,
						  PAP_CnfgOptions_t *po)
{
	int len, i, tlen;
	u_char *outp, *lenp, *len_pid;			 
	u_long dnsaddr_pri;
	u_long dnsaddr_sec;
	u_short IPCP_prot = PPP_IPCP;
	u_short UPAP_prot = PPP_UPAP;
	u_short CHAP_prot = PPP_CHAP;
	u_long  ipaddr = 0;
    u_long l;

	
	len = 0;							//start at the top			 
	dnsaddr_pri = 0;
	dnsaddr_sec = 0;

	ip_cnfg->options[len++] = 0x80;	  	/* octet2, Protocol Config Option, PPP for use with  */


	/** build Protocol Identifier, CHAP authentication (RFC 1994)*/
	if(authType == REQUIRE_CHAP)
	{

		/* Send CHAP challenge packet here */
    	if ( cc->flag ) 						    /* Request the network for CHAP authentication? */
		{
			outp = (u_char *)&ip_cnfg->options[len];  
			PUTSHORT(CHAP_prot, outp);				/* CHAP protocol */
			len += 2;

			ip_cnfg->options[len++] = cc->len;		/* Protocol ID length */

	   		IPCPDEBUG(("Build Option: chap cc len=%d total=%d\n",cc->len,len));

			for (i = 0; i < cc->len; i++)		    /* octet27, send PAP data           */
				ip_cnfg->options[len++] = cc->content[i];	/* octetm           */
			cc->flag = 0;						    /* Clear Request flag. We only do this once per request     */
		}			
		/* Send CHAP response packet here */
    	if ( cr->flag ) 						    /* Request the network for PAP authentication? */
		{
			outp = (u_char *)&ip_cnfg->options[len];  
			PUTSHORT(CHAP_prot, outp);				/* CHAP protocol */
			len += 2;
		
			ip_cnfg->options[len++] = cr->len;		/* Protocol ID length */
			
	   		IPCPDEBUG(("Build Option: chap resp len=%d total=%d\n",cr->len,len));
			for (i = 0; i < cr->len; i++)		    /* send PAP data           */
				ip_cnfg->options[len++] = cr->content[i];
			cr->flag = 0;						    /* Clear Request flag. We only do this once per request     */
		}
	}/** done CHAP */

	/** build Protocol Identifier, PAP authentication (RFC 1334)*/
	if(authType == REQUIRE_PAP)
	{
    	if ( po->flag ) 						  	/* Request the network for PAP authentication? */
		{
			outp = (u_char *)&ip_cnfg->options[len];  
			PUTSHORT(UPAP_prot, outp);				/* UPAP protocol */
			len += 2;
		
			ip_cnfg->options[len++] = po->len;		/* Protocol ID length */
			
	   		IPCPDEBUG(("Build Option: pap len=%d total=%d\n",po->len,len));
			for (i = 0; i < po->len; i++)		    /* send PAP data */
				ip_cnfg->options[len++] = po->content[i];  
			po->flag = 0;						 	/* Clear Request flag. We only do this once per request     */
		}			
	/** done PAP */
	}					 

	/** build Protocol Identifier, IPCP */
	outp = (u_char *)&ip_cnfg->options[len++];  
	PUTSHORT(IPCP_prot, outp);						/* IPCP protocol */
	len += 1;							  

	len_pid = (u_char *)&ip_cnfg->options[len++];	/* Protocol ID length */

	tlen = len;

/*  build code, id, length */
	ip_cnfg->options[len++] = 1;			/* configure-req */
	ip_cnfg->options[len++] = 1;			/* id */
	lenp = (u_char *)&ip_cnfg->options[len++];  
	len++;

/** build IPCP options 129 (RFC 1877)**/


   	IPCPDEBUG(("Build Option: PDNS"));
	ip_cnfg->options[len++] = CI_DNSADDR_PRI;
	ip_cnfg->options[len++] = 6;
	outp = (u_char *)&ip_cnfg->options[len];  
	l = ntohl(dnsaddr_pri);
	PUTLONG(l, outp);							/* allow network to NAK and give us primary DNS server addr */
	len += 4;

/** build IPCP options 130 (RFC 1877)**/

   	IPCPDEBUG(("Build Option: SDNS"));
	ip_cnfg->options[len++] = CI_DNSADDR_SEC;
	ip_cnfg->options[len++] = 6;
	outp = (u_char *)&ip_cnfg->options[len];  
	l = ntohl(dnsaddr_sec);
	PUTLONG(ipaddr, outp);					    	/* allow network to NAK and give us secondary DNS server addr*/ 	
	len += 4;

	tlen = len - tlen;
	PUTCHAR(tlen, len_pid);							/* store Protocol ID length */
	PUTSHORT(tlen, lenp);					    	/* store IPCP length */

	if (len > 1)
		ip_cnfg->length = len;		    	/* Protocol Config Option length Config protocol octet */
	else
		ip_cnfg->length = 0;		    		/* If no options, length is zero as per discussion with Iulia */

/* --- DEBUG print --- */
/*---------------------------------------------------------------*/
  IPCPDEBUG(("\r\nIP config options: %x", ip_cnfg->length));
  for (i = 0; i < len; i++)                                    
  		IPCPDEBUG(("%x ",ip_cnfg->options[i]));
/*---------------------------------------------------------------*/
}

//******************************************************************************
//	 			Generate Chap Request ( random string + username )
//******************************************************************************
void GenChapRequest(CHAP_ChallengeOptions_t* cc, char* chal_name)
{
	unsigned int chal_len;
	u_char *ptr;
	unsigned int i;
	UInt8 name_len,outlen;

	//Random challenge
	chal_len = 0;
	while(chal_len == 0)   /* get non-zero chal_len */
		//coverity[secure coding]
		chal_len = (unsigned)rand() & (MAX_CHALLENGE_LENGTH-1);   /* get length < MAX */
	while(chal_len < MIN_CHALLENGE_LENGTH)   /* raise len 'til >= MIN */
		chal_len <<= 1;
	
	name_len = strlen(chal_name);
	outlen = CHAP_HEADERLEN + sizeof (u_char) + chal_len + name_len;
	
	cc->len = outlen;	
	
	cc->content[0] = CHAP_CHALLENGE;
	cc->content[1] = gChid;
	cc->content[2] = 0;
	cc->content[3] = outlen;
	cc->content[4] = chal_len;
	
	ptr = &cc->content[5];
	
	/* generate a random string */
	for (i = 0; i < chal_len; i++ )
		//coverity[secure coding]
		*ptr++ = (char) (rand() & 0x00ff);
	
	// append challenge name
	outlen -= name_len;
	
	ptr = &cc->content[outlen];
	
	memcpy(ptr, chal_name, name_len);   /* append hostname */
}

//******************************************************************************
//	 			Generate Chap Response ( MD5 for secret + random string )
//******************************************************************************
void GenChapResponse(CHAP_ChallengeOptions_t*cc, 
					 CHAP_ResponseOptions_t *cr, 
					 char* chal_name,
					 char* secret
					 )
{
    UInt8 *ptr;
	UInt8 name_len,outlen;
	MD5_CTX mdContext;
	
	MD5Init(&mdContext);
	MD5Update(&mdContext, &gChid, 1);
	MD5Update(&mdContext, (UInt8*)secret,strlen(secret));
	MD5Update(&mdContext, &cc->content[5], cc->content[4]);
	MD5Final(&mdContext);

// fill in challenge len

	 name_len = strlen(chal_name);
	 outlen = CHAP_HEADERLEN + sizeof (u_char) + MD5_SIGNATURE_SIZE + name_len;
	 cr->len = outlen;	

	 cr->content[0] = CHAP_RESPONSE;
	 cr->content[1] = gChid;
	 cr->content[2] = 0;
	 cr->content[3] = outlen;
	 cr->content[4] = MD5_SIGNATURE_SIZE;

	 ptr = &cr->content[5];

     memcpy(ptr, mdContext.digest, MD5_SIGNATURE_SIZE);

// append host name
	 outlen -= name_len;
	 ptr = &cr->content[outlen];
	 memcpy(ptr, chal_name, name_len);   /* append hostname */

}


//******************************************************************************
//	 			Generate PAP options ( username + password)
//******************************************************************************
void UPAPGenWapAuth(PAP_CnfgOptions_t *po, char * user,  char * password)
{
    char *us_user;              /* User */
    int us_userlen;             /* User length */
    char *us_passwd;            /* Password */
    int us_passwdlen;           /* Password length */
	
	int outlen;
	u_char *outp;

    /* Save the username and password we're given */
    us_user = user;
    us_userlen = strlen(user);
    us_passwd = password;
    us_passwdlen = strlen(password);

   outlen = UPAP_HEADERLEN + 2 * sizeof (u_char) +
      us_userlen + us_passwdlen;
   outp = po->content;

   PUTCHAR(UPAP_AUTHREQ, outp);
   PUTCHAR(gChid, outp);
   PUTSHORT(outlen, outp);
   PUTCHAR(us_userlen, outp);
   memcpy(outp, us_user, us_userlen);
   INCPTR(us_userlen, outp);
   PUTCHAR(us_passwdlen, outp);
   memcpy(outp, us_passwd, us_passwdlen);

   po->len = outlen;
   po->flag = 1;
}

//******************************************************************************
//	 			Parse DNS address from ipconfig options
//******************************************************************************
Boolean Capi2ReadDnsSrv( PCHProtConfig_t *ipcnfg,
						   u_long* primaryDns1,
						   u_long* secDns1,
						   u_long* primaryDns2,
						   u_long* secDns2)        
{                                                                         
	u_char *input;                                                          
	u_char code, id, len_pid;
	u_short protocol;
	int len, t_len;               				  
	int i=0;
    int type, templ;							 
	u_long DNSsrvr;				
	u_long l;
	
	*primaryDns1 = 0;
	*secDns1 = 0;
	*primaryDns2 = 0;
	*secDns2 = 0;

	IPCPDEBUG(("ReadDnsSrv, read dns_servers"));
	/* if option length == 2, no option data */
	if (ipcnfg->length ==  0)                                 
    {                                                                     
		IPCPDEBUG(("Network ignored protocol options"));                  
		/* (Not an error, network does not want to bother) */
		return TRUE;                                                           
    }                                                                     
                                                                          
	len = 0;                                                              
	if (( ipcnfg->options[len++] & 0x83 ) == 0x81)	  	                  
    {                                                                     
		IPCPDEBUG(("Bad protocol config option"));                        
		return FALSE;                                                           
    }                                                                     
                                                                          
	t_len = len;                                                          
	while ( t_len < (ipcnfg->length) )                      
    {                                                                     
		input = (u_char *) &ipcnfg->options[t_len];  	                  
        GETSHORT(protocol, input);
        GETCHAR(len_pid, input);
        GETCHAR(code, input);
        GETCHAR(id, input);
        GETSHORT(len, input);
		switch (protocol)				                                  
    	{									                              
			/* Check LCP option from the Network */
			case PPP_LCP:			   
				break;                                                      

			/* Check IPCP option from the Network */
			case PPP_IPCP:
				if ( code == CONFREQ )
                {
                	IPCPDEBUG((" ReadOptions: CONFREQ"));
					i=0;
					while (i<(len-4))
                    {
   						GETCHAR(type, input);     			//RFC 1661, this is option type
   						GETCHAR(templ, input);		  		//RFC 1661, next byte is length
      					GETLONG(l, input);					//read but don't use
						i += templ;
					}
				}
				else if ( code == CONFACK || code == CONFNAK )													
                {	
                	IPCPDEBUG((" ReadOptions: CONFACK or CONFNAK"));
					
					i=0;
					while (i<(len-4))
                    {
   						GETCHAR(type, input);     //RFC 1661, this is option type
   						GETCHAR(templ, input);		  //RFC 1661, next byte is length
						switch (type)
                        {
							case CI_ADDR:
      							GETLONG(l, input);
								IPCPDEBUG(("CI_ADDR"));
								break;
							case CI_DNSADDR_PRI:
      							GETLONG(l, input);
								DNSsrvr = htonl(l);
								*primaryDns1 = DNSsrvr;
   								IPCPDEBUG(("CI_DNSADDR_PRI =%x", DNSsrvr));
								break;
						
							case (CI_DNSADDR_PRI+1):
      							GETLONG(l, input);
								DNSsrvr = htonl(l);
								*primaryDns2 = DNSsrvr;
   								IPCPDEBUG(("CI_DNSADDR_PRI =%x", DNSsrvr));
								break;
						
							case CI_DNSADDR_SEC:
      							GETLONG(l, input);
								DNSsrvr = htonl(l);
								*secDns1 = DNSsrvr;
   								IPCPDEBUG(("CI_DNSADDR_SEC"));
								break;
						
							case (CI_DNSADDR_SEC+1):
      							GETLONG(l, input);
								DNSsrvr = htonl(l);
								*secDns2 = DNSsrvr;
   								IPCPDEBUG(("CI_DNSADDR_SEC"));
								break;	  
							default:
								break;
                        }
						i += templ;

                    }
                }	
				else	/* Network had rejected */
                {
					while(i<(len-4))
                    {
   						GETCHAR(type, input);
   						GETCHAR(templ, input);								 
						switch (type)
                        {
							case CI_DNSADDR_PRI:
								IPCPDEBUG(("CI_DNSADDR_PRI rejected"));
								break;
						
							case (CI_DNSADDR_PRI+1):
								IPCPDEBUG(("CI_DNSADDR_PRI+1 rejected"));
								break;
						
							case CI_DNSADDR_SEC:
								IPCPDEBUG(("CI_DNSADDR_SEC rejected"));
								break;
						
							case (CI_DNSADDR_SEC+1):			 // Not supported
								IPCPDEBUG(("CI_DNSADDR_SEC+1 rejected"));
								break;	  
							default:
								break;
                        }
						i += templ;
                    }
					IPCPDEBUG((" ReadOptions: IPCP REJed"));	
                }	
				break;
			default:
				break;
    	}
	   	t_len += (len + 3);
     }                                                                                                            

	return TRUE;                                                           
}
