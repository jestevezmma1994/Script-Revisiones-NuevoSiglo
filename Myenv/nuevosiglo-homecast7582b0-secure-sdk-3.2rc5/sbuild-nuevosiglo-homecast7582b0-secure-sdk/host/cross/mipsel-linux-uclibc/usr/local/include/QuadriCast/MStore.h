/*
 *---------------------------------------------------------------------------
 *    Société          :   Quadrille Ingénierie    Copyright (c) 2009
 *    Projet           :   Push VOD
 *    Sous-système     :   MStore
 *    Version          :   1.0
 *---------------------------------------------------------------------------
 *    Fichier          :   MStore.h
 *    Description      :   This header describs the MStore API
 *---------------------------------------------------------------------------
 *    Evolutions       :
 *      XB  12/08   :   Creation 
 *      OM  03/09   :   Ajout fonction notification 
 *      XB  12/09   :   Implémentation V2
 *      JPI  12/10   :   correction ANO #13
 *
 * JPO 28/10/2011 v1.2.3 : ANO 48, EVO 41, 18 & 21
 * JPI 23/06/2011 v2.0.0 : EVO #39 #15
 * CCA 07/02/2012 v3.0.0 : Ajout de la méthode PMStopPostProcessing
 * JPI 19/07/2012 v3.0.1 : Unused cFullXMLPath et cInterfaceForData dans PMPackDescriptor
 * JPI 04/09/2012 v3.1.0 : Rename PMGetContentsListV2 to PMGetContentsList 
 * JPI 06/09/2012 v3.1.0 : EVO #370 Ajout fonction PMGetKeyWord et PMGetKeyWordsNumber
 * JPO 18/10/2012 v3.2.0 : EVO #411 modification du type de PMPackHandle
 *                         packages are now identified by an integer value
 * JPI 13/11/2012 v3.2.0 : EVO #416 Ajout de la méthode PMGetDirectoryPath
*---------------------------------------------------------------------------
 */
#ifndef _MSTORE_H_
#define _MSTORE_H_

#ifdef OS21
#include "stdint.h"
#endif

#ifdef WIN32
typedef unsigned __int64     uint64_t;
// Project cubiware compilation intek
#elif BDM_LINUX
    # if __WORDSIZE == 64
    typedef unsigned long    uint64_t;
    # else
    typedef unsigned long long    uint64_t;
    #endif
#else
#include <stdint.h>
#endif

/*#elif BDM_LINUX
typedef unsigned long long    uint64_t;
#else
#include <stdint.h>
#endif*/
// Project cubiware compilation intek


#ifdef _UNICODE
    #include <wchar.h>
    #define TCHAR wchar_t
#else
    #define TCHAR char
#endif

/* 
 * includes
 * 
 */



#define PMPackHandle int
#define PMPackHandleSize (20)
#define PMURL char[256]

/*!\enum PMError
 * \brief Common error codes.
 *
 * These codes are returned by ESLDMLib methods and MPackManagerLib methods.
 */
