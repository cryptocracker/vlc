/*****************************************************************************
 * menus.cpp : wxWindows plugin for vlc
 *****************************************************************************
 * Copyright (C) 2000-2004 VideoLAN
 * $Id$
 *
 * Authors: Gildas Bazin <gbazin@videolan.org>
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

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#include <stdlib.h>                                      /* malloc(), free() */
#include <errno.h>                                                 /* ENOMEM */
#include <string.h>                                            /* strerror() */
#include <stdio.h>

#include <vlc/vlc.h>
#include <vlc/intf.h>

#include "wxwindows.h"

class wxMenuItemExt: public wxMenuItem
{
public:
    /* Constructor */
    wxMenuItemExt( wxMenu* parentMenu, int id, const wxString& text,
                   const wxString& helpString, wxItemKind kind,
                   char *_psz_var, int _i_object_id, vlc_value_t _val,
                   int _i_val_type );

    virtual ~wxMenuItemExt();

    char *psz_var;
    int  i_val_type;
    int  i_object_id;
    vlc_value_t val;

private:

};

class Menu: public wxMenu
{
public:
    /* Constructor */
    Menu( intf_thread_t *p_intf, int i_start_id );
    virtual ~Menu();

    void Populate( int i_count, char **ppsz_names, int *pi_objects );
    void Clear();

private:
    wxMenu *Menu::CreateDummyMenu();
    void   Menu::CreateMenuItem( wxMenu *, char *, vlc_object_t * );
    wxMenu *Menu::CreateChoicesMenu( char *, vlc_object_t *, bool );

    DECLARE_EVENT_TABLE();

    intf_thread_t *p_intf;

    int i_start_id;
    int i_item_id;
};

/*****************************************************************************
 * Event Table.
 *****************************************************************************/

/* IDs for the controls and the menu commands */
enum
{
    /* menu items */
    MenuDummy_Event = wxID_HIGHEST + 1000,
    OpenFileSimple_Event = wxID_HIGHEST + 1100,
    OpenFile_Event,
    OpenDisc_Event,
    OpenNet_Event,
    OpenCapture_Event,
    MediaInfo_Event,
    Messages_Event,
    Preferences_Event,
    FirstAutoGenerated_Event = wxID_HIGHEST + 1999,
    SettingsMenu_Events = wxID_HIGHEST + 5000,
    AudioMenu_Events = wxID_HIGHEST + 2000,
    VideoMenu_Events = wxID_HIGHEST + 3000,
    NavigMenu_Events = wxID_HIGHEST + 4000,
    PopupMenu_Events = wxID_HIGHEST + 6000,
    Hotkeys_Events = wxID_HIGHEST + 7000
};

BEGIN_EVENT_TABLE(Menu, wxMenu)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MenuEvtHandler, wxEvtHandler)
    EVT_MENU(OpenFileSimple_Event, MenuEvtHandler::OnShowDialog)
    EVT_MENU(OpenFile_Event, MenuEvtHandler::OnShowDialog)
    EVT_MENU(OpenDisc_Event, MenuEvtHandler::OnShowDialog)
    EVT_MENU(OpenNet_Event, MenuEvtHandler::OnShowDialog)
    EVT_MENU(OpenCapture_Event, MenuEvtHandler::OnShowDialog)
    EVT_MENU(MediaInfo_Event, MenuEvtHandler::OnShowDialog)
    EVT_MENU(Messages_Event, MenuEvtHandler::OnShowDialog)
    EVT_MENU(Preferences_Event, MenuEvtHandler::OnShowDialog)
    EVT_MENU(-1, MenuEvtHandler::OnMenuEvent)
END_EVENT_TABLE()

wxMenu *OpenStreamMenu( intf_thread_t *p_intf )
{
    wxMenu *menu = new wxMenu;
    menu->Append( OpenFileSimple_Event, wxU(_("Quick &Open File...")) );
    menu->Append( OpenFile_Event, wxU(_("Open &File...")) );
    menu->Append( OpenDisc_Event, wxU(_("Open &Disc...")) );
    menu->Append( OpenNet_Event, wxU(_("Open &Network Stream...")) );
    menu->Append( OpenCapture_Event, wxU(_("Open &Capture Device...")) );
    return menu;
}

