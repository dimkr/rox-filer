/*
 * ROX-Filer, filer for the ROX desktop project
 * By Thomas Leonard, <tal197@users.sourceforge.net>.
 */

#ifndef __VIEW_DETAILS_H__
#define __VIEW_DETAILS_H__

#include <gtk/gtk.h>

typedef struct _ViewDetailsClass ViewDetailsClass;

typedef struct _ViewItem ViewItem;

struct _ViewItem {
	DirItem *item;
	MaskedPixmap *image;
	int	old_pos;	/* Used while sorting */
	gchar   *utf8_name;	/* NULL => leafname is valid */
};

typedef struct _ViewDetails ViewDetails;

struct _ViewDetails {
	GtkTreeView treeview;
	GtkTreeSelection *selection;

	FilerWindow *filer_window;	/* Used for styles, etc */

	GPtrArray   *items;		/* ViewItem */
	
	int	    (*sort_fn)(const void *, const void *);

	int	    cursor_base;	/* Cursor when minibuffer opened */

	int	    wink_item;		/* -1 => not winking */
	gint	    wink_timeout;
	int	    wink_step;

	int	    can_change_selection;

	GtkRequisition desired_size;

	gboolean	lasso_box;
	int		lasso_start_index;
	int		drag_box_x[2];	/* Index 0 is the fixed corner */
	int		drag_box_y[2];

	GtkTreeViewColumn *icon_column;
	GtkTreeViewColumn *name_column;
	GtkTreeViewColumn *type_column;
	GtkTreeViewColumn *size_column;
	GtkTreeViewColumn *permissions_column;
	GtkTreeViewColumn *owner_column;
	GtkTreeViewColumn *group_column;
	GtkTreeViewColumn *atime_column;
	GtkTreeViewColumn *ctime_column;
	GtkTreeViewColumn *mtime_column;
};


#define VIEW_DETAILS(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), view_details_get_type(), ViewDetails))

GtkWidget *view_details_new(FilerWindow *filer_window);
GType view_details_get_type(void);

#endif /* __VIEW_DETAILS_H__ */
