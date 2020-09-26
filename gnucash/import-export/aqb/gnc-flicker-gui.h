/*
 * gnc-flicker-gui.h
 *
 *
 *
 *
 */

/**
 * @addtogroup Import_Export
 * @{
 * @addtogroup AqBanking
 * @{
 * @file gnc-flicker-gui.h
 * @brief GUI callbacks for Flicker and ChipTAN(optisch)
 * @author Copyright (C) 2020 Christian Wehling <mail>
 */

#ifndef GNC_FLICKER_GUI_H
#define GNC_FLICKER_GUI_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

// structured data for the GUI
typedef struct _GncFlickerGui GncFlickerGui;

char *FlickerDaten (char const *challenge);

void doFlickerDrawing (cairo_t *cr);

gboolean time_handler (GtkWidget *widget); //GncFlickerGui *flickergui

/**
 * called when the drawing area at the dialog is shown
 *
 * @param widget: The drawing area for the challenge
 */
void on_flicker_challenge_map (GtkWidget *widget);

/**
 * Initialize the drawingarea to black and paint the flickerchallenge
 * Call the function flickerStep for the flickering
 *
 * @param widget: The Widget which send the Callback
 * @param cr: Pointer to the Cairo
 * @param data: user_data
 * @return gboolean
 */
gboolean on_flicker_challenge_draw (GtkWidget *widget, cairo_t *cr, gpointer user_data);

/**
 * called when the drawing area is destroyed
 *
 * @param widget: The drawing area
 * @param user_data: The User data
 */
void on_flicker_challenge_destroy (GtkWidget *widget, gpointer user_data);



/**
 * The Adjustvalue for the Spinbutton "Distance"
 *
 * @param spin: The Spinbutton to chance the space betwween the bar
 * @param user_data: The user_data for the callback
 */
void on_spin_Distance_value_changed (GtkSpinButton *spin, gpointer user_data);

/**
 * The Adjustvalue for the Spinbutton "Barwidth"
 *
 * @param spin: The Spinbutton for the bar width
 * @param user_data: The user_data for the callback
 */
void on_spin_Barwidth_value_changed (GtkSpinButton *spin, gpointer user_data);

/**
 * The Adjustvalue for the Spinbutton "Delay"
 *
 * @param spin: The adjustment for the delay
 * @param widget: The widget to redraw the flicker_challenge
 */
void on_spin_Delay_value_changed (GtkSpinButton *spin, GtkWidget *widget);

/**
 * Initialize the dialog and drawing area
 *
 * @param pChallenge: The answer from the bank which is shown as a flickering picture
 * @param gui: The structure of the Dialog-Widgets
 */
void ini_flicker_gui (const char *pChallenge, GncFlickerGui *gui);

G_END_DECLS

#endif /* GNC_FLICKER_GUI_H */
