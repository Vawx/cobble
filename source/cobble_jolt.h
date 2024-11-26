/* cobble_jolt.h : date = November 9th 2024 8:13 pm */

#if !defined(COBBLE_JOLT_H)
#include "joltc/joltc.h"

#define NOT_MOVING 0
#define MOVING 1

typedef void (*dynamic_body_model_matrix)(JPH_BodyID *id, m4 *out);

typedef enum jolt_shape_type {
    JOLT_SHAPE_SPHERE,
    JOLT_SHAPE_BOX,
    JOLT_SHAPE_CAPSULE,
    JOLT_SHAPE_COUNT
} jolt_shape_type;

typedef struct {
    v3 extents;
    JPH_BoxShape *shape;
} jolt_box;

typedef struct {
    r32 radius;
    JPH_SphereShape *shape;
} jolt_sphere;

typedef struct {
    r32 radius;
    r32 height;
} jolt_capsule;

typedef struct {
    jolt_shape_type type;
    JPH_BodyID id;
    
    union {
        jolt_box box;
        jolt_sphere sphere;
        jolt_capsule capsule;
    };
} jolt_dynamic_object;

typedef enum jolt_object_layer {
    JOLT_OBJECT_LAYER_NON_MOVING = 0,
    JOLT_OBJECT_LAYER_MOVING     = 1,
    JOLT_OBJECT_LAYER_COUNT      = 2,
} jolt_object_layer;

static JPH_BroadPhaseLayer BROADPHASE_LAYER_NON_MOVING = 0;
static JPH_BroadPhaseLayer BROADPHASE_LAYER_MOVING     = 1;
static u32 NUM_LAYERS                                  = 2;

typedef struct cobble_jolt {
    JPH_PhysicsSystemSettings physics_system_settings;
    JPH_JobSystem *job_system;
    JPH_ObjectLayerPairFilter *object_layer_pair_filter_table;
    JPH_BroadPhaseLayerInterface *broadphase_layer_interface_table;
    JPH_ObjectVsBroadPhaseLayerFilter *object_vs_broadphase_layer_filter;
    JPH_PhysicsSystem *system;
	JPH_BodyInterface *body_interface;
} cobble_jolt;

static void jolt_trace_impl(const char* message);
static void jolt_init();
static void jolt_step(r32 delta);
static void jolt_close();
static void jolt_get_dynamic_object_model_matrix(JPH_BodyID *id, m4 *result);
static jolt_dynamic_object jolt_make_dynamic_box(v3 extents, v3 pos, u8 moving);
static jolt_dynamic_object jolt_make_dynamic_sphere(r32 rad, v3 pos, u8 moving);
static jolt_dynamic_object jolt_make_dynamic_capsule(r32 rad, r32 height, v3 pos);

#define COBBLE_JOLT_H
#endif //COBBLE_JOLT_H
