/*
* This file includes code from the animal-proc-anim project.
* animal-proc-anim is licensed under the MIT License.
* See the LICENSES directory for the full text of the MIT License.
*
* Original code by argonaut 
* Adapted by s4ys
* August 2024
*
* Description of changes or adaptations made:
* - Port from PDE to C
* - Focusing on optimization for N64
*
*
* Original source: https://github.com/argonautcode/animal-proc-anim/blob/main/Chain.pde
*/

#ifndef CHAIN_H
#define CHAIN_H

#include <libdragon.h>
#include "control.h"

typedef struct {
    PointArray* joints;
    Point center;
    float* angles;
    float angleConstraint;
    int linkSize;
} Chain;

Chain* chain;

void chain_init(Chain* chain, Point origin, size_t jointCount, int linkSize, float angleConstraint) {
    chain->center = origin;
    chain->angleConstraint = angleConstraint;
    chain->linkSize = linkSize;

    // Allocate memory for the PointArray structure itself
    chain->joints = (PointArray*)malloc_uncached(sizeof(PointArray));
    init_point_array(chain->joints);

    // Allocate memory for the initial point (origin)
    add_existing_point(chain->joints, origin);

    // Allocate memory for the angles array
    chain->angles = (float*)malloc_uncached(sizeof(float) * (jointCount - 1));

    Point offset = point_new(0, chain->linkSize);

    // Reallocate space for each additional joint and add it to the array
    for (size_t i = 1; i < jointCount; i++) {
        Point jointDiff = point_sub(&chain->joints->points[i - 1], &offset);
        add_existing_point(chain->joints, jointDiff);
        chain->angles[i-1] = 0.0f;
    }
}

void chain_resolve(Chain* chain, Point pos) {
    if (chain->joints->count > 0) {
        chain->angles[0] = point_heading(point_sub(&pos, &chain->joints->points[1]));
        chain->joints->points[0] = pos;

        float precomputedLinkSize = chain->linkSize; // Assuming chain->linkSize doesn't change
        float precomputedConstraint = chain->angleConstraint; // Assuming chain->angleConstraint doesn't change

        for (size_t i = 1; i < chain->joints->count; i++) {
            // Use precomputed values
            float currAngle = point_heading(point_sub(&chain->joints->points[i - 1], &chain->joints->points[i]));
            chain->angles[i] = constrain_angle(currAngle, chain->angles[i - 1], precomputedConstraint);
            Point anglePoint = point_from_angle(chain->angles[i]);
            Point offset = point_set_mag(&anglePoint, precomputedLinkSize);
            chain->joints->points[i] = point_sub(&chain->joints->points[i - 1], &offset);
        }
    } else {
        debugf("Error: Chain has no joints\n");
    }
}

void chain_fabrik_resolve(Chain* chain, Point pos, Point anchor){
    chain->joints->points[0] = pos;

    // Forward pass
    for (int i = 1; i < chain->joints->count; i++) {
        chain->joints->points[i] = constrain_distance(chain->joints->points[i], chain->joints->points[i-1], chain->linkSize);
    }

    // Backward pass
    chain->joints->points[chain->joints->count - 1] = anchor;
    for (int i = chain->joints->count - 2; i >= 0; i--) {
        chain->joints->points[i] = constrain_distance(chain->joints->points[i], chain->joints->points[i+1], chain->linkSize);
    }

}


void chain_display(Chain* chain, float width) {
    if (chain == NULL || chain->joints == NULL || chain->joints->points == NULL) {
        debugf("Chain or joints are not properly initialized\n");
        return;
    }

    int jointCount = chain->joints->count;
    if (jointCount < 4) {
        debugf("Not enough joints to display Bézier curves\n");
        return;
    }


    // Loop through joints in groups of 4 to draw Bézier curves
    set_render_color(YELLOW);
    for (int i = 0; i < jointCount - 3; i += 3) { // Increment by 3 for the Bézier curves
        // Control points for the Bézier curve
        Point* p0 = &chain->joints->points[i];
        Point* p1 = &chain->joints->points[i + 1];
        Point* p2 = &chain->joints->points[i + 2];
        Point* p3 = &chain->joints->points[i + 3];

        set_render_color(BLACK);
        draw_bezier_curve(p0, p1, p2, p3, 1, 0.0f, width*2.0f);
        set_render_color(YELLOW);
        draw_bezier_curve(p0, p1, p2, p3, 1, 0.0f, width);
    }
}

#endif // CHAIN_H