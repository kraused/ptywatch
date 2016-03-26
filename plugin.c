
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>

#include "config.h"
#include "common.h"
#include "error.h"
#include "plugin.h"

SInt32 Load_Plugin(const char *path, struct PtyWatch_Plugin **plugin)
{
	void *handle;
	SInt32 (*Plugin_Construct)(struct PtyWatch_Plugin **);
	SInt32 err;

	if (UNLIKELY(!plugin)) {
		return -EINVAL;
	}

	*plugin = NULL;

	dlerror();

	handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
	if (UNLIKELY(!handle)) {
		PTYWATCH_ERROR("dlopen() failed. dlerror() says '%s'", dlerror());
		return -1;
	}

	Plugin_Construct = dlsym(handle, "Plugin_Construct");
	if (UNLIKELY(!Plugin_Construct)) {
		PTYWATCH_ERROR("Module does not provide the 'Plugin_Construct' function");
		dlclose(handle);
		return -1;
	}

	err = Plugin_Construct(plugin);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("Plugin constructor failed");
		dlclose(handle);
		*plugin = NULL;
		return err;
	}

	(*plugin)->_handle = handle;

	return 0;
}

SInt32 Unload_Plugin(struct PtyWatch_Plugin *plugin)
{
	void *handle;
	SInt32 (*Plugin_Destruct)(struct PtyWatch_Plugin *);
	SInt32 retval;
	SInt32 err;

	retval = 0;
	handle = plugin->_handle;

	Plugin_Destruct = dlsym(handle, "Plugin_Destruct");
	if (UNLIKELY(!Plugin_Destruct)) {
		PTYWATCH_ERROR("Module does not provide the 'Plugin_Destruct' function");
		retval = -1;
	}

	err = Plugin_Destruct(plugin);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("Plugin destruct failed");
		retval = -1;
	}

	dlerror();

	err = dlclose(handle);
	if (UNLIKELY(err)) {
		PTYWATCH_ERROR("dlopen() failed. dlerror() says '%s'", dlerror());
		retval = -1;
	}

	return retval;
}