wxMenu *MiscMenu( intf_thread_t *p_intf )
{
    wxMenu *menu = new wxMenu;
    menu->Append( MediaInfo_Event, wxU(_("Media &Info...")) );
    menu->Append( Messages_Event, wxU(_("&Messages...")) );
    menu->Append( Preferences_Event, wxU(_("&Preferences...")) );
    return menu;
}

void PopupMenu( intf_thread_t *p_intf, wxWindow *p_parent,
                const wxPoint& pos )
{
#define MAX_POPUP_ITEMS 35

    vlc_object_t *p_object;
    char *ppsz_varnames[MAX_POPUP_ITEMS];
    int pi_objects[MAX_POPUP_ITEMS];
    int i = 0;

    /* Initializations */
    memset( pi_objects, 0, MAX_POPUP_ITEMS * sizeof(int) );

    /* Audio menu */
    ppsz_varnames[i++] = _("Audio menu");
    ppsz_varnames[i++] = NULL; /* Separator */

    p_object = (vlc_object_t *)vlc_object_find( p_intf, VLC_OBJECT_AOUT,
                                                FIND_ANYWHERE );
    if( p_object != NULL )
    {
        ppsz_varnames[i] = "audio-device";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "audio-channels";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "visual";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "equalizer";
        pi_objects[i++] = p_object->i_object_id;
        vlc_object_release( p_object );
    }

    /* Video menu */
    ppsz_varnames[i++] = NULL; /* Separator */
    ppsz_varnames[i++] = _("Video menu");
    ppsz_varnames[i++] = NULL; /* Separator */

    p_object = (vlc_object_t *)vlc_object_find( p_intf, VLC_OBJECT_VOUT,
                                                FIND_ANYWHERE );
    if( p_object != NULL )
    {
        vlc_object_t *p_dec_obj;

        ppsz_varnames[i] = "fullscreen";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "zoom";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "deinterlace";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "aspect-ratio";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "crop";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "video-on-top";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "directx-wallpaper";
        pi_objects[i++] = p_object->i_object_id;

        p_dec_obj = (vlc_object_t *)vlc_object_find( p_object,
                                                     VLC_OBJECT_DECODER,
                                                     FIND_PARENT );
        if( p_dec_obj != NULL )
        {
            ppsz_varnames[i] = "ffmpeg-pp-q";
            pi_objects[i++] = p_dec_obj->i_object_id;
            vlc_object_release( p_dec_obj );
        }

        vlc_object_release( p_object );
    }

    /* Input menu */
    ppsz_varnames[i++] = NULL; /* Separator */
    ppsz_varnames[i++] = _("Input menu");
    ppsz_varnames[i++] = NULL; /* Separator */

    p_object = (vlc_object_t *)vlc_object_find( p_intf, VLC_OBJECT_INPUT,
                                                FIND_ANYWHERE );
    if( p_object != NULL )
    {
        ppsz_varnames[i] = "bookmark";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "title";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "chapter";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "program";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "navigation";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "dvd_menus";
        pi_objects[i++] = p_object->i_object_id;

        ppsz_varnames[i] = "video-es";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "audio-es";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "spu-es";
        pi_objects[i++] = p_object->i_object_id;

        vlc_object_release( p_object );
    }

    /* Interface menu */
    ppsz_varnames[i++] = NULL; /* Separator */
    ppsz_varnames[i++] = _("Interface menu");
    ppsz_varnames[i++] = NULL; /* Separator */

    /* vlc_object_find is needed because of the dialogs provider case */
    p_object = (vlc_object_t *)vlc_object_find( p_intf, VLC_OBJECT_INTF,
                                                FIND_PARENT );
    if( p_object != NULL )
    {
        ppsz_varnames[i] = "intf-switch";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "intf-add";
        pi_objects[i++] = p_object->i_object_id;

        vlc_object_release( p_object );
    }

    /* Build menu */
    Menu popupmenu( p_intf, PopupMenu_Events );
    popupmenu.Populate( i, ppsz_varnames, pi_objects );

    /* Add static entries */
    popupmenu.AppendSeparator();
    popupmenu.Append( MenuDummy_Event, wxU(_("Open")),
                      OpenStreamMenu( p_intf ), wxT("") );
    popupmenu.Append( MenuDummy_Event, wxU(_("Miscellaneous")),
                      MiscMenu( p_intf ), wxT("") );

    p_intf->p_sys->p_popup_menu = &popupmenu;
    p_parent->PopupMenu( &popupmenu, pos.x, pos.y );
    p_intf->p_sys->p_popup_menu = NULL;
}

