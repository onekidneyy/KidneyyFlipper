#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include "wifi_manager.h"
#include "Solana_app_icons.h"

#define TAG "SolanaWalletApp"

// Our application menu has 3 items. You can add more items if you want.
typedef enum {
    SolanaSubmenuIndexConfig,
    SolanaSubmenuIndexPlay,
    SolanaSubmenuIndexAbout,
} SolanaSubmenuIndex;

// Each view is a screen we show the user.
typedef enum {
    SolanaViewSubmenu, // The menu when the app starts
    SolanaViewComingSoon, // Coming soon screen
} SolanaView;

typedef struct {
    ViewDispatcher* view_dispatcher; // Switches between our views
    NotificationApp* notifications; // Used for controlling the backlight
    Submenu* submenu; // The application menu
    Widget* widget_about; // The about screen
} SolanaApp;

/**
 * @brief Callback for exiting the application.
 * @details This function is called when user press back button. We return VIEW_NONE to
 * indicate that we want to exit the application.
 * @param _context The context - unused
 * @return next view id
 */
static uint32_t solana_navigation_exit_callback(void* _context) {
    UNUSED(_context);
    return VIEW_NONE;
}

/**
 * @brief Callback for returning to submenu.
 * @details This function is called when user press back button. We return VIEW_NONE to
 * indicate that we want to navigate to the submenu.
 * @param _context The context - unused
 * @return next view id
 */
static uint32_t solana_navigation_submenu_callback(void* _context) {
    UNUSED(_context);
    return SolanaViewSubmenu;
}

/**
 * @brief Handle submenu item selection.
 * @details This function is called when user selects an item from the submenu.
 * @param context The context - SolanaApp object.
 * @param index The SolanaSubmenuIndex item that was clicked.
 */
static void solana_submenu_callback(void* context, uint32_t index) {
    SolanaApp* app = (SolanaApp*)context;

    switch(index) {
    case SolanaSubmenuIndexConfig:
        // Initialize and connect to Wi-Fi
        wifi_init_sta();
        notification_message(app->notifications, &sequence_success);
        break;
    case SolanaSubmenuIndexPlay:
    case SolanaSubmenuIndexAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, SolanaViewComingSoon);
        break;
    default:
        break;
    }
}

/**
 * @brief Callback for drawing the coming soon screen.
 * @details This function is called when the screen needs to be redrawn.
 * @param canvas The canvas to draw on.
 * @param model The model - unused.
 */
static void solana_view_coming_soon_draw_callback(Canvas* canvas, void* model) {
    UNUSED(model);
    canvas_draw_str(canvas, 10, 10, "Coming Soon");
}

/**
 * @brief Allocate the solana application.
 * @details This function allocates the solana application resources.
 * @return SolanaApp object.
 */
static SolanaApp* solana_app_alloc() {
    SolanaApp* app = (SolanaApp*)malloc(sizeof(SolanaApp));

    Gui* gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    app->submenu = submenu_alloc();
    submenu_add_item(
        app->submenu, "Config", SolanaSubmenuIndexConfig, solana_submenu_callback, app);
    submenu_add_item(app->submenu, "Play", SolanaSubmenuIndexPlay, solana_submenu_callback, app);
    submenu_add_item(app->submenu, "About", SolanaSubmenuIndexAbout, solana_submenu_callback, app);
    view_set_previous_callback(submenu_get_view(app->submenu), solana_navigation_exit_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, SolanaViewSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_switch_to_view(app->view_dispatcher, SolanaViewSubmenu);

    app->widget_about = widget_alloc();
    widget_add_text_scroll_element(app->widget_about, 0, 0, 128, 64, "Coming Soon");
    view_set_previous_callback(
        widget_get_view(app->widget_about), solana_navigation_submenu_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, SolanaViewComingSoon, widget_get_view(app->widget_about));

    // Set the drawing callback for the coming soon view
    view_set_draw_callback(
        widget_get_view(app->widget_about), solana_view_coming_soon_draw_callback);

    app->notifications = furi_record_open(RECORD_NOTIFICATION);

#ifdef BACKLIGHT_ON
    notification_message(app->notifications, &sequence_display_backlight_enforce_on);
#endif

    return app;
}

/**
 * @brief Free the solana application.
 * @details This function frees the solana application resources.
 * @param app The solana application object.
 */
static void solana_app_free(SolanaApp* app) {
#ifdef BACKLIGHT_ON
    notification_message(app->notifications, &sequence_display_backlight_enforce_auto);
#endif
    furi_record_close(RECORD_NOTIFICATION);

    view_dispatcher_remove_view(app->view_dispatcher, SolanaViewComingSoon);
    widget_free(app->widget_about);
    view_dispatcher_remove_view(app->view_dispatcher, SolanaViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);

    free(app);
}

/**
 * @brief Main function for solana application.
 * @details This function is the entry point for the solana application. It should be defined in
 * application.fam as the entry_point setting.
 * @param _p Input parameter - unused
 * @return 0 - Success
 */
int32_t main_solana_app(void* _p) {
    UNUSED(_p);

    SolanaApp* app = solana_app_alloc();
    view_dispatcher_run(app->view_dispatcher);

    solana_app_free(app);
    return 0;
}
