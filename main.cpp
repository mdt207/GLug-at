/* C*   Bismillohir Rahmonir Rahiym  C* */

/***********************************************************\
 * Turkiston Dasturlari (c) hijriy 1436                    *
 * Turkiston Software (c) 2015                             *
 * GLug'at dasturi                                         *
 * Version 1.91                                            *
\***********************************************************/

/************************************************************\
 * Muqobil Dasturlar To'plami (c) 2006-2012                 *
 *       Set of Alternative Software                        *
\************************************************************/

/***********************************************************\
 * Special thanks and greetingz fly to Yusuf Erturk CEO of *
 * MP group www.internetyazilim.com                        *
 *                 Proton Computing Intl.                  *
 * & to Muzaffar Egamberdiyev                              *
 ***********************************************************/
// O'zbekiston jumhuriyati, Toshkent shahri, 2006-2015

//Phonetic transcriptions were obtained from www.photransedit.com

//Some codes excerpts were borrowed from gtk-demo of GTK+ team

//original code by TKL K96 207 (c) 2006, 2011, 2015

//Released under the terms of GPL version 2
//Recent changes 29 March of 2015

/*** Tested on GNU/Linux, NetBSD, FreeBSD, OpenBSD, WNT systems ***/

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "clugat.h"


enum
{
  COLUMN_LIST_WORDS,
  NUM_COLUMNS
};

enum
{
    GTK_SCROLL_BACKWARD = -1,
    GTK_SCROLL_FORWARD  =  1
};

//consts
const char ver[]      = { "1.91" };
const char prog_name[]= {"GLug'at"};

/*--------------------------------------------------------------------------------------*/
//this is because of GTK's C style programming, i hope there would be OOP based style

//GLOBALS
GtkWidget *window; //main window of app.

GtkWidget *text_trans_view,  *clist, *font_btn, *text_trans;
GtkWidget *entry, *words_tree_view, *vpaned;

static GtkWidget *find_box = NULL;

GtkTextBuffer *buffer,*trans_buffer;

CLugat *m_search; //searching engine

unsigned short gl_row = 0, gl_fpos = 0, gl_num_enwords = DEFAULT_NUM_ENWORDS;

bool _format = false, _auto_search = false;
bool follow_link = false, new_quest = true;

gulong kdb_handler_id = 0, entry_handler_id = 0;


GtkTreeModel *words_list_model;
GtkTreeSelection *tree_select;

GtkTreePath *path;
//static GdkColor text_color, bg_color;
static GdkRGBA text_color, bg_color;

gchar *path_str, *font_name;

int not_found_count = 0;  //not to waste the time in searching
gint gl_width, gl_height, gl_paned_pos; //to restore window sizes

int _errno = 0;
string errmsg;

/*--------------------------------forwards-----------------------------------*/
static gboolean
  kbd_press_event ( GtkWidget *text_view, GdkEventKey *event);

void entry_changed  ( GtkWidget *widget,    GtkWidget   *entry );
void enter_callback ( GtkWidget *widget,    GtkWidget   *entry );

int update_words_list();
void update_cursor_location( unsigned short row);

void config();

void MessageBox( string &msg, GtkMessageType msgtype = GTK_MESSAGE_INFO);
/*--------------------------------forwards-----------------------------------*/

static void
add_columns (GtkTreeView  *treeview,
             GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;

 /* yummy column */

  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER ( COLUMN_LIST_WORDS ));

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "Words", renderer,
                                               "text", COLUMN_LIST_WORDS,
                                               NULL);
}

static GtkTreeModel *
create_list_model (void)
{
  //gint i = 0;
  GtkListStore *store;
  GtkTreeIter iter;

  /* create list store */
  store = gtk_list_store_new (NUM_COLUMNS,   G_TYPE_STRING);

  gchar *word;
  vector<_data> vdata = m_search->GetDataSeq();

  for(int c=0; c < m_search->GetNextN(); c++ )
   {
      //word = g_convert( (gchar*) &(vdata[c].word[0]) , -1, "UTF-8", "ISO8859-9", NULL, NULL, NULL);
      word = g_convert( (gchar*) &(vdata[c].word[0]) , -1, "UTF-8", "UTF-8", NULL, NULL, NULL);

      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
		  COLUMN_LIST_WORDS,
		  word,  -1);
	 g_free(word);
   }

  return GTK_TREE_MODEL (store);
}

static void
insert_link (GtkTextBuffer *buffer,
	     GtkTextIter   *iter,
	     gchar         *text,
	     gint           page)
{
  GtkTextTag *tag;

  tag = gtk_text_buffer_create_tag (trans_buffer, NULL,
				    "foreground", "blue",
				    "underline", PANGO_UNDERLINE_SINGLE,
				    NULL);


  g_object_set_data (G_OBJECT (tag), "syns", text);
  gtk_text_buffer_insert_with_tags (trans_buffer, iter, text, -1, tag, NULL);
;
}

//Tags for decoration of output
static void create_tags (GtkTextBuffer *buffer)
{
  gtk_text_buffer_create_tag (buffer, "blue_foreground",
			      "foreground", "blue", NULL);
}

/* Create a scrolled text area  */
GtkWidget *create_text( int _add )
{
   GtkWidget *scrolled_window;

   text_trans_view = gtk_text_view_new ();
   gtk_text_view_set_editable( GTK_TEXT_VIEW (text_trans_view ), false);
   gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (text_trans_view ), GTK_WRAP_WORD);

   switch(_add)
   {
    case 1: trans_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_trans_view));break;
    case 2:
     {
       //clist = gtk_clist_new_with_titles( 1, titles);

     }break;
    //case 2: buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));break;
   }


   gtk_widget_set_size_request (GTK_WIDGET (text_trans_view), 295, 150);

   scrolled_window = gtk_scrolled_window_new (NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
		   	           GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);

   gtk_container_add (GTK_CONTAINER (scrolled_window), text_trans_view);

   gtk_widget_show_all (scrolled_window);

   return scrolled_window;
}

