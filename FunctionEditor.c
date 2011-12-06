#include <stdio.h>
#include <stdlib.h>
#include "gtk/gtk.h"

#include "RGnumeric.h"

USER_OBJECT_ RGnumeric_editFunctions();

typedef struct {
    char *name;
    GtkWidget *nameEntry;
    GtkWidget *defEntry;
    GtkWidget *argsEntry;
    GtkWidget *functionList;
    GtkWidget *win;
} R_FunctionEditor;


R_FunctionEditor *createEditor();
int addFunctions(GtkWidget *funList);

void RGnumeric_setSelectedFunction(GtkCList *w, const gchar *val, R_FunctionEditor *editor);

void
RGnumeric_destroyEditor(GtkWidget *widget, gpointer   data)
{
    R_FunctionEditor *editor = (R_FunctionEditor*)data;
    gtk_widget_hide(editor->win);
    gtk_widget_destroy(editor->win);
}

void
RGnumeric_applyEditorChanges(GtkWidget *widget, gpointer   data)
{
    R_FunctionEditor *editor = (R_FunctionEditor*)data;

    gchar *name, *args, *def;
    USER_OBJECT_ fun;
    Rboolean isNew;

     name = gtk_entry_get_text( GTK_ENTRY(editor->nameEntry));
     args = gtk_entry_get_text( GTK_ENTRY(editor->argsEntry));
     def = gtk_editable_get_chars( GTK_EDITABLE(editor->defEntry), 0, -1);
     fprintf(stderr, "Defining function:\n%s, %s\n%s", name, args, def);fflush(stderr);

     fun = RGnumeric_getFunctionObject(def);
     isNew = RGnumeric_internal_registerFunction(name, fun, args, "", NULL, "R");

     if(isNew) {
	gtk_clist_append( GTK_CLIST(editor->functionList), &name);
     }
}

int
getSelectedCListRow(GtkWidget *w)
{
  GList *slist;
  int n = -1;
  slist = GTK_CLIST(w)->selection;
  while(slist) {
      n = (int) slist->data;
      break;
  }
  return(n);
}


void
RGnumeric_removeFunction(GtkWidget *widget, gpointer   data)
{
    gchar *val;
    R_FunctionEditor *editor = (R_FunctionEditor*)data;
    int row;
    row = getSelectedCListRow(editor->functionList);
    if(row > -1) {
      gtk_clist_get_text(GTK_CLIST(editor->functionList), row, 0, &val);
      RGnumeric_unregisterFunctionDef(val);
      gtk_clist_remove(GTK_CLIST(editor->functionList), row);
    }
}

void
RGnumeric_selectFunction(GtkCList *w, gint r, gint c, GdkEvent *ev, gpointer data)
{
    gchar *val;
    R_FunctionEditor *editor = (R_FunctionEditor*)data;
    gtk_clist_get_text(w, r, c, &val);
    RGnumeric_setSelectedFunction(w, val, editor);
}

void
RGnumeric_setSelectedFunction(GtkCList *w, const gchar *val, R_FunctionEditor *editor)
{
    USER_OBJECT_ sbody;
    FuncData *funDef;
    gchar *body;

    funDef = RGnumeric_getFunctionDef(val, NULL);
    if(funDef) {
      gtk_entry_set_text(GTK_ENTRY(editor->nameEntry), val);
      gtk_entry_set_text(GTK_ENTRY(editor->argsEntry), funDef->argTypes);
      sbody = RGnumeric_getFunctionText(funDef->codeobj);
      body = CHAR_DEREF(STRING_ELT(sbody, 0));

      gtk_text_set_point(GTK_TEXT(editor->defEntry), 0);
      gtk_text_forward_delete(GTK_TEXT(editor->defEntry), gtk_text_get_length(GTK_TEXT(editor->defEntry)));
      gtk_text_insert(GTK_TEXT(editor->defEntry), NULL, NULL, NULL, body, -1);
    }
}

void
RGnumeric_closeEditor(GtkWidget *widget, gpointer   data)
{
    RGnumeric_applyEditorChanges(widget, data);
    RGnumeric_destroyEditor(widget, data);
}


int 
main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    RGnumeric_editFunctions();
    gtk_main();
    return(0);
}

USER_OBJECT_
RGnumeric_editFunctions()
{
    R_FunctionEditor *editor;
    editor = createEditor();
    gtk_widget_show(editor->win);

    return(NULL_USER_OBJECT);
}