wxMenu *AudioMenu( intf_thread_t *_p_intf, wxWindow *p_parent, wxMenu *p_menu )
{
#define MAX_AUDIO_ITEMS 10

    vlc_object_t *p_object;
    char *ppsz_varnames[MAX_AUDIO_ITEMS];
    int pi_objects[MAX_AUDIO_ITEMS];
    int i = 0;

    /* Initializations */
    memset( pi_objects, 0, MAX_AUDIO_ITEMS * sizeof(int) );

    p_object = (vlc_object_t *)vlc_object_find( _p_intf, VLC_OBJECT_INPUT,
                                                FIND_ANYWHERE );
    if( p_object != NULL )
    {
        ppsz_varnames[i] = "audio-es";
        pi_objects[i++] = p_object->i_object_id;
        vlc_object_release( p_object );
    }

    p_object = (vlc_object_t *)vlc_object_find( _p_intf, VLC_OBJECT_AOUT,
                                                FIND_ANYWHERE );
    if( p_object != NULL )
    {
        ppsz_varnames[i] = "audio-device";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "audio-channels";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "visual";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "equalizer";
        pi_objects[i++] = p_object->i_object_id;
        vlc_object_release( p_object );
    }

    /* Build menu */
    Menu *p_vlc_menu = (Menu *)p_menu;
    if( !p_vlc_menu )
        p_vlc_menu = new Menu( _p_intf, AudioMenu_Events );
    else
        p_vlc_menu->Clear();

    p_vlc_menu->Populate( i, ppsz_varnames, pi_objects );

    return p_vlc_menu;
}

wxMenu *VideoMenu( intf_thread_t *_p_intf, wxWindow *p_parent, wxMenu *p_menu )
{
#define MAX_VIDEO_ITEMS 15

    vlc_object_t *p_object;
    char *ppsz_varnames[MAX_VIDEO_ITEMS];
    int pi_objects[MAX_VIDEO_ITEMS];
    int i = 0;

    /* Initializations */
    memset( pi_objects, 0, MAX_VIDEO_ITEMS * sizeof(int) );

    p_object = (vlc_object_t *)vlc_object_find( _p_intf, VLC_OBJECT_INPUT,
                                                FIND_ANYWHERE );
    if( p_object != NULL )
    {
        ppsz_varnames[i] = "video-es";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "spu-es";
        pi_objects[i++] = p_object->i_object_id;
        vlc_object_release( p_object );
    }

    p_object = (vlc_object_t *)vlc_object_find( _p_intf, VLC_OBJECT_VOUT,
                                                FIND_ANYWHERE );
    if( p_object != NULL )
    {
        vlc_object_t *p_dec_obj;

        ppsz_varnames[i] = "fullscreen";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "zoom";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "deinterlace";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "aspect-ratio";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "crop";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "video-on-top";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "directx-wallpaper";
        pi_objects[i++] = p_object->i_object_id;

        p_dec_obj = (vlc_object_t *)vlc_object_find( p_object,
                                                     VLC_OBJECT_DECODER,
                                                     FIND_PARENT );
        if( p_dec_obj != NULL )
        {
            ppsz_varnames[i] = "ffmpeg-pp-q";
            pi_objects[i++] = p_dec_obj->i_object_id;
            vlc_object_release( p_dec_obj );
        }

        vlc_object_release( p_object );
    }

    /* Build menu */
    Menu *p_vlc_menu = (Menu *)p_menu;
    if( !p_vlc_menu )
        p_vlc_menu = new Menu( _p_intf, VideoMenu_Events );
    else
        p_vlc_menu->Clear();

    p_vlc_menu->Populate( i, ppsz_varnames, pi_objects );

    return p_vlc_menu;
}