/*gint GlobalInit()
{
  path_str = new gchar[WORD_SIZE];

  ifstream input( "config.txt", ios::in );

   if ( !input.fail())
   {
     input >> _format >> _auto_search; input.getline(path_str, WORD_SIZE);
     input.getline(path_str, WORD_SIZE);

     if( strlen(path_str) > 0 )     font = g_strdup( path_str );
     else
     {
         strcpy(path_str, "Sans 12");
         font = g_strdup( path_str );
         //cout << path_str << endl;
     }

     input >> gl_width >> gl_height >> gl_paned_pos;

     if( gl_width == 0 || gl_height == 0 || gl_paned_pos == 0)
     {
        gl_width = 335; gl_height = 400; gl_paned_pos = 170;
     }

     /*input >> gl_num_enwords;
     //cout << gl_num_enwords << endl;
     if( gl_num_enwords < 0 || gl_num_enwords > MAX_NUM_WORDS)  gl_num_enwords = DEFAULT_NUM_ENWORDS;*

     input >> gl_fpos >> gl_row;
     //cout << gl_fpos << " --- " << gl_row << endl;
     if( gl_fpos > 3 || gl_fpos < 0) gl_fpos = 0;

     //cout << gl_width << " -- " << gl_height << " -- " << gl_paned_pos << endl;
     //cout << _format << _auto_search << font << gl_row << endl;
     input.close();

     config();
     //gl_fpos = gl_row = 0;
     //delete font;

   }
   else
   {
        _format = 1;  _auto_search = 1;
        strcpy(path_str, "Sans 12");
        font = g_strdup( path_str );

        gl_fpos = 0;  gl_row = 0;
        gl_width = 335; gl_height = 400; gl_paned_pos = 170;
   }

  return 0;
}*/

//rewritten by relativist ultrix
// defaulting in case failure should be reconsidered
gint GlobalInit()
{
  string bg, text; //colors

  path_str = new gchar[WORD_SIZE];

  ifstream input( "config.txt", ios::in );

   if ( !input.fail())
   {
     char i;
     short c=0;
     while( input >> i)  {
         switch(i)
         {
                case '1': {
                            c++;
                            input >> _format >> _auto_search;  break;
                          }
                case '2': {
                            c++;

                            //get font name saved in previous session
                            input.getline(path_str, WORD_SIZE);
                            if( strlen(path_str) > 0 )     font_name = g_strdup( path_str );
                            else
                            {
                                strcpy(path_str, "Sans 12");
                                font_name = g_strdup( path_str );
                            }
                    }break;
                case '3': {
                            c++;

                            input >> gl_width >> gl_height >> gl_paned_pos;

                            if( gl_width == 0 || gl_height == 0 || gl_paned_pos == 0)  {
                                gl_width = 335; gl_height = 400; gl_paned_pos = 170;
                                }
                        }break;
                case '4': {
                            c++;

                            input >> gl_fpos >> gl_row;

                            if( gl_fpos > 15 || gl_fpos < 0) gl_fpos = 0;
                            break;
                          }
                case '5': {
                            c++;

                            int next_n;
                            input >> next_n;
                            break;
                           }
                case '6': {
                            c++;

                            input >> bg >> text;

                            if( bg.empty() ) {
                                bg = "#ffffffffffff";
                            }

                            if(text.empty()){
                                text = "#000000000000";

                            }

                        }break;

                default:
                {
                    goto defaulting;
                }break;
         }
    }

    if(c < 6) goto defaulting;

     input.close();

   }
   else
   {
     defaulting:
        _format = 1;  _auto_search = 1;
        strcpy(path_str, "Sans 12");
        font_name = g_strdup( path_str );

        gl_fpos = 0;  gl_row = 0;
        gl_width = 365; gl_height = 400; gl_paned_pos = 170;

        bg = "#ffffffffffff";
        text = "#000000000000";


   }


   gdk_rgba_parse ( &bg_color, (gchar*)&bg[0]);
   gdk_rgba_parse ( &text_color, (gchar*)&text[0]);

  config();
  return 0;
}

void GlobalDone(void)
{
    int i=1;
    fstream output( "config.txt", ios::out );
    output << i++ << " " << _format << " " << _auto_search << endl;

    if(NULL != font_name)
        output << i++ << font_name << endl;

    //cout << gl_fpos << " " << m_search->GetRow() << endl;
    output << i++ << " " << gl_width << " " << gl_height << " " << gl_paned_pos <<endl;

    output << i++ << " " << gl_fpos << " " << m_search->GetRow() << endl;

    output << i++ << " " << m_search->GetNextN() << endl;

    output << i++ << " " << gdk_rgba_to_string(&bg_color) << " " << gdk_rgba_to_string(&text_color) << endl;

    output.close();
    g_free(font_name);

    delete [] path_str;

    delete m_search;
}

//prints phonetic transcription
GtkTextIter Print_IPA()
{
  GtkTextIter iter, start, end;

  const gchar *entry_text;
  entry_text = (gchar*) m_search->GetIPA(1);

  if( entry_text != NULL)
   {

        gtk_text_buffer_get_start_iter (trans_buffer, &start);
        gtk_text_buffer_get_end_iter (trans_buffer, &end);

        //matn yozish joyini tozalash
        gtk_text_buffer_delete(trans_buffer, &start, &end);
        gtk_text_buffer_get_iter_at_offset (trans_buffer, &iter, 0);

        entry_text = g_convert( entry_text, -1, "UTF-8", "UTF-8", NULL, NULL, NULL);
        gtk_text_buffer_insert (trans_buffer, &iter, entry_text , -1);
   }

    return iter;
}

/*------------------------------------------------------*\
    Prints translations and syns if found.
    Syns finding and distinguishing is not effective.
    Function itself got bloated.
\*-----------------------------------------------------*/
void PrintTrans_with_Syns( char *s, short choice)
{

  const char* match_word = m_search->Get1MatchWord(choice);

  const gchar *entry_text;

  GtkTextIter iter; //, start, end;


  iter = Print_IPA();

  //matn joyini tozalash

  entry_text = g_convert( (gchar*) s, -1, "UTF-8", "UTF-8", NULL, NULL, NULL);
  gtk_text_buffer_insert (trans_buffer, &iter,entry_text , -1);

 if(_errno != 2){
 set<string> set_syn = m_search->get_setof_syns(choice);
 set<string>::const_iterator set_iter = set_syn.begin();

// gets syns, buggy approach, needs reimplementation
  string str;

   if( !set_syn.empty() )
   {


      str.assign( match_word );
      int _step = (int)m_search->get_syn_order(str);


      if( (_step > 0)  && (choice == 1) )
      {
        _step = abs(_step - (int)gl_row);
        m_search->SetRow( (unsigned int)_step );


        if( str.compare( m_search->Get1MatchWord(1) ) != 0)
        {
            m_search->SetRow( (unsigned int)gl_row );
           return;
        }
        else
        {
            --_step;
            m_search->SetRow( (unsigned int)_step );
            if( str.compare( m_search->Get1MatchWord(1) ) == 0)
            {
                m_search->SetRow( (unsigned int)gl_row );
                return;
            }
        }


      }// _step > 0

      gtk_text_buffer_insert_with_tags_by_name (trans_buffer, &iter,
					    "\nRelated words:\n", -1,
					    "blue_foreground", NULL);

     for( ; set_iter != set_syn.end(); set_iter++)
     {
        str = *set_iter;
        {
            insert_link (trans_buffer, &iter, (gchar*)(str.c_str()), 0);
            gtk_text_buffer_insert (trans_buffer, &iter,"\n" , -1);
        }
     }//for lap

    }
 }//_errno != 2
}

