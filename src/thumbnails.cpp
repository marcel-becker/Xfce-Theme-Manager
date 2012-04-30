/*
 * Original author of parts of gtkpreview David D Lowe <daviddlowe.flimm@gmail.com>
 * Parts of cursor preveiw Copyright (c) 2008 Nick Schermer <nick@xfce.org> & Jannis Pohlmann <jannis@xfce.org>
 * from xfce4-settings-4.6.5/dialogs/mouse-settings
 *
 * Parts of xwm4 4.10pre
 *
 * oroborus - (c) 2001 Ken Lynch
 * xfwm4    - (c) 2002-2011 Olivier Fourdan
 *
 * Seriously mucked about by:
 *
 * K.D.Hedger 2012 <kdheger@yahoo.co.uk>
 *
 * thumbnails.cpp
 */

#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <string.h>
#include <X11/Xcursor/Xcursor.h>

#include "globals.h"

int			boxhite=90;
int			button_offset,button_spacing;

GdkPixbuf *cursorprev (const char *ptrname,char* themename)
{
	XcursorImage	*image;
	GdkPixbuf	*scaled=NULL, *pixbuf=NULL;
	gsize		bsize;
	guchar		*buffer, *p, tmp;

    /* load the image */

	image=XcursorLibraryLoadImage (ptrname,themename,32);

	if (G_LIKELY(image))
		{
			bsize=image->width*image->height*4;
			buffer=(guchar*)g_malloc(bsize);

        /* copy pixel data to buffer */
			memcpy(buffer,image->pixels,bsize);
        /* swap bits */
			for (p=buffer;p<buffer+bsize;p+=4)
				{
					tmp=p[0];
					p[0]=p[2];
					p[2]=tmp;
				}
        /* create pixbuf */
			pixbuf=gdk_pixbuf_new_from_data(buffer,GDK_COLORSPACE_RGB,TRUE,8,image->width,image->height,4*image->width,(GdkPixbufDestroyNotify) g_free,NULL);

        /* don't leak when creating the pixbuf failed */
			if (G_UNLIKELY(pixbuf==NULL))
				g_free(buffer);

			if (pixbuf!=NULL)
				{
					scaled=gdk_pixbuf_scale_simple(pixbuf,32,32,GDK_INTERP_BILINEAR);
					g_object_unref (G_OBJECT (pixbuf));
				}
				XcursorImageDestroy (image);
		}
	return scaled;
}

void makecursor(char* theme,char* outPath)
{
	GdkPixbuf*	arrow;
	GdkPixbuf*	move;
	GdkPixbuf*	wait;
	GdkPixbuf*	hand;
	cairo_surface_t *surface;
	cairo_t *cr;

	controlsPixbuf=NULL;
	surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,128,32);
	cr=cairo_create(surface);

	arrow=cursorprev("left_ptr",theme);
	move=cursorprev("fleur",theme);
	wait=cursorprev("watch",theme);
	hand=cursorprev("hand2",theme);

	if (arrow==NULL || move==NULL || wait==NULL || hand==NULL)
		exit(1);
	cairo_save (cr);
		gdk_cairo_set_source_pixbuf(cr,arrow,0,0);
		cairo_paint_with_alpha(cr,100);
		gdk_cairo_set_source_pixbuf(cr,move,32,0);
		cairo_paint_with_alpha(cr,100);
		gdk_cairo_set_source_pixbuf(cr,wait,64,0);
		cairo_paint_with_alpha(cr,100);
		gdk_cairo_set_source_pixbuf(cr,hand,96,0);
		cairo_paint_with_alpha(cr,100);
	cairo_restore (cr);

	cairo_surface_write_to_png(surface,outPath);

	g_object_unref(arrow);
	g_object_unref(move);
	g_object_unref(wait);
	g_object_unref(hand);

	cairo_surface_destroy(surface);
	cairo_destroy(cr);
}

