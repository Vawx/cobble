/* cobble_log.h : date = November 23rd 2024 0:57 am */

#if !defined(COBBLE_LOG_H)

typedef enum cobble_log_severity {
    LOG_TELL,
    LOG_YELL,
    LOG_PANIC,
    LOG_SHUTDOWN,
    LOG_COUNT,
} cobble_log_severity;

const char *log_severity_str[LOG_COUNT] = {
    "[TELL] : ",
    "[YELL] : ",
    "[PANIC] : ",
    "[SHUTDOWN] : ",
};

typedef struct cobble_log {
    u8 *ptr;
    u32 len;
    cobble_log_severity severity;
} cobble_log;

#define LOGGER_MEMORY_RESERVE mega(10)
#define LOGGER_LOGS_COUNT mega(1)
typedef struct cobble_logger {
    cobble_log *logs;
    u32 log_idx;
    
    u8 *ptr;
    u32 ptr_idx;
} cobble_logger;

static void logger_init();
static void logger_shutdown();
static void tell(const char *file, const u32 line, const char *str, ...);
static void yell(const char *file, const u32 line, const char *str, ...);
static void panic(const char *file, const u32 line, const char *str, ...);
static void shutdown(const char *file, const u32 line, const char *str, ...);

#define LOG_TELL(str, ...) log_store_ouput(LOG_TELL, __FILE__, __LINE__, str, ##__VA_ARGS__)
#define LOG_YELL(str, ...) log_store_ouput(LOG_YELL, __FILE__, __LINE__, str, ##__VA_ARGS__)
#define LOG_PANIC(str, ...) log_store_ouput(LOG_PANIC, __FILE__, __LINE__, str, ##__VA_ARGS__)
#define LOG_SHUTDOWN(str, ...) log_store_ouput(LOG_SHUTDOWN, __FILE__, __LINE__, str, ##__VA_ARGS__)

#define COBBLE_LOG_H
#endif //COBBLE_LOG_H
