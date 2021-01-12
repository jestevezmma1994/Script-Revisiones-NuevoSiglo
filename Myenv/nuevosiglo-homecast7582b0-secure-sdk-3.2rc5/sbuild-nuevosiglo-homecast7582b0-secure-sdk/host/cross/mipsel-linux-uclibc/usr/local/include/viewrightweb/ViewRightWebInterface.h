#ifndef _VIEWRIGHTWEBINTERFACE_H_
#define _VIEWRIGHTWEBINTERFACE_H_

#define COMPANY_NAME_MAX_LENGTH 256

#if defined(WIN32) && (defined(VIEWRIGHTWEBCLIENT_EXPORTS) || defined(VIEWRIGHTWEBCLIENT_IMPORTS))
#ifdef VIEWRIGHTWEBCLIENT_EXPORTS
#define VIEWRIGHTWEBCLIENT_API __declspec(dllexport)
#else // VIEWRIGHTWEBCLIENT_EXPORTS
#define VIEWRIGHTWEBCLIENT_API __declspec(dllimport)
#endif // VIEWRIGHTWEBCLIENT_EXPORTS
#else // WIN32
#define VIEWRIGHTWEBCLIENT_API
#endif // WIN32

#include <cstdio>
#include <cstring>

class VCASCommunicationHandler;

static const char* errorStrings[] =
{
"VR_Success = 0 ",
"NoConnect = 1 ",
"GeneralError = 2 ",
"BadMemoryAlloc = 3 ",
"BadRandomNumberGenerator = 4 ",
"BadURL = 5 ",
"BadReply = 6",
"BadReplyMoved = 7",
"BadVerifyCertificateChain = 8",
"BadCreateKeyPair = 9",
"NotEntitled = 10",
"BadCreateStore = 11",
"BadWriteStore = 12",
"BadReadStore = 13",
"BadStoreIntegrity = 14",
"BadStoreFileNoExist = 15",
"BadCertificate = 16",
"BadINI = 17",
"BadPrivateKey = 18",
"BadConvertPEMToX509 = 19",
"BadPublicEncrypt = 20",
"BadAddX509Entry = 21",
"BadAddX509Subject = 22",
"BadX509Sign = 23",
"CantRetrieveBoot = 24",
"CantProvision = 25",
"BadArguments = 26",
"BadKeyGeneration = 27",
"NotProvisioned = 28",
"CommunicationObjectNotInitialized = 29",
"NoOTTSignature = 30",
"BadOTTSignature = 31",
"KeyFileNotEntitled = 32",
"CertificateExpired = 33",
"IntegrityCheckFailed = 34",
"SecurityError = 35",
"FeatureUnavailable = 36",
"NoUniqueIdentifier = 37",
"HandshakeError = 38",
"FailedKeyURLParse = 39",
"UnsupportedAssetType = 40",
"OfflineNotAllowed = 41",
"FailedNoKeysAvailableOffline = 42",
"VR_OfflineMode = 43",
"InvalidCertificateRequest = 44",
"BadHash = 45",
"FailedStoreMove = 46",
"GlobalPolicySecurityError = 47",
"AssetPolicySecurityError = 48",
"DashInfoUnavailable = 49"
};

class ViewRightWebInterface
{
public:
	int test;
    
    enum MethodInfo
	{
		NONE = 0,
        AES_128_CBC,
        AES_128_CTR
	};
    
	enum VRWebClientError_t
	{
		VR_Success = 0,
		NoConnect,
		GeneralError,
		BadMemoryAlloc,
		BadRandNumGen,
		BadURL,
		BadReply,
		BadReplyMoved,
		BadVerifyCertificateChain,
		BadCreateKeyPair,
		NotEntitled,
		BadCreateStore,
		BadWriteStore,
		BadReadStore,
		BadStoreIntegrity,
		BadStoreFileNoExist,
		BadCert,
		BadINI,
		BadPrivateKey,
		BadConvertPEMToX509,
		BadPublicEncrypt,
		BadAddX509Entry,
		BadAddX509Subject,
		BadX509Sign,
		CantRetrieveBoot,
		CantProvision,
		BadArguments,
		BadKeyGeneration,
		NotProvisioned,
		CommunicationObjectNotInitialized,
		NoOTTSignature,
		BadOTTSignature,
		KeyFileNotEntitled,
		CertificateExpired,
		IntegrityCheckFailed,
		SecurityError,
		FeatureUnavailable,
		NoUniqueIdentifier,
		HandshakeError,
		FailedKeyURLParse,
		UnsupportedAssetType,
		OfflineNotAllowed,
		FailedNoKeysAvailableOffline,
		VR_OfflineMode,
		InvalidCertificateRequest,
		BadHash,
		FailedStoreMove,
		GlobalPolicySecurityError,
		AssetPolicySecurityError,
        DashInfoUnavailable,
        LAST_ERROR_CODE,
	};
    
    struct _VRDashInfo
    {
        _VRDashInfo(): pssh(NULL), psshLength(0), mpd(NULL), mpdLength(0) { memset(keyID, 0, 16);}
        unsigned char keyID[16];
        unsigned char * pssh;
        unsigned long psshLength;
        unsigned char * mpd;
        unsigned long mpdLength;
    };

	virtual ~ViewRightWebInterface();

	ViewRightWebInterface();

	// Initialization

	VIEWRIGHTWEBCLIENT_API VRWebClientError_t SetVCASCommunicationHandlerSettings(const char * VCASBootAddress, const char * storePath, const char * companyName = "");
	VIEWRIGHTWEBCLIENT_API VRWebClientError_t SetVCASCommunicationHandlerSettings(const char * VCASBootAddress, const wchar_t * storePath, const char * companyName = "");
	VIEWRIGHTWEBCLIENT_API VRWebClientError_t InitializeSSL(void);
    VIEWRIGHTWEBCLIENT_API VRWebClientError_t MoveStoreInformation(const char * fromPath, const char * toPath);
    VIEWRIGHTWEBCLIENT_API VRWebClientError_t MoveStoreInformation(const wchar_t * fromPath, const wchar_t * toPath);
    VIEWRIGHTWEBCLIENT_API VRWebClientError_t IsDeviceProvisioned(const char * VCASBootAddress, const char * storePath, const char * companyName);
    VIEWRIGHTWEBCLIENT_API VRWebClientError_t IsDeviceProvisioned(const char * VCASBootAddress, const wchar_t * storePath, const char * companyName);

	// Provisioning and unique identifier

	VIEWRIGHTWEBCLIENT_API VRWebClientError_t ConnectAndProvisionDevice(void);
	VIEWRIGHTWEBCLIENT_API VRWebClientError_t CheckVCASConnection(void);
	VIEWRIGHTWEBCLIENT_API VRWebClientError_t GetUniqueIdentifier(char * * uniqueID, int * length);
    VIEWRIGHTWEBCLIENT_API VRWebClientError_t IsDeviceProvisioned(void);

	// Logging

	VIEWRIGHTWEBCLIENT_API void SetLogging(bool OnOff);

	// Version Info

	VIEWRIGHTWEBCLIENT_API const char * GetVersion(void);

	//Last error code
    
    VIEWRIGHTWEBCLIENT_API VRWebClientError_t GetLastErrorCode();
    
protected:

	bool m_bSSLInitialized;
    
    VRWebClientError_t m_lastErrorCode;	

	VCASCommunicationHandler * m_pVCASCommunicationHandler;

};

#endif
