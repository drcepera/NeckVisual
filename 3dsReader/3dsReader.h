/*
 * 3created by: Rene Reiter <renereiter@hotmail.com>
 *								www.spacesimulator.net
 *
 * modifed by: crol tony89@yandex.ry
 */


#pragma once

#include <QtCore>

/**********************************************************
 *
 * TYPES DECLARATION
 *
 *********************************************************/

//#define MAX_VERTICES 800000ul // Max number of vertices (for each object)
//#define MAX_POLYGONS 800000ul // Max number of polygons (for each object)

const unsigned long int MAX_VERTICES = 800000;
const unsigned long int MAX_POLYGONS = 800000;

// Our vertex type
typedef struct{
    float xyz[3];
}vertex_type;

// Our normal type
typedef struct{
    float xyz[3];
}normal_type;

// The polygon (triangle), 3 numbers that aim 3 vertices
typedef struct{
    unsigned short abc[3];
}polygon_type;

// The mapcoord type, 2 texture coordinates for each vertex
typedef struct{
    float uv[2];
}mapcoord_type;

// The object type
typedef struct {
    char name[30];
    
    long int vertices_qty;
    long int polygons_qty;

    QList<vertex_type> vertexList;
    QList<normal_type> normalList;
    QList<polygon_type> polygonList;
    QList<mapcoord_type> mapcoordList;
//    vertex_type vertex[MAX_VERTICES];
//    polygon_type polygon[MAX_POLYGONS];
//    mapcoord_type mapcoord[MAX_VERTICES];
    int id_texture;
} model_type, *model_type_ptr;
