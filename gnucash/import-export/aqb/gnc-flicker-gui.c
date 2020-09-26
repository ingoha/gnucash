/// gcc  -Wall -Wextra -o flicker_draw flicker_draw.c $(pkg-config gtk+-3.0 --cflags --libs) -rdynamic
/// siehe Javascript- Implementierug von: https://6xq.net/flickercodes/

#include <cairo.h>
#include <gtk/gtk.h>

#include "gnc-flicker-gui.h"

/* structured data for the flicker variables */
struct _flickerdraw {
	const char *challenge;
	guint maxWert;
    guint margin;           /* Abstand zwischen Balken */
    guint barwidth;         /* Balkenbreite */
    guint barheight;        /* Balkenhöhe */
    guint x_barpos;			/* x-Wert für die Position des Balken */
    guint y_barpos;			/* y-Wert für die Position des Balken */
    guint delay;            /* Wartezeit zwischen Frames in Millisekunden */
    guint height;			/* Höhe der Zeichenfläche */
    guint width;			/* Breite der Zeichenfläche */
    guint halfbyteid;
    guint clock;
    guint x_drawpos;		/* Erste Zeichenposition */
    guint y_drawpos;		/* Erste Zeichenposition */
    guint interval;
    gboolean change_interval;
} flickerdraw;

/* structured data for the GUI */
struct _GncFlickerGui
{
	GtkWidget *dialog;
	GtkWidget *flicker_challenge;
	GtkWidget *flicker_hbox;
	GtkAdjustment *adj_Distance;
	GtkAdjustment *adj_Barwidth;
	GtkAdjustment *adj_Delay;
	GtkSpinButton *spin_Distance;
	GtkSpinButton *spin_Barwidth;
	GtkSpinButton *spin_Delay;
};

GncFlickerGui *flickergui = NULL;
_Bool bitarray[100][5];

/* *************************************
*
* the functions are processed here
*
****************************************/

/* convert the bank challenge into the 5 bits for the flicker data */
char
*FlickerDaten (char const *challenge)
{

    /* bitfield: clock, bits 2^0, 2^1, 2^2, 2^3 */
    _Bool bits[16][5] = { {FALSE, FALSE, FALSE, FALSE, FALSE},  /* '0' */
                      {FALSE, TRUE, FALSE, FALSE, FALSE},       /* '1' */
                      {FALSE, FALSE, TRUE, FALSE, FALSE},       /* '2' */
                      {FALSE, TRUE, TRUE, FALSE, FALSE},        /* '3' */
                      {FALSE, FALSE, FALSE, TRUE, FALSE},       /* '4' */
                      {FALSE, TRUE, FALSE, TRUE, FALSE},        /* '5' */
                      {FALSE, FALSE, TRUE, TRUE, FALSE},        /* '6' */
                      {FALSE, TRUE, TRUE, TRUE, FALSE},         /* '7' */
                      {FALSE, FALSE, FALSE, FALSE, TRUE},       /* '8' */
                      {FALSE, TRUE, FALSE, FALSE, TRUE},        /* '9' */
                      {FALSE, FALSE, TRUE, FALSE, TRUE},        /* 'A' */
                      {FALSE, TRUE, TRUE, FALSE, TRUE},         /* 'B' */
                      {FALSE, FALSE, FALSE, TRUE, TRUE},        /* 'C' */
                      {FALSE, TRUE, FALSE, TRUE, TRUE},         /* 'D' */
                      {FALSE, FALSE, TRUE, TRUE, TRUE},         /* 'E' */
                      {FALSE, TRUE, TRUE, TRUE, TRUE}           /* 'F' */
    };

    /* prepend synchronization identifier */
    char *code = g_malloc (strlen (challenge) + 5);

    int i; int j;
    char *arr1 = g_malloc (2);
    char *arr2 = g_malloc (2);

    strcpy (code, "0FFF");
    strcat (code, challenge);
    code[strlen (code)+1] = '\0';

    for (i = 0; i < strlen (code); i += 2) {
        sprintf (arr1, "%c", code[i+1]);
		sprintf (arr2, "%c", code[i]);
        for (j = 0; j < 16; j++) {
            if (strtol(arr1, NULL, 16) == j) {
                bitarray[i][0] = bits[j][0];
                bitarray[i][1] = bits[j][1];
                bitarray[i][2] = bits[j][2];
                bitarray[i][3] = bits[j][3];
                bitarray[i][4] = bits[j][4];
			}
			if (strtol(arr2, NULL, 16) == j) {
                bitarray[i+1][0] = bits[j][0];
                bitarray[i+1][1] = bits[j][1];
                bitarray[i+1][2] = bits[j][2];
                bitarray[i+1][3] = bits[j][3];
                bitarray[i+1][4] = bits[j][4];
			}
		}
    }
    g_free (arr1);
    g_free (arr2);

    return code;
}

