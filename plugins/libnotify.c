
/* Plugin for writing messages to libnotify
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <libnotify/notify.h>

#include "common.h"
#include "error.h"
#include "plugin.h"

static SInt32 Stdout_Send_Msg(struct PtyWatch_Plugin *plugin, const char *msg, SInt64 msglen)
{
	NotifyNotification *n;
	gboolean success;
	GError *error;
	SInt32 retval;

	retval = 0;

	n = notify_notification_new("PtyWatch message", msg, NULL);
	if (UNLIKELY(!n)) {
		PTYWATCH_ERROR("notify_notification_new() returned NULL");
		return -1;
	}

	error   = NULL;
	success = notify_notification_show(n, &error);
	if (UNLIKELY(FALSE == success)) {
		PTYWATCH_ERROR("notify_notification_show() failed: %s", error->message);
		g_error_free(error);
		retval = -1;
	}

	g_object_unref(G_OBJECT(n));

	return retval;
}

struct PtyWatch_Plugin libnotify_plugin =
{
	.name     = "libnotify",
	.version  = 1,

	.Send_Msg = Stdout_Send_Msg
};

SInt32 Plugin_Construct(struct PtyWatch_Plugin **plugin)
{
	gboolean success;

	success = notify_init("ptywatch");
	if (UNLIKELY(FALSE == success)) {
		PTYWATCH_ERROR("notify_init() failed");
		return -1;
	}

	*plugin = &libnotify_plugin;

	return 0;
}

SInt32 Plugin_Destruct(struct PtyWatch_Plugin *plugin)
{
	notify_uninit();

	return 0;
}

