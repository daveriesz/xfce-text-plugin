
#ifndef __XFCETEXTPLUGIN_DEBUG_H__
#define __XFCETEXTPLUGIN_DEBUG_H__

#include <libxfce4panel/xfce-panel-plugin.h>

#include "data.h"

void xtp_printf(XfceTextPluginData *xtpd, const char *fmt, ...);

#if DEBUG_PRINT_STATEMENTS
#define xpf(...) xtp_printf(xtpd, __VA_ARGS__)
#define XPF_ASSIGN_XTPD(x) XfceTextPluginData *xtpd = (x)
#else
#define xpf(...)
#define XPF_ASSIGN_XTPD(x)
#endif

#endif /* __XFCETEXTPLUGIN_DEBUG_H__ */
