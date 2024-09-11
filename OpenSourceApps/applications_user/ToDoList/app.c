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
} TodoView;

typedef struct {
    char task[TASK_LENGTH];
} TaskInputModel;

typedef struct {
    ViewDispatcher* view_dispatcher; // Switches between our views
    Submenu* submenu; // The application menu
    Widget* widget_about; // The about screen
    TextInput* text_input; // Text input for adding a task
    View* list_view; // Custom list view for displaying tasks
    TaskInputModel task_input_model; // Task input model
    char tasks[MAX_TASKS][TASK_LENGTH]; // Array to store tasks
    size_t task_count; // Number of tasks stored
} TodoApp;

// Callback to exit the app
static uint32_t todo_navigation_exit_callback(void* _context) {
    UNUSED(_context);
    FURI_LOG_I(TAG, "Exiting ToDo App.");
    return VIEW_NONE;
}

// Callback to return to the submenu
static uint32_t todo_navigation_submenu_callback(void* _context) {
    UNUSED(_context);
    FURI_LOG_I(TAG, "Returning to submenu.");
    return TodoViewSubmenu;
}

// Handle submenu item selection
static void todo_submenu_callback(void* context, uint32_t index) {
    TodoApp* app = (TodoApp*)context;
    FURI_LOG_I(TAG, "Switching to submenu index: %lu", (unsigned long)index);
    switch(index) {
    case TodoSubmenuIndexAddTask:
        FURI_LOG_I(TAG, "Switching to Add Task view.");
        view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewAddTask);
        break;
    case TodoSubmenuIndexViewTasks:
        FURI_LOG_I(TAG, "Switching to View Tasks view.");
        view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewViewTasks);
        break;
    case TodoSubmenuIndexAbout:
        FURI_LOG_I(TAG, "Switching to About view.");
        view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewAbout);
        break;
    default:
        break;
    }
}

// Callback for handling task input
static void todo_view_add_task_result_callback(void* context) {
    TodoApp* app = (TodoApp*)context;

    FURI_LOG_I(TAG, "Entered task: %s", app->task_input_model.task);

    if(strlen(app->task_input_model.task) > 0) {
        if(app->task_count < MAX_TASKS) {
            FURI_LOG_I(TAG, "Adding task to list. Task count: %zu", app->task_count);
            strncpy(app->tasks[app->task_count], app->task_input_model.task, TASK_LENGTH);
            app->tasks[app->task_count][TASK_LENGTH - 1] = '\0'; // Ensure null termination
            app->task_count++;
            FURI_LOG_I(TAG, "Task added successfully. New task count: %zu", app->task_count);

            // After adding the task, go back to the submenu
            view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewSubmenu);
        } else {
            FURI_LOG_W(TAG, "Task list full.");
            view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewSubmenu);
        }
    } else {
        FURI_LOG_W(TAG, "No task entered.");
        view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewSubmenu);
    }
}

// Draw view tasks screen
static void todo_view_view_tasks_draw_callback(Canvas* canvas, void* context) {
    TodoApp* app = (TodoApp*)context;
    if(app && app->task_count > 0) {
        FURI_LOG_I(TAG, "Drawing tasks. Task count: %zu", app->task_count);
        canvas_draw_str(canvas, 10, 10, "Tasks:");
        for(size_t i = 0; i < app->task_count; i++) {
            FURI_LOG_I(TAG, "Task %zu: %s", i, app->tasks[i]);
            canvas_draw_str(canvas, 10, 20 + (i * 10), app->tasks[i]);
        }
    } else {
        FURI_LOG_W(TAG, "No tasks recorded.");
        canvas_draw_str(canvas, 10, 10, "No tasks recorded.");
    }
}

// Allocate the ToDo app
static TodoApp* todo_app_alloc() {
    FURI_LOG_I(TAG, "Allocating memory for ToDo App.");
    TodoApp* app = (TodoApp*)malloc(sizeof(TodoApp));
    if(!app) {
        FURI_LOG_E(TAG, "Failed to allocate memory for TodoApp.");
        return NULL;
    }

    Gui* gui = furi_record_open(RECORD_GUI);
    if(!gui) {
        FURI_LOG_E(TAG, "Failed to open GUI record.");
        free(app);
        return NULL;
    }

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    // Submenu setup
    app->submenu = submenu_alloc();
    submenu_add_item(
        app->submenu, "Add Task", TodoSubmenuIndexAddTask, todo_submenu_callback, app);
    submenu_add_item(
        app->submenu, "View Tasks", TodoSubmenuIndexViewTasks, todo_submenu_callback, app);
    submenu_add_item(app->submenu, "About", TodoSubmenuIndexAbout, todo_submenu_callback, app);
    view_set_previous_callback(submenu_get_view(app->submenu), todo_navigation_exit_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, TodoViewSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_switch_to_view(app->view_dispatcher, TodoViewSubmenu);

    // About screen
    app->widget_about = widget_alloc();
    widget_add_text_scroll_element(
        app->widget_about, 0, 0, 128, 64, "This is a simple ToDo list app.");
    view_set_previous_callback(
        widget_get_view(app->widget_about), todo_navigation_submenu_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, TodoViewAbout, widget_get_view(app->widget_about));

    // Text input for adding tasks
    app->text_input = text_input_alloc();
    text_input_set_header_text(app->text_input, "Enter Task");

    // Corrected function call with the necessary arguments
    text_input_set_result_callback(
        app->text_input,
        todo_view_add_task_result_callback,
        app,
        app->task_input_model.task,
        TASK_LENGTH,
        true // Set to true if you want to clear default text after the task is added
    );
    view_dispatcher_add_view(
        app->view_dispatcher, TodoViewAddTask, text_input_get_view(app->text_input));

    // List view for tasks
    app->list_view = view_alloc();
    view_set_context(app->list_view, app); // Set the context to the app
    view_set_draw_callback(app->list_view, todo_view_view_tasks_draw_callback);
    view_set_previous_callback(app->list_view, todo_navigation_submenu_callback);
    view_dispatcher_add_view(app->view_dispatcher, TodoViewViewTasks, app->list_view);

    // Initialize tasks
    app->task_input_model.task[0] = '\0';
    app->task_count = 0;

    FURI_LOG_I(TAG, "ToDo App allocated successfully.");

    return app;
}

// Free the ToDo app
static void todo_app_free(TodoApp* app) {
    FURI_LOG_I(TAG, "Freeing ToDo App.");
    text_input_free(app->text_input);
    view_free(app->list_view);
    widget_free(app->widget_about);
    submenu_free(app->submenu);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    free(app);
}

// Main function
int32_t main_todolist_app(void* _p) {
    UNUSED(_p);

    TodoApp* app = todo_app_alloc();
    if(!app) return -1;

    view_dispatcher_run(app->view_dispatcher);
    todo_app_free(app);

    return 0;
}
