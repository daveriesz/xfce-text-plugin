
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkdialog.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>

#include "data.h"
#include "debug.h"
#include "config.h"

static void xtp_toggle(GtkWidget *w, gboolean v) { gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), v); }

#define MODECASE_1(g,m) case xtp_##m: xtp_toggle(g.btn_##m, TRUE); break;
static void xtp_config_dialog_show(GtkWidget *dlg, XfceTextPluginData *xtpd)
{
	xpf("xtp_config_dialog_show()");

	xtp_toggle(xtpd->tt.chk_ebl, xtpd->tt.enabled);
	xtp_toggle(xtpd->pp.chk_ebl, xtpd->pp.enabled);

	switch(xtpd->ll.mode) { MODECASE_1(xtpd->ll,exec) MODECASE_1(xtpd->ll,file) MODECASE_1(xtpd->ll,stat) default: break; }
	switch(xtpd->tt.mode) { MODECASE_1(xtpd->tt,exec) MODECASE_1(xtpd->tt,file) MODECASE_1(xtpd->tt,stat) default: break; }
	switch(xtpd->pp.mode) { MODECASE_1(xtpd->pp,exec) MODECASE_1(xtpd->pp,file) MODECASE_1(xtpd->pp,stat) default: break; }

	gtk_entry_set_text(GTK_ENTRY(xtpd->ll.txt_entry), xtpd->ll.cmd);
	gtk_entry_set_text(GTK_ENTRY(xtpd->tt.txt_entry), xtpd->tt.cmd);
	gtk_entry_set_text(GTK_ENTRY(xtpd->pp.txt_entry), xtpd->pp.cmd);

	xtp_toggle(xtpd->ll.chk_upd, xtpd->ll.update);
	xtp_toggle(xtpd->tt.chk_upd, xtpd->tt.update);
	xtp_toggle(xtpd->pp.chk_upd, xtpd->pp.update);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(xtpd->ll.spn_upd), xtpd->ll.interval);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(xtpd->tt.spn_upd), xtpd->tt.interval);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(xtpd->pp.spn_upd), xtpd->pp.interval);

	xfce_panel_plugin_block_menu(xtpd->plugin);
}

static void xtp_config_dialog_hide(GtkWidget *dlg, XfceTextPluginData *xtpd)
{
	xpf("xtp_config_dialog_hide( %p , %p )", dlg, xtpd);
	xfce_panel_plugin_unblock_menu(xtpd->plugin);
	xpf("    done\n");
}

static gboolean xtp_config_dialog_prevent_delete(GtkWidget *widget, GdkEvent *event, XfceTextPluginData *xtpd)
{
	xpf("xtp_config_dialog_prevent_delete() start\n");
	gtk_widget_hide(widget);
	xpf("xtp_config_dialog_prevent_delete() finish\n");
	return TRUE;
}

static void xtp_emit_delete(XfceTextPluginData *xtpd, GtkWidget *w)
{
	gboolean b;
	xpf("xtp_emit_delte() start\n");
	g_signal_emit_by_name(G_OBJECT(w), "delete-event", NULL, &b); // where is this syntax documented?
	xpf("xtp_emit_delte() finished\n");
}

#ifdef DEBUG_PRINT_STATEMENTS
#define MODECASE_2(x) case GTK_RESPONSE_##x: xtp_printf(xtpd, "    " #x "\n");
#else
#define MODECASE_2(x) case GTK_RESPONSE_##x:
#endif
static void xtp_config_dialog_response(GtkWidget *dlg, gint response_id, XfceTextPluginData *xtpd)
{
	xpf("xtp_config_dialog_response()");
	switch(response_id)
	{
		MODECASE_2(NONE)         break;
		MODECASE_2(REJECT)                        xtp_emit_delete(xtpd, dlg); break;
		MODECASE_2(ACCEPT)       xtp_apply(xtpd); xtp_emit_delete(xtpd, dlg); break;
		MODECASE_2(APPLY)        xtp_apply(xtpd);                       break;
		MODECASE_2(DELETE_EVENT) break;
		MODECASE_2(OK)           break;
		MODECASE_2(CANCEL)       break;
		MODECASE_2(CLOSE)        break;
		MODECASE_2(YES)          break;
		MODECASE_2(NO)           break;
		MODECASE_2(HELP)         break;
		default: xpf("    UNKNOWN\n"); 
	}
}

void xtp_configure_plugin(XfcePanelPlugin *plugin, gpointer user_data)
{
	XfceTextPluginData *xtpd = (XfceTextPluginData *)user_data;

	xpf("xtp_configure_plugin()\n    dialog = %p", xtpd->cfg);

	gtk_widget_show_all(xtpd->cfg);
}