/*Looks at all tags covering the position of iter in the text view,
  and if one of them is a link, follow it by showing the page identified
  by the data attached to it.*/

static void follow_if_link (GtkWidget   *text_view,
		GtkTextIter *iter)
{
  GSList *tags = NULL, *tagp = NULL;
  gchar *syn;

  tags = gtk_text_iter_get_tags (iter);
  for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
   {
      GtkTextTag *tag = (GtkTextTag *)tagp->data;
      syn = (gchar*)(g_object_get_data (G_OBJECT (tag), "syns"));

      if(syn != NULL)
      {
          //go & get trans
        bool  is_found = false;
        int old_fpos = gl_fpos;

        m_search->get_syn_order(syn);

        is_found = m_search->process((char*)syn);

       if( !is_found )
       {
            gl_fpos = m_search->process_file( (char*)syn, gl_fpos );
            if( old_fpos != gl_fpos)
            {
                is_found=true;
                update_words_list();
            }
       }

        //synonyms
        if(is_found)
        {
                char *s = m_search->GetTrans(_format);

                unsigned int row = m_search->GetRow();

                update_cursor_location( row );

                PrintTrans_with_Syns( s, 0 );

        }

     }

   }


  if (tags)
    g_slist_free (tags);
}

/*---------------------------------- callbacks go here --------------------------------*/

static void
tree_selection_changed_cb (GtkTreeSelection *selection, gpointer data)
{
        GtkTreeIter iter;
        GtkTreeModel *model;
        gchar *word;
        GtkTreePath *_path = (GtkTreePath*)data;

        if (gtk_tree_selection_get_selected (selection, &model, &iter))
        {

                new_quest = true;

                word = gtk_tree_model_get_string_from_iter (model, &iter);
                gl_row = g_ascii_strtod ( (const gchar *)word, NULL);
                //gtk_tree_selection_select_path(selection, _path);
                //cout << gl_row << endl;

                m_search->process(gl_row);

                char *s = m_search->GetTrans(_format);

                //synonyms
                PrintTrans_with_Syns( s, 1 );
                g_free (word);
        }
}

static void
color_select_cb (GtkWidget *button,
                    gpointer	  data)
{
  GtkWidget *dialog;
  GtkColorSelection *colorsel;
  gint response;
  gint i = GPOINTER_TO_INT(data); //which part of trans_text_view change color

  dialog = gtk_color_selection_dialog_new ("Changing color");

  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));

  //colorsel = GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dialog)->colorsel);
  colorsel = GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(dialog)));
  //cout << i << endl;

  //continue from last selected color: 0 -> text, 1->background; relativist ultrix
  gtk_color_selection_set_previous_rgba(colorsel, i > 0 ? &bg_color : &text_color);
  //gtk_color_selection_set_previous_color (colorsel, i > 0 ? &bg_color : &text_color);
  gtk_color_selection_set_current_rgba (colorsel,  i > 0 ? &bg_color : &text_color);
  gtk_color_selection_set_has_palette (colorsel, TRUE);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_OK)
    {

      switch(i)
      {
            case 0: gtk_color_selection_get_current_rgba (colorsel,
                                                            &text_color); break;

            case 1: gtk_color_selection_get_current_rgba (colorsel,
                                                            &bg_color);break;
       }

    }

  gtk_widget_destroy (dialog);
}

void font_select(GtkFontButton *widget,
                  gpointer       user_data)
{
  //gchar *font;

  if ( !font_name )
    g_free(font_name);

  font_name = g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_btn)) );
  //cout << font << endl;

  PangoFontDescription *font_desc;
 /* Change default font throughout the widget */
  font_desc = pango_font_description_from_string (font_name);

  gtk_widget_modify_font ( text_trans_view, font_desc);

  pango_font_description_free (font_desc);

}

void entry_toggle_format( GtkWidget *check_btn,
                           GtkWidget *dlg  )
{
  //_format =   GTK_TOGGLE_BUTTON(check_btn)->active;
}

void auto_toggle_search( GtkWidget *check_btn,
                           GtkWidget *dlg  )
{
  //_auto_search =   GTK_TOGGLE_BUTTON(check_btn)->active;
}
//config func
void config()
{

     if(_format )  g_signal_emit_by_name( G_OBJECT (tree_select), "changed" );

      if(_auto_search)
      {
        kdb_handler_id = g_signal_connect_object (G_OBJECT (window),
                             "key-press-event",
                             G_CALLBACK (kbd_press_event),
                             NULL,
                             (GConnectFlags)0) ;

        entry_handler_id = g_signal_connect_object (G_OBJECT (entry),
                             "changed",
                             G_CALLBACK (entry_changed),
                             entry,
                             (GConnectFlags)0) ;

      }
     else
     {
        if( kdb_handler_id > 0 && entry_handler_id > 0)
        {
            g_signal_handler_disconnect (G_OBJECT (window),   kdb_handler_id);

            g_signal_handler_disconnect (G_OBJECT (entry),    entry_handler_id);
        }

     }

    PangoFontDescription *font_desc;
    /* Change default font throughout the widget */
    font_desc = pango_font_description_from_string (font_name);

    gtk_widget_modify_font ( text_trans_view, font_desc);

    pango_font_description_free (font_desc);

    follow_link = true;

    if(!gtk_widget_has_focus(words_tree_view))

    //if ( !GTK_WIDGET_HAS_FOCUS (words_tree_view) )
    {

        gtk_widget_grab_focus( GTK_WIDGET( words_tree_view));

        if (gl_fpos != 0)
        {
            m_search->LoadNextN(gl_fpos);
            update_words_list();
        }

        update_cursor_location( gl_row );

    }

    gtk_widget_override_color (text_trans_view, GTK_STATE_FLAG_NORMAL, &text_color);
    gtk_widget_override_background_color (text_trans_view, GTK_STATE_FLAG_NORMAL, &bg_color);
}

