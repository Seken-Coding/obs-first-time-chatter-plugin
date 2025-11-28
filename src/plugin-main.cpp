#include <obs-module.h>
#include <obs-frontend-api.h>
#include "first-time-chatter-dock.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("first-time-chatter-dock", "en-US")

static FirstTimeChatterDock *dock = nullptr;

bool obs_module_load(void)
{
    blog(LOG_INFO, "[First-Time Chatter Dock] Plugin loaded");
    
    // Create the dock widget
    dock = new FirstTimeChatterDock();
    dock->setObjectName(QStringLiteral("FirstTimeChatterDock"));
    
    // Register the dock with OBS
    if (!obs_frontend_add_dock_by_id("first-time-chatter-dock", "First-Time Chatters", dock)) {
        blog(LOG_ERROR, "[First-Time Chatter Dock] Failed to add dock");
        delete dock;
        dock = nullptr;
        return false;
    }
    
    blog(LOG_INFO, "[First-Time Chatter Dock] Dock registered successfully");
    return true;
}

void obs_module_unload(void)
{
    blog(LOG_INFO, "[First-Time Chatter Dock] Plugin unloaded");
    // Note: OBS takes ownership of the dock widget, so we don't delete it here
}

const char *obs_module_name(void)
{
    return "First-Time Chatter Dock";
}

const char *obs_module_description(void)
{
    return "Displays first-time chatters in a dock panel with greeted tracking";
}
