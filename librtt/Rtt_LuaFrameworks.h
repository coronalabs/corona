//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_LuaFrameworks_H__
#define _Rtt_LuaFrameworks_H__

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

// Frameworks that are implemented as pure Lua files are pre-compiled into bytecodes
// and then placed in a C byte array constant. The following functions load those
// bytecodes via luaL_loadbuffer. The corresponding .cpp file is dynamically generated.

int luaload_easing(lua_State* L);
int luaload_launchpad(lua_State* L);

int luaload_dkjson(lua_State* L);
int luaload_json(lua_State* L);

int luaload_re(lua_State* L);
extern "C" { int luaopen_lpeg (lua_State *L); }

int luaload_composer(lua_State* L);
int luaload_composer_scene(lua_State* L);

int luaload_timer(lua_State* L);

int luaload_transition(lua_State* L);
int luaload_transition_v1(lua_State* L);

int luaload_widget(lua_State* L);
int luaload_widget_button(lua_State* L);
int luaload_widget_momentumScrolling(lua_State* L);
int luaload_widget_pickerWheel(lua_State* L);
int luaload_widget_progressView(lua_State* L);
int luaload_widget_scrollview(lua_State* L);
int luaload_widget_searchField(lua_State* L);
int luaload_widget_segmentedControl(lua_State* L);
int luaload_widget_slider(lua_State* L);
int luaload_widget_spinner(lua_State* L);
int luaload_widget_stepper(lua_State* L);
int luaload_widget_switch(lua_State* L);
int luaload_widget_tabbar(lua_State* L);
int luaload_widget_tableview(lua_State* L);
int luaload_widget_theme_android(lua_State *L);
int luaload_widget_theme_android_sheet(lua_State *L);
int luaload_widget_theme_android_holo_light(lua_State *L);
int luaload_widget_theme_android_holo_light_sheet(lua_State *L);
int luaload_widget_theme_android_holo_dark(lua_State *L);
int luaload_widget_theme_android_holo_dark_sheet(lua_State *L);
// These are for iOS pre 7
int luaload_widget_theme_ios(lua_State *L);
int luaload_widget_theme_ios_sheet(lua_State *L);
// These are for iOS 7+
int luaload_widget_theme_ios7(lua_State *L);
int luaload_widget_theme_ios7_sheet(lua_State *L);
#ifdef Rtt_DEBUGGER
int luaload_remdebug_engine(lua_State *L);
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Frameworks_H__