static void
search_entry_destroyed (GtkWidget  *widget)
{
  new_quest = true;
  find_box = NULL;
}

static GtkWidget *notebook = NULL;

static void
start_search (GtkButton *button,
              GtkEntry  *entry)
{
  static bool res;
  gchar *entry_text;

  entry_text = (gchar*)gtk_entry_get_text (GTK_ENTRY (entry));

  static GtkTextIter iter, start, end;

  if( new_quest )
    {
        gtk_text_buffer_get_start_iter (trans_buffer, &iter);

        new_quest = false;
    }
  else if( res )
    {
    cout << entry_text << endl;

        iter = end;

        gtk_text_buffer_place_cursor  (trans_buffer,  &iter);

        entry_text = gtk_text_buffer_get_text (trans_buffer, &start, &end, FALSE );
        int len = strlen( entry_text );

        if(gtk_text_iter_forward_cursor_positions ( &iter, len ) )
            gtk_text_buffer_get_iter_at_mark( trans_buffer, &iter,
                                          gtk_text_buffer_get_insert(trans_buffer ) );
        g_free( entry_text);
    }


  res = gtk_text_iter_forward_search    (&iter,
                                         entry_text,
                                         (GtkTextSearchFlags)0,//GTK_TEXT_SEARCH_TEXT_ONLY,
                                         &start,
                                         &end,
                                         NULL);

  if(res)
    {
        gtk_widget_grab_focus( text_trans_view );

        gtk_text_buffer_select_range (trans_buffer, &start, &end);
        gtk_text_view_scroll_to_iter( GTK_TEXT_VIEW ( text_trans_view ), &start, 0.1, FALSE,
                                       0.5, 0.5 );
    }


}

static void
text_changed_cb (GtkEntry   *entry,
                 GParamSpec *pspec,
                 GtkWidget  *button)
{
  new_quest = gtk_entry_get_text_length(entry) > 0;

  //new_quest = entry->text_length > 0;

  gtk_widget_set_sensitive (button, new_quest);
}

GtkWidget *
do_search_entry (GtkWidget *do_widget)
{
  GtkWidget *content_area;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *entry;

  GtkWidget *find_button;
  GtkWidget *cancel_button;

  if (!find_box)
    {
      find_box = gtk_dialog_new_with_buttons ("Search Entry",
                                            GTK_WINDOW (do_widget),
 					    (GtkDialogFlags)0,//(GTK_DIALOG_DESTROY_WITH_PARENT),
					    GTK_STOCK_CLOSE,
					    GTK_RESPONSE_NONE,
					    NULL);
      gtk_window_set_resizable (GTK_WINDOW (find_box), FALSE);

      g_signal_connect (find_box, "response",
                        G_CALLBACK (gtk_widget_destroy), NULL);
      g_signal_connect (find_box, "destroy",
                        G_CALLBACK (search_entry_destroyed), &find_box);

      content_area = gtk_dialog_get_content_area (GTK_DIALOG (find_box));

      vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);//gtk_vbox_new (FALSE, 5);
      //gtk_box_pack_start (GTK_BOX (GTK_DIALOG (find_box)), vbox, TRUE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (content_area), vbox, TRUE, TRUE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

      label = gtk_label_new (NULL);
      gtk_label_set_markup (GTK_LABEL (label), "Find:");
      gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

      hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);//gtk_hbox_new (FALSE, 10);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

      /* Create our entry */
      entry = gtk_entry_new ();
      gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

      /* Create the find and cancel buttons */
      notebook = gtk_notebook_new ();
      gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);
      gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);
      gtk_box_pack_start (GTK_BOX (hbox), notebook, FALSE, FALSE, 0);

      find_button = gtk_button_new_with_mnemonic("_Find");//gtk_button_new_with_label ();
      g_signal_connect (find_button, "clicked",
                        G_CALLBACK (start_search), entry);
      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), find_button, NULL);
      gtk_widget_show (find_button);

      cancel_button = gtk_button_new_with_label ("Cancel");

      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), cancel_button, NULL);
      gtk_widget_show (cancel_button);


      g_signal_connect (entry, "notify::text",
                        G_CALLBACK (text_changed_cb), find_button);

      g_signal_connect (entry, "activate",
                        G_CALLBACK (start_search), entry);

    }

  //if (!GTK_WIDGET_VISIBLE (find_box))
  if(!gtk_widget_get_visible(find_box))
    gtk_widget_show_all (find_box);
  else
    {

      gtk_widget_destroy (find_box);
      find_box = NULL;
    }

  return find_box;
}

//config dialog
static void
conf_dialog_clicked (GtkButton *btn,
			    gpointer   user_data)
{
  GtkWidget *dialog;
  GtkWidget *content_area;
  GtkWidget *hbox, *button;
  GtkWidget *stock;
  GtkWidget *table;
  GtkWidget *edcheck, *auto_search;

  GtkWidget *label;
  gint response, text_or_bg;

  dialog = gtk_dialog_new_with_buttons("Configurations:",
					(GtkWindow*)user_data,//GTK_WINDOW (window),
					(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
					GTK_STOCK_OK,
					GTK_RESPONSE_OK,
					"_Cancel",
					GTK_RESPONSE_CANCEL,
					NULL);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);// gtk_hbox_new (FALSE, 8);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
  gtk_box_pack_start (GTK_BOX (content_area), hbox, FALSE, FALSE, 0);

  stock = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), stock, FALSE, FALSE, 0);

  table = gtk_grid_new(); //gtk_table_new (2, 2, FALSE);

  gtk_grid_set_row_spacing (GTK_GRID (table), 4);
  gtk_grid_set_column_spacing (GTK_GRID (table), 4);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);

/*----------------------------------------font selection--------------------------------------*/
  label = gtk_label_new ("Font:");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  font_btn = gtk_font_button_new ();

  gtk_font_button_set_font_name(GTK_FONT_BUTTON(font_btn),   font_name);
  gtk_grid_attach (GTK_GRID(table), label,    0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (table), font_btn, 1, 0, 1, 1);
  g_signal_connect(G_OBJECT(font_btn), "font-set",
                        G_CALLBACK(font_select), window);
/*-------------------------------------end of font selection--------------------------------------*/

/*----------------------------------------color selection--------------------------------------*/
  text_or_bg = 0;
  label = gtk_label_new ("Text color:");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  button = gtk_button_new_with_mnemonic ("_Text color");

  gtk_grid_attach (GTK_GRID (table), label,    0, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (table), button, 1, 1, 1, 1);
  g_signal_connect(G_OBJECT(button), "clicked",
                        G_CALLBACK(color_select_cb), GINT_TO_POINTER(text_or_bg));
