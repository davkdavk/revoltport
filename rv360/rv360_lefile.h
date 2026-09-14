#ifndef RV360_LEFILE_H
#define RV360_LEFILE_H

// Explicit little-endian readers for Re-Volt disk structs (M1).
// Disk files are LE x86; Xenon is BE. Readers swap only on big-endian.
// Self-contained: mirrors game structs with fixed-width types so this
// header compiles without xtl.h / game headers (testable on Linux).
// Game integration (M2): call these instead of fread(&struct,sizeof).

#include "rv360_endian.h"

// --- model.h:175-237 (packed-load structs only; runtime MODEL has pointers) ---
typedef struct { float x, y, z, nx, ny, nz; } rv_le_vertexload;

typedef struct {
    rv_s16 type, tpage;
    rv_s16 vi0, vi1, vi2, vi3;
    rv_s32 c0, c1, c2, c3;
    float u0, v0, u1, v1, u2, v2, u3, v3;
} rv_le_pollload;

typedef struct { rv_s16 polynum, vertnum; } rv_le_modelheader;

static inline int rv_read_vertexload(rv_le_vertexload *o, FILE *fp) {
    if (!rv_fread_f32le(&o->x, fp)) return 0;
    if (!rv_fread_f32le(&o->y, fp)) return 0;
    if (!rv_fread_f32le(&o->z, fp)) return 0;
    if (!rv_fread_f32le(&o->nx, fp)) return 0;
    if (!rv_fread_f32le(&o->ny, fp)) return 0;
    if (!rv_fread_f32le(&o->nz, fp)) return 0;
    return 1;
}

static inline int rv_read_pollload(rv_le_pollload *o, FILE *fp) {
    uint16_t t16; uint32_t t32;
    if (fread(&t16, 2, 1, fp) != 1) return 0;
#ifdef RV360_BIG_ENDIAN
    t16 = rv_swap16(t16);
#endif
    o->type = (rv_s16)t16;
    if (fread(&t16, 2, 1, fp) != 1) return 0;
#ifdef RV360_BIG_ENDIAN
    t16 = rv_swap16(t16);
#endif
    o->tpage = (rv_s16)t16;
    rv_s16 *idx[4] = { &o->vi0, &o->vi1, &o->vi2, &o->vi3 };
    for (int i = 0; i < 4; i++) {
        if (fread(&t16, 2, 1, fp) != 1) return 0;
#ifdef RV360_BIG_ENDIAN
        t16 = rv_swap16(t16);
#endif
        *idx[i] = (rv_s16)t16;
    }
    rv_s32 *col[4] = { &o->c0, &o->c1, &o->c2, &o->c3 };
    for (int i = 0; i < 4; i++) {
        if (!rv_fread_u32le(&t32, fp)) return 0;
        *col[i] = (rv_s32)t32;
    }
    float *uv[8] = { &o->u0, &o->v0, &o->u1, &o->v1, &o->u2, &o->v2, &o->u3, &o->v3 };
    for (int i = 0; i < 8; i++)
        if (!rv_fread_f32le(uv[i], fp)) return 0;
    return 1;
}

static inline int rv_read_modelheader(rv_le_modelheader *o, FILE *fp) {
    uint16_t t16;
    if (fread(&t16, 2, 1, fp) != 1) return 0;
#ifdef RV360_BIG_ENDIAN
    t16 = rv_swap16(t16);
#endif
    o->polynum = (rv_s16)t16;
    if (fread(&t16, 2, 1, fp) != 1) return 0;
#ifdef RV360_BIG_ENDIAN
    t16 = rv_swap16(t16);
#endif
    o->vertnum = (rv_s16)t16;
    return 1;
}

// --- edfield.h:27-41 FILE_FIELD (VEC=3xf32, MAT=9xf32, REAL=float on PC) ---
typedef struct {
    rv_s32 type;
    float pos[3], mat[9], size[3], dir[3];
    float mag, damping, radstart, radend, gradstart, gradend;
} rv_le_filefield;

