/*
 *---------------------------------------------------------------------------
 *    Société          :   Quadrille Ingénierie    Copyright (c) 2009
 *    Projet           :   QuadriCast
 *    Sous-système     :   BDMDVBInterface
 *    Version          :   1.0
 *---------------------------------------------------------------------------
 *    Fichier          :   BDMDVBInterface.h
 *    Description      :   This header describes the BDMDVBInterface API
 *---------------------------------------------------------------------------
 *    Evolutions       :
 *      NC  02/08/11   :   Creation 
 *      SLE 30/03/12   :   Ajout du Unregister Tuner State Callback

 *---------------------------------------------------------------------------
 */
#ifndef _BDMDVBINTERFACE_H_
#define _BDMDVBINTERFACE_H_


/*!\enum TunerID
 * \brief the ID of a tuner.
 */
typedef enum
{
    PRIMARY_TUNER = 0, /*!< primary tuner*/
    SECONDARY_TUNER,   /*!< secondary tuner*/
    INVALID_TUNER,     /*!< invalid tuner*/
    TUNER_NOT_AVAILABLE,  /*!< tuner not disponible*/
} BDMDVBInterface_TunerID;


/*!\struct BDMDVBInterface_DVBChannel
 * \brief DVB triplet describing a specific DVB channel
 *
 */
typedef struct
{
    int TSID;
    int ONID;
    int PID;
} BDMDVBInterface_DVBChannel;



/*!\brief DVB Section reception notification callback
 *
 * <i>BDMDVBInterface_SectionReceivedCallBack</i> is an entry point pointer. The user application 
 * must supply such a pointer to its own receive notification treatment 
 * when calling the <i>BDMDVBInterface_CreateFilter()</i> entry point. the middleware
 * will call the supplied entry point each time a DVB section is received on
 * the DVB channel supplied to <i>BDMDVBInterface_CreateFilter()</i> . 
 *
 * \param nBufferLength  the length of the buffer containing the received DVB section.
 * \param buffer A pointer the buffer containing the received DVB section.
 * \param puserData Provided user back to the application.
 *
 * \return 0
 *
 */
typedef int (*BDMDVBInterface_SectionReceivedCallBack)(  int nBufferLength, char * buffer, const void * pUserData);


/*!\brief Tuner state change notification callback
 *
 * <i>BDMDVBInterface_TunerStateCallBack</i> is an entry point pointer. the middleware
 * will call the supplied entry point each time the state of one of the Tuners change . 
 *
 * \param tuner  the ID of the Tuner that its state changed.
 * \param state the NEW state of the Tuner.
 * \param pUserData Provided user back to the application.
 *
 * \return 0
 *
 */
typedef int (*BDMDVBInterface_TunerStateCallBack) (BDMDVBInterface_TunerID tuner,
                                                   const void * pUserData);

/*!\brief PMT change notification callback
 *
 * <i>BDMDVBInterface_PMTChangeCallBack</i> is an entry point pointer. the middleware
 * will call the supplied entry point each time the PMT change . 
 *
 * \param pUserData Provided user back to the application.
 *
 * \return 0
 *
 */
typedef int (*BDMDVBInterface_PMTChangeCallBack) (const void * pUserData);

#ifdef __cplusplus
extern "C" {
#endif

/*!\brief get the number of DVB channels.
 *
 * The <i>BDMDVBInterface_GetNumberOfDVBChannels()</i> entry point gets the number of DVB
 * channels related to a a push service 
 * 
 *\param serviceName : the name of the push service
 * 
 *\return the number of DVB channels.
 *
 */
 int BDMDVBInterface_GetNumberOfDVBChannels(char* serviceName);

/*!\brief get an array of DVBChannel specific to a push service .
 *
 * The <i>BDMDVBInterface_GetDVBChannelsFromService()</i> entry point gets the DVB
 * channels related to a push service 
 * 
 *\param serviceName : the name of the push service
 *\param pDVBChannels : table of the DVB channels (allocated by the caller)
 *\param nSizeOfDVBChannels : number of elements allocated in the table of the DVB channels 
 * 
 *\return the number of DVB channels filled in the tab if OK. -1 if Ko
 */
 int BDMDVBInterface_GetDVBChannelsFromService(char* serviceName,
                                               BDMDVBInterface_DVBChannel* pDVBChannels,
                                               int nSizeOfDVBChannels);

/*!\brief set a filter on a specic DVB channel .
 *
 * The <i>BDMDVBInterface_CreateFilter()</i> entry point set a filter on a specific DVB channel
 * 
 *\param channel : the DVB channel
 *\param callBack : the function called by the middleware each time a DVB section is received on the specific DVB channel
 *\param pTunerID : when successfuul indicates the tuner ID used to set the filter
 * 
 *\return 
 *   0 if OK
 *  -1 if Error
 *  -2 if No tuner is available to listen to the provided channel
 */
 int BDMDVBInterface_CreateFilter(BDMDVBInterface_DVBChannel channel,
                                  BDMDVBInterface_SectionReceivedCallBack callBack,
                                  void* pUserData,
                                  BDMDVBInterface_TunerID * pTunerID );

/*!\brief remove a filter from a specic DVB channel .
 *
 * The <i>BDMDVBInterface_RemoveFilter()</i> entry point remove a filter from a specific DVB channel
 * 
 *\param channel : the DVB channel
 *\param tunerID : the Tuner ID where the filter was set
 * 
 *\return 
 *   0 if OK
 *  -1 if Error
 */
 int BDMDVBInterface_RemoveFilter(BDMDVBInterface_DVBChannel channel,
                                  BDMDVBInterface_TunerID tunerID);


/*!\brief Register the Tuner State callback .
 *
 * The <i>BDMDVBInterface_RegisterTunerStateCallBack()</i> entry point Register the Tuner State callback
 * 
 *\param callBack : the Tuner State change callback
 *\param pUserData : Provided user data .
 * 
 *\return 
 */
 int BDMDVBInterface_RegisterTunerStateCallBack(BDMDVBInterface_TunerStateCallBack callBack,
                                                 void* pUserData);

/*!\brief Register the PMT Change callback .
 *
 * The <i>BDMDVBInterface_PMTChangeCallBack()</i> entry point Register the PMT Change callback
 * 
 *\param callBack : the PMT Change callback
 *\param pUserData : Provided user data .
 * 
 *\return 
 */
 int BDMDVBInterface_RegisterPMTChangeCallBack(BDMDVBInterface_PMTChangeCallBack callBack,
                                                 void* pUserData);

/*!\brief Register the PMT Change callback .
 *
 * The <i>BDMDVBInterface_PMTChangeCallBack()</i> entry point Register the PMT Change callback
 * 
 * 
 *\return 
 */
 int BDMDVBInterface_UnregisterPMTChangeCallBack(void);


/*!\brief Unregister the Tuner State callback .
 *
 * The <i>BDMDVBInterface_UnregisterTunerStateCallBack()</i> entry point Unregister the Tuner State callback
 * 
 *\param callBack : the Tuner State change callback
 *\param pUserData : Provided user data .
 * 
 *\return 
 */
 int BDMDVBInterface_UnregisterTunerStateCallBack(BDMDVBInterface_TunerStateCallBack callBack,
                                                 void* pUserData);

#ifdef __cplusplus
}
#endif


#endif //_BDMDVBINTERFACE_H_