GdkPixmap* draw_window_on_pixbuf(GtkWidget *widget)
{
	GdkVisual	*visual;
	GdkPixmap	*pixmap;
	GtkStyle	*style;
	GdkScreen	*screen=gdk_screen_get_default();
	GdkWindow	*window;
	gint		width,height;

	gtk_widget_ensure_style(widget);

	style=gtk_widget_get_style(widget);

	g_assert(style);
	g_assert(style->font_desc);

	gtk_window_get_size(GTK_WINDOW(widget), &width, &height);

	visual=gtk_widget_get_visual(widget);
	pixmap=gdk_pixmap_new(NULL, width, height,gdk_visual_get_depth(visual));
	gdk_drawable_set_colormap(GDK_DRAWABLE(pixmap),gtk_widget_get_colormap(widget));

	window=gtk_widget_get_window(widget);

	gdk_window_redirect_to_drawable(window, pixmap, 0, 0, 0, 0, width, height);
	gdk_window_set_override_redirect(window, TRUE);
	gtk_window_move(GTK_WINDOW(widget), gdk_screen_get_width(screen), gdk_screen_get_height(screen));
	gtk_widget_show(widget);

	gdk_window_process_updates(window, TRUE);
	gtk_widget_hide(widget);

	return pixmap;
}

GdkPixbuf * create_gtk_theme_pixbuf(char* name)
{
	GtkSettings *settings;
	GtkWidget *window, *vbox, *box, *stock_button, *checkbox, *radio;
	GtkRequisition requisition;
	GtkAllocation allocation;
	GdkPixmap *pixmap;
	GdkPixbuf *pixbuf, *retval;
	gint width, height;

	settings=gtk_settings_get_default();
	g_object_set(settings, "gtk-theme-name",(char*)name,"gtk-color-scheme", "default",NULL);

	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	gtk_window_set_default_size((GtkWindow*)window,controlWidth,controlHeight);

	vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	box=gtk_hbox_new(FALSE, 6);
	gtk_container_set_border_width(GTK_CONTAINER(box), 6);
	gtk_box_pack_start(GTK_BOX(vbox), box, FALSE, FALSE, 0);
	stock_button=gtk_button_new_from_stock(GTK_STOCK_OPEN);
	gtk_box_pack_start(GTK_BOX(box), stock_button, FALSE, FALSE, 0);
	checkbox=gtk_check_button_new();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), TRUE);
	gtk_box_pack_start(GTK_BOX(box), checkbox, FALSE, FALSE, 0);
	radio=gtk_radio_button_new_from_widget(NULL);
	gtk_box_pack_start(GTK_BOX(box), radio, FALSE, FALSE, 0);

	gtk_widget_show_all(vbox);
	gtk_widget_realize(stock_button);
	gtk_widget_realize(gtk_bin_get_child(GTK_BIN(stock_button)));
	gtk_widget_realize(checkbox);
	gtk_widget_realize(radio);
	gtk_widget_map(stock_button);
	gtk_widget_map(gtk_bin_get_child(GTK_BIN(stock_button)));
	gtk_widget_map(checkbox);
	gtk_widget_map(radio);

	gtk_widget_size_request(window, &requisition);
	allocation.x=0;
	allocation.y=0;
	allocation.width=requisition.width;
	allocation.height=requisition.height;
	gtk_widget_size_allocate(window, &allocation);
	gtk_widget_size_request(window, &requisition);

	gtk_window_get_size(GTK_WINDOW(window), &width, &height);

	pixmap=draw_window_on_pixbuf(window);

	pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
	gdk_pixbuf_get_from_drawable(pixbuf, pixmap, NULL, 0, 0, 0, 0, width, height);

	retval=pixbuf;
	gtk_widget_destroy(window);
	g_object_unref(pixmap);

	g_object_set(settings,"gtk-theme-name",currentGtkTheme,"gtk-color-scheme","default",NULL);
	return retval;
}

static const char* image_types[]={"png","svg","gif","jpg","bmp",NULL};

GdkPixbuf* composePixbuf(char* bordername,const char* name)
{
	char*		pixmapname;
	GdkPixbuf*	basepixbuf=NULL;
	GdkPixbuf*	alpha=NULL;
	int		i=0;
	gint		width,height;

	asprintf(&pixmapname,"%s/xfwm4/%s.xpm",bordername,name);
	basepixbuf=gdk_pixbuf_new_from_file((char*)pixmapname,NULL);

	while ((image_types[i]) && (!alpha))
		{
			asprintf(&pixmapname,"%s/xfwm4/%s.%s",bordername,name,image_types[i]);
        		if (g_file_test(pixmapname,G_FILE_TEST_IS_REGULAR))
            			alpha=gdk_pixbuf_new_from_file(pixmapname,NULL);
        		++i;
		}
/* We have no suitable image to layer on top of the XPM, stop here... */
	if (!alpha)
 			return (basepixbuf);

/* We have no XPM canvas and found a suitable image, use it... */
    if (!basepixbuf)
        return (alpha);

	width=MIN(gdk_pixbuf_get_width(basepixbuf),gdk_pixbuf_get_width (alpha));
	height=MIN(gdk_pixbuf_get_height(basepixbuf),gdk_pixbuf_get_height (alpha));

	gdk_pixbuf_composite(alpha,basepixbuf,0,0,width,height,0,0,1.0,1.0,GDK_INTERP_NEAREST,0xFF);

	g_object_unref (alpha);

	return basepixbuf;
}

