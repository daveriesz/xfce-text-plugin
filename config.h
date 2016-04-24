
#ifndef __XFCETEXTPLUGIN_CONFIG_H__
#define __XFCETEXTPLUGIN_CONFIG_H__

#include <libxfce4panel/xfce-panel-plugin.h>

#include "data.h"

void xtp_configure_plugin(XfcePanelPlugin *plugin, gpointer user_data);
void xtp_build_config_dialog(XfceTextPluginData *xtpd);
gboolean xtp_update(XfceTextPluginData *xtpd);


#endif /* __XFCETEXTPLUGIN_CONFIG_H__ */
