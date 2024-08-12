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
* Original source: https://github.com/argonautcode/animal-proc-anim/blob/main/Snake.pde
*/

#ifndef SNAKE_H
#define SNAKE_H

#include <libdragon.h>
#include "chain.h"


typedef struct {
    Chain* spine;
    Point center;
    float* bodyWidth;
    color_t color;
} Snake;

Snake* snake1;
Snake* snake2;
Snake* snake3;
Snake* snake4;

void snake_init(Snake* snake, Point origin, int jointCount, color_t color) {

    // Allocate memory for the "spine" of the snake
    snake->spine = (Chain*)malloc_uncached(sizeof(Chain));
    chain_init(snake->spine, origin, jointCount, 4, M_PI / (jointCount/4)); 

    // Allocate an array of floats to hold the widths of different sections
    snake->bodyWidth = (float*)malloc(sizeof(float) * jointCount); 
    float* tempBodyWidth = (float*)malloc(sizeof(float) * jointCount);

    // First 4 widths shape the snake head
    snake->bodyWidth[0] = 4.5f;
    for (int i = 1; i < 3; ++i) {
        tempBodyWidth[i] = snake->bodyWidth[0] + i;
        snake->bodyWidth[i] = tempBodyWidth[i];
    }

    snake->bodyWidth[3] = 4.0f; // "Neck" joint

    // For every other joint, slowly taper the width
    for (int i = 4; i < jointCount; ++i) {
        tempBodyWidth[i] = 5.0f - (i * 0.1f);
        if(tempBodyWidth[i] < 2.0f)tempBodyWidth[i] = 2.0f;
        snake->bodyWidth[i] = tempBodyWidth[i];
    }

    free(tempBodyWidth);

    snake->color = color;

}

void snake_resolve(Snake* snake, float stickX, float stickY) {

    Point headPos = snake->spine->joints->points[0];

    // Apply deadzone to the joystick inputs
    float adjustedX = apply_deadzone(stickX);
    float adjustedY = apply_deadzone(stickY);

    // Normalize the joystick input to get the direction
    Point direction = point_new(adjustedX, -adjustedY);
    point_normalize(&direction);

    // Determine the target position based on the direction and a fixed magnitude
    float move_mag = 3.5f;
    direction = point_set_mag(&direction, move_mag);
    Point targetPos = point_sub(&headPos, &direction);
    chain_resolve(snake->spine, targetPos);
}

float snake_get_body_width(Snake* snake, int i) {
    return snake->bodyWidth[i];
}

float snake_get_posX(Snake* snake, int i, float angleOffset, float lengthOffset) {
    return snake->spine->joints->points[i].x + fm_cosf(snake->spine->angles[i] + angleOffset) * (snake->bodyWidth[i] + lengthOffset);
}

float snake_get_posY(Snake* snake, int i, float angleOffset, float lengthOffset) {
    return snake->spine->joints->points[i].y + fm_sinf(snake->spine->angles[i] + angleOffset) * (snake->bodyWidth[i] + lengthOffset);
}


