
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gtk/gtkdialog.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>

#include "data.h"
#include "debug.h"
#include "config.h"

static void xtp_load(XfceTextPluginData *xtpd);

XfceTextPluginData *xtp_create_plugin_data(XfcePanelPlugin *plugin)
{
	XfceTextPluginData *xtpd;
	
	xtpd = (XfceTextPluginData *)malloc(sizeof(XfceTextPluginData));

	memset(xtpd, 0, sizeof(XfceTextPluginData));
	
	xtpd->plugin = plugin;

	xpf("****************************************\n");
	xpf("**************** plugin ****************\n");
	xpf("**************** loaded ****************\n");
	xpf("****************************************\n");

	xpf("xtp_create_plugin_data()\n");
	xpf("    rc file      : %s\n", xfce_panel_plugin_lookup_rc_file(plugin));
	xpf("    save location: %s\n", xfce_panel_plugin_save_location(plugin, FALSE));

	xtpd->ll.xtpd = xtpd;
	xtpd->tt.xtpd = xtpd;
	xtpd->pp.xtpd = xtpd;
	
	xtpd->ll.set_markup = xtp_ll_set_markup;
	xtpd->tt.set_markup = xtp_tt_set_markup;
	xtpd->pp.set_markup = xtp_pp_set_markup;
	
	strncpy(xtpd->ll.name, "Panel Label", NAMESZ);
	strncpy(xtpd->tt.name, "Tooltip"    , NAMESZ);
	strncpy(xtpd->pp.name, "Popup"      , NAMESZ);
	
	xtp_load(xtpd);
	
	return xtpd;
}

void xtp_free_data(XfcePanelPlugin *plugin, XfceTextPluginData *xtpd)
{
	xpf("xtp_free_data()\n");
}

#define DBL_TO_RC(r,k,d) \
{ \
	gchar rkd[32]; \
	g_snprintf(rkd, 32, "%.2f", d); \
	xfce_rc_write_entry(r,k,rkd); \
}

enum _XtpTextMode xtp_dialog_to_mode(XtpDialogWidgets *dlg)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->btn_exec))) { return xtp_exec; }
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->btn_file))) { return xtp_file; }
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->btn_stat))) { return xtp_stat; }
	return xtp_stat;
}

void xtp_apply(XfceTextPluginData *xtpd)
{
	xpf("xtp_apply()\n");

	xtpd->ll.enabled  = TRUE;
	xtpd->ll.mode     = xtp_dialog_to_mode(&(xtpd->ll));
	xtpd->ll.interval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(xtpd->ll.spn_upd));
	xtpd->ll.update   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(xtpd->ll.chk_upd));
	strncpy(xtpd->ll.cmd, gtk_entry_get_text(GTK_ENTRY(xtpd->ll.txt_entry)), CMDBUFSZ);

	xtpd->tt.enabled  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(xtpd->tt.chk_ebl));
	xtpd->tt.mode     = xtp_dialog_to_mode(&(xtpd->tt));
	xtpd->tt.interval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(xtpd->tt.spn_upd));
	xtpd->tt.update   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(xtpd->tt.chk_upd));
	strncpy(xtpd->tt.cmd, gtk_entry_get_text(GTK_ENTRY(xtpd->tt.txt_entry)), CMDBUFSZ);

	xtpd->pp.enabled  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(xtpd->pp.chk_ebl));
	xtpd->pp.mode     = xtp_dialog_to_mode(&(xtpd->pp));
	xtpd->pp.interval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(xtpd->pp.spn_upd));
	xtpd->pp.update   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(xtpd->pp.chk_upd));
	strncpy(xtpd->pp.cmd, gtk_entry_get_text(GTK_ENTRY(xtpd->pp.txt_entry)), CMDBUFSZ);

	xtpd->is_dirty = TRUE;

	xtp_save(xtpd);
	xtp_update(xtpd);
}

