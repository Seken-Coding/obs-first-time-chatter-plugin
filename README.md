# OBS First-Time Chatter Plugin

Native OBS Studio plugin that displays first-time chatters in a dock panel. Perfect for streamers who want to greet new viewers!

## Features

- **Twitch IRC Connection** - Connects to Twitch chat via WebSocket
- **First-time chatter detection** - Tracks unique usernames per session
- **Bot filtering** - Filters out known bots (Nightbot, StreamElements, etc.) with user-editable list
- **Click-to-mark as greeted** - Click a chatter name to strikethrough and dim (toggle on/off)
- **Reset button** - Clear all chatters and greeted state
- **Daily auto-reset** - Automatically resets chatter list when a new day begins
- **Persistent state** - Saves clicked/greeted state across OBS restarts

## UI

- Dark theme matching OBS aesthetic
- Newest chatters appear at top of list
- Connection status indicator (🟢 Connected, 🟡 Connecting, ⚫ Disconnected, 🔴 Error)
- Settings dialog for configuration

## Requirements

- OBS Studio 28.0 or later
- Qt6 (Widgets, Network, WebSockets)
- CMake 3.16+

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Dependencies

- libobs
- obs-frontend-api
- Qt6::Widgets
- Qt6::Network
- Qt6::WebSockets

## Usage

1. Build and install the plugin to your OBS plugins directory
2. In OBS, go to View → Docks → First-Time Chatters
3. Click the ⚙ settings button to configure:
   - Your Twitch channel name
   - Your Twitch username
   - OAuth token (get one from https://twitchapps.com/tmi/)
   - Bot filter list
4. Click "Connect" to start monitoring chat
5. Click on chatter names to mark them as greeted (strikethrough)
6. Use "Reset" to clear the list

## License

This project is available under the MIT License
