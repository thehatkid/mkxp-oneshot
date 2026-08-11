#ifndef XFCONF_FUN_H
#define XFCONF_FUN_H

#include "gnome-fun.h"

#ifndef __XFCONF_CHANNEL_H__
typedef struct _XfconfChannel XfconfChannel;
#endif // __XFCONF_CHANNEL_H__

/* Xfconf prototypes */
typedef gboolean (*XFCONFINITPROC)(GError **error);
typedef void (*XFCONFSHUTDOWNPROC)(void);
typedef XfconfChannel *(*XFCONFCHANNELGETPROC)(const gchar *channel_name);
typedef gboolean (*XFCONFCHANNELGETPROPERTYPROC)(XfconfChannel *channel, const gchar *property, GValue *value);
typedef gboolean (*XFCONFCHANNELSETPROPERTYPROC)(XfconfChannel *channel, const gchar *property, const GValue *value);
typedef void (*XFCONFCHANNELRESETPROPERTYPROC)(XfconfChannel *channel, const gchar *property_base, gboolean recursive);
typedef gchar *(*XFCONFCHANNELGETSTRINGPROC)(XfconfChannel *channel, const gchar *property, const gchar *default_value);
typedef gboolean (*XFCONFCHANNELSETSTRINGPROC)(XfconfChannel *channel, const gchar *property, const gchar *value);
typedef gint32 (*XFCONFCHANNELGETINTPROC)(XfconfChannel *channel, const gchar *property, gint32 default_value);
typedef gboolean (*XFCONFCHANNELSETINTPROC)(XfconfChannel *channel, const gchar *property, gint32 value);
typedef gboolean (*XFCONFCHANNELGETARRAYPROC)(XfconfChannel *channel, const gchar *property, GType first_value_type, ...);
typedef gboolean (*XFCONFCHANNELSETARRAYPROC)(XfconfChannel *channel, const gchar *property, GType first_value_type, ...);

#define XFCONF_FUNC(name, type) type name;

#define DYN_XFCONF_FUNCS \
	XFCONF_FUNC(xfconf_init, XFCONFINITPROC) \
	XFCONF_FUNC(xfconf_shutdown, XFCONFSHUTDOWNPROC) \
	XFCONF_FUNC(xfconf_channel_get, XFCONFCHANNELGETPROC) \
	XFCONF_FUNC(xfconf_channel_get_property, XFCONFCHANNELGETPROPERTYPROC) \
	XFCONF_FUNC(xfconf_channel_set_property, XFCONFCHANNELSETPROPERTYPROC) \
	XFCONF_FUNC(xfconf_channel_reset_property, XFCONFCHANNELRESETPROPERTYPROC) \
	XFCONF_FUNC(xfconf_channel_get_string, XFCONFCHANNELGETSTRINGPROC) \
	XFCONF_FUNC(xfconf_channel_set_string, XFCONFCHANNELSETSTRINGPROC) \
	XFCONF_FUNC(xfconf_channel_get_int, XFCONFCHANNELGETINTPROC) \
	XFCONF_FUNC(xfconf_channel_set_int, XFCONFCHANNELSETINTPROC) \
	XFCONF_FUNC(xfconf_channel_get_array, XFCONFCHANNELGETARRAYPROC) \
	XFCONF_FUNC(xfconf_channel_set_array, XFCONFCHANNELSETARRAYPROC)

struct XfconfFunctions
{
	DYN_XFCONF_FUNCS
};

#undef XFCONF_FUNC

extern struct XfconfFunctions dynXfconf;

void initXfconfFunctions();

#endif // XFCONF_FUN_H