void xtp_save(XfceTextPluginData *xtpd)
{
	XfceRc *rc;
	gchar *file;	

	xpf("xtp_save()\n");
	
	if(!(file = xfce_panel_plugin_save_location(xtpd->plugin, TRUE)))
	{
		xpf("   unable to open find file name\n%s\n", file);
		return;
	}
	xpf("    deleting old file\n");
	unlink(file);
	
	xpf("    opening new file\n");
	rc = xfce_rc_simple_open(file, FALSE);
	g_free(file);
	
	if(!rc)
	{
		xpf("    cannot open new file\n");
		return;
	}

	xpf("    writing values\n");
	
	xfce_rc_write_bool_entry(rc, KEY_LL_ENABLED , xtpd->ll.enabled );
	xfce_rc_write_int_entry (rc, KEY_LL_MODE    , xtpd->ll.mode    );
	DBL_TO_RC               (rc, KEY_LL_INTERVAL, xtpd->ll.interval);
	xfce_rc_write_bool_entry(rc, KEY_LL_UPDATE  , xtpd->ll.update  );
	xfce_rc_write_entry     (rc, KEY_LL_COMMAND , xtpd->ll.cmd     );

	xfce_rc_write_bool_entry(rc, KEY_TT_ENABLED , xtpd->tt.enabled );
	xfce_rc_write_int_entry (rc, KEY_TT_MODE    , xtpd->tt.mode    );
	DBL_TO_RC               (rc, KEY_TT_INTERVAL, xtpd->tt.interval);
	xfce_rc_write_bool_entry(rc, KEY_TT_UPDATE  , xtpd->tt.update  );
	xfce_rc_write_entry     (rc, KEY_TT_COMMAND , xtpd->tt.cmd     );

	xfce_rc_write_bool_entry(rc, KEY_PP_ENABLED , xtpd->pp.enabled );
	xfce_rc_write_int_entry (rc, KEY_PP_MODE    , xtpd->pp.mode    );
	DBL_TO_RC               (rc, KEY_PP_INTERVAL, xtpd->pp.interval);
	xfce_rc_write_bool_entry(rc, KEY_PP_UPDATE  , xtpd->pp.update  );
	xfce_rc_write_entry     (rc, KEY_PP_COMMAND , xtpd->pp.cmd     );
	
    xfce_rc_close(rc);

	xpf("    saved\n");
}

static void xtp_load_defaults(XfceTextPluginData *xtpd)
{
	xpf("xtp_load_defaults()\n");

	xtpd->ll.enabled  = DFL_LL_ENABLED ;
	xtpd->ll.mode     = DFL_LL_MODE    ;
	xtpd->ll.interval = DFL_LL_INTERVAL;
	xtpd->ll.update   = DFL_LL_UPDATE  ;
	strncpy(xtpd->ll.cmd, DFL_LL_COMMAND, CMDBUFSZ);

	xtpd->tt.enabled  = DFL_TT_ENABLED ;
	xtpd->tt.mode     = DFL_TT_MODE    ;
	xtpd->tt.interval = DFL_TT_INTERVAL;
	xtpd->tt.update   = DFL_TT_UPDATE  ;
	strncpy(xtpd->tt.cmd, DFL_TT_COMMAND, CMDBUFSZ);

	xtpd->pp.enabled  = DFL_PP_ENABLED ;
	xtpd->pp.mode     = DFL_PP_MODE    ;
	xtpd->pp.interval = DFL_PP_INTERVAL;
	xtpd->pp.update   = DFL_PP_UPDATE  ;
	strncpy(xtpd->pp.cmd, DFL_PP_COMMAND, CMDBUFSZ);
}

#define RC_TO_STR(r,k,d,b,s) \
{ \
	const gchar *vv = xfce_rc_read_entry(r,k,d); \
	if(vv) { strncpy(b,vv,s); } \
	/* g_free(vv); */ /* cannot g_free const? */ \
}

