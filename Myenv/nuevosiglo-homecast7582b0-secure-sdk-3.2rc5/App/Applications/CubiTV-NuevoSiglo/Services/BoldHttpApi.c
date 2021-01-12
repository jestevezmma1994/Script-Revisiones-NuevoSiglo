/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
 **
 ** Any rights which are not expressly granted in this License are entirely and
 ** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
 ** modify, translate, reverse engineer, decompile, disassemble, or create
 ** derivative works based on this Software. You may not make access to this
 ** Software available to others in connection with a service bureau,
 ** application service provider, or similar business, or make any other use of
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#define SvLogModule "com.sentivision.SvHTTPClient.SvHTTPGet"
#define SvLogModuleCustomized

#include "BoldHttpApi.h"


#include <errno.h>
#include <expat.h>
#include <fcntl.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <QBCAS.h>
#include <QBConf.h>
#include <QBTextUtils/base64.h>
#include <stdlib.h>
#include <string.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvHTTPClient/SvHTTPRequest.h>
#include <SvHTTPClient/SvHTTPClientListener.h>
#include <SvProfile.h>
#include <unistd.h>


// listener class


struct BoldHttpApi_t
{
    struct SvObject_ super_;
    SvHTTPClientEngine engine;
    SvHTTPRequest request;
    SvHTTPRequestID reqID;
    bool started;
    SvString id;
    SvString user_name;
    SvString user_password;
    BoldHttpApiCallback callback;
    void * callbackData;
    char buffer[HTTP_BUF_LEN];
    int buffer_offset;
};

struct BoldHttpListenerMessage_t
{
    struct SvObject_ super_;
    
    BoldHttpApiCallback callback;
    void * callbackData;
    
    SvHTTPClientEngine engine;
    SvHTTPRequest request;
    SvHTTPRequestID reqID;
    char buffer[HTTP_BUF_LEN];
    int buffer_offset;
};

typedef struct BoldHttpListenerMessage_t* BoldHttpListenerMessage;

// private methods

static int BoldEncrypt(unsigned const char *plaintext, int plaintext_len, unsigned char *key,
                                unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len=0;

    int ciphertext_len=0;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
    {
        SvLogError("Initialize CIPHER");
        return -1;
    } 

    /* Initialise the encryption operation. IMPORTANT - ensure you use a key
    * and IV size appropriate for your cipher
    * In this example we are using 256 bit AES (i.e. a 256 bit key). The
    * IV size for *most* modes is the same as the block size. For AES this
    * is 128 bits */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
    {
        SvLogError("Encypt Init");
        return -1;
    } 


    /* Provide the message to be encrypted, and obtain the encrypted output.
    * EVP_EncryptUpdate can be called multiple times if necessary
    */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    {
        SvLogError("Encypt update");
        return -1;
    } 
    ciphertext_len = len;

    /* Finalise the encryption. Further ciphertext bytes may be written at
    * this stage.
    */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) 
    {
        SvLogError("Encypt Final");
        return -1;
    } 
    
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

void BoldHttpApiCleanHTTPRequest(BoldHttpApi self)
{
    if(self->request)
        SvHTTPRequestCancel(self->request, NULL);
    
    SVTESTRELEASE(self->request);
    self->request = NULL;
}

// virtual methods

static void BoldHttpApiTransferFinished(SvGenericObject self_,
        SvHTTPRequest req, off_t total)
{
    // TODO quitar dbugging
    //char temp_buff[2048];

    BoldHttpApi self = (BoldHttpApi) self_;
    BoldHttpApiCleanHTTPRequest(self);
    // TODO quitar debugging
    //char temp_buff[2048];
    //SvLogNotice("********************* Transfer finished ********************");
    //memset(temp_buff, 0, 2048);
    //snprintf(temp_buff, self->buffer_offset + 1, "%s", self->buffer);
    //SvLogNotice("%s", temp_buff);
    //SvLogNotice("************************************************************");
    self->callback (self->callbackData, self->buffer, self->buffer_offset);
    memset (self->buffer, 0, HTTP_BUF_LEN);
    self->buffer_offset = 0;
}

