// Copyright (c) 2015-2016 Sergio Gonzalez. All rights reserved.
// License: https://github.com/serge-rgb/milton#license
//

#include "canvas.h"

#include "common.h"
#include "canvas.h"
#include "memory.h"
#include "platform.h"
#include "utils.h"


v2i canvas_to_raster(CanvasView* view, v2i canvas_point)
{
    v2i raster_point = {
        ((view->pan_vector.x + canvas_point.x) / view->scale) + view->screen_center.x,
        ((view->pan_vector.y + canvas_point.y) / view->scale) + view->screen_center.y,
    };
    return raster_point;
}

v2i raster_to_canvas(CanvasView* view, v2i raster_point)
{
    v2i canvas_point = {
        ((raster_point.x - view->screen_center.x) * view->scale) - view->pan_vector.x,
        ((raster_point.y - view->screen_center.y) * view->scale) - view->pan_vector.y,
    };

    return canvas_point;
}

// Does point p0 with radius r0 contain point p1 with radius r1?
b32 stroke_point_contains_point(v2i p0, i32 r0, v2i p1, i32 r1)
{
    v2i d = sub2i(p1, p0);
    // using manhattan distance, less chance of overflow. Still works well enough for this case.
    u32 m = (u32)abs(d.x) + abs(d.y) + r1;
    //i32 m = magnitude_i(d) + r1;
    b32 contained = false;
    if ( r0 >= 0 ) {
        contained = (m < (u32)r0);
    } else {
        contained = true;
    }
    return contained;
}

Rect bounding_box_for_stroke(Stroke* stroke)
{
    Rect bb = bounding_rect_for_points(stroke->points, stroke->num_points);
    Rect bb_enlarged = rect_enlarge(bb, stroke->brush.radius);
    return bb_enlarged;
}

Rect bounding_box_for_last_n_points(Stroke* stroke, i32 last_n)
{
    i32 forward = max(stroke->num_points - last_n, 0);
    i32 num_points = min(last_n, stroke->num_points);
    Rect bb = bounding_rect_for_points(stroke->points + forward, num_points);
    Rect bb_enlarged = rect_enlarge(bb, stroke->brush.radius);
    return bb_enlarged;
}

Rect canvas_rect_to_raster_rect(CanvasView* view, Rect canvas_rect)
{
    Rect raster_rect;
    raster_rect.bot_right = canvas_to_raster(view, canvas_rect.bot_right);
    raster_rect.top_left = canvas_to_raster(view, canvas_rect.top_left);
    return raster_rect;
}

Layer* layer_get_topmost(Layer* root)
{
    Layer* layer = root;
    while ( layer->next ) {
        layer = layer->next;
    }
    return layer;
}
Layer* layer_get_by_id(Layer* root_layer, i32 id)
{
    for(Layer* layer = root_layer; layer; layer = layer->next) {
        if ( layer->id == id ) {
            return layer;
        }
    }
    return root_layer;
}

// Push stroke at the top of the current layer
Stroke* layer_push_stroke(Layer* layer, Stroke stroke)
{
    sb_push(layer->strokes, stroke);
    return &sb_peek(layer->strokes);
}

void layer_toggle_visibility(Layer* layer)
{
    b32 visible = layer->flags & LayerFlags_VISIBLE;
    if ( visible ) {
        layer->flags &= ~LayerFlags_VISIBLE;
    } else {
        layer->flags |= LayerFlags_VISIBLE;
    }
}

i32 number_of_layers(Layer* layer)
{
    int n = 0;
    while ( layer ) {
        ++n;
        layer = layer->next;
    }
    return n;
}

void stroke_free(Stroke* stroke)
{
    mlt_free(stroke->points);
    mlt_free(stroke->pressures);
}
