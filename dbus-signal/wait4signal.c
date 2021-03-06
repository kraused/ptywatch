
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <glib.h>
#include <gio/gio.h>

#include "common.h"
#include "error.h"

struct Message
{
	char	*argv0;
	char	*origin;
	char	*summary;
	char	*body;
};

void *Thread_Start_Routine(void *arg)
{
	struct Message *msg = (struct Message *)arg;

#if !defined(NDEBUG)
        fprintf(stderr, "Origin : %s\n", msg->origin);
        fprintf(stderr, "Summary: %s\n", msg->summary);
	/* +1 to get rid of the prepended \r of the wall message.
	 */
        fprintf(stderr, "Body   : \"%s\"\n", msg->body + 1);
#endif

	GSubprocess *proc;
	gboolean success;
	GError *error;
	GBytes *buf;

	error = NULL;
	proc  = g_subprocess_new(G_SUBPROCESS_FLAGS_STDIN_PIPE | G_SUBPROCESS_FLAGS_STDOUT_SILENCE | G_SUBPROCESS_FLAGS_STDERR_SILENCE,
	                         &error,
	                         (char *)msg->argv0,
	                         "--origin", msg->origin,
	                         "--summary", msg->summary,
	                         NULL);
	if (UNLIKELY(!proc)) {
		PTYWATCH_ERROR("g_subprocess_new() failed: %s\n", error->message);
		g_error_free(error);
	}

	buf = g_bytes_new(msg->body, strlen(msg->body));

	error   = NULL;
	success = g_subprocess_communicate(proc,
	                                   buf,
	                                   NULL,
	                                   NULL,
	                                   NULL,
	                                   &error);
	if (UNLIKELY((!success) && error)) {
		PTYWATCH_ERROR("g_subprocess_communicate() failed: %s\n", error->message);
		g_error_free(error);
	}

	g_object_unref(G_OBJECT(proc));
	g_bytes_unref(buf);

	free(msg->argv0);
	free(msg->origin);
	free(msg->summary);
	free(msg->body);
	free(msg);

	return NULL;
}

SInt32 Create_Detached_Thread(struct Message *msg)
{
	SInt32 err;
	pthread_attr_t attr;
	pthread_t thr;

	err = pthread_attr_init(&attr);
	if (UNLIKELY(err)) {
		errno = err;
		PTYWATCH_ERROR("pthread_attr_init() failed with error %d (%s)", errno, strerror(errno));
		return -err;
	}

	err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (UNLIKELY(err)) {
		errno = err;
		PTYWATCH_ERROR("pthread_attr_setdetachstate() failed with error %d (%s)", errno, strerror(errno));
		return -err;
	}

	err = pthread_create(&thr, &attr, Thread_Start_Routine, (void *)msg);
	if (UNLIKELY(err)) {
		errno = err;
		PTYWATCH_ERROR("pthread_create() failed with error %d (%s)", errno, strerror(errno));
		return -err;
	}

	err = pthread_attr_destroy(&attr);
	if (UNLIKELY(err)) {
		errno = err;
		PTYWATCH_ERROR("pthread_attr_destroy() failed with error %d (%s)", errno, strerror(errno));
	}

	return 0;
}

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
	struct Message *msg;

	summary = g_variant_get_child_value(parameters, 0);
	body    = g_variant_get_child_value(parameters, 1);

	msg = malloc(sizeof(struct Message));
	if (UNLIKELY(!msg)) {
		PTYWATCH_ERROR("malloc() failed.");
		return;
	}

	msg->argv0   = strdup((char *)user_data);
	msg->origin  = strdup(object_path);
	msg->summary = strdup(g_variant_get_string(summary, NULL));
	msg->body    = strdup(g_variant_get_string(body, NULL));

	(void )Create_Detached_Thread(msg);

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
	                                            argv[1],
	                                            NULL);

	g_main_loop_run(loop);

	g_dbus_connection_signal_unsubscribe(conn, subscr);
	g_object_unref(G_OBJECT(conn));
	g_main_loop_unref(loop);

	return 0;
}