/* display the flicker graphic in the drawing area */
 void
 doFlickerDrawing (cairo_t *cr)
 {

	guint i;

    /* Initialize the drawing area to black */
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    bitarray[flickerdraw.halfbyteid][0] = flickerdraw.clock;

    for ( i = 0; i <= 4; i++ ) {
    	if ( bitarray[flickerdraw.halfbyteid][i] == TRUE ) {
    		cairo_set_source_rgb(cr, 1, 1, 1); // white
    	}
    	else {
    		cairo_set_source_rgb(cr, 0, 0, 0); // black
    	}
    	flickerdraw.x_barpos = (i+1)*flickerdraw.margin + i*flickerdraw.barwidth;
    	cairo_rectangle (cr, flickerdraw.x_barpos, flickerdraw.y_barpos, flickerdraw.barwidth, flickerdraw.barheight);
    	cairo_fill (cr);
    }

/* jeder Flickerpunkt wird zweimal gezeichnet,
 * einmal mit Takt = 1 und einmal mit Takt = 0
 */
    if ( flickerdraw.clock == 0 ) {
        flickerdraw.clock = 1;
        flickerdraw.halfbyteid++;
        if ( flickerdraw.halfbyteid >= flickerdraw.maxWert ) {
            flickerdraw.halfbyteid = 0;
        }
    }
    else if ( flickerdraw.clock == 1 ) {
        flickerdraw.clock = 0;
    }

}

/* A clock is started here and called up again when the "Delay" value is changed */
gboolean
time_handler (GtkWidget *widget)
{

    /* Änderung der Wartezeit */
    if (flickerdraw.change_interval)
    {
        g_source_remove (flickerdraw.interval);
        flickerdraw.interval = g_timeout_add (flickerdraw.delay, (GSourceFunc) time_handler, (gpointer) widget);
        flickerdraw.change_interval = FALSE;
        return FALSE;
    }
    gtk_widget_queue_draw (widget);

    return TRUE;
}

/* *************************************
*
* from here the signals of the GUI are processed
* sequence: ini_flicker_gui / on_flicker_map / time_handler / on_flicker_draw / time_handler / on_flicker_draw etc.
*
****************************************/

/* called when drawing area is shown */
void
on_flicker_challenge_map (GtkWidget *widget)
{

    gchar *code = g_malloc (strlen (flickerdraw.challenge)+4);
    code = FlickerDaten (flickerdraw.challenge);
    flickerdraw.maxWert = strlen(code);

    /* Zeitfunktion aufrufen und Flickeranzeige starten */
    flickerdraw.interval = g_timeout_add (flickerdraw.delay, (GSourceFunc) time_handler, (gpointer) widget);

}

/* Redraw the screen from the surface. */
gboolean
on_flicker_challenge_draw (__attribute__((unused)) GtkWidget *widget, cairo_t *cr, __attribute__((unused)) gpointer user_data)
{

    doFlickerDrawing (cr);

    return FALSE;
}

/* called when drawing area is destroyed */
void
on_flicker_challenge_destroy (__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer user_data)
{

    g_source_remove (flickerdraw.interval);

}

/* The value for "Distance" has been changed on the spin button and the flicker display is updated */
void
on_spin_Distance_value_changed (GtkSpinButton *spin, __attribute__((unused)) gpointer user_data)
{

    flickerdraw.margin = gtk_spin_button_get_value_as_int (spin);

}

/* The value for "Field width" has been changed on the spin button and the flicker display is updated */
void
on_spin_Barwidth_value_changed (GtkSpinButton *spin, __attribute__((unused)) gpointer user_data)
{

	flickerdraw.barwidth = gtk_spin_button_get_value_as_int (spin);

}