typedef enum
{
    PM_E_NOERROR = 0, /*!<\brief no error */

    PM_E_LICENCE_CORRUPTED, /*!<\brief Error licence file 
    authentification failed. The MD5 diggest field into the licence file do 
    not correspond to the licence file content. For instance if something 
    is changed manually into the licence file (and the MD5 diggest not 
    updated accordingly), this error is returned by the <i>PMInit()</i> 
    entry point. */

    PM_E_NO_LICENCE,     /*!<\brief There is no licence file. Licence 
    file passed to the <i>PMInit()</i> entry point must correspond to a full
    path and file name into the host system. Otherwise this error is returned. */
  
    PM_E_BAD_LICENSE,  /*!<\brief There is a licence file but not for a receiver. 
    For example, you try to launch a receiver with a sender license file. */

    PM_E_NO_PROTOCOL_FILE, /*!<\brief There's no protocol defaults file. 
    This file contains every protocol specific parameters default values and
    some internal working parameters. Protocol default file passed to the 
    <i>EsldmOpen()</i> entry point must correspond to a full path and file name 
    into the host system. Otherwise this error is returned. */

    PM_E_BAD_PROTOCOL_FILE, /*!<\brief The protocol file you try to use is not
    recognized by the API. */

    PM_E_SLDM_INITERROR, /*!<\brief The SLDM library couldn't be initialised. 
                        this error is returned by the <i>PMInit()</i> entry point */
    PM_E_LISTEN,    /*!<\brief Cannot or stop launch a listening service, 
                        <i>PMStartService()</i> or <i>PMStopService()</i> has failed */
    PM_E_NOWORKSPACE, /*!<\brief the workspace's path provided at the initialisation doesn't exist */
    PM_E_NOLOGVOLUMEMOUNTPOINT, /*!<\brief the log volume mount point path provided at the initialisation doesn't exist */
    PM_E_NOINIFILE, /*!<\brief the .ini file provided at the initialisation doesn't exist */
    PM_E_PBINIFILE, /*!<\brief a parameter not found in the .ini file. Look the log file */ 
    PM_E_FILENOTEXIST, /*!<\brief the file doesn't exist */     
    PM_E_IMPOSSIBLETODELETE, /*!<\brief impossible to delete the file */
    PM_E_FILEALREADYPLAYED,   /*<\brief file alredy played*/
    PM_E_CONTENTNOTEXIST,   /*<\brief unknowmn content*/     
    PM_E_UNKNOWNNATTRIBUTE,     /*<\brief unknowmn content attribute*/   
    PM_E_BADTYPE,   /*<\brief content has not the expected type*/
    PM_E_TOOLONGURL, /*<\brief one url is longer than MaxUrlLen*/

    PM_E_SERVICE_NOT_FOUND,  /*<\brief service not found*/
    PM_E_SERVICE_ALREADYCREATED,  /*<\brief service has been already created*/
    PM_E_PURGEMODE_NOT_FOUND,   /*<\brief purge mode not found*/
    PM_E_BADSTORAGEFILE,   /*<\brief storage file is not valid*/
    PM_E_CAN_NOT_STOP_SERVICES, /*<\brief services can't be stopped*/
    PM_E_CAN_NOT_DELETE_SERVICES, /*<\brief services can't be deleted*/
    PM_E_LIB_NOT_INITIALIZED,  /*<\brief services can't be created, started, stopped, deleted,
                                    if the libraries are not initialized */
    PM_E_CAN_NOT_GET_PROGRESS,  /*<\brief content progress can't be obtained*/
    PM_E_CAN_NOT_GET_BITMAP,    /*<\brief content bitmap can't be obtained*/
    PM_E_IMPOSIBLETOCHANGE,     /*<\brief key words can't be modified*/
    PM_E_NOKEYWORDS,     /*<\brief there is not key words*/

    PM_E_CANTSTOPPOSTPROCESSING, /*<\brief can't stop the post processing, need to wait till the end of the processing*/
    PM_E_CANTRESUMEPOSTPROCESSING, /*<\brief can't resume the current process*/
}
PMError;


/*!\enum PMPackType
 * \brief Pack types managed by the application.
 *
 */
typedef enum
{
    PM_VOD=0, /*!<\brief received pack is a VOD pack with XML file and video file */ 
    PM_EPG,   /*!<\brief not yet implemented */ 
    PM_CATCHUP, /*!<\brief received pack is a Catch Up pack with XML file for PVR programming */ 
    PM_SOFT, /*!<\brief not yet implemented */
    PM_ORDER, /*!<\brief order file */
}
PMPackType;


/*!\enum PMPurgeMode
 * \brief Defines the two modes for purge process.
 *
 */
typedef enum
{
    PM_FIFO=0, /*!<\brief if HDD size needed, oldest packs are deleted first */ 
    PM_VALIDITY, /*!<\brief A pack is removed as soon as its validity period is ended */
}
PMPurgeMode;