void makeborder(char* folder,char* outframe)
{
	GdkPixbuf*	topleft;
	GdkPixbuf*	toprite;
	GdkPixbuf*	title1;
	GdkPixbuf*	title2;
	GdkPixbuf*	title3;
	GdkPixbuf*	title4;
	GdkPixbuf*	title5;
	GdkPixbuf*	riteside;
	GdkPixbuf*	leftside;
	GdkPixbuf*	bottomleft;
	GdkPixbuf*	bottomrite;
	GdkPixbuf*	bottom;
	GdkPixbuf*	close;
	GdkPixbuf*	max;
	GdkPixbuf*	min;
	GdkPixbuf*	menu;
	GdkPixbuf*	arrow;

	GtkIconTheme*	theme;
	
	int		lsegwid,rsegwid,boxwid,hiteoffset=0;
	int		closewid=0,maxwid=0,minwid=0,menuwid=0;
	int		closehite=0,maxhite=0,minhite=0,menuhite=0;
	int		topleftwid=0,toplefthite=0,topritewid=0,topritehite=0;
	int		bottomleftwid=0,bottomlefthite=0,bottomritewid=0,bottomritehite=0;
	int		leftsidewid=0,leftsidehite=0,ritesidewid=0,ritesidehite=0;
	int		bottomwid=0,bottomhite=0;

	int		title1wid=0,title1hite=0;
	int		title2wid=0,title2hite=0;
	int		title3wid=0,title3hite=0;
	int		title4wid=0,title4hite=0;
	int		title5wid=0,title5hite=0;

	int		padwid=PADWIDTH;

	cairo_surface_t *surface;
	cairo_t *cr;

	topleft=composePixbuf(folder,"top-left-active");
	toprite=composePixbuf(folder,"top-right-active");
	title1=composePixbuf(folder,"title-1-active");
	title2=composePixbuf(folder,"title-2-active");
	title3=composePixbuf(folder,"title-3-active");
	title4=composePixbuf(folder,"title-4-active");
	title5=composePixbuf(folder,"title-5-active");
	riteside=composePixbuf(folder,"right-active");
	leftside=composePixbuf(folder,"left-active");
	bottomleft=composePixbuf(folder,"bottom-left-active");
	bottomrite=composePixbuf(folder,"bottom-right-active");
	bottom=composePixbuf(folder,"bottom-active");
	close=composePixbuf(folder,"close-active");
	max=composePixbuf(folder,"maximize-active");
	min=composePixbuf(folder,"hide-active");
	menu=composePixbuf(folder,"menu-active");

	if (title1!=NULL)
		{
			title1wid=gdk_pixbuf_get_width((const GdkPixbuf *)title1);
			title1hite=gdk_pixbuf_get_height((const GdkPixbuf *)title1);
		}
	if (title2!=NULL)
		{
			title2wid=gdk_pixbuf_get_width((const GdkPixbuf *)title2);
			title2hite=gdk_pixbuf_get_height((const GdkPixbuf *)title2);
		}
	if (title3!=NULL)
		{
			title3wid=gdk_pixbuf_get_width((const GdkPixbuf *)title3);
			title3hite=gdk_pixbuf_get_height((const GdkPixbuf *)title3);
		}
	if (title4!=NULL)
		{
			title4wid=gdk_pixbuf_get_width((const GdkPixbuf *)title4);
			title4hite=gdk_pixbuf_get_height((const GdkPixbuf *)title4);
		}
	if (title5!=NULL)
		{
			title5wid=gdk_pixbuf_get_width((const GdkPixbuf *)title5);
			title5hite=gdk_pixbuf_get_height((const GdkPixbuf *)title5);
		}

	topleftwid=gdk_pixbuf_get_width((const GdkPixbuf *)topleft);
	toplefthite=gdk_pixbuf_get_height((const GdkPixbuf *)topleft);
	
	topritewid=gdk_pixbuf_get_width((const GdkPixbuf *)toprite);
	topritehite=gdk_pixbuf_get_height((const GdkPixbuf *)toprite);

	bottomleftwid=gdk_pixbuf_get_width((const GdkPixbuf *)bottomleft);
	bottomlefthite=gdk_pixbuf_get_height((const GdkPixbuf *)bottomleft);
	
	bottomritewid=gdk_pixbuf_get_width((const GdkPixbuf *)bottomrite);
	bottomritehite=gdk_pixbuf_get_height((const GdkPixbuf *)bottomrite);

	leftsidewid=gdk_pixbuf_get_width((const GdkPixbuf *)leftside);
	leftsidehite=gdk_pixbuf_get_height((const GdkPixbuf *)leftside);
	
	ritesidewid=gdk_pixbuf_get_width((const GdkPixbuf *)riteside);
	ritesidehite=gdk_pixbuf_get_height((const GdkPixbuf *)riteside);
	
	bottomwid=gdk_pixbuf_get_width((const GdkPixbuf *)bottom);
	bottomhite=gdk_pixbuf_get_height((const GdkPixbuf *)bottom);

	if(close!=NULL)
		{
			closewid=gdk_pixbuf_get_width((const GdkPixbuf *)close);
			closehite=gdk_pixbuf_get_height((const GdkPixbuf *)close);
		}
	if(max!=NULL)
		{
			maxwid=gdk_pixbuf_get_width((const GdkPixbuf *)max);
			maxhite=gdk_pixbuf_get_height((const GdkPixbuf *)max);
		}
	if(min!=NULL)
		{
			minwid=gdk_pixbuf_get_width((const GdkPixbuf *)min);
			minhite=gdk_pixbuf_get_height((const GdkPixbuf *)min);
		}
	if(menu!=NULL)
		{
			menuwid=gdk_pixbuf_get_width((const GdkPixbuf *)menu);
			menuhite=gdk_pixbuf_get_height((const GdkPixbuf *)menu);
		}

	lsegwid=menuwid+button_spacing+1;
	rsegwid=closewid+maxwid+minwid+(button_spacing*3)+1;
	boxwid=topleftwid+lsegwid+title2wid+PADWIDTH+title4wid+rsegwid+topritewid;
	if (boxwid<MAXBOXWIDTH)
		{
			padwid=MAXBOXWIDTH-(topleftwid+lsegwid+title2wid+title4wid+rsegwid+topritewid);
			boxwid=MAXBOXWIDTH;
		}

	if (boxhite-bottomhite-title3hite<ICONPAD+ICONPAD+ICONSIZE+title3hite+bottomhite)
		boxhite=title3hite+ICONPAD+ICONPAD+ICONSIZE+bottomhite;

	if (boxhite-bottomritehite-topritehite<=0)
		boxhite=bottomritehite+topritehite+10;

	if (boxhite-bottomlefthite-toplefthite<=0)
		boxhite=bottomritehite+topritehite+10;

		
	surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,boxwid,boxhite);
	cr=cairo_create(surface);