wxMenu *NavigMenu( intf_thread_t *_p_intf, wxWindow *p_parent, wxMenu *p_menu )
{
#define MAX_NAVIG_ITEMS 10

    vlc_object_t *p_object;
    char *ppsz_varnames[MAX_NAVIG_ITEMS];
    int pi_objects[MAX_NAVIG_ITEMS];
    int i = 0;

    /* Initializations */
    memset( pi_objects, 0, MAX_NAVIG_ITEMS * sizeof(int) );

    p_object = (vlc_object_t *)vlc_object_find( _p_intf, VLC_OBJECT_INPUT,
                                                FIND_ANYWHERE );
    if( p_object != NULL )
    {
        ppsz_varnames[i] = "bookmark";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "title";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "chapter";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "program";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "navigation";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "dvd_menus";
        pi_objects[i++] = p_object->i_object_id;

        ppsz_varnames[i] = "prev-title";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "next-title";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "prev-chapter";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "next-chapter";
        pi_objects[i++] = p_object->i_object_id;

        vlc_object_release( p_object );
    }

    /* Build menu */
    Menu *p_vlc_menu = (Menu *)p_menu;
    if( !p_vlc_menu )
        p_vlc_menu = new Menu( _p_intf, NavigMenu_Events );
    else
        p_vlc_menu->Clear();

    p_vlc_menu->Populate( i, ppsz_varnames, pi_objects );

    return p_vlc_menu;
}

wxMenu *SettingsMenu( intf_thread_t *_p_intf, wxWindow *p_parent,
                      wxMenu *p_menu )
{
#define MAX_SETTINGS_ITEMS 10

    vlc_object_t *p_object;
    char *ppsz_varnames[MAX_SETTINGS_ITEMS];
    int pi_objects[MAX_SETTINGS_ITEMS];
    int i = 0;

    /* Initializations */
    memset( pi_objects, 0, MAX_SETTINGS_ITEMS * sizeof(int) );

    p_object = (vlc_object_t *)vlc_object_find( _p_intf, VLC_OBJECT_INTF,
                                                FIND_PARENT );
    if( p_object != NULL )
    {
        ppsz_varnames[i] = "intf-switch";
        pi_objects[i++] = p_object->i_object_id;
        ppsz_varnames[i] = "intf-add";
        pi_objects[i++] = p_object->i_object_id;
        vlc_object_release( p_object );
    }

    /* Build menu */
    Menu *p_vlc_menu = (Menu *)p_menu;
    if( !p_vlc_menu )
        p_vlc_menu = new Menu( _p_intf, SettingsMenu_Events );
    else
        p_vlc_menu->Clear();

    p_vlc_menu->Populate( i, ppsz_varnames, pi_objects );

    return p_vlc_menu;
}

/*****************************************************************************
 * Constructor.
 *****************************************************************************/
Menu::Menu( intf_thread_t *_p_intf, int _i_start_id ) : wxMenu( )
{
    /* Initializations */
    p_intf = _p_intf;
    i_start_id = _i_start_id;
}

Menu::~Menu()
{
}

/*****************************************************************************
 * Public methods.
 *****************************************************************************/
void Menu::Populate( int i_count, char **ppsz_varnames, int *pi_objects )
{
    vlc_object_t *p_object;
    vlc_bool_t b_section_empty = VLC_FALSE;
    int i;

    i_item_id = i_start_id;

    for( i = 0; i < i_count; i++ )
    {
        if( !ppsz_varnames[i] )
        {
            if( b_section_empty )
            {
                Append( MenuDummy_Event + i, wxU(_("Empty")) );
                Enable( MenuDummy_Event + i, FALSE );
            }

            AppendSeparator();
            b_section_empty = VLC_TRUE;
            continue;
        }

        if( !pi_objects[i] )
        {
            Append( MenuDummy_Event, wxU(ppsz_varnames[i]) );
            b_section_empty = VLC_FALSE;
            continue;
        }

        p_object = (vlc_object_t *)vlc_object_get( p_intf, pi_objects[i] );
        if( p_object == NULL ) continue;

        b_section_empty = VLC_FALSE;
        CreateMenuItem( this, ppsz_varnames[i], p_object );
        vlc_object_release( p_object );
    }

    /* Special case for empty menus */
    if( GetMenuItemCount() == 0 || b_section_empty )
    {
        Append( MenuDummy_Event + i, wxU(_("Empty")) );
        Enable( MenuDummy_Event + i, FALSE );
    }
}

/* Work-around helper for buggy wxGTK */
static void RecursiveDestroy( wxMenu *menu )
{
    wxMenuItemList::Node *node = menu->GetMenuItems().GetFirst();
    for( ; node; )
    {
        wxMenuItem *item = node->GetData();
        node = node->GetNext();

        /* Delete the submenus */
        wxMenu *submenu = item->GetSubMenu();
        if( submenu )
        {
            RecursiveDestroy( submenu );
        }
        menu->Delete( item );
    }
}

