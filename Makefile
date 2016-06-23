# necessary dev packages:
# + xfce4-panel-dev
# + libgtk-3-dev

PLUGINNAME = xfce-text-plugin
SHLIB = lib$(PLUGINNAME).so

XFCE4PANELVER = 2.0

CSOURCES = main.c debug.c config.c data.c
COBJECTS = $(CSOURCES:.c=.o)
CWARN    = -Wall -Wno-deprecated-declarations -Wno-parentheses # change to -Wall
CFLAGS   = -g -fPIC $(CWARN) \
	$(CDEFS) \
	$(shell pkg-config --cflags libxfce4panel-$(XFCE4PANELVER)) \
	$(shell pkg-config --cflags libxfce4ui-2) \
	$(shell pkg-config --cflags gtk+-3.0)
CDEFS    = -DDEBUG_PRINT_STATEMENTSx

LDFLAGS  = \
	$(shell pkg-config --libs libxfce4panel-$(XFCE4PANELVER)) \
	$(shell pkg-config --libs libxfce4ui-2) \
	$(shell pkg-config --libs gtk+-3.0)

DTFILE = $(PLUGINNAME).desktop
PLUGINDIR = /usr/lib/x86_64-linux-gnu/xfce4/panel-plugins

all: $(SHLIB) $(DTFILE)

$(SHLIB): $(COBJECTS)
	@echo " + LD $^ \b\b> $@" | sed "s/\.o /\.o\n      /g"
	@gcc -shared -o $@ $^ $(LDFLAGS)

$(DTFILE): Makefile
	@echo " + DT $@"
	@$(RM) "$@"
	@echo "[Xfce Panel]" >> "$@"
	@echo "Name=XFCE Text Plugin" >> "$@"
	@echo "Comment=XFCE Text Plugin" >> "$@"
	@echo "X-XFCE-Module=$(PLUGINNAME)" >> "$@"
	@echo "X-XFCE-Module-Path=$(PLUGINDIR)" >> "$@"

%.o: %.c
	@echo " + CC $<"
	@$(CC) -c -o $@ $< $(CFLAGS)

%.i: %.c
	@echo " + PP $<"
	@$(CC) -E -o $@ $< $(CFLAGS)

clean: objclean libclean misclean

objclean:
	$(RM) -f $(COBJECTS)

libclean:
	$(RM) -f $(SHLIB)

misclean:
	$(RM) -f $(DTFILE)

install: $(SHLIB) $(DTFILE)
	sudo cp $(SHLIB) $(PLUGINDIR)
	sudo cp $(DTFILE) /usr/share/xfce4/panel-plugins

restart:
	DISPLAY=:0 xfce4-panel -r

debug:
	xfce4-panel -q
	PANEL_DEBUG=1 xfce4-panel

rerun: clean all install restart
