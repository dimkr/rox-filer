/*
 * $Id$
 *
 * ROX-Filer, filer for the ROX desktop project
 * Copyright (C) 2002, the ROX-Filer team.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* view_iface.c - operations supported by all views */

#include "config.h"

#include "global.h"

#include "view_iface.h"
#include "support.h"
#include "filer.h"
#include "diritem.h"

/* A word about interfaces:
 *
 * gobject's documentation's explanation of interfaces leaves something[1] to
 * be desired, so I'd better explain here...
 *
 * [1] Like, eg, an explanation.
 *
 * A ViewIfaceClass is a struct which contains a number of function
 * pointers. Each class that implements the View interface creates its
 * own ViewIfaceClass with pointers to its implementation. This is stored
 * with the class.
 * 
 * When you want to call a method (eg, sort()) on a View, you call
 * view_sort(object) here, which gets the class of object and then looks
 * for that class's implementation of the View interface, and then calls
 * the actual function through that.
 */

/* ViewIters
 *
 * A ViewIter is used to index items. They are usually allocated
 * on the stack and then initialised using view_get_iter().
 * 
 * Normally, an iterator starts of not pointing at any item, but
 * each call to iter->next(iter) returns the next item, and leaves
 * the iterator pointing at the returned item. If you like the item,
 * you can then pass the iterator to view_cursor_to_iter(), etc.
 *
 * Using flags passed to view_get_iter, you can start the sequence from the
 * beginning, end, cursor or 'base' (a saved cursor position). You can
 * go forwards or backwards. You can opt to only get selected items returned.
 *
 * You can also have one-shot iterators which already point to an item, and
 * you never call the next method (view_get_cursor, for example). In fact,
 * these iterators return a sequence of one item, but next() gets called
 * automatically for you.
 *
 * You don't need to free iterators, and they become invalid if the
 * View changes (items added, removed or altered), so don't hang on to
 * them!
 */

/****************************************************************
 *			EXTERNAL INTERFACE			*
 ****************************************************************/

GType view_iface_get_type(void)
{
	static GType iface_type = 0;

	if (!iface_type)
	{
		static const GTypeInfo iface_info =
		{
			sizeof (ViewIfaceClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
		};

		iface_type = g_type_register_static(G_TYPE_INTERFACE,
						"ViewIface", &iface_info, 0);

		/* Actually, all Views should be GTK_TYPE_WIDGETs, to be more
		 * accurate, but including gtk.h takes so long, and noone's
		 * going to get this wrong ;-)
		 */
		g_type_interface_add_prerequisite(iface_type, G_TYPE_OBJECT);
	}

	return iface_type;
}

/* The sort function has changed -- resort */
void view_sort(ViewIface *obj)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));
	VIEW_IFACE_GET_CLASS(obj)->sort(obj);
}

/* The style has changed -- shrink the grid and redraw.
 * Also update ViewData (and name layout too) if appropriate
 * flags are set.
 */
void view_style_changed(ViewIface *obj, int flags)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));
	VIEW_IFACE_GET_CLASS(obj)->style_changed(obj, flags);
}

/* Wink or move the cursor to this item, if present. Return TRUE on
 * success (iff leaf was present).
 */
gboolean view_autoselect(ViewIface *obj, const gchar *leaf)
{
	g_return_val_if_fail(VIEW_IS_IFACE(obj), FALSE);
	g_return_val_if_fail(leaf != NULL, FALSE);

	return VIEW_IFACE_GET_CLASS(obj)->autoselect(obj, leaf);
}

/* Scanning has turned up some new items... */
void view_add_items(ViewIface *obj, GPtrArray *items)
{
	VIEW_IFACE_GET_CLASS(obj)->add_items(obj, items);
}

/* These items are already known, but have changed... */
void view_update_items(ViewIface *obj, GPtrArray *items)
{
	VIEW_IFACE_GET_CLASS(obj)->update_items(obj, items);
}

/* Call test(item) for each item in the view and delete all those for
 * which it returns TRUE.
 */
void view_delete_if(ViewIface *obj,
		    gboolean (*test)(gpointer item, gpointer data),
		    gpointer data)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->delete_if(obj, test, data);
}

/* Remove all items from the view (used when changing directory) */
void view_clear(ViewIface *obj)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->clear(obj);
}

/* Select all items */
void view_select_all(ViewIface *obj)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->select_all(obj);
}

/* Unselect all items */
void view_clear_selection(ViewIface *obj)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->clear_selection(obj);
}

/* Return the total number of items */
int view_count_items(ViewIface *obj)
{
	g_return_val_if_fail(VIEW_IS_IFACE(obj), 0);

	return VIEW_IFACE_GET_CLASS(obj)->count_items(obj);
}

/* Return the number of selected items */
int view_count_selected(ViewIface *obj)
{
	g_return_val_if_fail(VIEW_IS_IFACE(obj), 0);

	return VIEW_IFACE_GET_CLASS(obj)->count_selected(obj);
}

void view_show_cursor(ViewIface *obj)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->show_cursor(obj);
}

/* Create an iterator which will return each element selected by 'flags'
 * from successive calls to iter.next(&iter). NULL indicates the end
 * of the sequence.
 *
 * The iterator does not need to be freed. It becomes invalid if the
 * view is changed in any way.
 */
