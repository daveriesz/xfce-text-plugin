
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <gtk/gtkdialog.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>

#include "data.h"
#include "debug.h"
#include "config.h"

static void xtp_orientation_changed(XfcePanelPlugin *plugin, GtkOrientation orientation, XfceTextPluginData *xtpd);
static void xtp_screen_position_changed(XfcePanelPlugin *plugin, XfceScreenPosition *position, XfceTextPluginData *xtpd);
static gboolean xtp_size_changed(XfcePanelPlugin *plugin, gint size, XfceTextPluginData *xtpd);
static void xtp_about(XfcePanelPlugin *plugin, XfceTextPluginData *xtpd);
static gboolean xtp_button_press(XfcePanelPlugin *plugin, GdkEvent *event, XfceTextPluginData *xtpd);
static void xtp_set_timer_func(XfceTextPluginData *xtpd, XtpDialogWidgets *dw);
static void xtp_remove_timer_func(XfceTextPluginData *xtpd, XtpDialogWidgets *dw);
//static void xtp_process_tooltip(GtkWidget *w, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip, XfceTextPluginData *xtpd);

static void xtp_build_panel_plugin(XfceTextPluginData *xtpd)
{
	xpf("xtp_build_panel_plugin()\n");

	xtpd->ll.markup_widget = gtk_label_new("");
	gtk_widget_set_has_tooltip(xtpd->ll.markup_widget, xtpd->tt.enabled);
	xtpd->tt.set_markup(xtpd, xtpd->tt.cmd);
	xtpd->pp.markup_widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
}

static void xtp_connect_signals(XfceTextPluginData *xtpd)
{
	g_signal_connect(G_OBJECT(xtpd->plugin), "size-changed"           , G_CALLBACK(xtp_size_changed           ), xtpd);
	g_signal_connect(G_OBJECT(xtpd->plugin), "configure-plugin"       , G_CALLBACK(xtp_configure_plugin       ), xtpd);
	g_signal_connect(G_OBJECT(xtpd->plugin), "about"                  , G_CALLBACK(xtp_about                  ), xtpd);
	g_signal_connect(G_OBJECT(xtpd->plugin), "orientation-changed"    , G_CALLBACK(xtp_orientation_changed    ), xtpd);
	g_signal_connect(G_OBJECT(xtpd->plugin), "screen-position-changed", G_CALLBACK(xtp_screen_position_changed), xtpd);
	g_signal_connect(G_OBJECT(xtpd->plugin), "free-data"              , G_CALLBACK(xtp_free_data              ), xtpd);
//	g_signal_connect(G_OBJECT(xtpd->plugin), "save"                   , G_CALLBACK(xtp_save                   ), xtpd);
	g_signal_connect(G_OBJECT(xtpd->plugin), "button-press-event"     , G_CALLBACK(xtp_button_press           ), xtpd);

//	g_signal_connect(G_OBJECT(xtpd->ll.markup_widget), "query-tooltip", G_CALLBACK(xtp_process_tooltip), xtpd);
}

//static void xtp_process_tooltip(GtkWidget *w, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip, XfceTextPluginData *xtpd)
//{
//	xpf("xtp_process_tooltip()\n");
//}

static void xtp_orientation_changed(XfcePanelPlugin *plugin, GtkOrientation orientation, XfceTextPluginData *xtpd)
{
	xpf("xtp_orientation_changed()\n");
}

static void xtp_screen_position_changed(XfcePanelPlugin *plugin, XfceScreenPosition *position, XfceTextPluginData *xtpd)
{
	xpf("xtp_screen_position_changed()\n");
}

static gboolean xtp_size_changed(XfcePanelPlugin *plugin, gint size, XfceTextPluginData *xtpd)
{
	xpf("xtp_size_changed()\n");
	return TRUE;
}

static void xtp_about(XfcePanelPlugin *plugin, XfceTextPluginData *xtpd)
{
	GdkPixbuf *icon = NULL;
	const gchar *auth[] = {
		"Dave Riesz <dave@riesz.net>",
		NULL };
	xpf("xtp_about()\n");
	icon = xfce_panel_pixbuf_from_source("plugin-notification", NULL, 48);
	gtk_show_about_dialog
		(NULL,
		 "logo", icon,
		 "license", xfce_get_license_text(XFCE_LICENSE_TEXT_GPL),
		 "version", "0.1",
		 "program-name", "XFCE Text Plugin",
		 "comments", "Show custom information",
		 "website", "http://xyz.riesz.net/",
		 "copyright", "Copyright (c) 2016\n",
		 "authors", auth,
		 NULL);

	if (icon) { g_object_unref(G_OBJECT(icon)); }
}

static gboolean xtp_button_press(XfcePanelPlugin *plugin, GdkEvent *event, XfceTextPluginData *xtpd)
{
	xpf("xtp_button_press() button %d\n", ((GdkEventButton *)event)->button);
	if((((GdkEventButton*)event)->button == 1) && xtpd->pp.enabled && xtpd->pp.markup_widget)
	{
		xpf("   popup window is enabled\n");
		if(gtk_widget_get_visible(xtpd->pp.markup_widget))
		{
			xpf("   hide popup window\n");
			xtp_remove_timer_func(xtpd, &(xtpd->pp));
			gtk_widget_hide(xtpd->pp.markup_widget);
		}
		else
		{
			xpf("   show popup window\n");
			xtp_set_timer_func(xtpd, &(xtpd->pp));
			gtk_widget_show(xtpd->pp.markup_widget);
		}
	}
	return FALSE;
}