static void BoldHttpApiHeaderReceived(SvGenericObject self_,
        SvHTTPRequest req, const char *name, const char *value)
{
    //SvLogNotice("********************* Header Received **********************");
    //SvLogNotice("%s: %s", name, value);
    //SvLogNotice("************************************************************");
}

static void BoldHttpApiErrorOccured(SvGenericObject self_, SvHTTPRequest req, int httpError)
{
    BoldHttpApi self = (BoldHttpApi) self_;
    SvLogError("%s: [%02u] failed with error %d", __func__, self->reqID, httpError);
    BoldHttpApiCleanHTTPRequest(self);
    self->callback (self->callbackData, NULL, 0);
}



static void BoldHttpApiDataChunkReceived(SvGenericObject self_,
        SvHTTPRequest req, off_t offset, const uint8_t *buffer, size_t length)
{
    BoldHttpApi self = (BoldHttpApi) self_;
    
    if (self->buffer_offset + (int)length >= HTTP_BUF_LEN)
    {
        SvLogError("%s: Buffer size exceeded. Buffer size (%d) actual size (%d) chunk length (%zu)", __func__, HTTP_BUF_LEN, self->buffer_offset, length);
        return;
    }
    
    memcpy (&self->buffer[self->buffer_offset], buffer, length);
    self->buffer_offset += (int)length;
}

static void BoldHttpApiTransferCancelled(SvGenericObject self_, SvHTTPRequest req)
{

}



// public methods

static void
BoldHttpApiDestroy(void *self);

