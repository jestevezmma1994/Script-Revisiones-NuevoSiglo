/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013-2015 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef MPEG_CA_GENERATOR_H_
#define MPEG_CA_GENERATOR_H_

/**
 * @file mpeg_ca_generator.h Generator of CA information
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvPlayerKit/SvChbuf.h>
#include <dataformat/sv_data_format.h>
#include <SvPlayerKit/SvBuf.h>
#include <SvFoundation/SvData.h>
#include <mpeg_tables/pmt.h>
#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @defgroup mpeg_ca_generator MPEG CA generator class
 * @ingroup mpeg
 * @{
 **/

/**
 * The CA message sections shall have a maximum length of 256 bytes (ETR 289).
 **/
#define MPEG_ECM_MAX_PACKETS_CNT 2

/**
 * New ca generation modes
 */
typedef enum mpeg_ca_generator_new_ca_table_mode {
    mpeg_ca_generator_new_ca_table_mode_one_to_one_mapping, /**< for each existing CA pid new CA pid will be generated */
    mpeg_ca_generator_new_ca_table_mode_single_global_ca_pid, /**< new one global CA pid will be generated */
    mpeg_ca_generator_new_ca_table_mode_empty, /**< new CA won't be generated */
} mpeg_ca_generator_new_ca_table_mode;

/**
 * MPEG CA generator class.
 *
 * Keeps all necessary information which is needed to provide conditional
 * access informations for a service.
 *
 * When content is scrambled there is information in PMT table about used
 * CA systems and ECM PIDs. Using this object it it possible to replace these
 * information in PMT with new ones (unique ECM PIDs and provided new system id)
 * It is also possible to manage ECMs assigned to new ECM PIDs.
 *
 * @note This is not an SvObject.
 **/
typedef struct mpeg_ca_generator_s *mpeg_ca_generator;

/**
 * Create MPEG CA generator.
 *
 * @param[in] CA_system_ID Identificator of CAS
 * @return created generator handle
 **/
mpeg_ca_generator mpeg_ca_generator_create(int CA_system_ID);

/**
 * Clear information about last ECMs used.
 *
 * @param[in] self generator handle
 **/
void mpeg_ca_generator_clear_last_ecm(mpeg_ca_generator self);

/**
 * Destroy CA generator.
 *
 * @param[in] self generator handle
 **/
void mpeg_ca_generator_destroy(mpeg_ca_generator self);

/**
 * Check if selected PID needs to be reencrypted.
 *
 * @param[in] self generator handle
 * @param[in] PID selected PID
 * @return @c true if selected PID should be reencrypted, @c false if selected PID shouldn't be reencrypted.
 */
bool mpeg_ca_generator_pid_requires_reencryption(mpeg_ca_generator self, int PID);

/**
 * Get generated (by mpeg_ca_generator_recalculate_ca_pids() method) CA PID associated to given ES PID or query about global CA descriptor PID.
 *
 * @param[in] self generator handle
 * @param[in] ES_PID elementary stream PID. It is possible to query about generic CA PID setting PID as -1, then @a allowGeneric is ignored
 * @param[in] allowGeneric if @c true then it is allowed to return generic PID (if present) when exact match not found
 * @return CA PID associated to given elementary stream PID, @c -1 if not found
 */
int mpeg_ca_generator_get_ca_pid(mpeg_ca_generator self, int ES_PID, bool allowGeneric);

/**
 * @param[in] self generator handle
 * @return buffer with merged TS packets of last ECMs associated with all CA PIDs in service
 **/
SvBuf mpeg_ca_generator_get_ecms_for_all_pids(mpeg_ca_generator self);

/**
 * Update ECMs.
 *
 * @param[in] self generator handle
 * @param[in] ecm buffor with ts packets associated with single CA PID, these packets should contain full ECM section
 **/
void mpeg_ca_generator_update_last_ecm(mpeg_ca_generator self, SvBuf ecm);

/**
 * Update orginal information about service, and calculates new CA PID for each ES PID.
 *
 * @param[in] self generator handle
 * @param[in] format orginal information about service id
 * @param[in] anyCASystemId if @c true all PIDs from CA table will be used, in other case they will be filter by selected CA system id
 * @param[in] CA_system_ID if @a anyCASystemId is @c false then only PIDs with selected CA system id will be used
 * @param[in] CAMode mode of generation new CA data
 **/
void mpeg_ca_generator_recalculate_ca_pids(mpeg_ca_generator self, struct svdataformat* format, bool anyCASystemId, int CA_system_ID, mpeg_ca_generator_new_ca_table_mode CAMode);

/**
 * Replace CA descriptors in given PMT parser with new ones.
 * Before calling this method, mpeg_ca_generator_recalculate_ca_pids() should be called to assign new PIDs.
 *
 * @param[in] self generator handle
 * @param[in] parser PMT parser with parsed PMT section
 * @param[in] keepOld if @c true then old CA descriptors won't be removed
 **/
void mpeg_ca_generator_replace_ca_descriptors(mpeg_ca_generator self, mpeg_pmt_parser* parser, bool keepOld);

/**
 * Serialize ECM section into TS stream.
 *
 * @param[in] self generator handle
 * @param[in] original_ecm_pid pid of corresponding orginal ECM or -1 in single global ca mode
 * @param[in] section CA message section with ECM
 * @param[out] buf buffer for TS packets, it should contain at least (@ref MPEG_ECM_MAX_PACKETS_CNT * @ref MPEG_TS_PACKET_LENGTH) bytes
 * @return number of bytes written to buf
 **/
int mpeg_ca_generator_serialize_ecm_to_ts(mpeg_ca_generator self, int original_ecm_pid, SvData section, uint8_t *buf);

/**
 * @}
 **/

#ifdef  __cplusplus
}
#endif

#endif /* MPEG_CA_GENERATOR_H_ */
