/*
 * created by: Rene Reiter <renereiter@hotmail.com>
 *								www.spacesimulator.net
 *
 * modifed by: crol tony89@yandex.ry
 */

#pragma once

#include <3dsReader.h>


/**********************************************************
 *
 * FUNCTION Load3DS (obj_type_ptr, char *)
 *
 * This function loads a mesh from a 3ds file.
 * Please note that we are loading only the vertices, polygons and mapping lists.
 * If you need to load meshes with advanced features as for example: 
 * multi objects, materials, lights and so on, you must insert other chunk parsers.
 *
 *********************************************************/

extern char Load3DS (model_type_ptr ogg, char *filename);
extern char Copy3DS (model_type_ptr from, model_type_ptr to);