void view_get_iter(ViewIface *obj, ViewIter *iter, IterFlags flags)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));
	g_return_if_fail(iter != NULL);

	VIEW_IFACE_GET_CLASS(obj)->get_iter(obj, iter, flags);
}

/* Make an 'iter' for the cursor item, if any. Use iter->peek() to get
 * the DirItem (will be NULL if the cursor isn't on an item).
 */
void view_get_cursor(ViewIface *obj, ViewIter *iter)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));
	g_return_if_fail(iter != NULL);

	VIEW_IFACE_GET_CLASS(obj)->get_iter(obj, iter,
				VIEW_ITER_FROM_CURSOR | VIEW_ITER_ONE_ONLY);
}

/* Position cursor on the last item returned by iter.next().
 * If iter is NULL, remove the cursor.
 */
void view_cursor_to_iter(ViewIface *obj, ViewIter *iter)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->cursor_to_iter(obj, iter);
}

/* Select the item at this iter */
void view_set_selected(ViewIface *obj, ViewIter *iter, gboolean selected)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->set_selected(obj, iter, selected);
}

/* TRUE if this item is selected */
gboolean view_get_selected(ViewIface *obj, ViewIter *iter)
{
	g_return_val_if_fail(VIEW_IS_IFACE(obj), FALSE);

	return VIEW_IFACE_GET_CLASS(obj)->get_selected(obj, iter);
}

/* Flash / draw attention to this item */
void view_wink_item(ViewIface *obj, ViewIter *iter)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->wink_item(obj, iter);
}

/* Clear the selection, then select this item. Does it atomically to avoid
 * problems with giving up and quickly reclaiming the primary selection.
 */
void view_select_only(ViewIface *obj, ViewIter *iter)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->select_only(obj, iter);
}

void view_select_if(ViewIface *obj,
		    gboolean (*test)(ViewIter *iter, gpointer data),
		    gpointer data)
{
	ViewIter iter;
	gboolean should_select_first;

	g_return_if_fail(VIEW_IS_IFACE(obj));

	view_get_iter(obj, &iter, 0);

	if (!iter.next(&iter))
		return;		/* No items */

	view_freeze(obj);

	/* If anything is currently selected then select the first item now
	 * and set it to its correct value at the end (avoids losing the
	 * primary and regaining it quickly). Do the test first in case it
	 * relies on the selected state!
	 */
	should_select_first = test(&iter, data);
	if (view_count_selected(obj))
		view_set_selected(obj, &iter, TRUE);

	while (iter.next(&iter))
		view_set_selected(obj, &iter, test(&iter, data));

	view_get_iter(obj, &iter, 0);
	iter.next(&iter);
	view_set_selected(obj, &iter, should_select_first);

	view_thaw(obj);
}

/* Prevent selection_changed events from being emitted */
void view_freeze(ViewIface *obj)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->set_frozen(obj, TRUE);
}

/* Undo a view_freeze (and emit the changed signal) */
void view_thaw(ViewIface *obj)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->set_frozen(obj, FALSE);
}

/* Resize the filer window to a sensible size.
 * v_border is the height of the toolbar + the minibuffer (if visible).
 * space is
 * If allow_shrink is 
 */
void view_autosize(ViewIface *obj)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->autosize(obj);
}

/* Return TRUE if the cursor is shown. Note that the cursor may be visible
 * even if their are no items (so get_cursor().peek() would return NULL).
 */
gboolean view_cursor_visible(ViewIface *obj)
{
	g_return_val_if_fail(VIEW_IS_IFACE(obj), FALSE);

	return VIEW_IFACE_GET_CLASS(obj)->cursor_visible(obj);
}

/* The 'base' position is used to record the position of the cursor
 * when the minibuffer is opened, for interactive searching.
 */
void view_set_base(ViewIface *obj, ViewIter *iter)
{
	g_return_if_fail(VIEW_IS_IFACE(obj));

	VIEW_IFACE_GET_CLASS(obj)->set_base(obj, iter);
}

/* The the filer window for this view */
FilerWindow *view_get_filer_window(ViewIface *obj)
{
	g_return_val_if_fail(VIEW_IS_IFACE(obj), NULL);

	return VIEW_IFACE_GET_CLASS(obj)->get_filer_window(obj);
}

/* Return the selection as a text/uri-list.
 * g_free() the result.
 */
guchar *view_create_uri_list(ViewIface *obj)
{
	FilerWindow *filer_window;
	GString	*string;
	GString	*leader;
	ViewIter iter;
	DirItem	*item;
	guchar	*retval;

	g_return_val_if_fail(VIEW_IS_IFACE(obj), NULL);

	filer_window = view_get_filer_window(obj);

	string = g_string_new(NULL);

	leader = g_string_new("file://");
	g_string_append(leader, our_host_name_for_dnd());
	g_string_append(leader, filer_window->sym_path);
	if (leader->str[leader->len - 1] != '/')
		g_string_append_c(leader, '/');

	view_get_iter(obj, &iter, VIEW_ITER_SELECTED);
	while ((item = iter.next(&iter)))
	{
		g_string_append(string, leader->str);
		g_string_append(string, item->leafname);
		g_string_append(string, "\r\n");
	}

	g_string_free(leader, TRUE);
	retval = string->str;
	g_string_free(string, FALSE);

	return retval;
}

