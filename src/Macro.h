#pragma once

// Build Configuration Macros
#if SERVER_BUILD
#undef UNICODE
#define SERVER_ONLY(x) x;
#else 
#define SERVER_ONLY(x)
#endif //SERVER_BUILD

#if CLIENT_BUILD 
#define CLIENT_ONLY(x) x;
#else 
#define CLIENT_ONLY(x)
#endif //CLIENT_BUILD

// Network Configuration Macros
#define WIN32_LEAN_AND_MEAN
#define DEFAULT_BUFLEN 16
#define DEFAULT_PORT "27015"

// Result Code Macros
#define RESULT_SUCCEED 0 
#define RESULT_ERROR 1
#define RESULT_NOT_SUPPORTED 2

// Debug Macros
#define DEBUG_NET_LOG 0
#define DEBUG_LOG_FILE 1

#if DEBUG_NET_LOG
#define NET_LOG(...) printf(__VA_ARGS__);
#else 
#define NET_LOG(...) 
#endif //DEBUG_NET_LOG

// Screen Configuration Macros
#define SCREEN_Y_MAX 20
#define SCREEN_X_MAX 41 
#define CONSOLE_MAX_MESSAGE_LINES 10
#define CONSOLE_SCREEN_BUFFER_X 50
#define CONSOLE_SCREEN_BUFFER_Y (SCREEN_Y_MAX) + (CONSOLE_MAX_MESSAGE_LINES)

// Utility Macros
#define CLAMP_RANGE(x, Min, Max)  x = ((x) < Min) ?  Min : ((x) > Max) ? Max : (x);
#define CLAMP(Location) CLAMP_RANGE(Location.X, 2 ,SCREEN_X_MAX-3) CLAMP_RANGE(Location.Y, 1, SCREEN_Y_MAX-2) 