static inline int rv_read_filefield(rv_le_filefield *o, FILE *fp) {
    uint32_t t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->type = (rv_s32)t32;
    float *f[] = { &o->pos[0], &o->pos[1], &o->pos[2],
                   &o->mat[0], &o->mat[1], &o->mat[2], &o->mat[3], &o->mat[4],
                   &o->mat[5], &o->mat[6], &o->mat[7], &o->mat[8],
                   &o->size[0], &o->size[1], &o->size[2],
                   &o->dir[0], &o->dir[1], &o->dir[2],
                   &o->mag, &o->damping,
                   &o->radstart, &o->radend, &o->gradstart, &o->gradend };
    for (unsigned i = 0; i < sizeof(f) / sizeof(f[0]); i++)
        if (!rv_fread_f32le(f[i], fp)) return 0;
    return 1;
}

// --- level.h:42-53 record entries (Time/SplitTime are long on disk = s32) ---
typedef struct {
    rv_s32 time;
    char player[16];
    char car[32];
} rv_le_record1;

static inline int rv_read_record1(rv_le_record1 *o, FILE *fp) {
    uint32_t t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->time = (rv_s32)t32;
    if (fread(o->player, 1, 16, fp) != 16) return 0;
    if (fread(o->car, 1, 32, fp) != 32) return 0;
    return 1;
}

// --- newcoll.h: NEWCOLLPOLY (non-PSX) = s32 Type/Material + 4 + 16 + 6 floats ---
typedef struct {
    rv_s32 type, material;
    float plane[4], edge[16], bbox[6];
} rv_le_collpoly;

static inline int rv_read_collpoly(rv_le_collpoly *o, FILE *fp) {
    uint32_t t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->type = (rv_s32)t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->material = (rv_s32)t32;
    float *f[] = { &o->plane[0], &o->plane[1], &o->plane[2], &o->plane[3],
                   &o->edge[0], &o->edge[1], &o->edge[2], &o->edge[3],
                   &o->edge[4], &o->edge[5], &o->edge[6], &o->edge[7],
                   &o->edge[8], &o->edge[9], &o->edge[10], &o->edge[11],
                   &o->edge[12], &o->edge[13], &o->edge[14], &o->edge[15],
                   &o->bbox[0], &o->bbox[1], &o->bbox[2],
                   &o->bbox[3], &o->bbox[4], &o->bbox[5] };
    for (unsigned i = 0; i < sizeof(f) / sizeof(f[0]); i++)
        if (!rv_fread_f32le(f[i], fp)) return 0;
    return 1;
}

// --- newcoll.h: COLLGRID_DATA = 5 REALs (float on PC) ---
typedef struct { float xstart, zstart, xnum, znum, gridsize; } rv_le_collgriddata;

static inline int rv_read_collgriddata(rv_le_collgriddata *o, FILE *fp) {
    if (!rv_fread_f32le(&o->xstart, fp)) return 0;
    if (!rv_fread_f32le(&o->zstart, fp)) return 0;
    if (!rv_fread_f32le(&o->xnum, fp)) return 0;
    if (!rv_fread_f32le(&o->znum, fp)) return 0;
    if (!rv_fread_f32le(&o->gridsize, fp)) return 0;
    return 1;
}

// --- generic small readers: INDEX(short), VEC(3f), PLANE(4f), BBOX(6f), SPHERE(4f) ---
static inline int rv_read_index(rv_s16 *o, FILE *fp) {
    uint16_t t16;
    if (fread(&t16, 2, 1, fp) != 1) return 0;
#ifdef RV360_BIG_ENDIAN
    t16 = rv_swap16(t16);
#endif
    *o = (rv_s16)t16;
    return 1;
}

static inline int rv_read_f32n(float *o, int n, FILE *fp) {
    for (int i = 0; i < n; i++)
        if (!rv_fread_f32le(&o[i], fp)) return 0;
    return 1;
}

// --- instance.h: FILE_INSTANCE (Name[9], rgb chars, u32 EnvRGB, flags, f32, VEC, MAT) ---
typedef struct {
    char name[9];
    char r, g, b;
    rv_u32 envrgb;
    unsigned char priority, flag, pad[2];
    float lodbias, pos[3], mat[9];
} rv_le_fileinstance;

