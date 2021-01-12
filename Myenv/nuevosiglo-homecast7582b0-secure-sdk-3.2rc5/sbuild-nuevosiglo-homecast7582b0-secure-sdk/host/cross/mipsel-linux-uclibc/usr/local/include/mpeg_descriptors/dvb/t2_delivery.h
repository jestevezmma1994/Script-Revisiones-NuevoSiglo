/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2017 Cubiware Sp. z o.o. All rights reserved.
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
 **       sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **       a commercial product sold or licensed by, or on behalf of, User.
 ****************************************************************************/

#ifndef MPEG_T2_DELIVERY_DESC_H
#define MPEG_T2_DELIVERY_DESC_H

#include <mpeg_psi_parser.h>
#include <stdint.h>
#include <stdbool.h>

/// This 2-bit field indicates the SISO/MISO mode.
enum mpeg_t2_delivery_siso_miso {
    mpeg_t2_delivery_siso_miso__siso = 0,
    mpeg_t2_delivery_siso_miso__miso,
};

/// This 4-bit field indicates the bandwidth in use.
enum mpeg_t2_delivery_bandwidth {
    mpeg_t2_delivery_bandwidth__8_MHz = 0,
    mpeg_t2_delivery_bandwidth__7_MHz,
    mpeg_t2_delivery_bandwidth__6_MHz,
    mpeg_t2_delivery_bandwidth__5_MHz,
    mpeg_t2_delivery_bandwidth__10_MHz,
    mpeg_t2_delivery_bandwidth__1_712_MHz
};

/**
 * Function converts bandwidth enum to int representation.
 *
 * @param[in] bandwidth         enum representation of bandwidth
 * @return                      int representation of bandwidth
 *
 **/
int mpeg_t2_delivery_bandwidth_to_int(enum mpeg_t2_delivery_bandwidth bandwidth);

/// This 3-bit field indicates the guard interval.
enum mpeg_t2_delivery_guard_interval {
    mpeg_t2_delivery_guard_interval__1_32 = 0,
    mpeg_t2_delivery_guard_interval__1_19,
    mpeg_t2_delivery_guard_interval__1_8,
    mpeg_t2_delivery_guard_interval__1_4,
    mpeg_t2_delivery_guard_interval__1_128,
    mpeg_t2_delivery_guard_interval__19_128,
    mpeg_t2_delivery_guard_interval__19_256
};

/// This 3-bit field indicates the FFT size of the signals transmitted within
/// the associated cell.
enum mpeg_t2_delivery_transmission_mode {
    mpeg_t2_delivery_transmission_mode__2k_mode = 0,
    mpeg_t2_delivery_transmission_mode__8k_mode,
    mpeg_t2_delivery_transmission_mode__4k_mode,
    mpeg_t2_delivery_transmission_mode__1k_mode,
    mpeg_t2_delivery_transmission_mode__16k_mode,
    mpeg_t2_delivery_transmission_mode__32k_mode
};


typedef struct {
    // List linkage node.
    list_linkage_t linkage;

    /// Field uniquely identifies a cell.
    uint16_t cell_id;
    union {
        struct tfs {
            /// Total length in bytes of the following loop that enumerates
            /// two to six centre frequencies belonging to the TFS.
            uint8_t frequency_loop_length;
            /// Frequency value table.
            uint32_t centre_frequencies[6];
        } tfs;
        /// Frequency value.
        uint32_t centre_frequency;
    };
    /// Total length in bytes of the following loop indicates the frequencies
    /// used in subcells.
    uint8_t subcell_info_loop_length;
    struct mpeg_t2_delivery_subcell {
        /// Field used to indentify a sub-cell within a cell.
        uint8_t cell_id_extension;
        /// Center frequency that is used by a transposer in the sub-cell.
        uint32_t transposer_frequency;
    } mpeg_t2_delivery_subcell[256];
} mpeg_t2_delivery_cell;

typedef struct {
    /// This 2-bit field indicates the SISO/MISO mode.
    enum mpeg_t2_delivery_siso_miso siso_miso;

    /// This 4-bit field indicates the bandwidth in use.
    enum mpeg_t2_delivery_bandwidth bandwidth;

    /// This 3-bit field indicates the guard interval.
    enum mpeg_t2_delivery_guard_interval guard_interval;

    /// This 3-bit field indicates the FFT size of the signals transmitted within
    /// the associated cell.
    enum mpeg_t2_delivery_transmission_mode transmission_mode;

    /// This 1-bit flag indicates whether other frequencies (non-TFS case) or
    /// other groups of frequencies (TFS case) are in use.  The value 0 (zero)
    /// indicates that the set of frequencies (non-TFS case) or the set of groups
    /// of frequencies (TFS case) included in the descriptor is complete, whereas
    /// the value 1 (one) indicates that the set is incomplete.
    bool other_frequency_flag;

    /// This 1-bit flag indicates whether a TFS arrangement is in place or not.
    bool tfs_flag;

    ///
    /// \brief cells
    ///
    list_t cells;
} mpeg_t2_delivery_system;


typedef struct {
    /// This 8-bit field uniquely identifies a data PLP within a T2 System, within
    /// a T2 Network.  The term is defined in EN 302 755.
    uint8_t plp_id;

    /// This 16-bit field uniquely identifies a T2 system within a T2 network.
    /// The term is defined in EN 302 755.
    uint16_t T2_system_id;

    /// The remaining part of this descriptor is only present once per T2 system,
    /// because parameters are uniquely applicable to all Transport Streams carried
    /// over a particular T2 system.
    mpeg_t2_delivery_system *delivery_system;
} mpeg_t2_delivery_desc;

/**
 * Parses mpeg_data to T2 descriptor.
 *
 * @param[out] desc     handle to descriptor
 * @param[in] data      raw data to parse
 * @return              0 on success, -1 on error
 **/
int mpeg_t2_delivery_desc_init(mpeg_t2_delivery_desc *desc, mpeg_data *data);

/**
 * Destructor of T2 descriptor.
 *
 * @param[in] desc      handle to descriptor
 *
 **/
void mpeg_t2_delivery_desc_destroy(mpeg_t2_delivery_desc *desc);

/**
 * Reports information about T2 descriptor.
 *
 * @param[in] desc              handle to descriptor
 * @param[in] indent_string     additional prefix string
 *
 **/
void mpeg_t2_delivery_desc_report(mpeg_t2_delivery_desc *desc, const char *indent_string);

#endif // MPEG_T2_DELIVERY_DESC_H
