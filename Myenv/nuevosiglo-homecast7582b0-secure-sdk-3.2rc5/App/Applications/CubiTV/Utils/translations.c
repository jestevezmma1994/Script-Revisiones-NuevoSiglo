/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#include "translations.h"

#include <libintl.h>

const char *
QBTunerModulationToTranslatedString(QBTunerModulation modulation)
{
    switch (modulation) {
        case QBTunerModulation_unknown: return gettext("unknown");
        case QBTunerModulation_QAM16:   return gettext("QAM16");
        case QBTunerModulation_QAM32:   return gettext("QAM32");
        case QBTunerModulation_QAM64:   return gettext("QAM64");
        case QBTunerModulation_QAM128:  return gettext("QAM128");
        case QBTunerModulation_QAM256:  return gettext("QAM256");
        case QBTunerModulation_QPSK:    return gettext("QPSK");
        case QBTunerModulation_8PSK:    return gettext("8PSK");
        default:
                                        return gettext("???");
    }
}

const char *
QBTunerPolarizationToTranslatedString(QBTunerPolarization polarization)
{
    switch (polarization) {
        case QBTunerPolarization_unknown:     return gettext("unknown");
        case QBTunerPolarization_Vertical:    return gettext("Vertical");
        case QBTunerPolarization_Horizontal:  return gettext("Horizontal");
        default:
                                              return gettext("???");
    }
}

const char *
QBTunerPolarizationToTranslatedShortString(QBTunerPolarization polarization)
{
    switch (polarization) {
        case QBTunerPolarization_unknown:     return gettext("-");
        case QBTunerPolarization_Vertical:    return gettext("V");
        case QBTunerPolarization_Horizontal:  return gettext("H");
        default:
                                              return gettext("?");
    }
}