/*-------------------------------------end of color selection------------------------------------*/

/*----------------------------------------bg color selection--------------------------------------*/
  text_or_bg = 1;
  label = gtk_label_new ("Background color:");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  button = gtk_button_new_with_mnemonic ("_Background color");

  gtk_grid_attach(GTK_GRID (table), label,    0, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (table), button, 1, 2, 1, 1);
  g_signal_connect(G_OBJECT(button), "clicked",
                        G_CALLBACK(color_select_cb),  GINT_TO_POINTER(text_or_bg));
/*-------------------------------------end of bg color selection-----------------------------------*/

  label = gtk_label_new ("Format output:");
  gtk_grid_attach(GTK_GRID(table), label, 0,3,1,1);
  /*gtk_table_attach_defaults (GTK_TABLE (table),
			     label,
			     0, 1, 3, 4);*/
  edcheck = gtk_check_button_new ();
  gtk_grid_attach(GTK_GRID(table), edcheck, 1,3,1,1);
  /*gtk_table_attach_defaults (GTK_TABLE (table),
			     edcheck,
			     1, 2, 3, 4);*/

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (edcheck), _format);

  label = gtk_label_new ("Auto search:");
  gtk_grid_attach(GTK_GRID(table), label, 0,4,1,1);
  /*gtk_table_attach_defaults (GTK_TABLE (table),
			     label,
			     0, 1, 4, 5);*/
			     //0, 1, 2, 3);

  auto_search = gtk_check_button_new ();
  gtk_grid_attach(GTK_GRID(table), auto_search, 1,4,1,1);
  /*gtk_table_attach_defaults (GTK_TABLE (table),
			     auto_search,
			     1, 2, 4, 5);
			     //1, 3, 2, 3);*/

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (auto_search), _auto_search);


  gtk_widget_show_all (hbox);
  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_OK)
    {

      _format      =   gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(edcheck)); //GTK_TOGGLE_BUTTON(edcheck)->active;

      _auto_search =   gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(auto_search)/*->active*/);

      config();

    }
    else if(response == GTK_RESPONSE_CANCEL )
    {

        _format      = _format;
        _auto_search = _auto_search;
    }

  gtk_widget_destroy (dialog);
}

//needs some improvements, shoud be thoroughly checked against bugs
static gboolean
kbd_press_event (GtkWidget *text_view,
		 GdkEventKey *event)
{
    static bool _resize = false;

   if(    ((event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK ) ||
          ((event->state & GDK_MOD1_MASK)    == GDK_MOD1_MASK  )   ||
          ((event->state & GDK_META_MASK)    == GDK_META_MASK ) )
   {

       if( event->keyval == GDK_KEY_f )
       {
           follow_link = true;
           find_box = do_search_entry( window );
       }

       return FALSE;

   }
   else

  if (  (event->keyval==GDK_KEY_BackSpace)
        || (event->keyval==GDK_KEY_Down) || (event->keyval==GDK_KEY_Up) ||
        (event->keyval==GDK_KEY_Tab) || (event->keyval==GDK_KEY_Right) || (event->keyval==GDK_KEY_Left) ||
        (event->keyval==GDK_KEY_Home) || (event->keyval==GDK_KEY_End)
        || (event->keyval==GDK_KEY_Return) || (event->keyval==GDK_KEY_space) || (event->keyval==GDK_KEY_Delete)
        || (event->keyval==GDK_KEY_Escape) || (event->keyval==GDK_KEY_F8)
        || (event->keyval==GDK_KEY_Page_Up) || (event->keyval==GDK_KEY_Page_Down) )
  {
      switch(event->keyval)
        {
            case GDK_KEY_Down:
            {
                if ( (!gtk_widget_has_focus (words_tree_view) ) && !(gtk_widget_has_focus ( GTK_WIDGET(text_trans_view) ) ))
                {
                    if( !_resize )
                    {
                        not_found_count = 0;
                        gtk_widget_grab_focus ( GTK_WIDGET(words_tree_view) );//focus on tree
                    }

                } else return FALSE;
                if( _resize ) return FALSE;
            }

            case GDK_KEY_Up:
                if ( !( gtk_widget_has_focus (words_tree_view) ) && !(gtk_widget_has_focus ( GTK_WIDGET(text_trans_view) ) ) )
                {
                    if( !_resize )
                    {
                        not_found_count = 0;
                        gtk_widget_grab_focus ( GTK_WIDGET(words_tree_view) );//focus on tree
                    }


                } else return FALSE;
                if( _resize ) return FALSE;

            case GDK_KEY_Left:
                if(  !gtk_widget_has_focus( text_trans_view ) )
                {

                    if( !_resize )
                        gtk_widget_grab_focus ( GTK_WIDGET(text_trans_view) );//focus on output*/

                    return FALSE;
                } else return FALSE;

            case GDK_KEY_Right:return FALSE;


            case GDK_KEY_Page_Down:return FALSE;
            case GDK_KEY_Page_Up: return FALSE;
            case GDK_KEY_Tab:  return FALSE;

            case GDK_KEY_Return:

                     if( gtk_widget_has_focus( text_trans_view ) )//if(_resize )
                     {

                         follow_link = true;

                        return FALSE;
                     } else { _resize = !_resize; return FALSE; }


            case GDK_KEY_space:
             if(  !(gtk_widget_has_focus( words_tree_view ) ) && ( gtk_widget_has_focus( text_trans_view )) )
                {
                    if( !_resize )
                    {
                        not_found_count = 0;
                        gtk_widget_grab_focus ( GTK_WIDGET(words_tree_view) );//focus on output
                    }
                    return true;

                } else return FALSE;


            case GDK_KEY_BackSpace:
            if(  gtk_widget_has_focus( entry ) )
            {
              if(not_found_count > 0) not_found_count-=2; return FALSE;
            }
            else if( !_resize )
                  {
                        not_found_count = 0;
                        gtk_widget_grab_focus ( GTK_WIDGET( entry ) );//focus on input

                        gtk_editable_set_position   ( GTK_EDITABLE( entry ), -1);
                        return FALSE;
                  }

            case GDK_KEY_Delete:    return FALSE;
            case GDK_KEY_Home:      return FALSE;
            case GDK_KEY_End:       return FALSE;

            case GDK_KEY_Escape:{ _resize ? _resize = false: _resize = true; return FALSE;}

            case GDK_KEY_F8:
                    {
                        _resize ? _resize = false: _resize = true;//_resize =!_resize;
                        //if(_resize) return false;
                        return false;
                    }


            default: break;
        }

  }

  if  (   ( event->keyval >= GDK_KEY_a) && (event->keyval <= GDK_KEY_z)
       || ( event->keyval >= GDK_KEY_A) && (event->keyval <= GDK_KEY_Z)
       || ( event->keyval == GDK_KEY_minus ) || ( event->keyval == GDK_KEY_space)
       || ( event->keyval >= GDK_KEY_0 ) && ( event->keyval <= GDK_KEY_9 ) )

    {

        if( !gtk_widget_has_focus( entry ) )
        {

            gtk_entry_set_text(GTK_ENTRY(entry), gdk_keyval_name( event->keyval ) );
            gtk_widget_grab_focus (entry);//focus on Maggie
            g_signal_emit_by_name(G_OBJECT(entry), "activate" );
        }


    }
  else return TRUE;
  return FALSE;
}

/* Links can be activated by pressing Enter. */
static gboolean
key_press_event (GtkWidget *text_view,
		 GdkEventKey *event)
{
  GtkTextIter iter;
  GtkTextBuffer *buffer;

  switch (event->keyval)
    {
      case GDK_KEY_Return:
      case GDK_KEY_KP_Enter:
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                          gtk_text_buffer_get_insert (buffer));
        follow_if_link (text_view, &iter);
        break;

      default:
        break;
    }

  return FALSE;
}