//do theme

	if (controlsPixbuf!=NULL)
		{
			cairo_save (cr);
				gdk_cairo_set_source_pixbuf(cr,controlsPixbuf,leftsidewid,title3hite);
				cairo_paint_with_alpha(cr,100);

				arrow=cursorprev("left_ptr",cursorTheme);
				if(arrow!=NULL)
					{
						gdk_cairo_set_source_pixbuf(cr,arrow,boxwid-ritesidewid-ICONSIZE-ICONPAD,title3hite+ICONPAD);
						cairo_paint_with_alpha(cr,100);
						g_object_unref(arrow);
						
						theme=gtk_icon_theme_new();
						gtk_icon_theme_set_custom_theme(theme,iconTheme);
						arrow=gtk_icon_theme_load_icon(theme,"user-home",ICONSIZE,GTK_ICON_LOOKUP_FORCE_SIZE,NULL);
						if (arrow!=NULL)
							{
								gdk_cairo_set_source_pixbuf(cr,arrow,boxwid-ritesidewid-(ICONSIZE*2)-(ICONPAD*2),title3hite+ICONPAD);
								cairo_paint_with_alpha(cr,100);
								g_object_unref(arrow);
							}
						if(theme!=NULL)
							g_object_unref(theme);
					}
			cairo_restore (cr);
		}