void Menu::Clear( )
{
    RecursiveDestroy( this );
}

/*****************************************************************************
 * Private methods.
 *****************************************************************************/
static bool IsMenuEmpty( char *psz_var, vlc_object_t *p_object,
                         bool b_root = TRUE )
{
    vlc_value_t val, val_list;
    int i_type, i_result, i;

    /* Check the type of the object variable */
    i_type = var_Type( p_object, psz_var );

    /* Check if we want to display the variable */
    if( !(i_type & VLC_VAR_HASCHOICE) ) return FALSE;

    var_Change( p_object, psz_var, VLC_VAR_CHOICESCOUNT, &val, NULL );
    if( val.i_int == 0 ) return TRUE;

    if( (i_type & VLC_VAR_TYPE) != VLC_VAR_VARIABLE )
    {
        if( val.i_int == 1 && b_root ) return TRUE;
        else return FALSE;
    }

    /* Check children variables in case of VLC_VAR_VARIABLE */
    if( var_Change( p_object, psz_var, VLC_VAR_GETLIST, &val_list, NULL ) < 0 )
    {
        return TRUE;
    }

    for( i = 0, i_result = TRUE; i < val_list.p_list->i_count; i++ )
    {
        if( !IsMenuEmpty( val_list.p_list->p_values[i].psz_string,
                          p_object, FALSE ) )
        {
            i_result = FALSE;
            break;
        }
    }

    /* clean up everything */
    var_Change( p_object, psz_var, VLC_VAR_FREELIST, &val_list, NULL );

    return i_result;
}

void Menu::CreateMenuItem( wxMenu *menu, char *psz_var,
                           vlc_object_t *p_object )
{
    wxMenuItemExt *menuitem;
    vlc_value_t val, text;
    int i_type;

    /* Check the type of the object variable */
    i_type = var_Type( p_object, psz_var );

    switch( i_type & VLC_VAR_TYPE )
    {
    case VLC_VAR_VOID:
    case VLC_VAR_BOOL:
    case VLC_VAR_VARIABLE:
    case VLC_VAR_STRING:
    case VLC_VAR_INTEGER:
    case VLC_VAR_FLOAT:
        break;
    default:
        /* Variable doesn't exist or isn't handled */
        return;
    }

    /* Make sure we want to display the variable */
    if( IsMenuEmpty( psz_var, p_object ) ) return;

    /* Get the descriptive name of the variable */
    var_Change( p_object, psz_var, VLC_VAR_GETTEXT, &text, NULL );

    if( i_type & VLC_VAR_HASCHOICE )
    {
        menu->Append( MenuDummy_Event,
                      wxU(text.psz_string ? text.psz_string : psz_var),
                      CreateChoicesMenu( psz_var, p_object, TRUE ),
                      wxT("")/* Nothing for now (maybe use a GETLONGTEXT) */ );

        if( text.psz_string ) free( text.psz_string );
        return;
    }


    switch( i_type & VLC_VAR_TYPE )
    {
    case VLC_VAR_VOID:
        var_Get( p_object, psz_var, &val );
        menuitem = new wxMenuItemExt( menu, ++i_item_id,
                                      wxU(text.psz_string ?
                                        text.psz_string : psz_var),
                                      wxT(""), wxITEM_NORMAL, strdup(psz_var),
                                      p_object->i_object_id, val, i_type );
        menu->Append( menuitem );
        break;

    case VLC_VAR_BOOL:
        var_Get( p_object, psz_var, &val );
        val.b_bool = !val.b_bool;
        menuitem = new wxMenuItemExt( menu, ++i_item_id,
                                      wxU(text.psz_string ?
                                        text.psz_string : psz_var),
                                      wxT(""), wxITEM_CHECK, strdup(psz_var),
                                      p_object->i_object_id, val, i_type );
        menu->Append( menuitem );
        Check( i_item_id, val.b_bool ? FALSE : TRUE );
        break;
    }

    if( text.psz_string ) free( text.psz_string );
}

