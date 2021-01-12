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
#ifndef _VIEWRIGHTWEBCLIENT_INTERNAL_H_
#define _VIEWRIGHTWEBCLIENT_INTERNAL_H_

#include "ViewRightWebClient.h"

class ViewRightWebClient_Internal : public ViewRightWebClient
{
public:
	virtual ~ViewRightWebClient_Internal();

	// Key Retrieval

	VRWebClientError_t RetrieveKeyFile(const char * url, char * key);

	// Third Party Handshake

	//TODO : Remove testing only
	//VRWebClientError_t CreateHandshake(unsigned char * * bufferEncOut, int * bufferEncLenOut);

protected:
	friend class ViewRightWebClient;
	ViewRightWebClient_Internal();

private:
	VCASCommunicationHandler * m_pVCASCommunicationHandler;
};

#endif