static SvType BoldHttpApi_getType(void)
{
    static const struct SvObjectVTable_ objectVTable =
    {
        .destroy = BoldHttpApiDestroy
    };
    
     static const struct SvHTTPClientListener_ methods =
    {
        .headerReceived     = BoldHttpApiHeaderReceived,
        .transferFinished   = BoldHttpApiTransferFinished,
        .errorOccured       = BoldHttpApiErrorOccured,
        .dataChunkReceived  = BoldHttpApiDataChunkReceived,
        .transferCancelled  = BoldHttpApiTransferCancelled,
    };
    
    static SvType myType = NULL;

    if (unlikely(!myType))
    {
        SvTypeCreateManaged("BoldHttpApi",
                sizeof(struct BoldHttpApi_t),
                SvObject_getType(),
                &myType,
                SvObject_getType(), &objectVTable,
                SvHTTPClientListener_getInterface(),
                &methods,
                NULL);
    }
    return myType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Funciones de BoldHttpListener //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

SvLocal void BoldHttpListenerCleanHTTPRequest(BoldHttpListenerMessage self)
{
    if (!self) return;
    
    if(self->request)
        SvHTTPRequestCancel(self->request, NULL);
    
    SVTESTRELEASE(self->request);
    self->request = NULL;
}

static void BoldHttpListenerMessageHeaderReceived(SvGenericObject self_,
        SvHTTPRequest req, const char *name, const char *value)
{
    //SvLogNotice("********************* Header Received **********************");
    //SvLogNotice("%s: %s", name, value);
    //SvLogNotice("************************************************************");
}

static void BoldHttpListenerMessageTransferFinished(SvGenericObject self_,
        SvHTTPRequest req, off_t total)
{
    if (!self_) return;
    
    BoldHttpListenerMessage self = (BoldHttpListenerMessage) self_;
    BoldHttpListenerCleanHTTPRequest(self);
    //SvLogNotice("********************* Transfer finished ********************");
    //char temp_buff[2048];
    //memset(temp_buff, 0, 2048);
    //snprintf(temp_buff, self->buffer_offset + 1, "%s", self->buffer);
    //SvLogNotice("%s", temp_buff);
    //SvLogNotice("************************************************************");
    self->callback (self->callbackData, self->buffer, self->buffer_offset);
    memset (self->buffer, 0, HTTP_BUF_LEN);
    self->buffer_offset = 0;
    
    SVTESTRELEASE(self);
}

static void BoldHttpListenerMessageErrorOccured(SvGenericObject self_, SvHTTPRequest req, int httpError)
{
    if (!self_) return;
    
    BoldHttpListenerMessage self = (BoldHttpListenerMessage) self_;
    SvLogError("%s: [%02u] failed with error %d", __func__, self->reqID, httpError);
    BoldHttpListenerCleanHTTPRequest(self);
    self->callback (self->callbackData, NULL, 0);
}

static void BoldHttpListenerMessageDataChunkReceived(SvGenericObject self_,
        SvHTTPRequest req, off_t offset, const uint8_t *buffer, size_t length)
{
    if (!self_) return;
    
    BoldHttpListenerMessage self = (BoldHttpListenerMessage) self_;
    
    if (self->buffer_offset + (int)length >= HTTP_BUF_LEN)
    {
        SvLogError("%s: Buffer size exceeded. Buffer size (%d) actual size (%d) chunk length (%zu)", __func__, HTTP_BUF_LEN, self->buffer_offset, length);
        return;
    }
    
    memcpy (&self->buffer[self->buffer_offset], buffer, length);
    self->buffer_offset += (int)length;
    
    //SvLogNotice("********************* Data Chunk Received ******************");
    //SvLogNotice("BoldHttpListenerMessageDataChunkReceived: \"%s\" length=%zu", buffer, length);
    //SvLogNotice("************************************************************");
}

static void BoldHttpListenerMessageTransferCancelled(SvGenericObject self_, SvHTTPRequest req)
{

}

static void BoldHttpListenerMessageDestroy(void *self_)
{
    if (!self_) return;
    
    BoldHttpListenerMessage self = (BoldHttpListenerMessage) self_;
    BoldHttpListenerCleanHTTPRequest(self_);
    SVTESTRELEASE(self->engine);
}

static SvType BoldHttpListenerMessage_getType(void)
{
    static const struct SvObjectVTable_ objectVTable =
    {
        .destroy = BoldHttpListenerMessageDestroy
    };
    
    static struct SvHTTPClientListener_ methods =
    {
        .headerReceived     = BoldHttpListenerMessageHeaderReceived,
        .transferFinished   = BoldHttpListenerMessageTransferFinished,
        .errorOccured       = BoldHttpListenerMessageErrorOccured,
        .dataChunkReceived  = BoldHttpListenerMessageDataChunkReceived,
        .transferCancelled  = BoldHttpListenerMessageTransferCancelled,
    };
    
    static SvType myType = NULL;

    if (unlikely(!myType))
    {
        SvTypeCreateManaged("BoldHttpListenerMessage",
                sizeof(struct BoldHttpListenerMessage_t),
                SvObject_getType(),
                &myType,
                SvObject_getType(), &objectVTable,
                SvHTTPClientListener_getInterface(),
                &methods,
                NULL);
    }
    
    return myType;
}

SvLocal BoldHttpListenerMessage BoldHttpListenerMessageCreate(void)
{
    BoldHttpListenerMessage self = (BoldHttpListenerMessage) SvTypeAllocateInstance(BoldHttpListenerMessage_getType(), NULL);
    self->engine = (SvHTTPClientEngine) SvTypeAllocateInstance(SvHTTPClientEngine_getType(), NULL);
    SvHTTPClientEngineInit(self->engine, 1, NULL);
    memset (self->buffer, 0, HTTP_BUF_LEN);
    self->buffer_offset = 0;
    return self;
}


static int get_auth_value(BoldHttpApi self, unsigned const char* msg, unsigned char* value)
{

    const char* user_name = SvStringCString (self->user_name);
    const char* key       = SvStringCString (self->user_password);
    const char* iv        = SvStringCString (self->user_password);
    unsigned char msg_encrypted[1024];
    char msg_encoded[1024];
    char msg_encoded2[1024];
    unsigned char msg_tmp[1024];

    int len_msg            = 0;
    int len_value          = 0;
    int len_msg_encrypted  = 0;
    int len_msg_encoded    = 0;
    int len_msg_encoded2   = 0;
    int len_user_name;


    /* Initialise the library */
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);


    len_msg = strlen((char*)msg);
    len_msg_encrypted = BoldEncrypt (msg, len_msg, (unsigned char*)key, (unsigned char*)iv, msg_encrypted);
    len_msg_encoded = QBBase64Encode((const unsigned char*)msg_encrypted, len_msg_encrypted, msg_encoded);

    len_user_name = strlen(user_name);
    memcpy(msg_tmp,user_name, len_user_name);
    memcpy(&msg_tmp[len_user_name], msg_encoded, len_msg_encoded);
    
    len_msg_encoded2 = QBBase64Encode((const unsigned char*)msg_tmp, (len_user_name+len_msg_encoded), msg_encoded2);
    
    memcpy(value, "Basic ", 6);
    memcpy(&value[6], msg_encoded2, len_msg_encoded2); 
    len_value = len_msg_encoded2 + 6;
    
    return len_value;
}