wxMenu *Menu::CreateChoicesMenu( char *psz_var, vlc_object_t *p_object,
                                 bool b_root )
{
    vlc_value_t val, val_list, text_list;
    int i_type, i;

    /* Check the type of the object variable */
    i_type = var_Type( p_object, psz_var );

    /* Make sure we want to display the variable */
    if( IsMenuEmpty( psz_var, p_object, b_root ) ) return NULL;

    switch( i_type & VLC_VAR_TYPE )
    {
    case VLC_VAR_VOID:
    case VLC_VAR_BOOL:
    case VLC_VAR_VARIABLE:
    case VLC_VAR_STRING:
    case VLC_VAR_INTEGER:
    case VLC_VAR_FLOAT:
        break;
    default:
        /* Variable doesn't exist or isn't handled */
        return NULL;
    }

    if( var_Change( p_object, psz_var, VLC_VAR_GETLIST,
                    &val_list, &text_list ) < 0 )
    {
        return NULL;
    }

    wxMenu *menu = new wxMenu;
    for( i = 0; i < val_list.p_list->i_count; i++ )
    {
        vlc_value_t another_val;
        wxMenuItemExt *menuitem;

        switch( i_type & VLC_VAR_TYPE )
        {
        case VLC_VAR_VARIABLE:
          menu->Append( MenuDummy_Event,
                        wxU(text_list.p_list->p_values[i].psz_string ?
                        text_list.p_list->p_values[i].psz_string :
                        val_list.p_list->p_values[i].psz_string),
                        CreateChoicesMenu(
                            val_list.p_list->p_values[i].psz_string,
                            p_object, FALSE ), wxT("") );
          break;

        case VLC_VAR_STRING:
          var_Get( p_object, psz_var, &val );

          another_val.psz_string =
              strdup(val_list.p_list->p_values[i].psz_string);
          menuitem =
              new wxMenuItemExt( menu, ++i_item_id,
                                 wxU(text_list.p_list->p_values[i].psz_string ?
                                 text_list.p_list->p_values[i].psz_string :
                                 another_val.psz_string), wxT(""),
                                 i_type & VLC_VAR_ISCOMMAND ?
                                   wxITEM_NORMAL : wxITEM_RADIO,
                                 strdup(psz_var),
                                 p_object->i_object_id, another_val, i_type );

          menu->Append( menuitem );

          if( !(i_type & VLC_VAR_ISCOMMAND) && val.psz_string &&
              !strcmp( val.psz_string,
                       val_list.p_list->p_values[i].psz_string ) )
              menu->Check( i_item_id, TRUE );

          if( val.psz_string ) free( val.psz_string );
          break;

        case VLC_VAR_INTEGER:
          var_Get( p_object, psz_var, &val );

          menuitem =
              new wxMenuItemExt( menu, ++i_item_id,
                                 text_list.p_list->p_values[i].psz_string ?
                                 (wxString)wxU(
                                   text_list.p_list->p_values[i].psz_string) :
                                 wxString::Format(wxT("%d"),
                                 val_list.p_list->p_values[i].i_int), wxT(""),
                                 i_type & VLC_VAR_ISCOMMAND ?
                                   wxITEM_NORMAL : wxITEM_RADIO,
                                 strdup(psz_var),
                                 p_object->i_object_id,
                                 val_list.p_list->p_values[i], i_type );

          menu->Append( menuitem );

          if( !(i_type & VLC_VAR_ISCOMMAND) &&
              val_list.p_list->p_values[i].i_int == val.i_int )
              menu->Check( i_item_id, TRUE );
          break;

        case VLC_VAR_FLOAT:
          var_Get( p_object, psz_var, &val );

          menuitem =
              new wxMenuItemExt( menu, ++i_item_id,
                                 text_list.p_list->p_values[i].psz_string ?
                                 (wxString)wxU(
                                   text_list.p_list->p_values[i].psz_string) :
                                 wxString::Format(wxT("%.2f"),
                                 val_list.p_list->p_values[i].f_float),wxT(""),
                                 i_type & VLC_VAR_ISCOMMAND ?
                                   wxITEM_NORMAL : wxITEM_RADIO,
                                 strdup(psz_var),
                                 p_object->i_object_id,
                                 val_list.p_list->p_values[i], i_type );

          menu->Append( menuitem );

          if( !(i_type & VLC_VAR_ISCOMMAND) &&
              val_list.p_list->p_values[i].f_float == val.f_float )
              menu->Check( i_item_id, TRUE );
          break;

        default:
          break;
        }
    }

    /* clean up everything */
    var_Change( p_object, psz_var, VLC_VAR_FREELIST, &val_list, &text_list );

    return menu;
}

