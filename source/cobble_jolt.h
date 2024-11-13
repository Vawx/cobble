/* cobble_jolt.h : date = November 9th 2024 8:13 pm */

#if !defined(COBBLE_JOLT_H)

#include "joltc/joltc.h"

#define NOT_MOVING 0
#define MOVING 1

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

static cobble_jolt jolt;

static void jolt_trace_impl(const char* message) {
	printf("[cobble][jolt]: %s\n", message);
}

static void jolt_init() {
    if(!JPH_Init()) {
        c_assert_break(); // failed to init physics
    }
    
    JPH_SetTraceHandler(jolt_trace_impl);
	//JPH_SetAssertFailureHandler(JPH_AssertFailureFunc handler);
    
    jolt.job_system = JPH_JobSystemThreadPool_Create(NULL);
    
	// We use only 2 layers: one for non-moving objects and one for moving objects
	jolt.object_layer_pair_filter_table = JPH_ObjectLayerPairFilterTable_Create(2);
	JPH_ObjectLayerPairFilterTable_EnableCollision(jolt.object_layer_pair_filter_table, JOLT_OBJECT_LAYER_NON_MOVING, JOLT_OBJECT_LAYER_MOVING);
	JPH_ObjectLayerPairFilterTable_EnableCollision(jolt.object_layer_pair_filter_table, JOLT_OBJECT_LAYER_MOVING, JOLT_OBJECT_LAYER_MOVING);
    
	// We use a 1-to-1 mapping between object layers and broadphase layers
	jolt.broadphase_layer_interface_table = JPH_BroadPhaseLayerInterfaceTable_Create(2, 2);
	JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(jolt.broadphase_layer_interface_table, JOLT_OBJECT_LAYER_NON_MOVING, BROADPHASE_LAYER_NON_MOVING);
	JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(jolt.broadphase_layer_interface_table, JOLT_OBJECT_LAYER_MOVING, BROADPHASE_LAYER_MOVING);
    
	jolt.object_vs_broadphase_layer_filter = JPH_ObjectVsBroadPhaseLayerFilterTable_Create(jolt.broadphase_layer_interface_table, 2, jolt.object_layer_pair_filter_table, 2);
    
	jolt.physics_system_settings.maxBodies = 65536;
	jolt.physics_system_settings.numBodyMutexes = 0;
	jolt.physics_system_settings.maxBodyPairs = 65536;
	jolt.physics_system_settings.maxContactConstraints = 65536;
	jolt.physics_system_settings.broadPhaseLayerInterface = jolt.broadphase_layer_interface_table;
	jolt.physics_system_settings.objectLayerPairFilter = jolt.object_layer_pair_filter_table;
	jolt.physics_system_settings.objectVsBroadPhaseLayerFilter = jolt.object_vs_broadphase_layer_filter;
	jolt.system = JPH_PhysicsSystem_Create(&jolt.physics_system_settings);
	jolt.body_interface = JPH_PhysicsSystem_GetBodyInterface(jolt.system);
    
    // Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
	// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
	// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
	JPH_PhysicsSystem_OptimizeBroadPhase(jolt.system);
}

static void jolt_step(r32 delta) {
#define JOLT_COLLISION_STEPS 1
    JPH_PhysicsSystem_Update(jolt.system, delta, JOLT_COLLISION_STEPS, jolt.job_system);
}

static void jolt_close() {
    JPH_JobSystem_Destroy(jolt.job_system);
    JPH_PhysicsSystem_Destroy(jolt.system);
	JPH_Shutdown();
}

static m4 jolt_get_dynamic_object_model_matrix(JPH_BodyID *id) {
    JPH_RMatrix4x4 jph_mat = {0};
    JPH_BodyInterface_GetWorldTransform(jolt.body_interface, *id, &jph_mat);
    
    m4 result = {0};
    result.elements[0][0] = jph_mat.m11;
    result.elements[0][1] = jph_mat.m12;
    result.elements[0][2] = jph_mat.m13;
    result.elements[0][3] = jph_mat.m14;
    
    result.elements[1][0] = jph_mat.m21;
    result.elements[1][1] = jph_mat.m22;
    result.elements[1][2] = jph_mat.m23;
    result.elements[1][3] = jph_mat.m24;
    
    result.elements[2][0] = jph_mat.m31;
    result.elements[2][1] = jph_mat.m32;
    result.elements[2][2] = jph_mat.m33;
    result.elements[2][3] = jph_mat.m34;
    
    result.elements[3][0] = jph_mat.m41;
    result.elements[3][1] = jph_mat.m42;
    result.elements[3][2] = jph_mat.m43;
    result.elements[3][3] = jph_mat.m44;
    
    return result;
}

static jolt_dynamic_object jolt_make_dynamic_box(v3 extents, v3 pos, u8 moving) {
    jolt_dynamic_object result = {0};
    result.type = JOLT_SHAPE_BOX;
    
    JPH_Vec3 box_pos = jph_vec3_from_v3(pos);
    JPH_Vec3 box_extents = jph_vec3_from_v3(extents);
    
    result.box.shape = JPH_BoxShape_Create(&box_extents, JPH_DEFAULT_CONVEX_RADIUS);
    JPH_BodyCreationSettings* box_settings = JPH_BodyCreationSettings_Create3((const JPH_Shape*)result.box.shape,
                                                                              &box_pos,
                                                                              NULL,
                                                                              moving ? JPH_MotionType_Dynamic : JPH_MotionType_Static,
                                                                              moving ? JOLT_OBJECT_LAYER_MOVING : JOLT_OBJECT_LAYER_NON_MOVING);
    
    result.id = JPH_BodyInterface_CreateAndAddBody(jolt.body_interface, box_settings, moving ? JPH_Activation_Activate : JPH_Activation_DontActivate);
    JPH_BodyCreationSettings_Destroy(box_settings);
    return result;
}

static jolt_dynamic_object jolt_make_dynamic_sphere(r32 rad, v3 pos, u8 moving) {
    jolt_dynamic_object result = {0};
    result.type = JOLT_SHAPE_SPHERE;
    
    result.sphere.shape = JPH_SphereShape_Create(rad);
    result.sphere.radius = rad;
    JPH_Vec3 sphere_pos = jph_vec3_from_v3(pos);
    JPH_BodyCreationSettings* sphere_settings = JPH_BodyCreationSettings_Create3((const JPH_Shape*)result.sphere.shape,
                                                                                 &sphere_pos,
                                                                                 NULL,
                                                                                 moving ? JPH_MotionType_Dynamic : JPH_MotionType_Static,
                                                                                 moving ? JOLT_OBJECT_LAYER_MOVING : JOLT_OBJECT_LAYER_NON_MOVING);
    
    result.id = JPH_BodyInterface_CreateAndAddBody(jolt.body_interface, sphere_settings, moving ? JPH_Activation_Activate : JPH_Activation_DontActivate);
    JPH_BodyCreationSettings_Destroy(sphere_settings);
    return result;
}

static jolt_dynamic_object jolt_make_dynamic_capsule(r32 rad, r32 height, v3 pos, v3 rot, v3 vel) {
    
}

#define COBBLE_JOLT_H
#endif //COBBLE_JOLT_H
