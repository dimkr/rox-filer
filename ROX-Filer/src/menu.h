/*
 * $Id$
 *
 * ROX-Filer, filer for the ROX desktop project
 * By Thomas Leonard, <tal197@users.sourceforge.net>.
 */

#ifndef _MENU_H
#define _MENU_H

extern GtkAccelGroup	*filer_keys;

void menu_init(void);

GtkItemFactory *menu_create(GtkItemFactoryEntry *def, int n_entries,
			    const gchar *name, GtkAccelGroup *keys);
void menu_set_items_shaded(GtkWidget *menu, gboolean shaded, int from, int n);
void position_menu(GtkMenu *menu, gint *x, gint *y,
		   gboolean  *push_in, gpointer data);
void show_popup_menu(GtkWidget *menu, GdkEvent *event, int item);

void ensure_filer_menu(void);
void show_filer_menu(FilerWindow *filer_window, GdkEvent *event, int item);
void menu_popdown(void);

/* Public menu handlers */
void menu_rox_help(gpointer data, guint action, GtkWidget *widget);
void menu_show_options(gpointer data, guint action, GtkWidget *widget);
void open_home(gpointer data, guint action, GtkWidget *widget);
void menu_show_shift_action(GtkWidget *menu_item, DirItem *item, gboolean next);

#endif /* _MENU_H */