/*****************************************************************************
 * A small helper class which intercepts all popup menu events
 *****************************************************************************/
MenuEvtHandler::MenuEvtHandler( intf_thread_t *_p_intf,
                                Interface *_p_main_interface )
{
    /* Initializations */
    p_intf = _p_intf;
    p_main_interface = _p_main_interface;
}

MenuEvtHandler::~MenuEvtHandler()
{
}

void MenuEvtHandler::OnShowDialog( wxCommandEvent& event )
{
    if( p_intf->p_sys->pf_show_dialog )
    {
        int i_id;

        switch( event.GetId() )
        {
        case OpenFileSimple_Event:
            i_id = INTF_DIALOG_FILE_SIMPLE;
            break;
        case OpenFile_Event:
            i_id = INTF_DIALOG_FILE;
            break;
        case OpenDisc_Event:
            i_id = INTF_DIALOG_DISC;
            break;
        case OpenNet_Event:
            i_id = INTF_DIALOG_NET;
            break;
        case OpenCapture_Event:
            i_id = INTF_DIALOG_CAPTURE;
            break;
        case MediaInfo_Event:
            i_id = INTF_DIALOG_FILEINFO;
            break;
        case Messages_Event:
            i_id = INTF_DIALOG_MESSAGES;
            break;
        case Preferences_Event:
            i_id = INTF_DIALOG_PREFS;
            break;
        default:
            i_id = INTF_DIALOG_FILE;
            break;

        }

        p_intf->p_sys->pf_show_dialog( p_intf, i_id, 1, 0 );
    }
}

void MenuEvtHandler::OnMenuEvent( wxCommandEvent& event )
{
    wxMenuItem *p_menuitem = NULL;
    int i_hotkey_event = p_intf->p_sys->i_first_hotkey_event;
    int i_hotkeys = p_intf->p_sys->i_hotkeys;

    /* Check if this is an auto generated menu item */
    if( event.GetId() < FirstAutoGenerated_Event )
    {
        event.Skip();
        return;
    }

    /* Check if this is an hotkey event */
    if( event.GetId() >= i_hotkey_event &&
        event.GetId() < i_hotkey_event + i_hotkeys )
    {
        vlc_value_t val;

        val.i_int =
            p_intf->p_vlc->p_hotkeys[event.GetId() - i_hotkey_event].i_key;

        /* Get the key combination and send it to the hotkey handler */
        var_Set( p_intf->p_vlc, "key-pressed", val );
	msg_Err( p_intf, "received key event: %i", event.GetId() );
        return;
    }

    if( !p_main_interface ||
        (p_menuitem = p_main_interface->GetMenuBar()->FindItem(event.GetId()))
        == NULL )
    {
        if( p_intf->p_sys->p_popup_menu )
        {
            p_menuitem = 
                p_intf->p_sys->p_popup_menu->FindItem( event.GetId() );
        }
    }

    if( p_menuitem )
    {
        wxMenuItemExt *p_menuitemext = (wxMenuItemExt *)p_menuitem;
        vlc_object_t *p_object;

        p_object = (vlc_object_t *)vlc_object_get( p_intf,
                                       p_menuitemext->i_object_id );
        if( p_object == NULL ) return;

        wxMutexGuiLeave(); // We don't want deadlocks
        var_Set( p_object, p_menuitemext->psz_var, p_menuitemext->val );
        wxMutexGuiEnter();

        vlc_object_release( p_object );
    }
    else
        event.Skip();
}

/*****************************************************************************
 * A small helper class which encapsulate wxMenuitem with some other useful
 * things.
 *****************************************************************************/
wxMenuItemExt::wxMenuItemExt( wxMenu* parentMenu, int id, const wxString& text,
    const wxString& helpString, wxItemKind kind,
    char *_psz_var, int _i_object_id, vlc_value_t _val, int _i_val_type ):
    wxMenuItem( parentMenu, id, text, helpString, kind )
{
    /* Initializations */
    psz_var = _psz_var;
    i_val_type = _i_val_type;
    i_object_id = _i_object_id;
    val = _val;
};

wxMenuItemExt::~wxMenuItemExt()
{
    if( psz_var ) free( psz_var );
    if( ((i_val_type & VLC_VAR_TYPE) == VLC_VAR_STRING)
        && val.psz_string ) free( val.psz_string );
};