void draw_snake_shape(Snake* snake) {

    int vertexCount = 0;
    // 2 verts per joint (1 for each side), as well as 1 vert for each the start, midpoint, and end of the loop
    int maxVertices = (snake->spine->joints->count * 2) + 3; 

    Point* vertices = (Point*)malloc(maxVertices * sizeof(Point));
    
    if (!vertices) {
        debugf("Vertics allocation falied\n");
        return;
    }

    // Right half of the snake
    for (size_t i = 0; i < snake->spine->joints->count; i++) {
      vertices[vertexCount].x = snake_get_posX(snake, i, M_PI / 2, 0);
      vertices[vertexCount].y = snake_get_posY(snake, i, M_PI / 2, 0);
      vertexCount++;
    }

    // Top of the head (completes the loop)
    vertices[vertexCount].x = snake_get_posX(snake, snake->spine->joints->count - 1, M_PI, 0);
    vertices[vertexCount].y = snake_get_posY(snake, snake->spine->joints->count - 1, M_PI, 0);
    vertexCount++;

    // Left half of the snake
    for (int i = snake->spine->joints->count - 1; i >= 1; --i) {
      vertices[vertexCount].x = snake_get_posX(snake, i, -M_PI / 2, 0);
      vertices[vertexCount].y = snake_get_posY(snake, i, -M_PI / 2, 0);
      vertexCount++;
    }

    // Add vertices to complete the loop
    vertices[vertexCount].x = snake_get_posX(snake, 0, -M_PI / 6, 0);
    vertices[vertexCount].y = snake_get_posY(snake, 0, -M_PI / 6, 0);
    vertexCount++;
    vertices[vertexCount].x = snake_get_posX(snake, 0, 0, 0);
    vertices[vertexCount].y = snake_get_posY(snake, 0, 0, 0);
    vertexCount++;
    vertices[vertexCount].x = snake_get_posX(snake, 0, M_PI / 6, 0);
    vertices[vertexCount].y = snake_get_posY(snake, 0, M_PI / 6, 0);
    vertexCount++;

    // Calculate the center of the shape
    Point center = point_default();
    for (int i = 0; i < vertexCount; ++i) {
        center.x += vertices[i].x;
        center.y += vertices[i].y;
    }
    center.x /= vertexCount;
    center.y /= vertexCount;

    // Scale the vertices outward for shadow
    float e = 0.1f;
    Point* scaled_vertices = (Point*)malloc(vertexCount * sizeof(Point));
    if (!scaled_vertices) {
        debugf("Scaled vertices allocation failed\n");
        free(vertices);
        return;
    }
    float scale = 1.0f + e;
    for (int i = 0; i < vertexCount; ++i) {
        scaled_vertices[i] = point_scale(&center, &vertices[i], scale);
    }

    // Z depths for the different shapes
    float bodyZ = 0.8f;
    float shadowZ = 0.6f;

    for (int i = 0; i < snake->spine->joints->count - 1; ++i) {

        float v1S[] = { scaled_vertices[i].x, scaled_vertices[i].y, shadowZ };
        float v2S[] = { scaled_vertices[i + 1].x, scaled_vertices[i + 1].y, shadowZ };
        float v3S[] = { scaled_vertices[vertexCount - 1 - i].x, scaled_vertices[vertexCount - 1 - i].y, shadowZ };
        float v4S[] = { scaled_vertices[vertexCount - 2 - i].x, scaled_vertices[vertexCount - 2 - i].y, shadowZ };

        // Draw drop shadow
        set_render_color(BLACK);
        draw_strip(v1S,v2S,v3S,v4S); 
    }

    for (int i = 0; i < snake->spine->joints->count - 1; ++i) {
        // Draw a triangle between two points on the right edge and two points on the left edge
        float v1[] = { vertices[i].x, vertices[i].y, bodyZ };
        float v2[] = { vertices[i + 1].x, vertices[i + 1].y, bodyZ };
        float v3[] = { vertices[vertexCount - 1 - i].x, vertices[vertexCount - 1 - i].y, bodyZ };
        float v4[] = { vertices[vertexCount - 2 - i].x, vertices[vertexCount - 2 - i].y, bodyZ };
        
        // Draw snake body
        set_render_color(snake->color); 
        draw_strip(v1,v2,v3,v4);     
        
    }


    // Draw eye outline
    set_render_color(DARK_GREEN);
    draw_circle(snake_get_posX(snake, 0, M_PI / 2, -2), snake_get_posY(snake, 0, M_PI / 2, -2), 2.0f, 1.0f, 0.0f, 0.05f);
    draw_circle(snake_get_posX(snake, 0, -M_PI / 2, -2), snake_get_posY(snake,0, -M_PI / 2, -2), 2.0f, 1.0f, 0.0f, 0.05f);

    // Draw eyes
    set_render_color(GREEN);
    draw_circle(snake_get_posX(snake, 0, M_PI / 2, -2), snake_get_posY(snake, 0, M_PI / 2, -2), 1.0f, 1.0f, 0.0f, 0.05f);
    draw_circle(snake_get_posX(snake, 0, -M_PI / 2, -2), snake_get_posY(snake, 0, -M_PI / 2, -2), 1.0f, 1.0f, 0.0f, 0.05f);

    // Free vertices after drawing
    free(vertices);
    free(scaled_vertices);

}

#endif // SNAKE_H