/* The value for "waiting time" was changed on the spin button and the flicker display is updated */
void
on_spin_Delay_value_changed (GtkSpinButton *spin, GtkWidget *widget)
{

    flickerdraw.delay = gtk_spin_button_get_value_as_int (spin);

    flickerdraw.change_interval = TRUE;
    time_handler (widget);

}

/* The widgets for the GUI are prepared and the first parameters are set  */
void
ini_flicker_gui (const char *pChallenge, GncFlickerGui *gui)
{

    /* Globale Variablen mit Werten vorbelegen */
    flickerdraw.challenge = pChallenge;
    flickerdraw.x_barpos = 20;		/* Erste Zeichenposition */
    flickerdraw.y_barpos = 20;		/* Erste Zeichenposition */

    /* Anwendung initialisieren und Felder mit Werten vorbelegen */
    flickerdraw.height = 200;   /* Fix: Höhe der Zeichenfläche */
    flickerdraw.width = 250;    /* Fix: Breite der Zeichenfläche */
    flickerdraw.barheight = flickerdraw.height; /* Höhe der Balken */
    flickerdraw.barwidth = 44;  /* Eingabe per GUI: Breite der einzelnen Balken */
    flickerdraw.margin = 28;    /* Eingabe per GUI: horizontaler Abstand zwischen den Balken */
    flickerdraw.delay = 50;      /* Eingabe per GUI: Verzögerungszeit zwischen dem Blinken */
	flickerdraw.halfbyteid = 0;
	flickerdraw.clock = 1;

    /* Bezug auf die im Hauptprogramm erstellten Dialog-Widgets */
    flickergui = gui;

    gtk_window_set_resizable (GTK_WINDOW(flickergui->dialog), FALSE);
    gtk_widget_set_visible (GTK_WIDGET(flickergui->flicker_challenge), TRUE);
    gtk_widget_set_size_request (flickergui->flicker_challenge, flickerdraw.height, flickerdraw.width );

    g_signal_connect (GTK_WIDGET (flickergui->flicker_challenge), "map", G_CALLBACK (on_flicker_challenge_map), NULL);
    g_signal_connect (GTK_WIDGET (flickergui->flicker_challenge), "draw", G_CALLBACK (on_flicker_challenge_draw), NULL);
    g_signal_connect (GTK_WIDGET (flickergui->flicker_challenge), "destroy", G_CALLBACK (on_flicker_challenge_destroy), NULL);

    gtk_widget_set_visible (GTK_WIDGET (flickergui->flicker_hbox), TRUE);

    flickergui->adj_Distance = gtk_adjustment_new (0.0, 10.0, 50.0, 1.0, 10.0, 0.0);
    gtk_spin_button_set_adjustment (flickergui->spin_Distance, flickergui->adj_Distance);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (flickergui->spin_Distance), flickerdraw.margin);
    g_signal_connect (GTK_WIDGET (flickergui->spin_Distance), "value-changed", G_CALLBACK (on_spin_Distance_value_changed), NULL);
    gtk_widget_set_visible (GTK_WIDGET (flickergui->spin_Distance), TRUE);

    flickergui->adj_Barwidth = gtk_adjustment_new (0.0, 10.0, 80.0, 1.0, 10.0, 0.0);
    gtk_spin_button_set_adjustment (flickergui->spin_Barwidth, flickergui->adj_Barwidth);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (flickergui->spin_Barwidth), flickerdraw.barwidth);
    g_signal_connect (GTK_WIDGET (flickergui->spin_Barwidth), "value-changed", G_CALLBACK (on_spin_Barwidth_value_changed), NULL);
    gtk_widget_set_visible (GTK_WIDGET (flickergui->spin_Barwidth), TRUE);

    flickergui->adj_Delay = gtk_adjustment_new (0.0, 10.0, 1000.0, 10.0, 10.0, 0.0);
    gtk_spin_button_set_adjustment (flickergui->spin_Delay, flickergui->adj_Delay);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (flickergui->spin_Delay), flickerdraw.delay);
    g_signal_connect (GTK_WIDGET (flickergui->spin_Delay), "value-changed", G_CALLBACK (on_spin_Delay_value_changed), flickergui->flicker_challenge);
    gtk_widget_set_visible (GTK_WIDGET (flickergui->spin_Delay), TRUE);

}
