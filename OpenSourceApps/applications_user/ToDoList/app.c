#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <notification/notification_messages.h>
#include <gui/modules/text_input.h>

#define TAG         "ToDoList"
#define MAX_TASKS   10
#define TASK_LENGTH 64

// Change this to BACKLIGHT_AUTO if you don't want the backlight to be continuously on.
#define BACKLIGHT_ON 1

typedef enum {
    TodoSubmenuIndexAddTask,
    TodoSubmenuIndexViewTasks,
    TodoSubmenuIndexAbout,
} TodoSubmenuIndex;

typedef enum {
    TodoViewSubmenu, // The menu when the app starts
    TodoViewAddTask, // View for adding a task
    TodoViewViewTasks, // View for viewing tasks
    TodoViewAbout, // View for the about page
    TodoViewConfirm, // View for confirmation message
} TodoView;

typedef struct {
    char task[TASK_LENGTH];
} TaskInputModel;

typedef struct {
    ViewDispatcher* view_dispatcher; // Switches between our views
    Submenu* submenu; // The application menu
    Widget* widget_about; // The about screen
    Widget* widget_confirm; // The confirmation screen
    TextInput* text_input; // Text input for adding a task
    View* list_view; // Custom list view for displaying tasks
    TaskInputModel task_input_model; // Task input model
    char tasks[MAX_TASKS][TASK_LENGTH]; // Array to store tasks
    size_t task_count; // Number of tasks stored
} TodoApp;

static uint32_t todo_navigation_exit_callback(void* _context) {
    UNUSED(_context);
    return VIEW_NONE;
}

static uint32_t todo_navigation_submenu_callback(void* _context) {
    UNUSED(_context);
    return TodoViewSubmenu;
}

static void todo_submenu_callback(void* context, uint32_t index) {
    TodoApp* app = (TodoApp*)context;
    FURI_LOG_I(TAG, "Switching to submenu index: %lu", (unsigned long)index);
    switch(index) {
    case TodoSubmenuIndexAddTask:
        view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewAddTask);
        break;
    case TodoSubmenuIndexViewTasks:
        view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewViewTasks);
        break;
    case TodoSubmenuIndexAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewAbout);
        break;
    default:
        break;
    }
}

static void todo_view_add_task_result_callback(void* context) {
    TodoApp* app = (TodoApp*)context;

    text_input_get_text(app->text_input, app->task_input_model.task, TASK_LENGTH);

    if(app->task_count < MAX_TASKS) {
        strncpy(app->tasks[app->task_count], app->task_input_model.task, TASK_LENGTH);
        app->tasks[app->task_count][TASK_LENGTH - 1] = '\0'; // Ensure null termination
        app->task_count++;
        view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewConfirm);
    } else {
        FURI_LOG_W(TAG, "Task list full");
        view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewSubmenu);
    }
}

static void todo_view_confirm_draw_callback(Canvas* canvas, void* model) {
    UNUSED(model);
    FURI_LOG_I(TAG, "Drawing confirmation screen");
    canvas_draw_str(canvas, 10, 10, "Task Entered!");
}

static void todo_view_view_tasks_draw_callback(Canvas* canvas, void* model) {
    TodoApp* app = (TodoApp*)model;
    if(app) {
        FURI_LOG_I(TAG, "Drawing View Tasks screen. Task count: %zu", app->task_count);
        canvas_draw_str(canvas, 10, 10, "Tasks:");
        for(size_t i = 0; i < app->task_count; i++) {
            canvas_draw_str(canvas, 10, 20 + (i * 10), app->tasks[i]);
        }
    } else {
        FURI_LOG_E(TAG, "app is NULL in view tasks draw callback");
    }
}