/*!\enum PMAnnounceProtocol
 * \brief Defines the two different announce protocols.
 *
 */
typedef enum
{
    PM_VFDPV2_A = 0, /*!<\brief always use this one */ 
    PM_MACPV1_A,
    PM_VFDPV3_A,
}PMAnnounceProtocol;


/*!\enum PMNetworkMode
 * \brief Defines the two different network mode (DVB or IP)
 *
 */
typedef enum
{
    PM_IP = 0, 
    PM_DVB,
}PMNetworkMode;


/*!\struct PMNetworkParameters
 * \brief parameters for paquet reception
 *
 */
typedef struct 
{
    PMNetworkMode  mode;    
    const char*   cAddress; /*!< Either a PID or a multicast address */
    const char*   cInterface; /*!< Either a ethernet interface or MUX interface */
    unsigned short udpPort; /*!< Used only in IP mode ; refer to MUX configuration file sample : 4001 ... */
    const char*   cServiceName; /*!< The Push VOD Service name */
    const char*   cProtocol;    /*!< The protocol */
}
PMNetworkParameters;


/*!\brief PMService process handle type
 *
 * A <i>PMServiceHandle</i> uniquely identifies the service process 
 * within the library */
typedef const void* PMServiceHandle;


/*!\struct PMStorageParameters
 * \brief parameters for a storage area
 *
 */
typedef struct 
{
    const char*  VolumeMountpoint; 
    const char*  fullPath; /*!< path from de volumeMoutPoint */
    uint64_t maxSize;      /*!< max size of the storage area in octets */
    PMPurgeMode mode;      /*!< mode for purge process */
}
PMStorageParameters;


/*!\struct PMDate
 * \brief 
 *
 */
typedef struct 
{
    unsigned short year; 
    unsigned short month; /*!< month 1 based */
    unsigned short day; 
}
PMDate;


/*!\struct PMDateHour
 * \brief 
 *
 */
typedef struct 
{
    unsigned short year; 
    unsigned short month; /*!< month 1 based */
    unsigned short day; 
    unsigned short hour; 
    unsigned short min; 
    unsigned short sec; 
}
PMDateHour;

/*!\enum PMNotifyEvent
 * \brief storage notification event codes 
 *
 * This codes represent significant events notified to the user application
 * 
 *
 * 
 */
typedef enum
{
    PM_NTY_PACK_ANNOUNCED = 1,      /*!< Package download has been announced (or re-announced) */
    PM_NTY_PACK_ADDED,              /*!< A package has been added (correctly received and extracted) */
    PM_NTY_PACK_REMOVED,            /*!< A package has been removed (too old, or at the request of the head end) */
    PM_NTY_PACK_DOWNLOAD_STOPPED,   /*!< A package download has been stopped (error, or abort from the head end) */
    PM_NTY_PACK_UPDATE,             /*!< Package metadata has been updated */
    PM_NTY_PACK_VALIDITYDATE,       /*!< A package is available */
    PM_NTY_PACK_EXPIREDDATE         /*!< A package is no more available */
}
PMNotifyEvent;


/*!\struct PMPackDescriptor
 * \brief Pack descriptor
 *
 * A <i>PMPackDescriptor</i> is a data structure that is provided to the 
 * user application by notification <i>PMStorageCallback</i>
 * upon pack has been added <i>PM_NTY_PACK_ADDED</i> or pack has been removed 
 * <i>PM_NTY_PACK_REMOVED</i>. It holds the file description
 */
typedef struct 
{
    PMPackHandle    contentID;
    PMPackType      type;
    const char*     cFullXMLPath; /*!< Unused */
    const char*     cInterfaceForData; /*!< Unused */
}
PMPackDescriptor;

/*!\struct PMPackAnnounce
 * \brief Pack Announce
 *
 * A <i>PMPackAnnounce</i> is a data structure that is provided to the 
 * user application by notification <i>PMStorageCallback</i>
 * upon pack announce has been added <i>PM_NTY_PACK_ANNOUNCED</i>
 * It holds the contend ID and multicast address for data reception
 */