/* Links can also be activated by clicking. */
static gboolean event_after (GtkWidget *text_view,
	     GdkEvent  *ev)
{

  GtkTextIter start, end, iter;
  //GtkTextBuffer *buffer;
  GdkEventButton *event;
  gint x, y;

  if (ev->type != GDK_BUTTON_RELEASE)
    return FALSE;

  event = (GdkEventButton *)ev;

  if (event->button != 1)
    return FALSE;


  /* we shouldn't follow a link if the user has selected something */
  gtk_text_buffer_get_selection_bounds (trans_buffer, &start, &end);
  if (gtk_text_iter_get_offset (&start) != gtk_text_iter_get_offset (&end))
    return FALSE;

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view),
                                         GTK_TEXT_WINDOW_WIDGET,
                                         (gint)event->x, (gint)event->y, &x, &y);

  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (text_view), &iter, x, y);

  follow_if_link (text_view, &iter);

  return FALSE;
}

void entry_changed( GtkWidget *widget,
                     GtkWidget *entry )
{
    g_signal_emit_by_name(G_OBJECT(entry), "activate" );
}

void enter_callback( GtkWidget *widget,
                     GtkWidget *entry )
{

  if (follow_link ) { follow_link = false; return;}

  const gchar *entry_text;
  entry_text = gtk_entry_get_text (GTK_ENTRY (entry));

  gchar * down_case;
  down_case = g_utf8_strdown ( entry_text, -1 );


  bool  is_found = false;
  int old_fpos = gl_fpos;

  is_found = m_search->process((char*) down_case);//entry_text);

  if( !is_found )
  {
    if( not_found_count < 0) not_found_count = 0;

    ++not_found_count;

    if( not_found_count == 1)
    {

        gl_fpos = m_search->process_file( (char*) down_case/*entry_text*/, gl_fpos );
        if( old_fpos != gl_fpos)
        {
            is_found = true;
            not_found_count = 0;
            update_words_list(); /*GlobalInit();*/
        }
    }


  }


  g_free( down_case );

  if (is_found)
  {

    char *s = m_search->GetTrans(_format);


    gl_row =  m_search->GetRow();
    //synonyms
    PrintTrans_with_Syns( s, 0 );

    update_cursor_location( gl_row );

  }

}


void update_cursor_location(unsigned short row)
{

    memset(path_str, 0, strlen(path_str) );
    path_str = g_ascii_dtostr (path_str, 10, row);
    //cout << path_str << endl;

    path = gtk_tree_path_new_from_string( path_str );

    gtk_tree_selection_select_path (tree_select, path) ;

    //gtk_tree_view_set_cursor (GTK_TREE_VIEW(words_tree_view),  path, NULL, FALSE);
    //gtk_tree_view_scroll_to_point(GTK_TREE_VIEW(words_tree_view), row, 0);

    gtk_tree_view_scroll_to_cell     ( GTK_TREE_VIEW( words_tree_view),
                                                         path,
                                                         NULL,
                                                         FALSE,
                                                         0, 0);
    gtk_tree_view_set_cursor (GTK_TREE_VIEW(words_tree_view),  path, NULL, FALSE);

    gtk_tree_path_free ( path );
}

int update_words_list( )
{

  gboolean is_nxt_iter = TRUE;
  GtkTreeIter iter;
  gint i = 0;


  GtkTreePath *path;

  path = gtk_tree_path_new_from_string ("0");
  gtk_tree_model_get_iter ( words_list_model,
                           &iter,
                           path);
  gtk_tree_path_free (path);


  vector< string >::const_iterator Iter;

  gchar *temp_str;
   vector<_data> vdata = m_search->GetDataSeq();

 for(int c=0; c < m_search->GetNextN(); c++ )
   {

       temp_str =  g_convert( (gchar*) &(vdata[c].word[0]), -1, "UTF-8", "UTF-8", NULL, NULL, NULL);

       gchar *old_text;

       gtk_tree_model_get (words_list_model, &iter, COLUMN_LIST_WORDS , &old_text, -1);
       g_free (old_text);

       gtk_list_store_set (GTK_LIST_STORE (words_list_model), &iter,
                           COLUMN_LIST_WORDS,
                           temp_str,
                           -1);

        is_nxt_iter = gtk_tree_model_iter_next(words_list_model, &iter);

        g_free(temp_str);
    }

  return i;
}