/* universal upate */
gboolean xtp_update_universal(XtpDialogWidgets *dw)
{
	int fd, sz, ii;
	gchar *cp;
	FILE *fp;
//	XPF_ASSIGN_XTPD(dw->xtpd);

#if 0
	xpf("xtp_update_universal()\n");
	xpf("   item: %s\n", dw->name);
	xpf("   dw->mode = %s\n",
		(dw->mode==xtp_exec)?("xtp_exec"):((dw->mode==xtp_file)?("xtp_file"):((dw->mode==xtp_stat)?("xtp_stat"):("default"))));
	xpf("   dw->cmd = >>%s<<\n", dw->cmd);
#endif

	switch(dw->mode)
	{
		case xtp_exec:
			if(!(fp = popen(dw->cmd, "r")))
			{
				dw->set_markup(dw->xtpd, "exec failed");
				break;
			}
			for(ii=0 ; fgets(dw->buffer, CMDBUFSZ, fp) ; ii++)
			{
				if(ii > 0) { continue; }
				if(cp = strrchr(dw->buffer, '\n')) { *cp = '\0'; }
				dw->set_markup(dw->xtpd, dw->buffer);
			}
			pclose(fp);
			break;
		case xtp_file:
			if((fd = open(dw->cmd, O_RDONLY)) < 0)
			{
				dw->set_markup(dw->xtpd, "file open failed");
				break;
			}
			sz = read(fd, dw->buffer, CMDBUFSZ-1);
			dw->buffer[sz] = '\0';
			if(cp = strrchr(dw->buffer, '\n')) { *cp = '\0'; }
			dw->set_markup(dw->xtpd, dw->buffer);
			close(fd);
			break;
		case xtp_stat: /* fall-through to default */
		default:
			dw->set_markup(dw->xtpd, dw->cmd);
			break;
	}
	return TRUE;
}

void xtp_ll_set_markup(XfceTextPluginData *xtpd, const gchar *str)
{
	gtk_label_set_markup(GTK_LABEL(xtpd->ll.markup_widget), str);	
}

void xtp_tt_set_markup(XfceTextPluginData *xtpd, const gchar *str)
{
	xpf("xtp_tt_set_markup(): >>%s<<\n", str);
	gtk_widget_set_tooltip_markup(xtpd->ll.markup_widget, str);
}

void xtp_pp_set_markup(XfceTextPluginData *xtpd, const gchar *str)
{
}

#define OKTOGO(x) (((xtpd->is_dirty) && xtpd->x.enabled) || ((xtpd->x.update) && ((xtpd->x.interval) > 0.0) && (xtpd->x.enabled)))

static void xtp_remove_timer_func(XfceTextPluginData *xtpd, XtpDialogWidgets *dw)
{
	xpf("xtp_remove_timer_func()\n");
	if((dw->func_tag) > 0)
	{
		xpf("   removing func tag %d\n", dw->func_tag);
		g_source_remove(dw->func_tag);
	}
}

static void xtp_set_timer_func(XfceTextPluginData *xtpd, XtpDialogWidgets *dw)
{
	xtp_remove_timer_func(xtpd, dw);
	xpf("xtp_set_timer_func()\n");
	xpf("   a\n");
	if(!(dw->enabled)) { return; }
	xpf("   b\n");
	xtp_update_universal(dw); /* first instance */
	xpf("   c\n");
	if(!(dw->update)) { return; }
	xpf("   d\n");
	dw->func_tag = g_timeout_add((guint)((dw->interval)*1000.0), (GSourceFunc)xtp_update_universal, dw);
	xpf("   e\n");
}

gboolean xtp_update(XfceTextPluginData *xtpd)
{
	xpf("xtp_update()\n");

	xtp_set_timer_func(xtpd, &(xtpd->ll));
	xtp_set_timer_func(xtpd, &(xtpd->tt)); // call when set visible -- what signal?
//	xtp_set_timer_func(xtpd, &(xtpd->pp)); // call when set visible

	gtk_widget_set_has_tooltip(xtpd->ll.markup_widget, xtpd->tt.enabled);

	xtpd->is_dirty = FALSE;

	return TRUE;
}

static void xtp_constructor(XfcePanelPlugin *plugin)
{
	XfceTextPluginData *xtpd;

	xtpd = xtp_create_plugin_data(plugin);

	xpf("xtp_constructor()\n");

	xtp_build_panel_plugin (xtpd);
	xtp_build_config_dialog(xtpd);
	xtp_connect_signals    (xtpd);

	xfce_panel_plugin_menu_show_configure(plugin);
	xfce_panel_plugin_menu_show_about    (plugin);

	xtp_update(xtpd);

	gtk_container_add(GTK_CONTAINER(plugin), xtpd->ll.markup_widget);
	gtk_widget_show_all(GTK_WIDGET(plugin));
	
	xpf("   tooltip window = %p\n", gtk_widget_get_tooltip_window(GTK_WIDGET(plugin)));
}


XFCE_PANEL_PLUGIN_REGISTER_INTERNAL(xtp_constructor);