static TodoApp* todo_app_alloc() {
    TodoApp* app = (TodoApp*)malloc(sizeof(TodoApp));
    if(!app) {
        FURI_LOG_E(TAG, "Failed to allocate memory for TodoApp");
        return NULL;
    }

    Gui* gui = furi_record_open(RECORD_GUI);
    if(!gui) {
        FURI_LOG_E(TAG, "Failed to open GUI record");
        free(app);
        return NULL;
    }

    app->view_dispatcher = view_dispatcher_alloc();
    if(!app->view_dispatcher) {
        FURI_LOG_E(TAG, "Failed to allocate view dispatcher");
        furi_record_close(RECORD_GUI);
        free(app);
        return NULL;
    }

    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    app->submenu = submenu_alloc();
    if(!app->submenu) {
        FURI_LOG_E(TAG, "Failed to allocate submenu");
        view_dispatcher_free(app->view_dispatcher);
        furi_record_close(RECORD_GUI);
        free(app);
        return NULL;
    }

    submenu_add_item(
        app->submenu, "Add Task", TodoSubmenuIndexAddTask, todo_submenu_callback, app);
    submenu_add_item(
        app->submenu, "View Tasks", TodoSubmenuIndexViewTasks, todo_submenu_callback, app);
    submenu_add_item(app->submenu, "About", TodoSubmenuIndexAbout, todo_submenu_callback, app);
    view_set_previous_callback(submenu_get_view(app->submenu), todo_navigation_exit_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, TodoViewSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewSubmenu);

    app->widget_about = widget_alloc();
    if(!app->widget_about) {
        FURI_LOG_E(TAG, "Failed to allocate widget for about screen");
        submenu_free(app->submenu);
        view_dispatcher_free(app->view_dispatcher);
        furi_record_close(RECORD_GUI);
        free(app);
        return NULL;
    }

    widget_add_text_scroll_element(
        app->widget_about, 0, 0, 128, 64, "This is a simple ToDo list app.");
    view_set_previous_callback(
        widget_get_view(app->widget_about), todo_navigation_submenu_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, TodoViewAbout, widget_get_view(app->widget_about));

    app->widget_confirm = widget_alloc();
    if(!app->widget_confirm) {
        FURI_LOG_E(TAG, "Failed to allocate widget for confirmation screen");
        widget_free(app->widget_about);
        submenu_free(app->submenu);
        view_dispatcher_free(app->view_dispatcher);
        furi_record_close(RECORD_GUI);
        free(app);
        return NULL;
    }

    widget_add_text_scroll_element(app->widget_confirm, 0, 0, 128, 64, "Task Entered!");
    view_set_draw_callback(widget_get_view(app->widget_confirm), todo_view_confirm_draw_callback);
    view_set_previous_callback(
        widget_get_view(app->widget_confirm), todo_navigation_submenu_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, TodoViewConfirm, widget_get_view(app->widget_confirm));

    app->text_input = text_input_alloc();
    if(!app->text_input) {
        FURI_LOG_E(TAG, "Failed to allocate text input");
        widget_free(app->widget_about);
        widget_free(app->widget_confirm);
        submenu_free(app->submenu);
        view_dispatcher_free(app->view_dispatcher);
        furi_record_close(RECORD_GUI);
        free(app);
        return NULL;
    }
    text_input_set_header_text(app->text_input, "Enter Task");
    text_input_set_result_callback(app->text_input, todo_view_add_task_result_callback, app);
    view_dispatcher_add_view(
        app->view_dispatcher, TodoViewAddTask, text_input_get_view(app->text_input));

    app->list_view = view_alloc();
    if(!app->list_view) {
        FURI_LOG_E(TAG, "Failed to allocate list view");
        text_input_free(app->text_input);
        widget_free(app->widget_about);
        widget_free(app->widget_confirm);
        submenu_free(app->submenu);
        view_dispatcher_free(app->view_dispatcher);
        furi_record_close(RECORD_GUI);
        free(app);
        return NULL;
    }
    view_set_draw_callback(app->list_view, todo_view_view_tasks_draw_callback);
    view_set_previous_callback(app->list_view, todo_navigation_submenu_callback);
    view_dispatcher_add_view(app->view_dispatcher, TodoViewViewTasks, app->list_view);

    app->task_input_model.task[0] = '\0';
    app->task_count = 0; // Initialize task count to 0

    return app;
}

static void todo_app_free(TodoApp* app) {
    text_input_free(app->text_input);
    view_free(app->list_view);
    widget_free(app->widget_about);
    widget_free(app->widget_confirm);
    submenu_free(app->submenu);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t main_todolist_app(void* _p) {
    UNUSED(_p);

    TodoApp* app = todo_app_alloc();
    if(!app) {
        FURI_LOG_E(TAG, "Failed to allocate TodoApp");
        return -1;
    }

    view_dispatcher_run(app->view_dispatcher);

    todo_app_free(app);
    return 0;
}