//
// PROXY GET LINK TOKEN REQUEST 
//
BoldHttpApi BoldHttpApiProxyStartRequest(BoldHttpApi self, const char *URL, void * callbackData, BoldHttpApiCallback callback,SvErrorInfo *errorOut)
{

    SvErrorInfo error = NULL;
    SvHTTPRequest req = NULL;

    self->callback = callback;
    self->callbackData = callbackData; 
    
    BoldHttpApiCleanHTTPRequest(self);
    
    if (!self || !URL)
    {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                "NULL argument passed");
        goto fini;
    }

    req = (SvHTTPRequest) SvTypeAllocateInstance(SvHTTPRequest_getType(),&error);
    if (!req || !SvHTTPRequestInit(req, self->engine, SvHTTPRequestMethod_GET, URL,
            &error))
    {
        SVRELEASE(req);
        goto fini;
    }
    
    SvHTTPRequestSetTimeout(req, 60, NULL);

    if (!strncmp(URL, "https://", 8))
    {
        SvSSLParams ssl = (SvSSLParams) SvTypeAllocateInstance(
                SvSSLParams_getType(), &error);
        SvSSLParamsSetVerificationOptions(ssl, false, false, NULL, NULL);
        SvHTTPRequestSetSSLParams(req, ssl, NULL);
        SVRELEASE(ssl);
    }

    SvHTTPRequestSetUserAgent(req, SVSTRING("SvHTTPGet"), NULL);
    SvHTTPRequestSetListener(req, (SvGenericObject) self, true, NULL);
    SvHTTPRequestSetPriority(req, SvHTTPRequestPriority_max, NULL);
    SvHTTPRequestSetBufferSize(req, 256 * 1024, NULL);
    
    SvHTTPRequestStart(req, NULL);
    self->request = req;
    self->reqID = SvHTTPRequestGetID(req);
    
    fini: SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

//
// POST HTTP MESSAGE
//
BoldHttpApi BoldHttpApiPost(BoldHttpApi self, const char *URL, const char *data, void * callbackData, BoldHttpApiCallback callback, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SvHTTPRequest req = NULL;

    BoldHttpListenerMessage reqCallback = BoldHttpListenerMessageCreate();
    reqCallback->callback = callback;
    reqCallback->callbackData = callbackData;
    
    if (!self || !URL)
    {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                "NULL argument passed");
        goto fini;
    }
    
    req = (SvHTTPRequest) SvTypeAllocateInstance(SvHTTPRequest_getType(),&error);
    if (!req || !SvHTTPRequestInit(req, reqCallback->engine, SvHTTPRequestMethod_POST, URL,
            &error))
    {
        SVRELEASE(req);
        goto fini;
    }
    
    SvHTTPRequestSetTimeout(req, 60, NULL);
    
    if (!strncmp(URL, "https://", 8))
    {
        SvSSLParams ssl = (SvSSLParams) SvTypeAllocateInstance(
                SvSSLParams_getType(), &error);
        SvSSLParamsSetVerificationOptions(ssl, false, false, NULL, NULL);
        SvHTTPRequestSetSSLParams(req, ssl, NULL);
        SVRELEASE(ssl);
    }
    
    SvHTTPRequestSetUserAgent(req, SVSTRING("SvHTTPPost"), NULL);
    SvHTTPRequestSetPriority(req, SvHTTPRequestPriority_max, NULL);
    SvHTTPRequestSetBufferSize(req, 256 * 1024, NULL);
    
    SvHTTPRequestSetOutgoingData(req, SVSTRING("application/json"), (const uint8_t *)data, strlen(data), NULL);
    SvHTTPRequestSetListener(req, (SvGenericObject) reqCallback, true, NULL);
    SvHTTPRequestStart(req, NULL);
    reqCallback->request = req;
    reqCallback->reqID = SvHTTPRequestGetID(req);
    
    fini: SvErrorInfoPropagate(error, errorOut);
    
    return error ? NULL : self;
}