//topleft
	cairo_save (cr);
		gdk_cairo_set_source_pixbuf(cr,topleft,0,0);
		cairo_paint_with_alpha(cr,100);
	cairo_restore (cr);

//title1
	if (title1!=NULL)
		{
			cairo_save (cr);
				gdk_cairo_set_source_pixbuf(cr,gdk_pixbuf_scale_simple(title1,lsegwid,title3hite,GDK_INTERP_BILINEAR),topleftwid,0);
				cairo_paint_with_alpha(cr,100);
			cairo_restore (cr);
		}
//title2
	if (title2!=NULL)
		{
			cairo_save (cr);
				gdk_cairo_set_source_pixbuf(cr,title2,topleftwid+lsegwid,0);
				cairo_paint_with_alpha(cr,100);
			cairo_restore (cr);
		}
//title3
	if (title3!=NULL)
		{
			cairo_save (cr);
				gdk_cairo_set_source_pixbuf(cr,gdk_pixbuf_scale_simple(title3,padwid,title3hite,GDK_INTERP_BILINEAR),topleftwid+lsegwid+title2wid,0);
				cairo_paint_with_alpha(cr,100);
			cairo_restore (cr);
		}
//title4
	if (title4!=NULL)
		{
			cairo_save (cr);
				gdk_cairo_set_source_pixbuf(cr,title4,topleftwid+lsegwid+title2wid+padwid,0);
				cairo_paint_with_alpha(cr,100);
			cairo_restore (cr);
		}
//title5
	if (title5!=NULL)
		{
			cairo_save (cr);
				gdk_cairo_set_source_pixbuf(cr,gdk_pixbuf_scale_simple(title5,rsegwid,title3hite,GDK_INTERP_BILINEAR),topleftwid+lsegwid+title2wid+padwid+title4wid,0);
				cairo_paint_with_alpha(cr,100);
			cairo_restore (cr);
		}

//toprite
	cairo_save (cr);
		gdk_cairo_set_source_pixbuf(cr,toprite,boxwid-topritewid,0);
		cairo_paint_with_alpha(cr,100);
	cairo_restore (cr);

//leftside	
	cairo_save (cr);
		gdk_cairo_set_source_pixbuf(cr,gdk_pixbuf_scale_simple(leftside,leftsidewid,boxhite-bottomlefthite-toplefthite,GDK_INTERP_BILINEAR),0,toplefthite);
		cairo_paint_with_alpha(cr,100);
	cairo_restore (cr);

//riteside
	cairo_save (cr);
		gdk_cairo_set_source_pixbuf(cr,gdk_pixbuf_scale_simple(riteside,ritesidewid,boxhite-bottomritehite-topritehite,GDK_INTERP_BILINEAR),boxwid-ritesidewid,topritehite);
		cairo_paint_with_alpha(cr,100);
	cairo_restore (cr);

//bottomleft
	cairo_save (cr);
		gdk_cairo_set_source_pixbuf(cr,bottomleft,0,boxhite-bottomlefthite);
		cairo_paint_with_alpha(cr,100);
	cairo_restore (cr);

//bottomrite
	cairo_save (cr);
		gdk_cairo_set_source_pixbuf(cr,bottomrite,boxwid-bottomritewid,boxhite-bottomritehite);
		cairo_paint_with_alpha(cr,100);
	cairo_restore (cr);

//bottom
	cairo_save (cr);
		gdk_cairo_set_source_pixbuf(cr,gdk_pixbuf_scale_simple(bottom,boxwid-bottomritewid-bottomleftwid,bottomhite,GDK_INTERP_BILINEAR),bottomleftwid,boxhite-bottomhite);
		cairo_paint_with_alpha(cr,100);
	cairo_restore (cr);

//buttons
//menu
hiteoffset=0;
	cairo_save (cr);
		if(menu!=NULL)
			{
				hiteoffset=(title3hite-menuhite)/2;
				gdk_cairo_set_source_pixbuf(cr,menu,button_offset+leftsidewid,hiteoffset);
				cairo_paint_with_alpha(cr,100);
			}
//close
		if(close!=NULL)
			{
				hiteoffset=(title3hite-closehite)/2;
				gdk_cairo_set_source_pixbuf(cr,close,boxwid-button_offset-ritesidewid-closewid,hiteoffset);
				cairo_paint_with_alpha(cr,100);
			}