typedef struct 
{
    PMPackHandle    contentID;      
    const char*     cInterfaceForData; /*!< Interface which must be used for data reception */
}
PMPackAnnounce;



/*!\struct PMRecordingInstruction
 * \brief Recording instruction
 *
 * A <i>PMRecordingInstruction</i> is a data structure which holds instructions 
 * for a PVR recording 
 */
typedef struct 
{
    PMPackHandle    contentID;      
    PMDateHour      EPGDate;
    PMDateHour      EITDate;
    unsigned short  EITEventID;
    unsigned short  TSID;
    unsigned short  OriginalNetworkID;
}
PMRecordingInstruction;






/*!\brief Storage notification callback
 *
 * <i>PMStorageCallback</i> is an entry point pointer. The user application 
 * must supply such a pointer to its own receive notification treatment 
 * when calling the <i>PMStartService()</i> entry point. MStore library 
 * instance will call the supplied entry point each time a pack is added or removed. 
 * Event nature are precised by a <i>PMNotifyEvent</i> value.
 *
 * \param event An <i>PMNotifyEvent</i> code detailing the packs life cycle.
 *
 * \param data A pointer the user application should C-Style cast into the 
 * appropriate type accoring to <i>PMNotifyEvent</i> in the following way:
 *
 * - <i>PM_NTY_PACK_ANNOUNCED</i>: data is a <i>PMPackAnnounce</i>
 * - <i>PM_NTY_PACK_ADDED</i>: data is a <i>PMPackDescriptor</i>
 * - <i>PM_NTY_PACK_REMOVED</i>: data is a <i>PMPackDescriptor</i>
 * - <i>PM_NTY_PACK_DOWNLOAD_STOPPED</i>: data is a <i>PMPackDescriptor</i>
 * - <i>PM_NTY_PACK_UPDATE</i>: data is a <i>PMPackDescriptor</i>
 * - <i>PM_NTY_PACK_VALIDITYDATE</i>: data is a <i>PMPackDescriptor</i>
 * - <i>PM_NTY_PACK_EXPIREDDATE</i>: data is a <i>PMPackDescriptor</i>
 *
 * \param userData Provided user back to the application.
 *
 * \return 
 *   in PM_NTY_PACK_ANNOUNCED event, return -1 if multicast address cannot be
 *  activated (mux unavailable) and 0 otherwise.
 *   in PM_NTY_PACK_ADDED event, return -1 if something wrong with xml file
 *   and 0 otherwise.*
 *   in PM_NTY_PACK_DOWNLOAD_STOPPED event, return -1 if failed and 0 otherwise.
 *
 */
 typedef int (*PMStorageCallback) (PMNotifyEvent event,
                                   void *data,  
                                   const void *userData);