BoldHttpApi BoldHttpApiStartRequest(BoldHttpApi self,const char *URL,const unsigned char* msg, void * callbackData, BoldHttpApiCallback callback,SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SvHTTPRequest req = NULL;

    self->callback = callback;
    self->callbackData = callbackData; 
    
    BoldHttpApiCleanHTTPRequest(self);
    
    if (!self || !URL)
    {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                "NULL argument passed");
        goto fini;
    }

    req = (SvHTTPRequest) SvTypeAllocateInstance(SvHTTPRequest_getType(),&error);
    if (!req || !SvHTTPRequestInit(req, self->engine, SvHTTPRequestMethod_GET, URL,
            &error))
    {
        SVRELEASE(req);
        goto fini;
    }
    
    SvHTTPRequestSetTimeout(req, 5, NULL);

    if (!strncmp(URL, "https://", 8))
    {
        SvSSLParams ssl = (SvSSLParams) SvTypeAllocateInstance(
                SvSSLParams_getType(), &error);
        SvSSLParamsSetVerificationOptions(ssl, false, false, NULL, NULL);
        SvHTTPRequestSetSSLParams(req, ssl, NULL);
        SVRELEASE(ssl);
    }

    SvHTTPRequestSetUserAgent(req, SVSTRING("SvHTTPGet"), NULL);
    SvHTTPRequestSetListener(req, (SvGenericObject) self, true, NULL);
    SvHTTPRequestSetPriority(req, SvHTTPRequestPriority_max, NULL);
    SvHTTPRequestSetBufferSize(req, 256 * 1024, NULL);
    SvString auth_header_name = SVSTRING("Authorization");
    SvString content_type_header_name = SVSTRING("Content-Type");
    SvString content_type_header_value = SVSTRING("application/xml"); 
    
    if (msg)
    { 
        unsigned char auth_value [1024];
        int len = get_auth_value(self, msg, auth_value);
        auth_value[len]=0;
        SvString auth_header_value = SvStringCreate((char*)auth_value, NULL);    
        
        SvHTTPRequestSetHTTPHeader(req, auth_header_name, auth_header_value, NULL);
        SvHTTPRequestSetHTTPHeader(req, content_type_header_name, content_type_header_value, NULL);
        
        SvHTTPRequestStart(req, NULL);
        self->request = req;
        self->reqID = SvHTTPRequestGetID(req);
    
        SVRELEASE(auth_header_value);
    }
    else
    {
        SvHTTPRequestStart(req, NULL);
        self->request = req;
        self->reqID = SvHTTPRequestGetID(req);
    }
    SVRELEASE(auth_header_name);
    SVRELEASE(content_type_header_name);
    SVRELEASE(content_type_header_value);
    fini: SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}



static void BoldHttpApiDestroy(void *self_)
{
    BoldHttpApi self = (BoldHttpApi) self_;
    BoldHttpApiCleanHTTPRequest(self_);
    SVTESTRELEASE(self->user_name);
    SVTESTRELEASE(self->user_password);
    SVTESTRELEASE(self->engine);
    SVTESTRELEASE(self->id);
}


BoldHttpApi BoldHttpApiCreate(void)
{
    BoldHttpApi self = (BoldHttpApi) SvTypeAllocateInstance(BoldHttpApi_getType(), NULL);
    self->engine = (SvHTTPClientEngine) SvTypeAllocateInstance(SvHTTPClientEngine_getType(), NULL);
    SvHTTPClientEngineInit(self->engine, 1, NULL);
    memset (self->buffer, 0, HTTP_BUF_LEN);
    self->buffer_offset = 0;
    return self;
}


void BoldHttpApiSetCredentials (BoldHttpApi self, SvString user_name, SvString user_password)
{
    self->user_name = SVRETAIN(user_name);
    self->user_password = SVRETAIN(user_password);
}