//max
		if(max!=NULL)
			{
				gdk_cairo_set_source_pixbuf(cr,max,boxwid-button_offset-ritesidewid-closewid-maxwid-button_spacing,hiteoffset);
				cairo_paint_with_alpha(cr,100);
			}
//min
		if(min!=NULL)
			{
				gdk_cairo_set_source_pixbuf(cr,min,boxwid-button_offset-ritesidewid-closewid-maxwid-minwid-button_spacing-button_spacing,hiteoffset);
				cairo_paint_with_alpha(cr,100);
			}
	cairo_restore (cr);

	cairo_surface_write_to_png(surface,outframe);

	g_object_unref(topleft);
	g_object_unref(toprite);
if (title1!=NULL)
	g_object_unref(title1);
if (title2!=NULL)
	g_object_unref(title2);
if (title3!=NULL)
	g_object_unref(title3);
if (title4!=NULL)
	g_object_unref(title4);
if (title5!=NULL)
	g_object_unref(title5);
	g_object_unref(riteside);
	g_object_unref(leftside);
	g_object_unref(bottomleft);
	g_object_unref(bottomrite);
	g_object_unref(bottom);
if (close!=NULL)
	g_object_unref(close);
if (max!=NULL)
	g_object_unref(max);
if (min!=NULL)
	g_object_unref(min);
if (menu!=NULL)
	g_object_unref(menu);

	cairo_surface_destroy(surface);
	cairo_destroy(cr);
}

void getspace(char* folder)
{
	char	buffer[64];
	char*	filename;
	FILE*	fp=NULL;
	char*	offsetstr;
	char*	spacestr;

	asprintf(&filename,"%s/xfwm4/themerc",folder);
	fp=fopen(filename,"r");

	button_offset=0;
	button_spacing=0;

	if (fp==NULL)
		{
			printf("No themerc file... %s\n",filename);
			return;
		}

	while (fgets(buffer,64,fp)!=NULL)
		{
			offsetstr=NULL;
			spacestr=NULL;
			offsetstr=strstr((char*)&buffer,"button_offset=");
			spacestr=strstr((char*)&buffer,"button_spacing=");
			
			if (offsetstr!=NULL)
					button_offset=atoi((char*)&buffer[14]);

			if (spacestr!=NULL)
					button_spacing=atoi((char*)&buffer[15]);
		}

	freeAndNull(&filename);

	if (fp!=NULL)
		fclose(fp);
}

void makeIcon(char* themename,char* outPath)
{
	GdkPixbuf*	home;
	GdkPixbuf*	folder;
	GdkPixbuf*	computer;
	GdkPixbuf*	drive;
	cairo_surface_t	*surface;
	cairo_t		*cr;
	GtkIconTheme*	theme=NULL;

	controlsPixbuf=NULL;
	surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,128,32);
	cr=cairo_create(surface);

	theme=gtk_icon_theme_new();
	gtk_icon_theme_set_custom_theme(theme,themename);

	home=gtk_icon_theme_load_icon(theme,"user-home",ICONSIZE,GTK_ICON_LOOKUP_FORCE_SIZE,NULL);
	folder=gtk_icon_theme_load_icon(theme,"folder",ICONSIZE,GTK_ICON_LOOKUP_FORCE_SIZE,NULL);
	computer=gtk_icon_theme_load_icon(theme,"user-desktop",ICONSIZE,GTK_ICON_LOOKUP_FORCE_SIZE,NULL);
	drive=gtk_icon_theme_load_icon(theme,"drive-harddisk",ICONSIZE,GTK_ICON_LOOKUP_FORCE_SIZE,NULL);

	if(home!=NULL)
		{
			gdk_cairo_set_source_pixbuf(cr,home,0,0);
			cairo_paint_with_alpha(cr,100);
			g_object_unref(home);
		}
	if(folder!=NULL)
		{
			gdk_cairo_set_source_pixbuf(cr,folder,32,0);
			cairo_paint_with_alpha(cr,100);
			g_object_unref(folder);
		}
	if(computer!=NULL)
		{
			gdk_cairo_set_source_pixbuf(cr,computer,64,0);
			cairo_paint_with_alpha(cr,100);
			g_object_unref(computer);
		}
	if(drive!=NULL)
		{
			gdk_cairo_set_source_pixbuf(cr,drive,96,0);
			cairo_paint_with_alpha(cr,100);
			g_object_unref(drive);
		}

	cairo_surface_write_to_png(surface,outPath);

	cairo_surface_destroy(surface);
	cairo_destroy(cr);
}