static inline int rv_read_fileinstance(rv_le_fileinstance *o, FILE *fp) {
    uint32_t t32;
    if (fread(o->name, 1, 9, fp) != 9) return 0;
    if (fread(&o->r, 1, 1, fp) != 1) return 0;
    if (fread(&o->g, 1, 1, fp) != 1) return 0;
    if (fread(&o->b, 1, 1, fp) != 1) return 0;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->envrgb = t32;
    if (fread(&o->priority, 1, 1, fp) != 1) return 0;
    if (fread(&o->flag, 1, 1, fp) != 1) return 0;
    if (fread(o->pad, 1, 2, fp) != 2) return 0;
    float *f[] = { &o->lodbias, &o->pos[0], &o->pos[1], &o->pos[2],
                   &o->mat[0], &o->mat[1], &o->mat[2], &o->mat[3], &o->mat[4],
                   &o->mat[5], &o->mat[6], &o->mat[7], &o->mat[8] };
    for (unsigned i = 0; i < sizeof(f) / sizeof(f[0]); i++)
        if (!rv_fread_f32le(f[i], fp)) return 0;
    return 1;
}

// --- world.h: CUBE_HEADER_LOAD (10f + 2 s16), BIG_CUBE_HEADER_LOAD (4f + s32) ---
typedef struct {
    float cx, cy, cz, radius, xmin, xmax, ymin, ymax, zmin, zmax;
    rv_s16 polynum, vertnum;
} rv_le_cubeheaderload;

static inline int rv_read_cubeheaderload(rv_le_cubeheaderload *o, FILE *fp) {
    float *f[] = { &o->cx, &o->cy, &o->cz, &o->radius,
                   &o->xmin, &o->xmax, &o->ymin, &o->ymax, &o->zmin, &o->zmax };
    for (unsigned i = 0; i < sizeof(f) / sizeof(f[0]); i++)
        if (!rv_fread_f32le(f[i], fp)) return 0;
    uint16_t t16;
    if (fread(&t16, 2, 1, fp) != 1) return 0;
#ifdef RV360_BIG_ENDIAN
    t16 = rv_swap16(t16);
#endif
    o->polynum = (rv_s16)t16;
    if (fread(&t16, 2, 1, fp) != 1) return 0;
#ifdef RV360_BIG_ENDIAN
    t16 = rv_swap16(t16);
#endif
    o->vertnum = (rv_s16)t16;
    return 1;
}

typedef struct { float x, y, z, radius; rv_s32 cubenum; } rv_le_bigcubeheaderload;

static inline int rv_read_bigcubeheaderload(rv_le_bigcubeheaderload *o, FILE *fp) {
    if (!rv_fread_f32le(&o->x, fp)) return 0;
    if (!rv_fread_f32le(&o->y, fp)) return 0;
    if (!rv_fread_f32le(&o->z, fp)) return 0;
    if (!rv_fread_f32le(&o->radius, fp)) return 0;
    uint32_t t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->cubenum = (rv_s32)t32;
    return 1;
}

// --- world.h: TEXANIM_FRAME (s32 Tpage + 9f). WORLD_POLY_LOAD/VERTEX_LOAD ---
// --- reuse rv_le_pollload / rv_read_pollload and rv_le_vertexload / rv_read_vertexload (identical layouts) ---
typedef struct { rv_s32 tpage; float time, uv[8]; } rv_le_texanimframe;

static inline int rv_read_texanimframe(rv_le_texanimframe *o, FILE *fp) {
    uint32_t t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->tpage = (rv_s32)t32;
    if (!rv_fread_f32le(&o->time, fp)) return 0;
    for (int i = 0; i < 8; i++)
        if (!rv_fread_f32le(&o->uv[i], fp)) return 0;
    return 1;
}

// --- LevelLoad.h RECORD_ENTRY (_PC branch: Time s32, Player[16], Car[20]) ---
typedef struct { rv_s32 time; char player[16], car[20]; } rv_le_one_record;

static inline int rv_read_one_record(rv_le_one_record *o, FILE *fp) {
    uint32_t t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->time = (rv_s32)t32;
    if (fread(o->player, 1, 16, fp) != 16) return 0;
    if (fread(o->car, 1, 20, fp) != 20) return 0;
    return 1;
}

static inline int rv_write_one_record(const rv_le_one_record *o, FILE *fp) {
    if (!rv_fwrite_u32le((uint32_t)o->time, fp)) return 0;
    if (fwrite(o->player, 1, 16, fp) != 16) return 0;
    if (fwrite(o->car, 1, 20, fp) != 20) return 0;
    return 1;
}

// NOTE: game-side RECORD_ENTRY copy helpers live in the .cpp (struct types
// differ per platform); readers/writers above are the portable primitives.

