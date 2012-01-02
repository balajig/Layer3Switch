/** @file gprs.h
    @brief Opening a PPP connection through a GPRS compatible modem (mobile phone)

    Copyright 2010 j. Arzi.

    This file is part of SDPOS.

    SDPOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SDPOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with SDPOS.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef GPRS_H
#define GPRS_H

#include "defines.h"

/** @brief Setup a PPP link to the remote internet provider.
 *  @param stream         Input/output stream to the modem
 *  @param cgdcont        Provider dependant configuration string.
 *                        Should be "AT+CGDCONT=1,\"IP\",\"orange\"" for orange.
 *  @param provider_phone Should be "*99#" for orange or bouygues,
 *                        "*99***1#" or "*99***3#" for sfr. */
extern retcode gprs_open_ppp_link(iostream_t *stream,
                                  const char *cgd_cont,
                                  const char *provider_phone);


#endif
