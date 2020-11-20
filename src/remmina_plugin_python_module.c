/*
 * Remmina - The GTK+ Remote Desktop Client
 * Copyright (C) 2009-2011 Vic Lee
 * Copyright (C) 2014-2015 Antenore Gatta, Fabio Castelli, Giovanni Panozzo
 * Copyright (C) 2016-2020 Antenore Gatta, Giovanni Panozzo
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of portions of this program with the
 *  OpenSSL library under certain conditions as described in each
 *  individual source file, and distribute linked combinations
 *  including the two.
 *  You must obey the GNU General Public License in all respects
 *  for all of the code used other than OpenSSL. *  If you modify
 *  file(s) with this exception, you may extend this exception to your
 *  version of the file(s), but you are not obligated to do so. *  If you
 *  do not wish to do so, delete this exception statement from your
 *  version. *  If you delete this exception statement from all source
 *  files in the program, then also delete it here.
 *
 */

/**
 * @file remmina_plugin_python_module.c
 * @brief Implementation of the Python module 'remmina'.
 * @author Mathias Winterhalter
 * @date 14.10.2020
 *
 * This file acts as a broker between Remmina and the Python plugins. It abstracts the communication flow
 * over the RemminaPluginService and redirects calls to the correct Python plugin. The PyRemminaProtocolWidget
 * takes care of providing the API inside the Python script.
 *
 * This is a minimal example of the usage of the remmina protocol widget API:
 *  
 * @code
 * import remmina
 * 
 * class MyProtocol:
 *  def __init__(self):
 *      self.name = "MyProtocol"
 *      self.description = "Example protocol plugin to explain how Python plugins work."
 *      self.version = "0.1"
 *      self.icon_name = ""
 *      self.icon_name_ssh = ""
 *
 *  def init(self, protocol_widget):
 *      print("Protocol widget dimensions: %d x %d" % (protocol_widget.get_width(), protocol_widget.get_height()))
 * ...
 * @endcode
 * 
 * @see http://www.remmina.org/wp for more information.
 */

#include <glib.h>
#include <gtk/gtk.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "config.h"
#include "pygobject.h"
#include "remmina_plugin_manager.h"
#include "remmina_plugin_python_remmina.h"
#include "remmina/plugin.h"
#include "remmina_protocol_widget.h"

/**
 * @brief Handles the initialization of the Python plugin.
 * @details This function prepares the plugin structure and calls the init method of the
 * plugin Python class.
 * 
 * @param   gp  The protocol widget used by the plugin.
 */
static void remmina_protocol_init_wrapper(RemminaProtocolWidget *gp)
{
	TRACE_CALL(__func__);
    PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
    py_plugin->gp->gp = gp;
    PyObject_CallMethod(py_plugin, "init", "O", py_plugin->gp);
}

/**
 * @brief 
 * @details
 * 
 * @param   gp  The protocol widget used by the plugin.
 */
static gboolean remmina_protocol_open_connection_wrapper(RemminaProtocolWidget *gp)
{
	TRACE_CALL(__func__);
    remmina_plugin_manager_service.protocol_plugin_signal_connection_opened(gp);
    PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
    PyObject* result = PyObject_CallMethod(py_plugin, "open_connection", "O", py_plugin->gp);
    return result == Py_True;
}

/**
 * @brief 
 * @details
 * 
 * @param   gp  The protocol widget used by the plugin.
 */
static gboolean remmina_protocol_close_connection_wrapper(RemminaProtocolWidget *gp)
{
	TRACE_CALL(__func__);
    PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
    PyObject* result = PyObject_CallMethod(py_plugin, "close_connection", "O", py_plugin->gp);
    return result == Py_True;
}

/**
 * @brief 
 * @details
 * 
 * @param   gp  The protocol widget used by the plugin.
 */
static gboolean remmina_protocol_query_feature_wrapper(RemminaProtocolPlugin* plugin, RemminaProtocolWidget *gp, const RemminaProtocolFeature *feature)
{
	TRACE_CALL(__func__);
    PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
    PyObject* result = PyObject_CallMethod(py_plugin, "query_feature", "O", py_plugin->gp);
    return result == Py_True;
}

/**
 * @brief 
 * @details
 * 
 * @param   gp  The protocol widget used by the plugin.
 */
static void remmina_protocol_call_feature_wrapper(RemminaProtocolPlugin* plugin, RemminaProtocolWidget *gp, const RemminaProtocolFeature *feature)
{
	TRACE_CALL(__func__);
    PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
    PyObject* result = PyObject_CallMethod(py_plugin, "call_feature", "O", py_plugin->gp);
}