gboolean scroll_vertical (GtkTreeView    *treeview,
                           GtkMovementStep arg1,
                           gint            scroll_type,
                           gpointer        user_data)
{

    static int next_n = m_search->GetNextN();//Number of rows to be scrolled up
    //cout << next_n << endl;

   if(gl_row == 0)
   {

     if (scroll_type == GTK_SCROLL_BACKWARD)
     {
       if(gl_fpos == 0) { gl_row = 0; return false;}
       else
        {
          gl_fpos--;

          next_n = m_search->LoadNextN( gl_fpos );
          if(next_n < 0) {
                m_search->geterr(errmsg);
                MessageBox(errmsg);
              }
          update_words_list();   update_cursor_location(next_n-1);
          //gtk_tree_view_row_activated(GTK_TREE_VIEW(words_tree_view), path, NULL);
          //gtk_tree_selection_select_path(tree_select, path);
          return true;
        }
     }

   }


   if( gl_row == next_n-1)
   {

     if (scroll_type == GTK_SCROLL_FORWARD)
     {
        if( gl_fpos == m_search->GetFposNum() ){ gl_row = next_n-1;  return false;}
        else
        {
           gl_fpos++;

           next_n = m_search->LoadNextN( gl_fpos );
           update_words_list();
           update_cursor_location(0);

           return true;
        }
     }

   }

   return false;
}


static gboolean hovering_over_link = FALSE;
static GdkCursor *hand_cursor = NULL;
static GdkCursor *regular_cursor = NULL;

/* Looks at all tags covering the position (x, y) in the text view,
 * and if one of them is a link, change the cursor to the "hands" cursor
 * typically used by web browsers.
 */
static void
set_cursor_if_appropriate (GtkTextView    *text_view,
                           gint            x,
                           gint            y)
{
  GSList *tags = NULL, *tagp = NULL;
  GtkTextIter iter;
  gboolean hovering = FALSE; //not on hoverboard ;) BTTF
  gchar *syn;

  gtk_text_view_get_iter_at_location (text_view, &iter, x, y);

  tags = gtk_text_iter_get_tags (&iter);
  for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
    {
      GtkTextTag *tag = (GtkTextTag *)tagp->data;

      syn = (gchar*)(g_object_get_data (G_OBJECT (tag), "syns"));

       //if ( strlen((char*) syn) > 0 )
       if( syn != NULL)
        {
          hovering = TRUE;
          break;
        }
    }

  if (hovering != hovering_over_link)
    {
      hovering_over_link = hovering;

      if (hovering_over_link)
        gdk_window_set_cursor(gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT), hand_cursor);
      else
        gdk_window_set_cursor (gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT), regular_cursor);
    }

  if (tags)
    g_slist_free (tags);
}


/* Update the cursor image if the pointer moved. */
static gboolean
motion_notify_event (GtkWidget      *text_view,
		     GdkEventMotion *event)
{
  gint x, y;

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view),
                                         GTK_TEXT_WINDOW_WIDGET,
                                         (gint)event->x, (gint)event->y, &x, &y);

  set_cursor_if_appropriate (GTK_TEXT_VIEW (text_view), x, y);

  gdk_window_get_pointer (gtk_text_view_get_window(GTK_TEXT_VIEW(text_view), GTK_TEXT_WINDOW_WIDGET), NULL, NULL, NULL);
  return FALSE;
}

/* Also update the cursor image if the window becomes visible
 * (e.g. when a window covering it got iconified).
*/
static gboolean
visibility_notify_event (GtkWidget          *text_view,
			 GdkEventVisibility *event)
{
  gint wx, wy, bx, by;

  //gdk_window_get_pointer (text_view->window, &wx, &wy, NULL);

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view),
                                         GTK_TEXT_WINDOW_WIDGET,
                                         wx, wy, &bx, &by);

  set_cursor_if_appropriate (GTK_TEXT_VIEW (text_view), bx, by);

  return FALSE;
}

static gboolean
focus_in_callback  (GtkWidget     *widget,
                           GdkEventFocus *event,
                           gpointer       user_data)
{
    gtk_widget_grab_focus( entry );
}

static gboolean  wnd_is_visible  (GtkWidget          *widget,
                                  GdkEventFocus       *event,
                                  gpointer            user_data)
{

  if( ( TRUE == event->in ) && !gtk_widget_has_focus(entry) && !(follow_link) )

  {
      gtk_widget_grab_focus( entry );
  }

  return FALSE;
}

static gboolean  gdk_event_conf  (GtkWidget          *widget,
                                  GdkEventConfigure  *event,
                                  gpointer            user_data)
{
    gl_width = event->width ; gl_height = event->height;
    return FALSE;
}

void  gtk_main_loop_quit(void)
{
    gl_paned_pos = gtk_paned_get_position( GTK_PANED (vpaned) );

    gtk_main_quit();
}

/*void paste_clipboard_callback(GtkWidget *widget,
                               GtkWidget *entry )
{
 //empty
}*/



void about_us( GtkWidget *widget,
            gpointer   data )
{

   gchar *authors[] = { "1. Dictionary files by:", "\tM. Ali Vardar (c) 2001-2005",
                        "\tGorkem CETIN", "\tSertan KOLAT", "\twww.linuxprogramlama.com",
                        "\n2. Phonetic transcriptions was obtained from:\n\twww.photransedit.com",
                        "\n3. based on code by Mirzohid Aminjon Muhtor oglu\n\t(c) 2006-2015", NULL };


   gtk_show_about_dialog (NULL,
                          "program-name", prog_name,
                          "copyright", "Turkiston Dasturlari (c) 2015\nMuqobil Dasturlar To'plami (c) 2006-2012\nEnglish to Turkish edition",
                          "wrap-license", true,
                          "license", "The source code is released under the terms of GPL version 2.",
                          "version", ver,
                          "comments", "O'zbekiston, Toshkent shahri\n  muqobildasturlar@gmail.com",
                          "title", ("About GLug'at"),
                          "authors", authors,
                         NULL);

}

//add by relativist ultrix
//wrap shell of gtk message dialog
void MessageBox( string &msg, GtkMessageType msgtype)
{
    GtkWidget *msgbox;
    cout << "TKL: "<< msg << endl;

    msgbox = gtk_message_dialog_new(GTK_WINDOW (window), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_INFO,
                                                GTK_BUTTONS_OK, &msg[0]);
    gtk_dialog_run(GTK_DIALOG(msgbox));
    gtk_widget_destroy(msgbox);
}


