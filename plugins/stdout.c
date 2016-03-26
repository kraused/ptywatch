
/* Plugin for writing messages to stdout
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "error.h"
#include "plugin.h"

static SInt32 Stdout_Send_Msg(struct PtyWatch_Plugin *plugin, const char *msg, SInt64 msglen)
{
	fprintf(stdout, "%s", msg);

	return 0;
}

struct PtyWatch_Plugin stdout_plugin =
{
	.name     = "stdout",
	.version  = 1,

	.Send_Msg = Stdout_Send_Msg
};

SInt32 Plugin_Construct(struct PtyWatch_Plugin **plugin)
{
	*plugin = &stdout_plugin;

	return 0;
}

SInt32 Plugin_Destruct(struct PtyWatch_Plugin *plugin)
{
	return 0;
}

