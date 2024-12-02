/* log_t.h : date = November 23rd 2024 0:57 am */

#if !defined(COBBLE_LOG_H)

typedef enum log_severity {
    LOG_TELL,
    LOG_YELL,
    LOG_PANIC,
    LOG_SHUTDOWN,
    LOG_COUNT,
} log_severity;

const char *log_severity_str[LOG_COUNT] = {
    "[TELL] : ",
    "[YELL] : ",
    "[PANIC] : ",
    "[SHUTDOWN] : ",
};

typedef struct log_t {
    u8 *ptr;
    u32 len;
    log_severity severity;
} log_t;

#define LOGGER_MEMORY_RESERVE mega(10)
#define LOGGER_LOGS_COUNT mega(1)
typedef struct logger_t {
    log_t *logs;
    u32 log_idx;
    
    u8 *ptr;
    u32 ptr_idx;
} logger_t;

static void logger_init();
static void logger_shutdown();

#if COBBLE_DEBUG_LOG
#pragma message("COBBLE DEBUG LOG: Enabled")
#define LOG_TELL(str, ...) log_store_ouput(LOG_TELL, __FILE__, __LINE__, str, ##__VA_ARGS__)
#define LOG_YELL(str, ...) log_store_ouput(LOG_YELL, __FILE__, __LINE__, str, ##__VA_ARGS__)
#define LOG_PANIC(str, ...) log_store_ouput(LOG_PANIC, __FILE__, __LINE__, str, ##__VA_ARGS__)
#define LOG_SHUTDOWN(str, ...) log_store_ouput(LOG_SHUTDOWN, __FILE__, __LINE__, str, ##__VA_ARGS__)
#define LOG_TELL_COND(c, str, ...) stmnt( if(!(c)) {LOG_TELL(str, ##__VA_ARGS__);} )
#define LOG_YELL_COND(c, str, ...) stmnt( if(!(c)) {LOG_YELL(str, ##__VA_ARGS__);} )
#define LOG_PANIC_COND(c, str, ...) stmnt( if(!(c)) {LOG_PANIC(str, ##__VA_ARGS__);} )
#define LOG_SHUTDOWN_COND(c, str, ...) c_assert_msg(c, str, ##__VA_ARGS__) // log define is same as assert msg, so just call that.
#else 
#pragma message("COBBLE DEBUG LOG: Disabled")
#define LOG_TELL(str, ...) 
#define LOG_YELL(str, ...) 
#define LOG_PANIC(str, ...)
#define LOG_SHUTDOWN(str, ...)
#define LOG_TELL_COND(c, str, ...) 
#define LOG_YELL_COND(c, str, ...) 
#define LOG_PANIC_COND(c, str, ...)
#define LOG_SHUTDOWN_COND(c, str, ...) 
#endif // COBBLE_DEBUG_LOG

#define COBBLE_LOG_H
#endif //COBBLE_LOG_H
