
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "camera_transform.hpp"

#define PX_WIDTH (640)
#define PX_HEIGHT (480)

#define CAM_HEIGHT (0.32486)
#define CAM_X (0.08885)
#define IMG_X_BOTTOM (0.225)
#define IMG_X_TOP (0.575)
#define CALIB_LINE_X (0.325) // estimated from test image
#define CALIB_LINE_WIDTH (0.3125) // estimated from test image

static double theta_bottom;
static double theta_top;
static double theta_line;
static double camera_vertical_opening_angle;
static double camera_horizontal_opening_angle;
static double dist_to_center_of_horiz_line;

static double horiz_rd_per_px;
static double vert_rd_per_px;

static void init(void)
{
    theta_bottom = atan2((IMG_X_BOTTOM - CAM_X),CAM_HEIGHT);
    theta_top = atan2((IMG_X_TOP - CAM_X),CAM_HEIGHT);
    camera_vertical_opening_angle = theta_top - theta_bottom;

    dist_to_center_of_horiz_line = sqrt(CALIB_LINE_X * CALIB_LINE_X + CAM_HEIGHT * CAM_HEIGHT);

    camera_horizontal_opening_angle = 2 * atan2(CALIB_LINE_WIDTH/2, dist_to_center_of_horiz_line);

    horiz_rd_per_px = camera_horizontal_opening_angle / PX_WIDTH;
    vert_rd_per_px = camera_vertical_opening_angle / PX_HEIGHT;

    //printf("\n");
    //printf("base: %f, top: %f, horiz_opening: %f\n",theta_bottom,theta_top,camera_horizontal_opening_angle);
}

Pos px_to_pos(unsigned int px_x, unsigned int px_y, double z_plane)
{
    //printf("px_x: %u, px_y: %u\n",px_x,px_y);
    static bool is_init = false;
    if(!is_init)
    {
        init();
        is_init = true;
    }

    double vert_angle = theta_bottom + vert_rd_per_px * (PX_HEIGHT - px_y);
    double horiz_angle = -(((double)px_x - (PX_WIDTH/2)) * horiz_rd_per_px);

    double dir_x = cos(horiz_angle) * sin(vert_angle);
    double dir_y = sin(horiz_angle) * sin(vert_angle);
    double dir_z = cos(vert_angle);
    double ddir = sqrt(dir_x*dir_x + dir_y*dir_y + dir_z*dir_z);

    dir_x = dir_x / ddir;
    dir_y = dir_y / ddir;
    dir_z = dir_z / ddir;

    //printf("vert_angle: %f, horiz_angle: %f\n",vert_angle,horiz_angle);

    double d = (CAM_HEIGHT - z_plane) / dir_z; 

    double x_res = d * dir_x;
    double y_res = d * dir_y;

    Pos result;
    result.x = CAM_X + x_res;
    result.y = y_res;

    //printf("retx: %f, rety: %f\n",x_res,y_res);
    return result;
}