#define RC_TO_DBL(r,k,d,g) \
{ \
	const gchar *vv = xfce_rc_read_entry(r,k,"" #d); \
	if(vv) { g = g_ascii_strtod(vv, NULL); } \
}

static void xtp_load(XfceTextPluginData *xtpd)
{
	gchar *rcfile;
	XfceRc *rc;

	xtp_load_defaults(xtpd);

	xpf("xtp_load()\n");

	if((rcfile = xfce_panel_plugin_lookup_rc_file(xtpd->plugin))
		&& (rc = xfce_rc_simple_open(rcfile, TRUE)))
	{
		xpf("    reading rc file\n");

		xtpd->ll.enabled = xfce_rc_read_bool_entry(rc, KEY_LL_ENABLED, DFL_LL_ENABLED);
		xtpd->tt.enabled = xfce_rc_read_bool_entry(rc, KEY_TT_ENABLED, DFL_TT_ENABLED);
		xtpd->pp.enabled = xfce_rc_read_bool_entry(rc, KEY_PP_ENABLED, DFL_PP_ENABLED);

		xtpd->ll.mode = xfce_rc_read_int_entry(rc, KEY_LL_MODE, DFL_LL_MODE);
		xtpd->tt.mode = xfce_rc_read_int_entry(rc, KEY_TT_MODE, DFL_TT_MODE);
		xtpd->pp.mode = xfce_rc_read_int_entry(rc, KEY_PP_MODE, DFL_PP_MODE);

		RC_TO_STR(rc, KEY_LL_COMMAND, DFL_LL_COMMAND, xtpd->ll.cmd, CMDBUFSZ);
		RC_TO_STR(rc, KEY_TT_COMMAND, DFL_TT_COMMAND, xtpd->tt.cmd, CMDBUFSZ);
		RC_TO_STR(rc, KEY_PP_COMMAND, DFL_PP_COMMAND, xtpd->pp.cmd, CMDBUFSZ);

		RC_TO_DBL(rc, KEY_LL_INTERVAL, DFL_LL_INTERVAL, xtpd->ll.interval);
		RC_TO_DBL(rc, KEY_TT_INTERVAL, DFL_TT_INTERVAL, xtpd->tt.interval);
		RC_TO_DBL(rc, KEY_PP_INTERVAL, DFL_PP_INTERVAL, xtpd->pp.interval);
		
		xtpd->ll.update = xfce_rc_read_bool_entry(rc, KEY_LL_UPDATE, DFL_LL_UPDATE);
		xtpd->tt.update = xfce_rc_read_bool_entry(rc, KEY_TT_UPDATE, DFL_TT_UPDATE);
		xtpd->pp.update = xfce_rc_read_bool_entry(rc, KEY_PP_UPDATE, DFL_PP_UPDATE);

	    xfce_rc_close(rc);
	}
	else
	{
		xpf("    rc file could not be read... using defaults\n");
	}
	g_free(rcfile);

	xpf("    ll.enabled = %s\n", (xtpd->ll.enabled)?"true":"false");
	xpf("    tt.enabled = %s\n", (xtpd->tt.enabled)?"true":"false");
	xpf("    pp.enabled = %s\n", (xtpd->pp.enabled)?"true":"false");

	xpf("    ll.mode    = %s\n", (xtpd->ll.mode==xtp_exec)?"xtp_exec":((xtpd->ll.mode==xtp_file)?"xtp_file":"xtp_stat"));
	xpf("    tt.mode    = %s\n", (xtpd->tt.mode==xtp_exec)?"xtp_exec":((xtpd->tt.mode==xtp_file)?"xtp_file":"xtp_stat"));
	xpf("    pp.mode    = %s\n", (xtpd->pp.mode==xtp_exec)?"xtp_exec":((xtpd->pp.mode==xtp_file)?"xtp_file":"xtp_stat"));

	xpf("    ll.command = %s\n", xtpd->ll.cmd);
	xpf("    tt.command = %s\n", xtpd->tt.cmd);
	xpf("    pp.command = %s\n", xtpd->pp.cmd);

	xpf("    ll.interval = %.2f\n", xtpd->ll.interval);
	xpf("    tt.interval = %.2f\n", xtpd->tt.interval);
	xpf("    pp.interval = %.2f\n", xtpd->pp.interval);

	xpf("    ll.update = %s\n", (xtpd->ll.update)?"true":"false");
	xpf("    tt.update = %s\n", (xtpd->tt.update)?"true":"false");
	xpf("    pp.update = %s\n", (xtpd->pp.update)?"true":"false");
	
	xtpd->is_dirty = TRUE;

}