R_FunctionEditor *
createEditor()
{
 R_FunctionEditor *editor;
 GtkWidget *win, *box, *topBox, *label, *tmpBox, *dlgBox;
 GtkWidget *funList, *btn;
 char *titles[] = {"Functions"};
 gint expand = 1, fill = 1, padding = 1;


    editor = (R_FunctionEditor*) g_malloc(sizeof(R_FunctionEditor));
    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    dlgBox = gtk_vbox_new(FALSE, 0);
    topBox = gtk_hbox_new(FALSE, 0);

    box = gtk_vbox_new(FALSE, 0);
    funList = gtk_clist_new_with_titles(1, titles);
    gtk_signal_connect(GTK_OBJECT(funList), "select-row", 
		       GTK_SIGNAL_FUNC(RGnumeric_selectFunction), editor);
    gtk_box_pack_start(GTK_BOX(box), funList, TRUE, fill, padding);
    gtk_widget_show(funList);

    addFunctions(funList);
    editor->functionList = funList;

    label = gtk_button_new_with_label("Remove");
    gtk_signal_connect (GTK_OBJECT (label), "clicked",
                         GTK_SIGNAL_FUNC (RGnumeric_removeFunction), editor);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, fill, padding);
    gtk_container_add (GTK_CONTAINER (topBox), box);
    gtk_widget_show(label);

    gtk_widget_show(box);


    box = gtk_vbox_new(FALSE, 0);

    tmpBox = gtk_hbox_new(FALSE, 0);
    label = gtk_label_new("Name");
    gtk_box_pack_start(GTK_BOX(tmpBox), label, FALSE, fill, padding);
    gtk_widget_show(label);

    label = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(tmpBox), label, FALSE, fill, padding);
    gtk_widget_show(label);
    editor->nameEntry = label;


    gtk_container_add (GTK_CONTAINER (box), tmpBox);
    gtk_widget_show(tmpBox);

//    label = gtk_text_new(NULL, NULL);
    label = gtk_text_new(NULL, NULL);
    gtk_text_set_editable(GTK_TEXT(label), TRUE);
    gtk_box_pack_start(GTK_BOX(box), label, expand, fill, padding);
    gtk_widget_show(label);
    editor->defEntry = label;


    tmpBox = gtk_hbox_new(FALSE, 0);
    label = gtk_label_new("Arguments");
    gtk_box_pack_start(GTK_BOX(tmpBox), label, FALSE, fill, padding);
    gtk_widget_show(label);
    label = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(tmpBox), label, FALSE, fill, padding);
    gtk_widget_show(label);
    editor->argsEntry = label;

    gtk_container_add (GTK_CONTAINER (box), tmpBox);
    gtk_widget_show(tmpBox);


    gtk_container_add (GTK_CONTAINER (topBox), box);
    gtk_widget_show(box);


    box = gtk_hbox_new(FALSE, 0);
    btn = gtk_button_new_with_label("Close");
    gtk_box_pack_start(GTK_BOX(box), btn, FALSE, fill, padding);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                         GTK_SIGNAL_FUNC (RGnumeric_closeEditor), editor);
                       

    gtk_widget_show(btn);
    btn = gtk_button_new_with_label("Apply");
    gtk_box_pack_start(GTK_BOX(box), btn, FALSE, fill, padding);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                         GTK_SIGNAL_FUNC (RGnumeric_applyEditorChanges), editor);
    gtk_widget_show(btn);

    btn = gtk_button_new_with_label("Cancel");
    gtk_box_pack_start(GTK_BOX(box), btn, FALSE, fill, padding);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                         GTK_SIGNAL_FUNC (RGnumeric_destroyEditor), editor);
    gtk_widget_show(btn);

    gtk_container_add (GTK_CONTAINER (dlgBox), topBox);
    gtk_widget_show(topBox);
    gtk_container_add (GTK_CONTAINER (dlgBox), box);
    gtk_widget_show(box);


    gtk_container_add (GTK_CONTAINER (win), dlgBox);
    gtk_widget_show(dlgBox);


    editor->win = win;

    return(editor);
}

int
addFunctions(GtkWidget *funList)
{
 int i, n;

 GList *els;
 gchar *name; 
 els = RGnumeric_getFunctionList();
 n = g_list_length(els);
 for( i=0 ; i < n ; i++) {
	RGnumeric_FuncData *el;
        el = (RGnumeric_FuncData*) g_list_nth_data(els, i);
        name = (gchar *)function_def_get_name(el->fndef);
	gtk_clist_append( (GtkCList *) funList, &name);
 }

 return(i);
}


