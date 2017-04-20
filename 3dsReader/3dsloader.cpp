/*
 * created by: Rene Reiter <renereiter@hotmail.com>
 *								www.spacesimulator.net
 *
 * modifed by: crol tony89@yandex.ry
 */


#include <QtCore>

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include "3dsReader.h"
#include "3dsloader.h"

void norm(float* n)
{
    double nm=0;
    for (int i=0; i<3; i++) nm += n[i]*n[i];
    nm = sqrt(nm);
    for (int i=0; i<3; i++) n[i] = n[i]/nm;
}

void cross(float* a,float* b, float* c, float* n)
{
    n[0] = (b[1]-a[1])*(c[2]-a[2])-(b[2]-a[2])*(c[1]-a[1]);
    n[1] = (b[2]-a[2])*(c[0]-a[0])-(b[0]-a[0])*(c[2]-a[2]);
    n[2] = (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]);
    norm(n);
}

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

char Load3DS (model_type_ptr p_object, char *p_filename)
{
    long int i; //Index variable
	
	FILE *l_file; //File pointer
	
	unsigned short l_chunk_id; //Chunk identifier
	unsigned int l_chunk_lenght; //Chunk lenght

	unsigned char l_char; //Char variable
	unsigned short l_qty; //Number of elements in each chunk

	unsigned short l_face_flags; //Flag that stores some face information

	if ((l_file=fopen (p_filename, "rb"))== NULL) return 0; //Open the file

	while (ftell (l_file) < filelength (fileno (l_file))) //Loop to scan the whole file 
	{
		//getche(); //Insert this command for debug (to wait for keypress for each chuck reading)

		fread (&l_chunk_id, 2, 1, l_file); //Read the chunk header
		fread (&l_chunk_lenght, 4, 1, l_file); //Read the lenght of the chunk

		switch (l_chunk_id)
        {
			//----------------- MAIN3DS -----------------
			// Description: Main chunk, contains all the other chunks
			// Chunk ID: 4d4d 
			// Chunk Lenght: 0 + sub chunks
			//-------------------------------------------
			case 0x4d4d: 
			break;    

			//----------------- EDIT3DS -----------------
			// Description: 3D Editor chunk, objects layout info 
			// Chunk ID: 3d3d (hex)
			// Chunk Lenght: 0 + sub chunks
			//-------------------------------------------
			case 0x3d3d:
			break;
			
			//--------------- EDIT_OBJECT ---------------
			// Description: Object block, info for each object
			// Chunk ID: 4000 (hex)
			// Chunk Lenght: len(object name) + sub chunks
			//-------------------------------------------
			case 0x4000: 
				i=0;
				do
				{
					fread (&l_char, 1, 1, l_file);
                    p_object->name[i]=l_char;
					i++;
                }while(l_char != '\0' && i<20);
			break;

			//--------------- OBJ_TRIMESH ---------------
			// Description: Triangular mesh, contains chunks for 3d mesh info
			// Chunk ID: 4100 (hex)
			// Chunk Lenght: 0 + sub chunks
			//-------------------------------------------
			case 0x4100:
			break;
			
			//--------------- TRI_VERTEXL ---------------
			// Description: Vertices list
			// Chunk ID: 4110 (hex)
			// Chunk Lenght: 1 x unsigned short (number of vertices) 
			//             + 3 x float (vertex coordinates) x (number of vertices)
			//             + sub chunks
			//-------------------------------------------
			case 0x4110: 
				fread (&l_qty, sizeof (unsigned short), 1, l_file);
                p_object->vertices_qty = l_qty;
                printf("Number of vertices: %d\n",l_qty);
                for (i=0; i<l_qty; i++)
                {
                    vertex_type ver;
                    fread (&ver.xyz[0], sizeof(float), 1, l_file);
                    fread (&ver.xyz[1], sizeof(float), 1, l_file);
                    fread (&ver.xyz[2], sizeof(float), 1, l_file);
                    p_object->vertexList.append(ver);
				}
				break;

			//--------------- TRI_FACEL1 ----------------
			// Description: Polygons (faces) list
			// Chunk ID: 4120 (hex)
			// Chunk Lenght: 1 x unsigned short (number of polygons) 
			//             + 3 x unsigned short (polygon points) x (number of polygons)
			//             + sub chunks
			//-------------------------------------------
			case 0x4120:
				fread (&l_qty, sizeof (unsigned short), 1, l_file);
                p_object->polygons_qty = l_qty;
                printf("Number of polygons: %d\n",l_qty); 
                for (i=0; i<l_qty; i++)
                {
                    polygon_type polygon;
                    fread (&polygon.abc[0], sizeof (unsigned short), 1, l_file);
                    fread (&polygon.abc[1], sizeof (unsigned short), 1, l_file);
                    fread (&polygon.abc[2], sizeof (unsigned short), 1, l_file);
                    fread (&l_face_flags, sizeof (unsigned short), 1, l_file);

                    p_object->polygonList.append(polygon);

                    normal_type normal;
                    cross(  p_object->vertexList[p_object->polygonList[i].abc[0]].xyz,
                            p_object->vertexList[p_object->polygonList[i].abc[1]].xyz,
                            p_object->vertexList[p_object->polygonList[i].abc[2]].xyz,
                            normal.xyz);
                    p_object->normalList.append(normal);
				}
                break;

			//------------- TRI_MAPPINGCOORS ------------
			// Description: Vertices list
			// Chunk ID: 4140 (hex)
			// Chunk Lenght: 1 x unsigned short (number of mapping points) 
			//             + 2 x float (mapping coordinates) x (number of mapping points)
			//             + sub chunks
			//-------------------------------------------
			case 0x4140:
				fread (&l_qty, sizeof (unsigned short), 1, l_file);
				for (i=0; i<l_qty; i++)
				{
                    mapcoord_type mapcoord;
                    fread (&mapcoord.uv[0], sizeof (float), 1, l_file);
                    fread (&mapcoord.uv[1], sizeof (float), 1, l_file);
                    p_object->mapcoordList.append(mapcoord);
				}
                break;

			//----------- Skip unknow chunks ------------
			//We need to skip all the chunks that currently we don't use
			//We use the chunk lenght information to set the file pointer
			//to the same level next chunk
			//-------------------------------------------
			default:
				 fseek(l_file, l_chunk_lenght-6, SEEK_CUR);
        } 
	}
	fclose (l_file); // Closes the file stream
	return (1); // Returns ok
}


extern char Copy3DS (model_type_ptr from, model_type_ptr to)
{
    to->vertices_qty = from->vertices_qty;
    to->vertexList = from->vertexList;
    to->polygons_qty = from->polygons_qty;
    to->polygonList = from->polygonList;
    to->id_texture = from->id_texture;
    to->mapcoordList = from->mapcoordList;
    to->normalList = from->normalList;

    int i=0;
    char *copy = "_copy";
    for(; (char)from->name[i] != (char)'\0'; i++)
    {
        if(i >= (sizeof(from->name)-7))
            break;
        to->name[i] = from->name[i];
    }
    for(int j=0; i < (sizeof(from->name)-2), j<5; i++, j++)
    {
        to->name[i] = copy[j];
    }
    to->name[i] = '\0';
}
