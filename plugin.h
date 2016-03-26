
#ifndef PTYWATCVH_PLUGIN_HXX_INCLUDED
#define PTYWATCVH_PLUGIN_HXX_INCLUDED 1

/* Datastructure provided by DSO plugins to register
 * with the main application.
 */
struct PtyWatch_Plugin
{
	void		*_handle;

	const char	*name;
	SInt32		version;

	SInt32		(*Send_Msg)(struct PtyWatch_Plugin *plugin, const char *msg, SInt64 msglen);

};

SInt32 Load_Plugin(const char *path, struct PtyWatch_Plugin **plugin);
SInt32 Unload_Plugin(struct PtyWatch_Plugin *plugin);

#endif