// --- editai.h: FILE_AINODE (4 chars, 4 floats, 2 s32, 2+2 s32 links, 2x{s32,VEC}) ---
typedef struct {
    char priority, startnode, flags[2];
    float racingline, finishdist, overtakingline, fpad;
    rv_s32 racinglinespeed, centrespeed;
    rv_s32 prev[2], next[2];
    struct { rv_s32 speed; float pos[3]; } node[2];
} rv_le_ainode;

static inline int rv_read_ainode(rv_le_ainode *o, FILE *fp) {
    uint32_t t32;
    if (fread(&o->priority, 1, 1, fp) != 1) return 0;
    if (fread(&o->startnode, 1, 1, fp) != 1) return 0;
    if (fread(o->flags, 1, 2, fp) != 2) return 0;
    float *f[] = { &o->racingline, &o->finishdist, &o->overtakingline, &o->fpad };
    for (unsigned i = 0; i < 4; i++)
        if (!rv_fread_f32le(f[i], fp)) return 0;
    rv_s32 *s[] = { &o->racinglinespeed, &o->centrespeed,
                    &o->prev[0], &o->prev[1], &o->next[0], &o->next[1] };
    for (unsigned i = 0; i < 6; i++) {
        if (!rv_fread_u32le(&t32, fp)) return 0;
        *s[i] = (rv_s32)t32;
    }
    for (int n = 0; n < 2; n++) {
        if (!rv_fread_u32le(&t32, fp)) return 0;
        o->node[n].speed = (rv_s32)t32;
        if (!rv_read_f32n(o->node[n].pos, 3, fp)) return 0;
    }
    return 1;
}

// --- editzone.h: FILE_ZONE (s32 ID, VEC Pos, MAT Matrix, REAL Size[3]) ---
typedef struct { rv_s32 id; float pos[3], mat[9], size[3]; } rv_le_filezone;

static inline int rv_read_filezone(rv_le_filezone *o, FILE *fp) {
    uint32_t t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->id = (rv_s32)t32;
    if (!rv_read_f32n(o->pos, 3, fp)) return 0;
    if (!rv_read_f32n(o->mat, 9, fp)) return 0;
    if (!rv_read_f32n(o->size, 3, fp)) return 0;
    return 1;
}

// --- trigger.h: FILE_TRIGGER (s32 ID+Flag, VEC Pos, MAT Matrix, REAL Size[3]) ---
typedef struct { rv_s32 id, flag; float pos[3], mat[9], size[3]; } rv_le_filetrigger;

static inline int rv_read_filetrigger(rv_le_filetrigger *o, FILE *fp) {
    uint32_t t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->id = (rv_s32)t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->flag = (rv_s32)t32;
    if (!rv_read_f32n(o->pos, 3, fp)) return 0;
    if (!rv_read_f32n(o->mat, 9, fp)) return 0;
    if (!rv_read_f32n(o->size, 3, fp)) return 0;
    return 1;
}

// --- editcam.h: FILE_CAM_NODE (9 x s32; x/y/z are 16.16 fixed on disk) ---
typedef struct {
    rv_s32 type, x, y, z, zoomfactor, link, unused1, railtype, id;
} rv_le_filecamnode;

static inline int rv_read_filecamnode(rv_le_filecamnode *o, FILE *fp) {
    uint32_t t32;
    rv_s32 *s[] = { &o->type, &o->x, &o->y, &o->z, &o->zoomfactor,
                    &o->link, &o->unused1, &o->railtype, &o->id };
    for (unsigned i = 0; i < sizeof(s) / sizeof(s[0]); i++) {
        if (!rv_fread_u32le(&t32, fp)) return 0;
        *s[i] = (rv_s32)t32;
    }
    return 1;
}

// --- EditObject.h: FILE_OBJECT (s32 ID, s32 Flag[4], VEC Pos/Up/Look) ---
typedef struct {
    rv_s32 id, flag[4];
    float pos[3], up[3], look[3];
} rv_le_fileobject;

static inline int rv_read_fileobject(rv_le_fileobject *o, FILE *fp) {
    uint32_t t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->id = (rv_s32)t32;
    for (int i = 0; i < 4; i++) {
        if (!rv_fread_u32le(&t32, fp)) return 0;
        o->flag[i] = (rv_s32)t32;
    }
    if (!rv_read_f32n(o->pos, 3, fp)) return 0;
    if (!rv_read_f32n(o->up, 3, fp)) return 0;
    if (!rv_read_f32n(o->look, 3, fp)) return 0;
    return 1;
}

