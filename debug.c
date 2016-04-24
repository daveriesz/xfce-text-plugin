
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <glib.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "debug.h"

#define XTP_BUFFER_SIZE 16384

#ifdef DEBUG_PRINT_STATEMENTS

static char xtp_logfilename[1024] = { '\0' };

void xtp_printf(XfceTextPluginData *xtpd, const char *fmt, ...)
{
	static char str[XTP_BUFFER_SIZE], gstr[XTP_BUFFER_SIZE];
	char *cp1, *cp2;
	FILE *fp;
	XfcePanelPlugin *plugin = xtpd->plugin;

	memset( str, 0, XTP_BUFFER_SIZE);
	memset(gstr, 0, XTP_BUFFER_SIZE);

	if(xtp_logfilename[0] == '\0')
	{
		sprintf(xtp_logfilename, "%s/%s",
			getenv("HOME"),
			"xfce-text-plugin.log");
	}

	if(!(fp = fopen(xtp_logfilename, "a")))
	{
		return;
	}

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(str, XTP_BUFFER_SIZE, fmt, ap);
	va_end(ap);

	for(cp1=str, cp2=strchr(cp1, '\n') ; cp1 && *cp1 ; cp1=cp2?(cp2+1):NULL, cp2=cp1?strchr(cp1, '\n'):NULL)
	{
		if(cp2 == NULL) { cp2 = strchr(cp1, '\0'); }
		*cp2 = '\0';
		sprintf(gstr, "0x%.3x/%d -- %s", xfce_panel_plugin_get_unique_id(plugin), getpid(), cp1);
		fprintf(fp, "%s\n", gstr);
        g_message("%s", gstr);
	}

	fclose(fp);
}

#endif
