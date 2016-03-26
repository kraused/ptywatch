
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>

#include <glib.h>
#include <gio/gio.h>

#include "common.h"
#include "error.h"
#include "notifications.h"

static gboolean On_Close_Notification(Notifications *object, GDBusMethodInvocation *invocation, guint id)
{
	return FALSE;
}

static gboolean On_Get_Capabilities(Notifications *object, GDBusMethodInvocation *invocation)
{
	return FALSE;
}

static gboolean On_Get_Server_Information(Notifications *object, GDBusMethodInvocation *invocation)
{
	gchar *name;
	gchar *vendor;
	gchar *version;
	gchar *spec_version;

	name         = g_strdup_printf("notifyd");
	vendor       = g_strdup_printf("D. Krause");
	version      = g_strdup_printf("0.1");
	spec_version = g_strdup_printf("1.2");

	notifications_complete_get_server_information(object, invocation, name, vendor, version, spec_version);

	g_free(spec_version);
	g_free(version);
	g_free(vendor);
	g_free(name);

	return TRUE;
}

static SInt32 notificationId = 1;

static gboolean On_Handle_Notify(Notifications *object,
                             GDBusMethodInvocation *invocation,
                             const gchar *app_name,
                             guint replaces_id,
                             const gchar *app_icon,
                             const gchar *summary,
                             const gchar *body,
                             const gchar *const *actions,
                             GVariant *hints,
                             gint expire_timeout)
{
	guint id;

	fprintf(stdout, "Origin : %s\n", app_name);
	fprintf(stdout, "Summary: %s\n", summary);
	fprintf(stdout, "Body   : \"%s\"\n", body);

	id = replaces_id;
	if (0 == replaces_id) {
		id = notificationId++;
	}

	notifications_complete_notify(object, invocation, id);

	return TRUE;
}

static gboolean On_Action_Invoked(Notifications *object, guint id, const gchar *action_key)
{
	printf("....\n");
	return FALSE;
}

static gboolean On_Notification_Closed(Notifications *object, guint id, guint reason)
{
	return FALSE;
}

static Notifications *object = NULL;

static void On_Bus_Acquired(GDBusConnection *conn, const gchar *name, gpointer udata)
{
	GError *error;
	gboolean success;

	if (LIKELY(!object)) {
		object = notifications_skeleton_new();

		g_signal_connect(object, "handle-get-server-information", G_CALLBACK(On_Get_Server_Information), NULL);
		g_signal_connect(object, "handle-notify", G_CALLBACK(On_Handle_Notify), NULL);
		g_signal_connect(object, "handle-close-notification", G_CALLBACK(On_Close_Notification), NULL);
		g_signal_connect(object, "handle-get-capabilities", G_CALLBACK(On_Get_Capabilities), NULL);
		g_signal_connect(object, "action-invoked", G_CALLBACK(On_Action_Invoked), NULL);
		g_signal_connect(object, "notification-closed", G_CALLBACK(On_Notification_Closed), NULL);
	}

	error = NULL;

	success = g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(object),
	                                           conn,
	                                           "/org/freedesktop/Notifications",
	                                           &error);
	if (UNLIKELY(!success)) {
		PTYWATCH_ERROR("Failed to export skeleton: %s\n", error->message);
		g_error_free(error);
		return;
	}
}

static void On_Bus_Name_Acquired(GDBusConnection *conn, const gchar *name, gpointer udata)
{
}

static void On_Bus_Name_Lost(GDBusConnection *conn, const gchar *name, gpointer udata)
{
	g_dbus_interface_skeleton_unexport(G_DBUS_INTERFACE_SKELETON(object));
}

int main(int argc, char **argv)
{
	guint name;
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	name = g_bus_own_name(G_BUS_TYPE_SESSION,
	                      "org.freedesktop.Notifications",
	                      G_BUS_NAME_OWNER_FLAGS_NONE,
	                      On_Bus_Acquired,
	                      On_Bus_Name_Acquired,
	                      On_Bus_Name_Lost,
	                      loop,
	                      NULL);

	g_main_loop_run(loop);

	g_bus_unown_name(name);

	g_main_loop_unref(loop);

	return 0;
}