#ifdef __cplusplus
extern "C" {
#endif


/*!\brief Init MStore library.
 *
 * The <i>PMInit()</i> entry point performs all required system initializations.
 * It must be call prior to any other entry point. It also checks the 
 * licence file, sets the working pathname and sets the tcp server port 
 * (in other words, the information that are normally found into the command 
 * line argument and licence file in the PC implementation).
 * 
 *\param LogVolumeMountpoint Mounted point volume for log files
 *
 *\param licenceFilePath A full pathname for the licence file into the file system.
 * The method checks the licence authentication. If NULL, hardcoded licence
 *
 *\param esldmInitFilePath A full pathname for the esldminit file into the file system.
 * If NULL, hardcoded parameters
 *
 *\param initFilePath A full pathname for MStore init file.The method checks 
 * the parameters od this file.If NULL, hardcoded parameters
 *
 *\param workspace A full pathname into the host file system. All 
 * subsequent pathnames will be relative to this one.
 *
 *\return a <i>PMError</i> code.
 *
 */
 PMError PMInit(const char*  LogVolumeMountpoint,
                const char*  licenceFilePath,    /* can be NULL */
                const char*  esldmInitFilePath,  /* can be NULL */
                const char*  initFilePath,       /* can be NULL */
                char*   workspace);


/*!\brief Close MStore library.
 *
 * The <i>PMClose()</i> entry point performs all required system closing.
 *
 *\return a <i>PMError</i> code.
 *
 */
 PMError PMClose(void);


/*!\brief Create a service.
 *
 * The <i>PMCreateService()</i> entry point performs all required initializations for 
 * a service creation. 
 * It must be call prior to start a service. 
 * 
 *\param type : service type (VOD, CATCH UUP)
 * 
 *\param networkParameter Parameters for network listenning
 *
 *\param storageParameters Parameters for storage area. All packages received
 * within this service will be saved in the storage area        
 *
 *\param userData Provided user back to the application.
 *
 *\param storageCallbackEntry Pointer to the StorageCallback
 *
 *\param serviceHandle A pointer to a <i>PMServiceHandle</i> to be filled.
 * Further call to the service will reuse this handle.
 *
 *\param keyWordTab A list of the diferents key words for this service.
 *
 *\param sizeKeyWordTab. The number of the words in the table keyWordTab
 *
 *\return a <i>PMError</i> code.
 *
 */
 PMError PMCreateService(PMPackType type,
                         const PMNetworkParameters   networkParameter,
                         const PMStorageParameters   storageParameters,
                         const void*                 userData,
                         PMStorageCallback           storageCallbackEntry,
                         PMServiceHandle*            serviceHandle,
                         char**                      keyWordTab,
                         int                         sizeKeyWordTab);



/*!\brief Start a service 
 *
 *\param serviceHandle  The <i>PMServiceHandle</i> to identify the service
 *
 *\return a <i>PMError</i> code.
 *
 */
 PMError PMStartService(PMServiceHandle serviceHandle);


/*!\brief Stop a service. 
 *
 *\param serviceHandle  The <i>PMServiceHandle</i> to identify the service
 *
 *\return a <i>PMError</i> code.
 *
 */
 PMError PMStopService(PMServiceHandle serviceHandle);

/*!\brief Stop and destroy a service. 
 *\warning After calling this function, the service cannot be used anymore
 *
 *\param serviceHandle  The <i>PMServiceHandle</i> to identify the service
 *
 *\return a <i>PMError</i> code.
 *
 */
 PMError PMDeleteService(PMServiceHandle serviceHandle);

/*!\brief Stop all services 
 *
 *\return a <i>PMError</i> code.
 *
 */
 PMError PMStopAllServices(void);

/*!\brief Stop and destroy all services. 
 *\warning After calling this function, the services cannot be used anymore
 *
 *\return a <i>PMError</i> code.
 *
 */
 PMError PMDeleteAllServices(void);

/*!\brief This function notify that a content has been played. a content can't 
 * be available to the user for more than 48h starting from the the date of the
 * first playing of the content.
 *\param contentID  The <i>PMPackHandle</i> to identify the pack
 *
 *\return a <i>PMError</i> code : PM_E_NOERROR : content play successfully notified
 *                                or PM_E_FILENOTEXIST : content doesn't exist
 *                                or PM_E_FILEALREADYPLAYED : content play already notified
 *
 */
 PMError PMPackPlayed(PMPackHandle contentID);


/*!\brief This function allows client application to get the contents number. 
 *
 *\param cEditorName  A string to identify the editor from which the application wants to get the contents list.
 *                   If the EditorName is empty, that means the application wants all the contents.  
 *\return a <i>unsigned int</i> Found contents number 
 *
 */
unsigned int PMGetContentsNumber(const char* cEditorName);


/*!\brief This function allows client application to get a contents list. 
 *
 *\param cEditorName  A string to identify the editor from which the application wants to get the contents list.
 *                   If the EditorName is empty, that means the application wants all the contents.
 *\param contentsTab  <i>PMPackHandle</i> array of the selected contents
 *                    There is a double allocation to do
 *                    This table and every PMPackHandle must be allocated by the client application
 *                    PMPackHandle has a size of PMPackHandleSize 
 *\param usMaxContents   The len of the allocated array, means that the returned array would not 
 *                  contained more than usMaxContents. If the content number is higher, only first len contents  
 *                  handle are stored in the array 
 *\return a <i>unsigned int</i> contents number in the array 
 *
 */
unsigned int PMGetContentsList(const char* cEditorName,
                                 PMPackHandle* contentsTab,
                                 unsigned short usMaxContents);


/*!\brief This function allows client application to get the validity period of a Pack 
 *
 *\param contentID  The <i>PMPackHandle</i> to identify the pack
 *\param startDate  A pointer to a <i>PMDateHour</i> to be filled by the function to specify validity start date
 * of the content
 *\param endDate  A pointer to a <i>PMDateHour</i> to be filled by the function to specify validity end date
 * of the content
 *\return a <i>PMError</i> code (PM_E_NOERROR or PM_E_FILENOTEXIST)
 *
 */
PMError PMGetPackValidityPeriod(PMPackHandle contentID, PMDateHour* startDate, PMDateHour* endDate);


/*!\brief This function allows client application to get a content attribute in string format.
 *
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *\param cAttribute To define the attribute path(the same string than in the XML file) 
 *                  e.g. "content/description/longSummary"
 *\param cValue     Characters buffer in which the value of the attribute will be copied. 
 *                  This buffer is allocated by the user application and will
 *                  desallocate by the user application too
 *\param usMaxlen   The len of the allocated buffer, means that the returned string would not 
 *                  exceed this len. If the original string is longer, the returned string is the 
 *                  original string truncated. In return the len of the value od the returned buffer 
 *\return a <i>PMError</i> PM_E_NOERROR : content successfully deleted
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *                      or PM_E_UNKNOWNNATTRIBUTE : this attribute doesn't exist
 *
 */
PMError PMGetContentStringAttribute(PMPackHandle contentID,
                                    const char* cAttribute,
                                    TCHAR*  cValue,
                                    unsigned short* usMaxlen);



/*!\brief This function allows client application to get a content attribute in integer format. 
 *
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *\param cAttribute To define the attribute path(the same string than in the XML file) 
 *                  e.g. "content/description/productionDate"
 *\param iValue     Value of the attribute
 *\return a <i>PMError</i> PM_E_NOERROR : content successfully deleted
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *                      or PM_E_UNKNOWNNATTRIBUTE : this attribute doesn't exist
 *
 */
PMError PMGetContentIntAttribute(PMPackHandle contentID,
                                const char* cAttribute,
                                int*     iValue);


/*!\brief This function allows client application to get a content attribute in double format. 
 *
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *\param cAttribute To define the attribute path(the same string than in the XML file) 
 *                  e.g. "content/description/price"
 *\param dValue     Value of the attribute
 *\return a <i>PMError</i> PM_E_NOERROR : content successfully deleted
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *                      or PM_E_UNKNOWNNATTRIBUTE : this attribute doesn't exist
 *
 */
PMError PMGetContentDoubleAttribute(PMPackHandle contentID,
                                    const char* cAttribute,
                                    double*     dValue);


/*!\brief This function get the size of the metadata file.
 *
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *
 *\param usLen   The len of the metadata file
 *\return a <i>PMError</i> PM_E_NOERROR : content successfully deleted
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *
 */
PMError PMGetSizeOfMetadataFile(PMPackHandle contentID,
                                unsigned long* usLen);


/*!\brief This function allows client application to get the metadata file in a buffer.
 *
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *
 *\param cBuffer    Characters buffer in which the value will be copied. 
 *                  This buffer is allocated by the user application and will
 *                  desallocate by the user application too
 *\param usMaxlen   The len of the allocated buffer, means that the returned string would not 
 *                  exceed this len. If the original string is longer, the returned string is the 
 *                  original string truncated. In return the len of the value od the returned buffer 
 *\return a <i>PMError</i> PM_E_NOERROR : content successfully deleted
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *
 */
PMError PMGetAllContentAttributes(PMPackHandle contentID,
                                 TCHAR* pBuffer,
                                 unsigned long* usMaxlen);


/*!\brief This function allows client application to get the content directory path.
 *
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *
 *\param cValue     Value of the directory path
 *\param usMaxlen   The len of the allocated buffer, means that the returned string would not 
 *                  exceed this len. If the original string is longer, the returned string is the 
 *                  original string truncated. In return the len of the value od the returned buffer 
 *\return a <i>PMError</i> PM_E_NOERROR : content successfully deleted
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *                      or PM_E_TOOLONGURL : if url length is longer than iMaxLen
 *
 */
PMError PMGetDirectoryPath(PMPackHandle contentID,
                           char* cValue,
                           unsigned short* usMaxlen);


/*!\brief This function allows client application to get a content attribute in dateHour format. 
 * Can be used to get date with hour, or duration
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *\param cAttribute To define the attribute (the same string than in the XML file) 
 *\param dhValue     Value of the attribute
 *\return a <i>PMError</i> PM_E_NOERROR : content successfully deleted
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *                      or PM_E_UNKNOWNNATTRIBUTE : this attribute doesn't exist
 *
 */
PMError PMGetContentDateHourAttribute(PMPackHandle contentID,
                                      const char* cAttribute,
                                      PMDateHour    * dhValue);


/*!\brief This function allows client application to get recording instruction for a content. 
 *
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *\param pInstruction <i>PMRecordingInstruction</i> holds the instructions
 *
 *\return a <i>PMError</i> PM_E_NOERROR : content successfully deleted
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *                      or PM_E_BADTYPE :  content is not a catch up type one
 *
 */
PMError PMGetRecordingInstructions(PMPackHandle contentID,
                                   PMRecordingInstruction* pInstruction);


/*!\brief This function allows client application to get a content media. 
 *
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *\param videoUrl  <i>char*</i> url to get the video file
 *\param pictureUrl  <i>char*</i> url to get the picture file
 *\param trailerUrl  <i>char*</i> url to get the trailer file
 *\param thumbnailUrl  <i>char*</i> url to get the thumbnail file
 *\param iMaxUrlLen  <i>unsigned short</i> le max len a the url buffer
 *
 *\return a <i>PMError</i> PM_E_NOERROR : content successfully deleted
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *                      or PM_E_TOOLONGURL : if one url length is longer than iMaxUrlLen
 */
PMError PMGetContentMedia(PMPackHandle          contentID,
                          char*                 videoUrl,
                          char*                 pictureUrl,
                          char*                 trailerUrl,
                          char*                 thumbnailUrl,
                          unsigned short        iMaxUrlLen);


/*!\brief This function allows client application to delete a content media.
 * 
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *
 *\return a <i>PMError</i> PM_E_NOERROR : content successfully deleted
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 */
PMError PMDeleteContent(PMPackHandle contentID);


/*!\brief This function allows client application to get the content progress.
 * 
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *\param pNbrTotalPackets <i>int*<i/> to get the total packets
 *\param pNbrReceivedPackets <i>int*<i/> to get the received packets
 *
 *\return a <i>PMError</i> PM_E_NOERROR : content progress obtained
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *                      or PM_E_CAN_NOT_GET_PROGRESS : content progress doesn't obtained
 */
PMError PMGetContentProgress(PMPackHandle       contentID,
                             int*               pNbrTotalPackets,
                             int*               pNbrReceivedPackets);


/*!\brief This function allows client application to get the content bitmap size.
 * 
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *\param size <i>unsigned long*<i/> to get the bitmap size
 *
 *\return a <i>PMError</i> PM_E_NOERROR : content bitmap size obtained
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *                      or PM_E_CAN_NOT_GET_BITMAP : content bitmap size doesn't obtained
 */
PMError PMGetContentBitmapSize(PMPackHandle         contentID,
                               unsigned long*       ulSize);


/*!\brief This function allows client application to get the content bitmap.
 * 
 *\param contentID  <i>PMPackHandle</i> to identify the content.
 *\param pBitmap <i>unsigned char*<i/> to get the bitmap
 *
 *\return a <i>PMError</i> PM_E_NOERROR : content bitmap obtained
 *                      or PM_E_CONTENTNOTEXIST : content doesn't exist
 *                      or PM_E_CAN_NOT_GET_BITMAP : content bitmap not obtained
 */
PMError PMGetContentBitmap(PMPackHandle         contentID,
                           unsigned char*       pBitmap);


/*!\brief This function allows client application to set the key words.
 * 
 *\param serviceID  <i>PMPackHandle</i> to identify the service.
 *\param KeyWordTab <i>char**<i/> table of the key words
 *\param size   <i>int<i/> size of table. Number of words
 *
 *\return a <i>PMError</i> PM_E_NOERROR : key words set
 *                      or PM_E_SERVICE_NOT_FOUND : service doesn't exist
 *                      or PM_E_IMPOSIBLETOCHANGE : key words not set
 */
PMError PMSetKeyWord(PMServiceHandle    serviceID,
                     char**             pKeyWordTab,
                     int                nSize);


/*!\brief This function allows client application to get the keywords number. 
 *
 *\param serviceID  <i>PMServiceHandle</i> to identify the service.
 *\param usKeyWordsNumber <i>unsigned short*</i> keywords number
 *\return a <i>PMError</i> PM_E_NOERROR : key words obtained
 *                      or PM_E_SERVICE_NOT_FOUND : service doesn't exist 
 *                      or PM_E_NOKEYWORDS : there is not key words
 *
 */
PMError PMGetKeyWordsNumber(PMServiceHandle serviceID, 
                            unsigned short* usKeyWordsNumber);


/*!\brief This function allows client application to get a keywords list. 
 *
 *\param cServiceID  <i>PMPackHandle</i> to identify the service.
 *\param pkeyWordsTab  <i>PMPackHandle</i> array of the selected keywords
 *                    There is a double allocation to do
 *                    This table and every PMPackHandle must be allocated by the client application
 *                    PMPackHandle has a size of PMPackHandleSize 
 *\param usMaxKeyWords   The len of the allocated array, means that the returned array would not 
 *                  contained more than usMaxKeyWords. If the keywords number is higher, only first len keywords  
 *                  handle are stored in the array 
 *\return a <i>PMError</i> PM_E_NOERROR : key words obtained
 *                      or PM_E_SERVICE_NOT_FOUND : service doesn't exist 
 *                      or PM_E_NOKEYWORDS : there is not key words
 *
 */
PMError PMGetKeyWordsList(PMServiceHandle serviceID,
                          char** pkeyWordsTab,
                          unsigned short usMaxKeyWords);

/*!\brief This function allows client application to suspend post processings if ressources are needed.
 * 
 *
 *\return a <i>PMError</i> PM_E_NOERROR : post processings suspended
 *                      or PM_E_CANTSTOPPOSTPROCESSING : can't stop the current process
 */
PMError PMSuspendPostProcessing(void);

/*!\brief This function allows client application to resume post processings.
 * 
 *
 *\return a <i>PMError</i> PM_E_NOERROR : post processings suspended
 *                      or PM_E_CANTRESUMEPOSTPROCESSING : can't resume the current process
 */
PMError PMResumePostProcessing(void);

/*!\brief This function allows client application to stop post processings.
 * 
 *
 *\return a <i>PMError</i> PM_E_NOERROR : post processings stopped
 */
PMError PMStopPostProcessing(void);

/* brief cette fonction est un point d'entrée pour les tests de portage quadrille*/
int TestQuadrille(void);
int TestQuadrille2(int keynum);


/* Internal use */

void DumpContentDB(const char* path,const char* filename);


#ifdef __cplusplus
}
#endif


#endif //_MSTORE_H_