// --- ghost.h: GHOST_INFO (s32 CarType, char Name[16], u32 Time[11], s32 NFrames) ---
// (GHOST_HEADER is 8x char[32] — pure bytes, no swapping needed.)
typedef struct {
    rv_s32 cartype;
    char playername[16];
    rv_u32 time[11];
    rv_s32 nframes;
} rv_le_ghostinfo;

static inline int rv_read_ghostinfo(rv_le_ghostinfo *o, int ntimes, FILE *fp) {
    uint32_t t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->cartype = (rv_s32)t32;
    if (fread(o->playername, 1, 16, fp) != 16) return 0;
    for (int i = 0; i < ntimes; i++) {
        if (!rv_fread_u32le(&t32, fp)) return 0;
        o->time[i] = t32;
    }
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->nframes = (rv_s32)t32;
    return 1;
}

static inline int rv_write_ghostinfo(const rv_le_ghostinfo *o, int ntimes, FILE *fp) {
    if (!rv_fwrite_u32le((uint32_t)o->cartype, fp)) return 0;
    if (fwrite(o->playername, 1, 16, fp) != 16) return 0;
    for (int i = 0; i < ntimes; i++)
        if (!rv_fwrite_u32le(o->time[i], fp)) return 0;
    if (!rv_fwrite_u32le((uint32_t)o->nframes, fp)) return 0;
    return 1;
}

// --- ghost.h: GHOST_DATA (u32 Time, s8 WheelAngle, u8 WheelPos, s16 Pos[3], char Quat[4]) ---
typedef struct {
    rv_u32 time;
    signed char wheelangle;
    unsigned char wheelpos;
    rv_s16 pos[3];
    char quat[4];
} rv_le_ghostdata;

static inline int rv_read_ghostdata(rv_le_ghostdata *o, FILE *fp) {
    uint32_t t32;
    if (!rv_fread_u32le(&t32, fp)) return 0;
    o->time = t32;
    if (fread(&o->wheelangle, 1, 1, fp) != 1) return 0;
    if (fread(&o->wheelpos, 1, 1, fp) != 1) return 0;
    for (int i = 0; i < 3; i++)
        if (!rv_read_index(&o->pos[i], fp)) return 0;
    if (fread(o->quat, 1, 4, fp) != 4) return 0;
    return 1;
}

static inline int rv_write_ghostdata(const rv_le_ghostdata *o, FILE *fp) {
    if (!rv_fwrite_u32le(o->time, fp)) return 0;
    if (fwrite(&o->wheelangle, 1, 1, fp) != 1) return 0;
    if (fwrite(&o->wheelpos, 1, 1, fp) != 1) return 0;
    for (int i = 0; i < 3; i++)
        if (!rv_fwrite_u16le((uint16_t)o->pos[i], fp)) return 0;
    if (fwrite(o->quat, 1, 4, fp) != 4) return 0;
    return 1;
}

// --- posnode.h: FILE_POSNODE (VEC Pos, REAL Dist, s32 Prev[4], s32 Next[4]) ---
typedef struct {
    float pos[3], dist;
    rv_s32 prev[4], next[4];
} rv_le_fileposnode;

static inline int rv_read_fileposnode(rv_le_fileposnode *o, FILE *fp) {
    uint32_t t32;
    if (!rv_read_f32n(o->pos, 3, fp)) return 0;
    if (!rv_fread_f32le(&o->dist, fp)) return 0;
    for (int i = 0; i < 4; i++) {
        if (!rv_fread_u32le(&t32, fp)) return 0;
        o->prev[i] = (rv_s32)t32;
    }
    for (int i = 0; i < 4; i++) {
        if (!rv_fread_u32le(&t32, fp)) return 0;
        o->next[i] = (rv_s32)t32;
    }
    return 1;
}

// --- network.h:150-166 MSG_HEADER family (WORD Size is LE on wire) ---
static inline void rv_pack_msg_tcp(unsigned char out[4], unsigned type, unsigned ext, unsigned size) {
    out[0] = (unsigned char)(size & 0xFF);
    out[1] = (unsigned char)((size >> 8) & 0xFF);
    out[2] = (unsigned char)type;
    out[3] = (unsigned char)ext;
}

static inline void rv_unpack_msg_tcp(const unsigned char in[4], unsigned *type, unsigned *ext, unsigned *size) {
    *size = (unsigned)in[0] | ((unsigned)in[1] << 8);
    *type = in[2];
    *ext = in[3];
}

#endif
