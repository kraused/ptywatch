
/* Plugin for signaling via dbus
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <gio/gio.h>

#include "ptywatch-dbus-signal.h"

#include "common.h"
#include "error.h"
#include "plugin.h"

struct Dbus_Signal_Plugin
{
	struct PtyWatch_Plugin	base;

	GDBusConnection		*conn;
	guint			name;
	PtyWatchDbusSignal	*obj;
};


static SInt32 Stdout_Send_Msg(struct PtyWatch_Plugin *plugin, const char *msg, SInt64 msglen)
{
	struct Dbus_Signal_Plugin *self = (struct Dbus_Signal_Plugin *)plugin;
	gchar *summary;

	summary = g_strdup_printf("PtyWatch message");

	pty_watch_dbus_signal_emit_message(self->obj, summary, msg);

	while (g_main_pending()) {
		g_main_iteration(TRUE);
	}

	g_free(summary);

	return 0;
}

static void On_Bus_Name_Acquired(GDBusConnection *conn, const gchar *name, gpointer udata)
{
}

static void On_Bus_Name_Lost(GDBusConnection *conn, const gchar *name, gpointer udata)
{
}

struct Dbus_Signal_Plugin dbus_signal_plugin =
{
	.base =
	{
		.name     = "Dbus_Signal",
		.version  = 1,

		.Send_Msg = Stdout_Send_Msg
	},
	.conn = NULL,
	.obj  = NULL
};

SInt32 Plugin_Construct(struct PtyWatch_Plugin **plugin)
{
	GError *error;
	gboolean success;

	error = NULL;

	dbus_signal_plugin.conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (UNLIKELY(!dbus_signal_plugin.conn)) {
		PTYWATCH_ERROR("Failed to get bus: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	dbus_signal_plugin.name = g_bus_own_name_on_connection(dbus_signal_plugin.conn,
	                                                       "org.freedesktop.PtyWatchDbusSignal",
	                                                       G_BUS_NAME_OWNER_FLAGS_NONE,
	                                                       On_Bus_Name_Acquired,
	                                                       On_Bus_Name_Lost,
	                                                       NULL,
	                                                       NULL);

	dbus_signal_plugin.obj = pty_watch_dbus_signal_skeleton_new();

	error = NULL;

        success = g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(dbus_signal_plugin.obj),
                                                   dbus_signal_plugin.conn,
                                                   "/org/freedesktop/PtyWatchDbusSignal",
                                                   &error);
        if (UNLIKELY(!success)) {
                PTYWATCH_ERROR("Failed to export skeleton: %s\n", error->message);
                g_error_free(error);
                return -1;
        }

	while (g_main_pending()) {
		g_main_iteration(TRUE);
	}

	*plugin = (struct PtyWatch_Plugin *)&dbus_signal_plugin;

	return 0;
}

SInt32 Plugin_Destruct(struct PtyWatch_Plugin *plugin)
{
	g_bus_unown_name(dbus_signal_plugin.name);

	g_object_unref(G_OBJECT(dbus_signal_plugin.conn));
	g_object_unref(G_OBJECT(dbus_signal_plugin.obj));

	return 0;
}

