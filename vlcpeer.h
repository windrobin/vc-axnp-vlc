/*****************************************************************************
 * vlcpeer.h: scriptable peer descriptor
 *****************************************************************************
 * Copyright (C) 2002 VideoLAN
 * $Id: vlcpeer.h,v 1.3 2003/04/09 16:18:36 sam Exp $
 *
 * Authors: Samuel Hocevar <sam@zoy.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#include "vlcintf.h"
#include "support/classinfo.h"

class VlcPlugin;

class VlcPeer : public VlcIntf, public ClassInfo
{
public:
    NS_DECL_ISUPPORTS
    NS_DECL_VLCINTF

             VlcPeer();
             VlcPeer( VlcPlugin * );
    virtual ~VlcPeer();

    void     Disable();

private:
    VlcPlugin * p_plugin;
};