static void xtp_build_config_dialog_frame(const gchar *title, XtpDialogWidgets *dw, gboolean use_enabled, gboolean is_enabled)
{
	dw->frm = gtk_frame_new(title);
	dw->box = gtk_vbox_new(FALSE, 0);

	if(!is_enabled)
	{
		gtk_widget_set_sensitive(dw->frm, FALSE);
	}

	if(use_enabled)
	{
		dw->chk_ebl = gtk_check_button_new_with_label("Enabled ");
	}
	
	dw->btn_exec = gtk_radio_button_new_with_label(NULL, "Execute Command");
	dw->btn_file = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(dw->btn_exec),
		"Read From File");
	dw->btn_stat = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(dw->btn_exec),
		"Static Text");

	dw->lbl_entry = gtk_label_new("Text: ");
	dw->txt_entry = gtk_entry_new();
	dw->box_entry = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dw->box_entry), dw->lbl_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dw->box_entry), dw->txt_entry, TRUE , TRUE , 0);

	dw->chk_upd = gtk_check_button_new_with_label("Update ");
	dw->box_upd = gtk_hbox_new(FALSE, 0);
	dw->spn_upd = gtk_spin_button_new_with_range(0.0, 31556926.0, 0.01);
	dw->lbl_upd = gtk_label_new(" seconds");
	gtk_box_pack_start(GTK_BOX(dw->box_upd), dw->chk_upd, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dw->box_upd), dw->spn_upd, TRUE , TRUE , 0);
	gtk_box_pack_start(GTK_BOX(dw->box_upd), dw->lbl_upd, FALSE, FALSE, 0);
	
	if(use_enabled)
	{
		gtk_box_pack_start(GTK_BOX(dw->box), dw->chk_ebl, FALSE, FALSE, 0);
	}
	gtk_box_pack_start(GTK_BOX(dw->box), dw->btn_exec , FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dw->box), dw->btn_file , FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dw->box), dw->btn_stat , FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dw->box), dw->box_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dw->box), dw->box_upd  , FALSE, FALSE, 0);
	
	gtk_container_set_border_width(GTK_CONTAINER(dw->box), BORDER - 2);

	gtk_container_add(GTK_CONTAINER(dw->frm), dw->box);

}

void xtp_build_config_dialog(XfceTextPluginData *xtpd)
{
	GtkWidget *box_root  = NULL;
	GtkWindow *parent_window;

	xpf("xtp_build_config_dialog()\n");

	parent_window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(xtpd->plugin)));

#if 0
	xtpd->cfg = xfce_titled_dialog_new_with_buttons( // why doesn't this work?
		_("Text Plugin Configuration"),
		parent_window,
		GTK_DIALOG_DESTROY_WITH_PARENT, // | GTK_DIALOG_NO_SEPARATOR[deprecated],
		GTK_STOCK_HELP, GTK_RESPONSE_HELP,
		GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
		NULL);
#elif 0
	xtpd->cfg = xfce_titled_dialog_new();
#else
	xtpd->cfg = gtk_dialog_new_with_buttons(
		_("Text Plugin Configuration"),
		parent_window,
		GTK_DIALOG_DESTROY_WITH_PARENT,
//		GTK_STOCK_HELP  , GTK_RESPONSE_HELP, // GtkStock deprecated:  how to replace?
		GTK_STOCK_OK    , GTK_RESPONSE_ACCEPT,
//		_("_OK")        , GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
//		_("_Cancel")    , GTK_RESPONSE_REJECT,
		GTK_STOCK_APPLY , GTK_RESPONSE_APPLY ,
		NULL);
#endif

	gtk_window_set_icon_name(GTK_WINDOW(xtpd->cfg), "xfce4-settings");

	box_root = gtk_dialog_get_content_area(GTK_DIALOG(xtpd->cfg));
	gtk_container_set_border_width(GTK_CONTAINER(box_root), BORDER - 2);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(box_root), GTK_ORIENTATION_VERTICAL);

	xtp_build_config_dialog_frame(xtpd->ll.name, &(xtpd->ll), FALSE, TRUE );
	xtp_build_config_dialog_frame(xtpd->tt.name, &(xtpd->tt), TRUE , TRUE );
	xtp_build_config_dialog_frame(xtpd->pp.name, &(xtpd->pp), TRUE , TRUE );

/* put everything in the dialog */

	gtk_box_pack_start(GTK_BOX(box_root), xtpd->ll.frm, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box_root), xtpd->tt.frm, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box_root), xtpd->pp.frm, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(xtpd->cfg), "response"    , G_CALLBACK(xtp_config_dialog_response      ), xtpd);
    g_signal_connect(G_OBJECT(xtpd->cfg), "show"        , G_CALLBACK(xtp_config_dialog_show          ), xtpd);
    g_signal_connect(G_OBJECT(xtpd->cfg), "hide"        , G_CALLBACK(xtp_config_dialog_hide          ), xtpd);
	g_signal_connect(G_OBJECT(xtpd->cfg), "delete-event", G_CALLBACK(xtp_config_dialog_prevent_delete), xtpd);
}



