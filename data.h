
#ifndef __XFCETEXTPLUGIN_DATA_H__
#define __XFCETEXTPLUGIN_DATA_H__

#define CMDBUFSZ 16384
#define NAMESZ   32

#define DFL_LL_ENABLED  TRUE
#define DFL_LL_MODE     xtp_stat
#define DFL_LL_COMMAND  "<i>XFCE Text Plugin</i>"
#define DFL_LL_INTERVAL 0.0
#define DFL_LL_UPDATE   FALSE

#define DFL_TT_ENABLED  FALSE
#define DFL_TT_MODE     xtp_stat
#define DFL_TT_COMMAND  "<i>XFCE Text Plugin Tooltip</i>"
#define DFL_TT_INTERVAL 0.0
#define DFL_TT_UPDATE   FALSE

#define DFL_PP_ENABLED  FALSE
#define DFL_PP_MODE     xtp_stat
#define DFL_PP_COMMAND  "<i>XFCE Text Plugin Popup Window</i>"
#define DFL_PP_INTERVAL 0.0
#define DFL_PP_UPDATE   FALSE

#define KEY_LL_ENABLED  "ll_enabled"
#define KEY_LL_MODE     "ll_mode"
#define KEY_LL_COMMAND  "ll_command"
#define KEY_LL_INTERVAL "ll_interval"
#define KEY_LL_UPDATE   "ll_update"

#define KEY_TT_ENABLED  "tt_enabled"
#define KEY_TT_MODE     "tt_mode"
#define KEY_TT_COMMAND  "tt_command"
#define KEY_TT_INTERVAL "tt_interval"
#define KEY_TT_UPDATE   "tt_update"

#define KEY_PP_ENABLED  "pp_enabled"
#define KEY_PP_MODE     "pp_mode"
#define KEY_PP_COMMAND  "pp_command"
#define KEY_PP_INTERVAL "pp_interval"
#define KEY_PP_UPDATE   "pp_update"

typedef struct _XtpDialogWidgets   XtpDialogWidgets;
typedef struct _XfceTextPluginData XfceTextPluginData;

typedef void (*XtpMarkupFunc)(XfceTextPluginData *, const gchar *);

typedef struct _XfceTextPluginData
{
	XfcePanelPlugin *plugin;
	GtkWidget *cfg; /* configuration/properties dialog */
	GtkWidget *ppw; /* panel plugin widget */
	
	gboolean is_dirty;

	struct _XtpDialogWidgets
	{
		/* data widgets */
		GtkWidget *btn_exec;
		GtkWidget *btn_file;
		GtkWidget *btn_stat;
		GtkWidget *txt_entry;
		GtkWidget *chk_upd;
		GtkWidget *spn_upd;
		GtkWidget *chk_ebl;

		/* UI widgets */
		GtkWidget *frm, *box, *box_entry, *box_upd, *lbl_entry, *lbl_upd;
		
		/* Display Widget */
		GtkWidget *markup_widget;

		/* data holders */
		gboolean enabled;
		enum _XtpTextMode { xtp_exec , xtp_file , xtp_stat } mode;
		gchar cmd[CMDBUFSZ];
		gdouble interval;
		gboolean update;
		
		/* other */
		gchar buffer[CMDBUFSZ];
		guint func_tag;
		struct _XfceTextPluginData *xtpd;
		gchar name[NAMESZ];
		XtpMarkupFunc set_markup;
	} ll, tt, pp;
} XfceTextPluginData;

XfceTextPluginData *xtp_create_plugin_data(XfcePanelPlugin *plugin);
void xtp_free_data(XfcePanelPlugin *plugin, XfceTextPluginData *xtpd);
void xtp_save(XfceTextPluginData *xtpd);
void xtp_apply(XfceTextPluginData *xtpd);

void xtp_ll_set_markup(XfceTextPluginData *xtpd, const gchar *str);
void xtp_tt_set_markup(XfceTextPluginData *xtpd, const gchar *str);
void xtp_pp_set_markup(XfceTextPluginData *xtpd, const gchar *str);

#define BORDER (8)


#endif /* __XFCETEXTPLUGIN_DATA_H__ */

