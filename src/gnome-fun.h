#ifndef GNOME_FUN_H
#define GNOME_FUN_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

/* Copy-paste of necessary prototypes and definitions from GNOME include files.
   Therefore, GNOME includes are not needed to compile this code. */

#ifndef __G_TYPES_H__
typedef char gchar;
typedef int gint;
typedef short gshort;
typedef long glong;
typedef unsigned char guchar;
typedef unsigned int guint;
typedef unsigned short gushort;
typedef unsigned long gulong;
typedef float gfloat;
typedef double gdouble;
typedef size_t gsize;
typedef gint gboolean;
typedef void *gpointer;
typedef const void *gconstpointer;
#endif // __G_TYPES_H__

#ifndef __G_TYPE_H__
typedef gsize GType;

typedef struct _GTypeClass GTypeClass;
typedef struct _GTypeInstance GTypeInstance;
#endif // __G_TYPE_H__

#ifndef __G_MAIN_H__
typedef gboolean (*GSourceFunc)(gpointer user_data);
#endif // __G_MAIN_H__

#ifndef __GTK_H__
typedef enum
{
	GTK_MESSAGE_INFO,
	GTK_MESSAGE_WARNING,
	GTK_MESSAGE_QUESTION,
	GTK_MESSAGE_ERROR,
	GTK_MESSAGE_OTHER
} GtkMessageType;

typedef enum
{
	GTK_BUTTONS_NONE,
	GTK_BUTTONS_OK,
	GTK_BUTTONS_CLOSE,
	GTK_BUTTONS_CANCEL,
	GTK_BUTTONS_YES_NO,
	GTK_BUTTONS_OK_CANCEL
} GtkButtonsType;

typedef enum
{
	GTK_RESPONSE_NONE         = -1,
	GTK_RESPONSE_REJECT       = -2,
	GTK_RESPONSE_ACCEPT       = -3,
	GTK_RESPONSE_DELETE_EVENT = -4,
	GTK_RESPONSE_OK           = -5,
	GTK_RESPONSE_CANCEL       = -6,
	GTK_RESPONSE_CLOSE        = -7,
	GTK_RESPONSE_YES          = -8,
	GTK_RESPONSE_NO           = -9,
	GTK_RESPONSE_APPLY        = -10,
	GTK_RESPONSE_HELP         = -11
} GtkResponseType;

typedef enum
{
	GTK_DIALOG_MODAL               = 1 << 0,
	GTK_DIALOG_DESTROY_WITH_PARENT = 1 << 1,
	GTK_DIALOG_USE_HEADER_BAR      = 1 << 2
} GtkDialogFlags;

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkButton GtkButton;
typedef struct _GtkWindow GtkWindow;
typedef struct _GtkDialog GtkDialog;
#endif // __GTK_H__

/* Implementation bits */
#if defined(G_DISABLE_CAST_CHECKS) || defined(__OPTIMIZE__)
#define _DYN_G_TYPE_CIC(ip, gt, ct) ((ct *)(void *)ip)
#define _DYN_G_TYPE_CCC(cp, gt, ct) ((ct *)(void *)cp)
#else
#define _DYN_G_TYPE_CIC(ip, gt, ct) ((ct *)(void *)dynGObject.g_type_check_instance_cast((GTypeInstance *)ip, gt))
#define _DYN_G_TYPE_CCC(cp, gt, ct) ((ct *)(void *)dynGObject.g_type_check_class_cast((GTypeClass *)cp, gt))
#endif

#define DYN_G_TYPE_CHECK_INSTANCE_CAST(instance, g_type, c_type) (_DYN_G_TYPE_CIC((instance), (g_type), c_type))
#define DYN_G_TYPE_CHECK_CLASS_CAST(g_class, g_type, c_type) (_DYN_G_TYPE_CCC((g_class), (g_type), c_type))

#define DYN_GTK_TYPE_WIDGET (dynGtk.gtk_widget_get_type())
#define DYN_GTK_TYPE_BUTTON (dynGtk.gtk_button_get_type())
#define DYN_GTK_TYPE_WINDOW (dynGtk.gtk_window_get_type())
#define DYN_GTK_TYPE_DIALOG (dynGtk.gtk_dialog_get_type())

#define DYN_GTK_WIDGET(widget) (DYN_G_TYPE_CHECK_INSTANCE_CAST((widget), DYN_GTK_TYPE_WIDGET, GtkWidget))
#define DYN_GTK_BUTTON(obj) (DYN_G_TYPE_CHECK_INSTANCE_CAST((obj), DYN_GTK_TYPE_BUTTON, GtkButton))
#define DYN_GTK_WINDOW(obj) (DYN_G_TYPE_CHECK_INSTANCE_CAST((obj), DYN_GTK_TYPE_WINDOW, GtkWindow))
#define DYN_GTK_DIALOG(obj) (DYN_G_TYPE_CHECK_INSTANCE_CAST((obj), DYN_GTK_TYPE_DIALOG, GtkDialog))

