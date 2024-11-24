/* cobble_types.h : date = November 8th 2024 2:06 pm */

#if !defined(COBBLE_TYPES_H)

#define c_pi 3.14159265358979323846
#define c_2_pi (2.0 * c_pi)
#define c_half_pi (c_pi * 0.5f)
#define c_rads_per_deg (c_2_pi / 360.f)
#define c_degs_per_rad (360.f / c_2_pi)
#define c_sqrt12 (0.7071067811865475244008443621048490f)

#define c_deg_180 180.0
#define c_turn_half 0.5
#define c_rad_to_deg ((r32)(c_deg_180/c_pi))
#define c_rag_to_turn ((r32)(c_turn_half/c_pi))
#define c_deg_to_rad ((r32)(c_pi/c_deg_180))
#define c_deg_to_turn ((r32)(c_turn_half/c_deg_180))
#define c_turn_to_rad ((r32)(c_pi/c_turn_half))
#define c_turn_to_deg ((r32)(c_deg_180/c_turn_half))

#define c_rad(a) ((a)*c_deg_to_rad)
#define c_deg(a) ((a)*c_rad_to_deg)
#define c_turn(a) ((a)*c_turn_to_deg)

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float r32;
typedef double r64;

#define stmnt(s) do{ s }while(0)

#define c_assert_break() (*(int*)0 = 0xABCD)
#define c_assert(c) stmnt( if (!(c)){ c_assert_break(); } )
#define c_assert_msg(c, msg, ...) stmnt( if(!(c)){ LOG_SHUTDOWN(msg, ##__VA_ARGS__); c_assert_break(); } )

#define array_count(a) (sizeof(a) / sizeof((a)[0]))

#define kilo(n)  (((u64)(n)) << 10)
#define mega(n)  (((u64)(n)) << 20)
#define giga(n)  (((u64)(n)) << 30)
#define tera(n)  (((u64)(n)) << 40)

#define thousand(n)   ((n)*1000)
#define million(n)    ((n)*1000000)
#define billion(n)    ((n)*1000000000)


#define COBBLE_TYPES_H
#endif //COBBLE_TYPES_H
