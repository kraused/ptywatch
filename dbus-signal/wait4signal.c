
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <gio/gio.h>

#include "common.h"
#include "error.h"

void On_Caught_Signal(GDBusConnection *connection,
                      const gchar *sender_name,
                      const gchar *object_path,
                      const gchar *interface_name,
                      const gchar *signal_name,
                      GVariant *parameters,
                      gpointer user_data)
{
	GVariant *summary;
	GVariant *body;

	summary = g_variant_get_child_value(parameters, 0);
	body    = g_variant_get_child_value(parameters, 1);

        fprintf(stdout, "Origin : %s\n", object_path);
        fprintf(stdout, "Summary: %s\n", g_variant_get_string(summary, NULL));
	/* +1 to get rid of the prepended \r of the wall message.
	 */
        fprintf(stdout, "Body   : \"%s\"\n", g_variant_get_string(body, NULL) + 1);

	g_variant_unref(summary);
	g_variant_unref(body);
}

int main(int argc, char **argv)
{
	GMainLoop *loop;
	GDBusConnection *conn;
	GError *error;
	guint subscr;

	loop = g_main_loop_new(NULL, FALSE);

	error = NULL;
	conn  = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (UNLIKELY(!conn)) {
		PTYWATCH_ERROR("Failed to get bus: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	subscr = g_dbus_connection_signal_subscribe(conn,
	                                            NULL, /* sender */
	                                            "org.freedesktop.PtyWatchDbusSignal", /* interface */
	                                            NULL, /* member */
	                                            "/org/freedesktop/PtyWatchDbusSignal", /* object path */
	                                            NULL, /* arg0 */
	                                            G_DBUS_SIGNAL_FLAGS_NONE,
	                                            On_Caught_Signal,
	                                            NULL,
	                                            NULL);

	g_main_loop_run(loop);

	g_dbus_connection_signal_unsubscribe(conn, subscr);
	g_object_unref(G_OBJECT(conn));
	g_main_loop_unref(loop);

	return 0;
}

