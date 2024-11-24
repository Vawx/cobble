/* cobble_ufbx.h : date = November 18th 2024 6:41 pm */

#if !defined(COBBLE_UFBX_H)

#include "ufbx/ufbx.h"

#define UFBX_MAX_PIECES_PER_MESH 8

typedef struct ufbx_mesh_piece {
    mesh_vertex *vertices;
    u32 num_vertices;
    u16 *indices;
    u32 num_indices;
} ufbx_mesh_piece;

typedef struct ufbx_mesh_object {
    ufbx_mesh_piece *mesh_pieces;
    u32 mesh_pieces_count;
} ufbx_mesh_object;

static ufbx_mesh_object ufbx_load(const dir_t *dir);

#define COBBLE_UFBX_H
#endif //COBBLE_UFBX_H
