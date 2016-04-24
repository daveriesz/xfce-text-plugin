# xfce-text-plugin
XFCE Text Plugin

This is a basic text display plugin for the XFCE panel.  It's purpose is to
allow the display of arbitrary information in a GTK label in the XFCE panel.
Text is optionally formatted with the Pango markup language.

The text data may be selected from three sources:  static text, file contents,
or an executed shell command.  The displayed information can be updated in
intervals as small as 10ms.

There are three outputs available, each of which can have an independent source
and update interval:  the panel widget, the tooltip, and a popup window (not
fully implemented).

Currently, building the xfce-text-plugin is done with a basic Makefile and
supports only 64-bit environments.  Support for 32-bit environments is a matter
of correcting the output directory for the plugin library.  As written,
building requires:

  - libxfce4panel-2.0
  - libxfce4ui-2
  - gtk+-3.0

and any necessary dependencies of these three packages.

Building with the command:

  make clean install restart

will build the library, the .destktop file, and install them in their
respective system directories.  Before using the "install" target, please run:

  make install --dry-run

to make sure that the output locations are suitable.

This plugin has been developed and tested on x86_64 platforms with Xubuntu 14.04
and 16.04 installations.