int main( int   argc,
          char *argv[] )
{

    GtkWidget *scrolled_window;
    GtkWidget *vbox, *hbox;
    GtkWidget *button;


    GtkWidget *bbox;

    GtkWidget *frame,*frame1;//Group box

    GtkAdjustment *adj1;

    gboolean _mode;

    _mode = gtk_init_check (&argc, &argv);//check whether we`re in GUI or text mode

    if(_mode )
    {

     /* create a new window */
     window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

     gtk_window_set_title (GTK_WINDOW (window), prog_name);

     g_signal_connect (G_OBJECT (window), "destroy",
                       G_CALLBACK (gtk_main_loop_quit), NULL);

     g_signal_connect_swapped (G_OBJECT (window), "delete_event",
                               G_CALLBACK (gtk_widget_destroy),
                               window);

      m_search = new CLugat();

     _errno = m_search->geterr(errmsg);
     if( _errno >= 0)
     {

        if(_errno > 1) MessageBox(errmsg); //let complain little
        _errno = m_search->init();
        if(_errno < 0)
        {
            m_search->geterr(errmsg);
            //cout << "TKL:" << errmsg << endl;
            MessageBox(errmsg);
            return 0;
        }
     }
     else
     {
        MessageBox( errmsg);
         return 0;
      }



     vbox = gtk_vbox_new (FALSE, 0);
     gtk_container_add (GTK_CONTAINER (window), vbox);
     gtk_widget_show (vbox);

     entry = gtk_entry_new ();
     gtk_entry_set_max_length (GTK_ENTRY (entry), 50);
     gtk_widget_set_can_default(entry, true);
     gtk_widget_set_can_focus(entry ,true);
     //GTK_WIDGET_SET_FLAGS( entry, GTK_ _CAN_DEFAULT | GTK_CAN_FOCUS);


     gtk_window_set_default( GTK_WINDOW(window), entry);


     //signals for entry widget
     g_signal_connect (G_OBJECT (entry), "activate",
         		      G_CALLBACK (enter_callback),
	        	      entry);



     gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
     gtk_widget_show (entry);

     hbox = gtk_hbox_new (FALSE, 0);
     gtk_container_add (GTK_CONTAINER (vbox), hbox);
     gtk_widget_show (hbox);


     //Group box
     frame1 = gtk_frame_new ("Output");
     gtk_box_pack_start (GTK_BOX (hbox),frame1,TRUE, TRUE, 0);


      /* create a vpaned widget and add it to our toplevel window */
     vpaned = gtk_vpaned_new ();
     gtk_container_add (GTK_CONTAINER (frame1), vpaned);


/*-------------------------------------------------------------------------*/


     /*adjustment */
     adj1 = gtk_adjustment_new (0.0, 0.0, 10.0, 0.1, 1.0, 0.1);

/* Create a scrolled window to pack the CList widget into */
    scrolled_window = gtk_scrolled_window_new (NULL, GTK_ADJUSTMENT (adj1));

    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);


/*---------------------------------------------------------------------------*/

/*----------------------------------------words list----------------------------------------------*/
   /*build words list model*/
    words_list_model = create_list_model();

    /* create tree view */
    words_tree_view = gtk_tree_view_new_with_model (words_list_model);
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (words_tree_view), TRUE);
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (words_tree_view)),
                                   GTK_SELECTION_SINGLE);

    g_signal_connect (G_OBJECT (words_tree_view), "move-cursor",
                  G_CALLBACK (scroll_vertical),
                  NULL);

    add_columns (GTK_TREE_VIEW (words_tree_view), words_list_model);
    g_object_unref (words_list_model);

    gtk_container_add(GTK_CONTAINER(scrolled_window), words_tree_view);


    /* Setup the selection handler */

    tree_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (words_tree_view));
    gtk_tree_selection_set_mode (tree_select, GTK_SELECTION_SINGLE);
    g_signal_connect (G_OBJECT (tree_select), "changed",
                  G_CALLBACK (tree_selection_changed_cb),
                  path);


/*------------------------------------------words list---------------------------------------------*/

     //translation text
     text_trans = create_text (1);
     gtk_paned_add1 (GTK_PANED (vpaned), text_trans);

     /*------------event bindings for links-----------------*/
     hand_cursor = gdk_cursor_new (GDK_HAND2);
     regular_cursor = gdk_cursor_new (GDK_XTERM);

     g_signal_connect (text_trans_view, "event-after",
			G_CALLBACK (event_after), NULL);
     g_signal_connect (text_trans_view, "key-press-event",
			G_CALLBACK (key_press_event), NULL);

      g_signal_connect (text_trans_view, "motion-notify-event",
			G_CALLBACK (motion_notify_event), NULL);
      g_signal_connect (text_trans_view, "visibility-notify-event",
			G_CALLBACK (visibility_notify_event), NULL);
     /*------------event bindings for links-----------------*/

     gtk_paned_add2 (GTK_PANED (vpaned), scrolled_window);
     gtk_widget_show (scrolled_window);


     frame = gtk_frame_new ("Controls");
     gtk_box_pack_start (GTK_BOX (vbox),frame,FALSE, FALSE, 0);

     bbox = gtk_hbutton_box_new ();
     gtk_container_set_border_width (GTK_CONTAINER (bbox), 3);
     gtk_container_add (GTK_CONTAINER (frame), bbox);

     /* Set the appearance of the Button Box */
     gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_EDGE);
     gtk_box_set_spacing (GTK_BOX (bbox), 10);

     button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
     g_signal_connect_swapped (G_OBJECT (button), "clicked",
			      G_CALLBACK (gtk_widget_destroy),
			      window);
     gtk_container_add (GTK_CONTAINER (bbox), button);

     //button = gtk_button_new_with_mnemonic ("Configur_e");
     button = gtk_button_new_from_stock(GTK_STOCK_PREFERENCES);
     g_signal_connect (button, "clicked",
			G_CALLBACK (conf_dialog_clicked), NULL);
     gtk_container_add (GTK_CONTAINER (bbox), button);


     //about us if anyone wish to know

     button = gtk_button_new_from_stock(GTK_STOCK_ABOUT);
     g_signal_connect (G_OBJECT (button), "clicked",
 			G_CALLBACK(about_us), window);

     gtk_container_add (GTK_CONTAINER (bbox), button);


     gtk_widget_add_events ( window, GDK_FOCUS_CHANGE_MASK);


                      "focus-in-event",
     g_signal_connect(G_OBJECT (window),
                       G_CALLBACK (wnd_is_visible),
                       NULL);

    g_signal_connect(G_OBJECT (window),
                      "configure-event",
                       G_CALLBACK (gdk_event_conf),
                       NULL);

     GlobalInit();

     gtk_paned_set_position( GTK_PANED (vpaned), gl_paned_pos );
     gtk_widget_show (vpaned);

     gtk_widget_set_size_request (GTK_WIDGET (window), gl_width, gl_height);

     create_tags( trans_buffer );//create tags for text buffer

     gtk_widget_show_all (window);

     gtk_main();
    }
    
    GlobalDone();
    return 0;
}
