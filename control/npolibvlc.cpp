/*****************************************************************************
 * vlc.cpp: support for NPRuntime API for Netscape Script-able plugins
 *                 FYI: http://www.mozilla.org/projects/plugins/npruntime.html
 *****************************************************************************
 * Copyright (C) 2005 the VideoLAN team
 *
 * Authors: Damien Fouilleul <Damien.Fouilleul@laposte.net>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Mozilla stuff */
#ifdef HAVE_MOZILLA_CONFIG_H
#   include <mozilla-config.h>
#endif

#include "npolibvlc.h"
#include "vlcplugin.h"

/*
** implementation of libvlc root object
*/

LibvlcRootNPObject::LibvlcRootNPObject(NPP instance, const NPClass *aClass) :
        RuntimeNPObject(instance, aClass)
{
    static NPClass *audioClass = new RuntimeNPClass<LibvlcAudioNPObject>();
    static NPClass *inputClass = new RuntimeNPClass<LibvlcInputNPObject>();
    static NPClass *playlistClass = new RuntimeNPClass<LibvlcPlaylistNPObject>();
    static NPClass *videoClass = new RuntimeNPClass<LibvlcVideoNPObject>();

    audioObj = NPN_CreateObject(instance, audioClass);
    inputObj = NPN_CreateObject(instance, inputClass);
    playlistObj = NPN_CreateObject(instance, playlistClass);
    videoObj = NPN_CreateObject(instance, videoClass);
}

LibvlcRootNPObject::~LibvlcRootNPObject()
{
    NPN_ReleaseObject(audioObj);
    NPN_ReleaseObject(inputObj);
    NPN_ReleaseObject(playlistObj);
    NPN_ReleaseObject(videoObj);
}

const NPUTF8 * const LibvlcRootNPObject::propertyNames[] = 
{
    "audio",
    "input",
    "playlist",
    "video",
};

enum LibvlcRootNPObjectPropertyIds
{
    ID_audio = 0,
    ID_input,
    ID_playlist,
    ID_video,
};

const int LibvlcRootNPObject::propertyCount = sizeof(LibvlcRootNPObject::propertyNames)/sizeof(NPUTF8 *);

RuntimeNPObject::InvokeResult LibvlcRootNPObject::getProperty(int index, NPVariant *result)
{
    switch( index )
    {
        case ID_audio:
            OBJECT_TO_NPVARIANT(NPN_RetainObject(audioObj), *result);
            return INVOKERESULT_NO_ERROR;
        case ID_input:
            OBJECT_TO_NPVARIANT(NPN_RetainObject(inputObj), *result);
            return INVOKERESULT_NO_ERROR;
        case ID_playlist:
            OBJECT_TO_NPVARIANT(NPN_RetainObject(playlistObj), *result);
            return INVOKERESULT_NO_ERROR;
        case ID_video:
            OBJECT_TO_NPVARIANT(NPN_RetainObject(videoObj), *result);
            return INVOKERESULT_NO_ERROR;
    }
    return INVOKERESULT_GENERIC_ERROR;
}

const NPUTF8 * const LibvlcRootNPObject::methodNames[] =
{
    /* no methods */
};

const int LibvlcRootNPObject::methodCount = sizeof(LibvlcRootNPObject::methodNames)/sizeof(NPUTF8 *);

/*
** implementation of libvlc audio object
*/

const NPUTF8 * const LibvlcAudioNPObject::propertyNames[] = 
{
    "mute",
    "volume",
};

enum LibvlcAudioNPObjectPropertyIds
{
    ID_mute,
    ID_volume,
};

const int LibvlcAudioNPObject::propertyCount = sizeof(LibvlcAudioNPObject::propertyNames)/sizeof(NPUTF8 *);