/**
 * @brief 
 * @details
 * 
 * @param   gp  The protocol widget used by the plugin.
 */
static void remmina_protocol_send_keytrokes_wrapper(RemminaProtocolPlugin* plugin, RemminaProtocolWidget *gp, const guint keystrokes[], const gint keylen)
{
	TRACE_CALL(__func__);
    PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
    PyObject* result = PyObject_CallMethod(py_plugin, "send_keystrokes", "O", py_plugin->gp);
}

/**
 * @brief 
 * @details
 * 
 * @param   gp  The protocol widget used by the plugin.
 */
static gboolean remmina_protocol_get_plugin_screenshot_wrapper(RemminaProtocolPlugin* plugin, RemminaProtocolWidget *gp, RemminaPluginScreenshotData *rpsd)
{
	TRACE_CALL(__func__);
    PyPlugin* py_plugin = remmina_plugin_python_module_get_plugin(gp);
    PyObject* result = PyObject_CallMethod(py_plugin, "get_plugin_screenshot", "O", py_plugin->gp);
    return result == Py_True;
}

RemminaPlugin* remmina_plugin_python_create_protocol_plugin(PyObject* pluginInstance)
{
        RemminaProtocolPlugin* remmina_plugin = (RemminaProtocolPlugin*)malloc(sizeof(RemminaProtocolPlugin));
        
        if(!PyObject_HasAttrString(pluginInstance, "icon_name_ssh")) {
            g_printerr("Error creating Remmina plugin. Python plugin instance is missing member: icon_name_ssh\n");
            return NULL;
        }
        else if(!PyObject_HasAttrString(pluginInstance, "icon_name")) {
            g_printerr("Error creating Remmina plugin. Python plugin instance is missing member: icon_name\n");
            return NULL;
        }
       
        remmina_plugin->type = REMMINA_PLUGIN_TYPE_PROTOCOL;
        remmina_plugin->name = PyUnicode_AsUTF8(PyObject_GetAttrString(pluginInstance, "name"));                                               // Name
        remmina_plugin->description = PyUnicode_AsUTF8(PyObject_GetAttrString(pluginInstance, "description"));             // Description
        remmina_plugin->domain = GETTEXT_PACKAGE;                                    // Translation domain
        remmina_plugin->version = PyUnicode_AsUTF8(PyObject_GetAttrString(pluginInstance, "version"));                                           // Version number
        remmina_plugin->icon_name = PyUnicode_AsUTF8(PyObject_GetAttrString(pluginInstance, "icon_name"));                         // Icon for normal connection
        remmina_plugin->icon_name_ssh = PyUnicode_AsUTF8(PyObject_GetAttrString(pluginInstance, "icon_name_ssh"));                     // Icon for SSH connection
        remmina_plugin->basic_settings = NULL;                // Array for basic settings
        remmina_plugin->advanced_settings = NULL;                                    // Array for advanced settings
        remmina_plugin->ssh_setting = REMMINA_PROTOCOL_SSH_SETTING_TUNNEL;        // SSH settings type
        remmina_plugin->features = NULL;                     // Array for available features
        remmina_plugin->init = remmina_protocol_init_wrapper ;                             // Plugin initialization
        remmina_plugin->open_connection = remmina_protocol_open_connection_wrapper ;       // Plugin open connection
        remmina_plugin->close_connection = remmina_protocol_close_connection_wrapper ;     // Plugin close connection
        remmina_plugin->query_feature = remmina_protocol_query_feature_wrapper ;           // Query for available features
        remmina_plugin->call_feature = remmina_protocol_call_feature_wrapper ;             // Call a feature
        remmina_plugin->send_keystrokes = remmina_protocol_send_keytrokes_wrapper;                                      // Send a keystroke
        remmina_plugin->get_plugin_screenshot = remmina_protocol_get_plugin_screenshot_wrapper;                                // Screenshot support unavailable

        return remmina_plugin;
}

RemminaPlugin* remmina_plugin_python_create_entry_plugin(pluginInstance)
{

}
RemminaPlugin* remmina_plugin_python_create_file_plugin(pluginInstance)
{

}
RemminaPlugin* remmina_plugin_python_create_tool_plugin(pluginInstance)
{

}
RemminaPlugin* remmina_plugin_python_create_pref_plugin(pluginInstance)
{

}
RemminaPlugin* remmina_plugin_python_create_secret_plugin(pluginInstance)
{

}