/* Macro aliases */
#ifndef __GTK_H__
#define GTK_WIDGET DYN_GTK_WIDGET
#define GTK_BUTTON DYN_GTK_BUTTON
#define GTK_WINDOW DYN_GTK_WINDOW
#define GTK_DIALOG DYN_GTK_DIALOG
#endif // __GTK_H__

/* Glib prototypes */
typedef guint (*GIDLEADDPROC)(GSourceFunc function, gpointer data);

/* GObject prototypes */
typedef GTypeInstance *(*GTYPECHECKINSTANCECASTPROC)(GTypeInstance *instance, GType iface_typ);
typedef GTypeClass *(*GTYPECHECKCLASSCAST)(GTypeClass *g_class, GType is_a_type);

/* Gtk prototypes */
typedef void (*GTKINITPROC)(int *argc, char ***argv);
typedef gboolean (*GTKINITCHECKPROC)(int *argc, char ***argv);
typedef void (*GTKMAINPROC)(void);
typedef void (*GTKMAINQUITPROC)(void);
typedef GType (*GTKWIDGETGETTYPEPROC)(void);
typedef void (*GTKWIDGETDESTROYPROC)(GtkWidget *widget);
typedef GType (*GTKBUTTONGETTYPEPROC)(void);
typedef void (*GTKBUTTONSETLABELPROC)(GtkButton *button, const gchar *label);
typedef GType (*GTKWINDOWGETTYPEPROC)(void);
typedef void (*GTKWINDOWSETTITLEPROC)(GtkWindow *window, const gchar *title);
typedef GType (*GTKDIALOGGETTYPEPROC)(void);
typedef gint (*GTKDIALOGRUNPROC)(GtkDialog *dialog);
typedef GtkWidget *(*GTKDIALOGGETWIDGETFORRESPONSEPROC)(GtkDialog *dialog, gint response_id);
typedef GtkWidget *(*GTKMESSAGEDIALOGNEWPROC)(GtkWindow *parent, GtkDialogFlags flags, GtkMessageType type, GtkButtonsType buttons, const gchar *message_format, ...);

#define GNOME_FUNC(name, type) type name;

#define DYN_GLIB_FUNCS \
	GNOME_FUNC(g_idle_add, GIDLEADDPROC)

#define DYN_GOBJECT_FUNCS \
	GNOME_FUNC(g_type_check_instance_cast, GTYPECHECKINSTANCECASTPROC) \
	GNOME_FUNC(g_type_check_class_cast, GTYPECHECKCLASSCAST)

#define DYN_GTK_FUNCS \
	GNOME_FUNC(gtk_init, GTKINITPROC) \
	GNOME_FUNC(gtk_init_check, GTKINITCHECKPROC) \
	GNOME_FUNC(gtk_main, GTKMAINPROC) \
	GNOME_FUNC(gtk_main_quit, GTKMAINQUITPROC) \
	GNOME_FUNC(gtk_widget_get_type, GTKWIDGETGETTYPEPROC) \
	GNOME_FUNC(gtk_widget_destroy, GTKWIDGETDESTROYPROC) \
	GNOME_FUNC(gtk_button_get_type, GTKBUTTONGETTYPEPROC) \
	GNOME_FUNC(gtk_button_set_label, GTKBUTTONSETLABELPROC) \
	GNOME_FUNC(gtk_window_get_type, GTKWINDOWGETTYPEPROC) \
	GNOME_FUNC(gtk_window_set_title, GTKWINDOWSETTITLEPROC) \
	GNOME_FUNC(gtk_dialog_get_type, GTKDIALOGGETTYPEPROC) \
	GNOME_FUNC(gtk_dialog_run, GTKDIALOGRUNPROC) \
	GNOME_FUNC(gtk_dialog_get_widget_for_response, GTKDIALOGGETWIDGETFORRESPONSEPROC) \
	GNOME_FUNC(gtk_message_dialog_new, GTKMESSAGEDIALOGNEWPROC)

struct GlibFunctions
{
	DYN_GLIB_FUNCS
};

struct GObjectFunctions
{
	DYN_GOBJECT_FUNCS
};

struct GtkFunctions
{
	DYN_GTK_FUNCS
};

#undef GNOME_FUNC

extern struct GlibFunctions dynGlib;
extern struct GObjectFunctions dynGObject;
extern struct GtkFunctions dynGtk;

void initGlibFunctions();
void initGObjectFunctions();
void initGtkFunctions();

void initGnomeFunctions();

#endif // GNOME_FUN_H
