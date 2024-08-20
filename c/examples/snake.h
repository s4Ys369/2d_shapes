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

#define SNAKE_SEGMENTS 32
#define SNAKE_MAX_VERTS (SNAKE_SEGMENTS*4)

typedef struct {
    Chain* spine;
    Point center;
    float* bodyWidth;
    color_t color;
} Snake;

Snake* snake1;
Point* snake1Verts;
Point* snake1ShadowVerts;

Snake* snake2;
Point* snake2Verts;
Point* snake2ShadowVerts;

Snake* snake3;
Point* snake3Verts;
Point* snake3ShadowVerts;

Snake* snake4;
Point* snake4Verts;
Point* snake4ShadowVerts;

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
    float move_mag = 3.0f;
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


void draw_snake_shape(Snake* snake, Point* verts, Point* shadowVerts) {
    int vertexCount = 0;

    Point* vertices = verts;
    Point* scaled_vertices = shadowVerts;

    // Right half of the snake
    for (size_t i = 0; i < snake->spine->joints->count; i++) {
        vertices[vertexCount].x = snake_get_posX(snake, i, M_PI / 2, 0);
        vertices[vertexCount].y = snake_get_posY(snake, i, M_PI / 2, 0);
        vertexCount++;
    }

    // Top of the head
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
    float scale = 1.0f + e;
    for (int i = 0; i < vertexCount; ++i) {
        scaled_vertices[i] = point_scale(&center, &vertices[i], scale);
    }

    // Draw drop shadow and snake body
    for (int i = 0; i < snake->spine->joints->count - 1; ++i) {
        float v1S[] = { scaled_vertices[i].x, scaled_vertices[i].y };
        float v2S[] = { scaled_vertices[i + 1].x, scaled_vertices[i + 1].y };
        float v3S[] = { scaled_vertices[vertexCount - 1 - i].x, scaled_vertices[vertexCount - 1 - i].y };
        float v4S[] = { scaled_vertices[vertexCount - 2 - i].x, scaled_vertices[vertexCount - 2 - i].y };

        // Draw drop shadow
        set_render_color(T_BLACK);
        draw_strip(v1S, v2S, v3S, v4S);
    }

    // It is necessary to draw the shadow and body in separate loops, or they interlace
    for (int i = 0; i < snake->spine->joints->count - 1; ++i) {
        // Draw snake body
        float v1[] = { vertices[i].x, vertices[i].y };
        float v2[] = { vertices[i + 1].x, vertices[i + 1].y };
        float v3[] = { vertices[vertexCount - 1 - i].x, vertices[vertexCount - 1 - i].y };
        float v4[] = { vertices[vertexCount - 2 - i].x, vertices[vertexCount - 2 - i].y };
        
        set_render_color(snake->color); 
        draw_strip(v1, v2, v3, v4);
    }

    // Draw eyes
    float rightEyeOffsetX = snake_get_posX(snake, 0, M_PI / 2, -2);
    float rightEyeOffsetY = snake_get_posY(snake, 0, M_PI / 2, -2);
    float leftEyeOffsetX = snake_get_posX(snake, 0, -M_PI / 2, -2);
    float leftEyeOffsetY = snake_get_posY(snake, 0, -M_PI / 2, -2);

    set_render_color(DARK_GREEN);
    draw_circle(rightEyeOffsetX, rightEyeOffsetY, 2.0f, 1.0f, 0.0f, 0.05f);
    draw_circle(leftEyeOffsetX, leftEyeOffsetY, 2.0f, 1.0f, 0.0f, 0.05f);

    set_render_color(GREEN);
    draw_circle(rightEyeOffsetX, rightEyeOffsetY, 1.0f, 1.0f, 0.0f, 0.05f);
    draw_circle(leftEyeOffsetX, leftEyeOffsetY, 1.0f, 1.0f, 0.0f, 0.05f);
}

void init_snakes(){
    snake1 = (Snake*)malloc_uncached(sizeof(Snake));
    snake_init(snake1, screenCenter, SNAKE_SEGMENTS, RED);
    snake1Verts = malloc(sizeof(Point) * SNAKE_MAX_VERTS);
    snake1ShadowVerts = malloc(sizeof(Point) * SNAKE_MAX_VERTS);

    snake2 = (Snake*)malloc_uncached(sizeof(Snake));
    snake_init(snake2, screenCenter, SNAKE_SEGMENTS, BLUE);
    snake2Verts = malloc(sizeof(Point) * SNAKE_MAX_VERTS);
    snake2ShadowVerts = malloc(sizeof(Point) * SNAKE_MAX_VERTS);

    snake3 = (Snake*)malloc_uncached(sizeof(Snake));
    snake_init(snake3, screenCenter, SNAKE_SEGMENTS, ORANGE);
    snake3Verts = malloc(sizeof(Point) * SNAKE_MAX_VERTS);
    snake3ShadowVerts = malloc(sizeof(Point) * SNAKE_MAX_VERTS);

    snake4 = (Snake*)malloc_uncached(sizeof(Snake));
    snake_init(snake4, screenCenter, SNAKE_SEGMENTS, INDIGO);
    snake4Verts = malloc(sizeof(Point) * SNAKE_MAX_VERTS);
    snake4ShadowVerts = malloc(sizeof(Point) * SNAKE_MAX_VERTS);
}

void draw_snakes(){
    snake_resolve(snake1, stickX, stickY);
    draw_snake_shape(snake1, snake1Verts, snake1ShadowVerts);

    snake_resolve(snake2, -stickX, -stickY);
    draw_snake_shape(snake2, snake2Verts, snake2ShadowVerts);

    snake_resolve(snake3, -stickX, stickY);
    draw_snake_shape(snake3, snake3Verts, snake3ShadowVerts);

    snake_resolve(snake4, stickX, -stickY);
    draw_snake_shape(snake4, snake4Verts, snake4ShadowVerts);
    
}

#endif // SNAKE_H