RuntimeNPObject::InvokeResult LibvlcAudioNPObject::getProperty(int index, NPVariant *result)
{
    VlcPlugin *p_plugin = reinterpret_cast<VlcPlugin *>(_instance->pdata);
    if( p_plugin )
    {
        libvlc_exception_t ex;
        libvlc_exception_init(&ex);

        switch( index )
        {
            case ID_mute:
            {
                vlc_bool_t muted = libvlc_audio_get_mute(p_plugin->getVLC(), &ex);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                BOOLEAN_TO_NPVARIANT(muted, *result);
                return INVOKERESULT_NO_ERROR;
            }
            case ID_volume:
            {
                int volume = libvlc_audio_get_volume(p_plugin->getVLC(), &ex);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                INT32_TO_NPVARIANT(volume, *result);
                return INVOKERESULT_NO_ERROR;
            }
        }
    }
    return INVOKERESULT_GENERIC_ERROR;
}

RuntimeNPObject::InvokeResult LibvlcAudioNPObject::setProperty(int index, const NPVariant *value)
{
    VlcPlugin *p_plugin = reinterpret_cast<VlcPlugin *>(_instance->pdata);
    if( p_plugin )
    {
        libvlc_exception_t ex;
        libvlc_exception_init(&ex);

        switch( index )
        {
            case ID_mute:
                if( NPVARIANT_IS_BOOLEAN(*value) )
                {
                    libvlc_audio_set_mute(p_plugin->getVLC(),
                                          NPVARIANT_TO_BOOLEAN(*value), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    return INVOKERESULT_NO_ERROR;
                }
                return INVOKERESULT_INVALID_VALUE;
            case ID_volume:
                if( isNumberValue(*value) )
                {
                    libvlc_audio_set_volume(p_plugin->getVLC(),
                                            numberValue(*value), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    return INVOKERESULT_NO_ERROR;
                }
                return INVOKERESULT_INVALID_VALUE;
        }
    }
    return INVOKERESULT_GENERIC_ERROR;
}

const NPUTF8 * const LibvlcAudioNPObject::methodNames[] =
{
    "togglemute",
};

enum LibvlcAudioNPObjectMethodIds
{
    ID_togglemute,
};

const int LibvlcAudioNPObject::methodCount = sizeof(LibvlcAudioNPObject::methodNames)/sizeof(NPUTF8 *);

RuntimeNPObject::InvokeResult LibvlcAudioNPObject::invoke(int index, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    VlcPlugin *p_plugin = reinterpret_cast<VlcPlugin *>(_instance->pdata);
    if( p_plugin )
    {
        libvlc_exception_t ex;
        libvlc_exception_init(&ex);

        switch( index )
        {
            case ID_togglemute:
                if( argCount == 0 )
                {
                    libvlc_audio_toggle_mute(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            default:
                return INVOKERESULT_NO_SUCH_METHOD;
        }
    }
    return INVOKERESULT_GENERIC_ERROR;
}

/*
** implementation of libvlc input object
*/

const NPUTF8 * const LibvlcInputNPObject::propertyNames[] = 
{
    "length",
    "position",
    "time",
    "fps",
    "hasvout",
};

enum LibvlcInputNPObjectPropertyIds
{
    ID_length,
    ID_position,
    ID_time,
    ID_fps,
    ID_hasvout,
};

const int LibvlcInputNPObject::propertyCount = sizeof(LibvlcInputNPObject::propertyNames)/sizeof(NPUTF8 *);

RuntimeNPObject::InvokeResult LibvlcInputNPObject::getProperty(int index, NPVariant *result)
{
    VlcPlugin *p_plugin = reinterpret_cast<VlcPlugin *>(_instance->pdata);
    if( p_plugin )
    {
        libvlc_exception_t ex;
        libvlc_exception_init(&ex);

        libvlc_input_t *p_input = libvlc_playlist_get_input(p_plugin->getVLC(), &ex);
        if( libvlc_exception_raised(&ex) )
        {
            NPN_SetException(this, libvlc_exception_get_message(&ex));
            libvlc_exception_clear(&ex);
            return INVOKERESULT_GENERIC_ERROR;
        }

        switch( index )
        {
            case ID_length:
            {
                double val = (double)libvlc_input_get_length(p_input, &ex);
                libvlc_input_free(p_input);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                DOUBLE_TO_NPVARIANT(val, *result);
                return INVOKERESULT_NO_ERROR;
            }
            case ID_position:
            {
                double val = libvlc_input_get_position(p_input, &ex);
                libvlc_input_free(p_input);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                DOUBLE_TO_NPVARIANT(val, *result);
                return INVOKERESULT_NO_ERROR;
            }
            case ID_time:
            {
                double val = (double)libvlc_input_get_time(p_input, &ex);
                libvlc_input_free(p_input);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                DOUBLE_TO_NPVARIANT(val, *result);
                return INVOKERESULT_NO_ERROR;
            }
            case ID_fps:
            {
                double val = libvlc_input_get_fps(p_input, &ex);
                libvlc_input_free(p_input);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                DOUBLE_TO_NPVARIANT(val, *result);
                return INVOKERESULT_NO_ERROR;
            }
            case ID_hasvout:
            {
                vlc_bool_t val = libvlc_input_has_vout(p_input, &ex);
                libvlc_input_free(p_input);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                BOOLEAN_TO_NPVARIANT(val, *result);
                return INVOKERESULT_NO_ERROR;
            }
        }
        libvlc_input_free(p_input);
    }
    return INVOKERESULT_GENERIC_ERROR;
}

RuntimeNPObject::InvokeResult LibvlcInputNPObject::setProperty(int index, const NPVariant *value)
{
    VlcPlugin *p_plugin = reinterpret_cast<VlcPlugin *>(_instance->pdata);
    if( p_plugin )
    {
        libvlc_exception_t ex;
        libvlc_exception_init(&ex);

        libvlc_input_t *p_input = libvlc_playlist_get_input(p_plugin->getVLC(), &ex);
        if( libvlc_exception_raised(&ex) )
        {
            NPN_SetException(this, libvlc_exception_get_message(&ex));
            libvlc_exception_clear(&ex);
            return INVOKERESULT_GENERIC_ERROR;
        }

        switch( index )
        {
            case ID_position:
            {
                if( ! NPVARIANT_IS_DOUBLE(*value) )
                {
                    libvlc_input_free(p_input);
                    return INVOKERESULT_INVALID_VALUE;
                }

                float val = (float)NPVARIANT_TO_DOUBLE(*value);
                libvlc_input_set_position(p_input, val, &ex);
                libvlc_input_free(p_input);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                return INVOKERESULT_NO_ERROR;
            }
            case ID_time:
            {
                vlc_int64_t val;
                if( NPVARIANT_IS_INT32(*value) )
                    val = (vlc_int64_t)NPVARIANT_TO_INT32(*value);
                else if( NPVARIANT_IS_DOUBLE(*value) )
                    val = (vlc_int64_t)NPVARIANT_TO_DOUBLE(*value);
                else
                {
                    libvlc_input_free(p_input);
                    return INVOKERESULT_INVALID_VALUE;
                }

                libvlc_input_set_time(p_input, val, &ex);
                libvlc_input_free(p_input);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                return INVOKERESULT_NO_ERROR;
            }
        }
        libvlc_input_free(p_input);
    }
    return INVOKERESULT_GENERIC_ERROR;
}

const NPUTF8 * const LibvlcInputNPObject::methodNames[] =
{
    /* no methods */
};

const int LibvlcInputNPObject::methodCount = sizeof(LibvlcInputNPObject::methodNames)/sizeof(NPUTF8 *);

/*
** implementation of libvlc playlist object
*/


const NPUTF8 * const LibvlcPlaylistNPObject::propertyNames[] = 
{
    "itemscount",
    "isplaying",
};

enum LibvlcPlaylistNPObjectPropertyIds
{
    ID_itemscount,
    ID_isplaying,
};

const int LibvlcPlaylistNPObject::propertyCount = sizeof(LibvlcPlaylistNPObject::propertyNames)/sizeof(NPUTF8 *);

RuntimeNPObject::InvokeResult LibvlcPlaylistNPObject::getProperty(int index, NPVariant *result)
{
    VlcPlugin *p_plugin = reinterpret_cast<VlcPlugin *>(_instance->pdata);
    if( p_plugin )
    {
        libvlc_exception_t ex;
        libvlc_exception_init(&ex);

        switch( index )
        {
            case ID_itemscount:
            {
                int val = libvlc_playlist_items_count(p_plugin->getVLC(), &ex);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                INT32_TO_NPVARIANT(val, *result);
                return INVOKERESULT_NO_ERROR;
            }
            case ID_isplaying:
            {
                int val = libvlc_playlist_isplaying(p_plugin->getVLC(), &ex);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                BOOLEAN_TO_NPVARIANT(val, *result);
                return INVOKERESULT_NO_ERROR;
            }
        }
    }
    return INVOKERESULT_GENERIC_ERROR;
}

const NPUTF8 * const LibvlcPlaylistNPObject::methodNames[] =
{
    "add",
    "play",
    "togglepause"
    "stop",
    "next",
    "prev",
    "clear",
    "deleteitem"
};

enum LibvlcPlaylistNPObjectMethodIds
{
    ID_add,
    ID_play,
    ID_togglepause,
    ID_stop,
    ID_next,
    ID_prev,
    ID_clear,
    ID_deleteitem,
};

const int LibvlcPlaylistNPObject::methodCount = sizeof(LibvlcPlaylistNPObject::methodNames)/sizeof(NPUTF8 *);

RuntimeNPObject::InvokeResult LibvlcPlaylistNPObject::invoke(int index, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    VlcPlugin *p_plugin = reinterpret_cast<VlcPlugin *>(_instance->pdata);
    if( p_plugin )
    {
        libvlc_exception_t ex;
        libvlc_exception_init(&ex);

        switch( index )
        {
            case ID_add:
                if( (argCount == 1) && NPVARIANT_IS_STRING(args[0]) )
                {
                    const NPString &name = NPVARIANT_TO_STRING(args[0]);
                    NPUTF8 *s = new NPUTF8[name.utf8length+1];
                    if( s )
                    {
                        strncpy(s, name.utf8characters, name.utf8length);
                        s[name.utf8length] = '\0';

                        char *url = p_plugin->getAbsoluteURL(s);
                        delete s;
                        if( ! url )
                            // what happened ?
                            return INVOKERESULT_GENERIC_ERROR;
                             
                        int item = libvlc_playlist_add(p_plugin->getVLC(), url, NULL, &ex);
                        free(url);
                        if( libvlc_exception_raised(&ex) )
                        {
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                            return INVOKERESULT_GENERIC_ERROR;
                        }
                        else
                        {
                            INT32_TO_NPVARIANT(item, *result);
                            return INVOKERESULT_NO_ERROR;
                        }
                    }
                    else
                        return INVOKERESULT_OUT_OF_MEMORY;
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_play:
                if( argCount == 0 )
                {
                    libvlc_playlist_play(p_plugin->getVLC(), -1, 0, NULL, &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_togglepause:
                if( argCount == 0 )
                {
                    libvlc_playlist_pause(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_stop:
                if( argCount == 0 )
                {
                    libvlc_playlist_stop(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_next:
                if( argCount == 0 )
                {
                    libvlc_playlist_next(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_prev:
                if( argCount == 0 )
                {
                    libvlc_playlist_prev(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_clear:
                if( argCount == 0 )
                {
                    libvlc_playlist_clear(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_deleteitem:
                if( (argCount == 1) && isNumberValue(args[0]) )
                {
                    libvlc_playlist_delete_item(p_plugin->getVLC(), numberValue(args[0]), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            default:
                return INVOKERESULT_NO_SUCH_METHOD;
        }
    }
    return INVOKERESULT_GENERIC_ERROR;
}

/*
** implementation of libvlc video object
*/

const NPUTF8 * const LibvlcVideoNPObject::propertyNames[] = 
{
    "fullscreen",
    "height",
    "width",
};

enum LibvlcVideoNPObjectPropertyIds
{
    ID_fullscreen,
    ID_height,
    ID_width,
};

const int LibvlcVideoNPObject::propertyCount = sizeof(LibvlcVideoNPObject::propertyNames)/sizeof(NPUTF8 *);

RuntimeNPObject::InvokeResult LibvlcVideoNPObject::getProperty(int index, NPVariant *result)
{
    VlcPlugin *p_plugin = reinterpret_cast<VlcPlugin *>(_instance->pdata);
    if( p_plugin )
    {
        libvlc_exception_t ex;
        libvlc_exception_init(&ex);

        libvlc_input_t *p_input = libvlc_playlist_get_input(p_plugin->getVLC(), &ex);
        if( libvlc_exception_raised(&ex) )
        {
            NPN_SetException(this, libvlc_exception_get_message(&ex));
            libvlc_exception_clear(&ex);
            return INVOKERESULT_GENERIC_ERROR;
        }

        switch( index )
        {
            case ID_fullscreen:
            {
                int val = libvlc_get_fullscreen(p_input, &ex);
                libvlc_input_free(p_input);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                BOOLEAN_TO_NPVARIANT(val, *result);
                return INVOKERESULT_NO_ERROR;
            }
            case ID_height:
            {
                int val = libvlc_video_get_height(p_input, &ex);
                libvlc_input_free(p_input);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                INT32_TO_NPVARIANT(val, *result);
                return INVOKERESULT_NO_ERROR;
            }
            case ID_width:
            {
                int val = libvlc_video_get_width(p_input, &ex);
                libvlc_input_free(p_input);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                INT32_TO_NPVARIANT(val, *result);
                return INVOKERESULT_NO_ERROR;
            }
        }
        libvlc_input_free(p_input);
    }
    return INVOKERESULT_GENERIC_ERROR;
}

RuntimeNPObject::InvokeResult LibvlcVideoNPObject::setProperty(int index, const NPVariant *value)
{
    VlcPlugin *p_plugin = reinterpret_cast<VlcPlugin *>(_instance->pdata);
    if( p_plugin )
    {
        libvlc_exception_t ex;
        libvlc_exception_init(&ex);

        libvlc_input_t *p_input = libvlc_playlist_get_input(p_plugin->getVLC(), &ex);
        if( libvlc_exception_raised(&ex) )
        {
            NPN_SetException(this, libvlc_exception_get_message(&ex));
            libvlc_exception_clear(&ex);
            return INVOKERESULT_GENERIC_ERROR;
        }

        switch( index )
        {
            case ID_fullscreen:
            {
                if( ! NPVARIANT_IS_BOOLEAN(*value) )
                {
                    libvlc_input_free(p_input);
                    return INVOKERESULT_INVALID_VALUE;
                }

                int val = NPVARIANT_TO_BOOLEAN(*value);
                libvlc_set_fullscreen(p_input, val, &ex);
                libvlc_input_free(p_input);
                if( libvlc_exception_raised(&ex) )
                {
                    NPN_SetException(this, libvlc_exception_get_message(&ex));
                    libvlc_exception_clear(&ex);
                    return INVOKERESULT_GENERIC_ERROR;
                }
                return INVOKERESULT_NO_ERROR;
            }
        }
        libvlc_input_free(p_input);
    }
    return INVOKERESULT_GENERIC_ERROR;
}

const NPUTF8 * const LibvlcVideoNPObject::methodNames[] =
{
    "togglefullscreen",
};

enum LibvlcVideoNPObjectMethodIds
{
    ID_togglefullscreen,
};

const int LibvlcVideoNPObject::methodCount = sizeof(LibvlcVideoNPObject::methodNames)/sizeof(NPUTF8 *);

RuntimeNPObject::InvokeResult LibvlcVideoNPObject::invoke(int index, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    VlcPlugin *p_plugin = reinterpret_cast<VlcPlugin *>(_instance->pdata);
    if( p_plugin )
    {
        libvlc_exception_t ex;
        libvlc_exception_init(&ex);

        libvlc_input_t *p_input = libvlc_playlist_get_input(p_plugin->getVLC(), &ex);
        if( libvlc_exception_raised(&ex) )
        {
            NPN_SetException(this, libvlc_exception_get_message(&ex));
            libvlc_exception_clear(&ex);
            return INVOKERESULT_GENERIC_ERROR;
        }

        switch( index )
        {
            case ID_togglefullscreen:
                if( argCount == 0 )
                {
                    libvlc_toggle_fullscreen(p_input, &ex);
                    libvlc_input_free(p_input);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                else
                {
                    /* cannot get input, probably not playing */
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                    }
                    return INVOKERESULT_GENERIC_ERROR;
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            default:
                return INVOKERESULT_NO_SUCH_METHOD;
        }
    }
    return INVOKERESULT_GENERIC_ERROR;
}
