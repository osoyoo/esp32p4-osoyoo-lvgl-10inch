idf_component_register(
    SRCS
        "main.c"
        "osoyoo_panel.c"
        # ---- SquareLine Studio exported UI (main/ui/) ----
        "ui/ui.c"
        "ui/ui_events.c"
        "ui/ui_helpers.c"
        "ui/screens/ui_Screen1.c"
        "ui/components/ui_comp_hook.c"
    INCLUDE_DIRS
        "."
        "ui"
        "ui/screens"
        "ui/components"
)
