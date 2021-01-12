/*
 * Copyright (c) 2014 Verimatrix, Inc.  All Rights Reserved.
 * The Software or any portion thereof may not be reproduced in any form
 * whatsoever except as provided by license, without the written consent of
 * Verimatrix.
 *
 * THIS NOTICE MAY NOT BE REMOVED FROM THE SOFTWARE BY ANY USER THEREOF.
 * NEITHER VERIMATRIX NOR ANY PERSON OR ORGANIZATION ACTING ON BEHALF OF
 * THEM:
 *
 * 1. MAKES ANY WARRANTY OR REPRESENTATION WHATSOEVER, EXPRESS OR IMPLIED,
 *    INCLUDING ANY WARRANTY OF MERCHANTABILITY OR FITNESS FOR ANY
 *    PARTICULAR PURPOSE WITH RESPECT TO THE SOFTWARE;
 *
 * 2. ASSUMES ANY LIABILITY WHATSOEVER WITH RESPECT TO ANY USE OF THE
 *    PROGRAM OR ANY PORTION THEREOF OR WITH RESPECT TO ANY DAMAGES WHICH
 *     MAY RESULT FROM SUCH USE.
 *
 * RESTRICTED RIGHTS LEGEND:  Use, duplication or disclosure by the
 * Government is subject to restrictions set forth in subparagraphs
 * (a) through (d) of the Commercial Computer Software-Restricted Rights
 * at FAR 52.227-19 when applicable, or in subparagraph (c)(1)(ii) of the
 * Rights in Technical Data and Computer Software clause at
 * DFARS 252.227-7013, and in similar clauses in the NASA FAR supplement,
 * as applicable. Manufacturer is Verimatrix, Inc.
 */ 
#ifndef _VIEWRIGHTWEBCLIENT_H_
#define _VIEWRIGHTWEBCLIENT_H_

#define MAX_PAYLOAD_LENGTH  26

#include <vector>
#include <string>
#include "ViewRightWebInterface.h"

class VRWebClientCallbacks;

using namespace std;


class ViewRightWebClient : public ViewRightWebInterface
{
public:

	typedef struct VRWebClientStatus_Struct
	{
		bool				bIsProvisioned;
		bool				bIsCertExpired;
		unsigned long long	certExpirationTime;
		char				companyName[COMPANY_NAME_MAX_LENGTH];

	} VRWebClientStatus_t;

	VIEWRIGHTWEBCLIENT_API virtual ~ViewRightWebClient();

	VIEWRIGHTWEBCLIENT_API static ViewRightWebClient * GetInstance(); 
	
    //TODO - Depracate this API in the next release (i.e after Flora)
	
#if defined WIN32 || defined _WINRT_DLL
__declspec(deprecated)
#else
__attribute__((deprecated))
#endif
	VIEWRIGHTWEBCLIENT_API VRWebClientError_t Decrypt(unsigned char * data, unsigned long & dataSize, const char * decryptMethod, const char * keyfileURL, const unsigned char * keyIV, unsigned int keyIVSize = 16);
    
    //MPEG-DASH decrypt API
    VIEWRIGHTWEBCLIENT_API VRWebClientError_t Decrypt(unsigned char * data, unsigned long & dataSize, ViewRightWebClient::MethodInfo decryptMethod, const unsigned char * psshBytes, unsigned long psshLength, const unsigned char * keyIV, unsigned int keyIVSize, const unsigned char * keyID);
    
    //HLS decrypt API
    VIEWRIGHTWEBCLIENT_API VRWebClientError_t Decrypt(unsigned char * data, unsigned long & dataSize, ViewRightWebClient::MethodInfo decryptMethod, const char * keyfileURL, const unsigned char * keyIV, unsigned int keyIVSize = 16);

	VIEWRIGHTWEBCLIENT_API VRWebClientError_t SetCallbackClass(VRWebClientCallbacks * pCallbackClass);

	// Status

	VIEWRIGHTWEBCLIENT_API VRWebClientError_t GetClientStatus(VRWebClientStatus_t * clientStatus);

	VIEWRIGHTWEBCLIENT_API static void ResetInstance();
    
protected:
	ViewRightWebClient();

	VCASCommunicationHandler * GetVCASCommunicationHandler(void);

private:
	static ViewRightWebClient * m_pInstance;

};

class VRWebClientCallbacks
{
public:   
    struct OutputControlSettings
    {
        bool    BestEffort_Enabled; // Use best efforts on each setting, but not mandatory
        int     ccACP_Level; // 0=off, 1, 2, 3
        bool    DwightCavendish_Enabled;
        bool	HDCP_Enabled; // ON = use highest verison available
        int     ccCGMS_A_Level; // 0=Freely, No More, Once, Never
        bool    CIT_Analog_Enabled; // HD must be downres on Analog output
        bool    CIT_Digital_Enabled; // HD must be downres if no HDCP
        bool    DOT_Enabled; // Doesn't allow playback on Analog
        bool    Anti_Mirroring_Enabled; // Disable mirroring (e.g. Airplay on iOS)
    };

	struct OperatorDataSettings
	{
		bool bGlobal; // true if the data is Global Operator Data
		char data[257]; //256 max plus a char for \0
		unsigned int dataSize;
	};

	//Deprecated
	struct VideoMarkSettings
	{
		bool bEnabled;
		unsigned short Configuration;
		unsigned int Strength;
		char Payload[MAX_PAYLOAD_LENGTH];
	};

	virtual void KeyRetrievalStatus(const char * url, ViewRightWebInterface::VRWebClientError_t status) { }
	virtual void ConfigureOutputControlSettings(struct OutputControlSettings settings) { }
	virtual void OperatorData(struct OperatorDataSettings data) { }
    virtual ~VRWebClientCallbacks(void) { }
};

#endif
