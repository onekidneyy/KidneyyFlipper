#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include "Solana_app_icons.h"

#define TAG "Skeleton"

// Change this to BACKLIGHT_AUTO if you don't want the backlight to be continuously on.
#define BACKLIGHT_ON 1

// Our application menu has 3 items.  You can add more items if you want.
typedef enum {
    SkeletonSubmenuIndexConfigure,
    SkeletonSubmenuIndexGame,
    SkeletonSubmenuIndexAbout,
} SkeletonSubmenuIndex;

// Each view is a screen we show the user.
typedef enum {
    SkeletonViewSubmenu, // The menu when the app starts
    SkeletonViewComingSoon, // Coming soon screen
} SkeletonView;

typedef struct {
    ViewDispatcher* view_dispatcher; // Switches between our views
    NotificationApp* notifications; // Used for controlling the backlight
    Submenu* submenu; // The application menu
    Widget* widget_about; // The about screen
} SkeletonApp;

/**
 * @brief      Callback for exiting the application.
 * @details    This function is called when user press back button.  We return VIEW_NONE to
 *            indicate that we want to exit the application.
 * @param      _context  The context - unused
 * @return     next view id
*/
static uint32_t skeleton_navigation_exit_callback(void* _context) {
    UNUSED(_context);
    return VIEW_NONE;
}

/**
 * @brief      Callback for returning to submenu.
 * @details    This function is called when user press back button.  We return VIEW_NONE to
 *            indicate that we want to navigate to the submenu.
 * @param      _context  The context - unused
 * @return     next view id
*/
static uint32_t skeleton_navigation_submenu_callback(void* _context) {
    UNUSED(_context);
    return SkeletonViewSubmenu;
}

/**
 * @brief      Handle submenu item selection.
 * @details    This function is called when user selects an item from the submenu.
 * @param      context  The context - SkeletonApp object.
 * @param      index     The SkeletonSubmenuIndex item that was clicked.
*/
static void skeleton_submenu_callback(void* context, uint32_t index) {
    UNUSED(index);
    SkeletonApp* app = (SkeletonApp*)context;
    view_dispatcher_switch_to_view(app->view_dispatcher, SkeletonViewComingSoon);
}

/**
 * @brief      Callback for drawing the coming soon screen.
 * @details    This function is called when the screen needs to be redrawn.
 * @param      canvas  The canvas to draw on.
 * @param      model   The model - unused.
*/
static void skeleton_view_coming_soon_draw_callback(Canvas* canvas, void* model) {
    UNUSED(model);
    canvas_draw_str(canvas, 10, 10, "Coming Soon");
}

/**
 * @brief      Allocate the skeleton application.
 * @details    This function allocates the skeleton application resources.
 * @return     SkeletonApp object.
*/
static SkeletonApp* solana_app_alloc() {
    SkeletonApp* app = (SkeletonApp*)malloc(sizeof(SkeletonApp));

    Gui* gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    app->submenu = submenu_alloc();
    submenu_add_item(
        app->submenu, "Config", SkeletonSubmenuIndexConfigure, skeleton_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Play", SkeletonSubmenuIndexGame, skeleton_submenu_callback, app);
    submenu_add_item(
        app->submenu, "About", SkeletonSubmenuIndexAbout, skeleton_submenu_callback, app);
    view_set_previous_callback(submenu_get_view(app->submenu), skeleton_navigation_exit_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, SkeletonViewSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_switch_to_view(app->view_dispatcher, SkeletonViewSubmenu);

    app->widget_about = widget_alloc();
    widget_add_text_scroll_element(app->widget_about, 0, 0, 128, 64, "Coming Soon");
    view_set_previous_callback(
        widget_get_view(app->widget_about), skeleton_navigation_submenu_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, SkeletonViewComingSoon, widget_get_view(app->widget_about));

    // Set the drawing callback for the coming soon view
    view_set_draw_callback(
        widget_get_view(app->widget_about), skeleton_view_coming_soon_draw_callback);

    app->notifications = furi_record_open(RECORD_NOTIFICATION);

#ifdef BACKLIGHT_ON
    notification_message(app->notifications, &sequence_display_backlight_enforce_on);
#endif

    return app;
}

/**
 * @brief      Free the skeleton application.
 * @details    This function frees the skeleton application resources.
 * @param      app  The skeleton application object.
*/
static void solana_app_free(SkeletonApp* app) {
#ifdef BACKLIGHT_ON
    notification_message(app->notifications, &sequence_display_backlight_enforce_auto);
#endif
    furi_record_close(RECORD_NOTIFICATION);

    view_dispatcher_remove_view(app->view_dispatcher, SkeletonViewComingSoon);
    widget_free(app->widget_about);
    view_dispatcher_remove_view(app->view_dispatcher, SkeletonViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);

    free(app);
}

/**
 * @brief      Main function for skeleton application.
 * @details    This function is the entry point for the skeleton application.  It should be defined in
 *           application.fam as the entry_point setting.
 * @param      _p  Input parameter - unused
 * @return     0 - Success
*/
int32_t main_solana_app(void* _p) {
    UNUSED(_p);

    SkeletonApp* app = solana_app_alloc();
    view_dispatcher_run(app->view_dispatcher);

    solana_app_free(app);
    return 0;
}
