
#ifndef CAMERA_TRANSFORM_H
#define CAMERA_TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pos_t Pos;

struct pos_t 
{
    double x;
    double y;
};

Pos px_to_pos(unsigned int px_x, unsigned int px_y, double z_plane);

#ifdef __cplusplus
}
#endif

#endif

