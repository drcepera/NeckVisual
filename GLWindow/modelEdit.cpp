#include "modelEdit.h"

void vectFloatToDouble(float* from, double* to, int n)
{
    for(int i=0; i<n; i++)
    {
        to[i] = from[i];
    }
}

// умножить матрицу [m x n] на вектор [n x 1]
int Matrix_X_Vector(double **M, double *v, unsigned short m, unsigned short n, double *Result)
{
    double sum;

    for(int i=0; i<m; i++)
    {
        sum = 0;
        for(int j=0; j<n; j++)
        {
            sum += M[i][j] * v[j];
        }
        Result[i] = sum;
    }
}

void vectDoubleToFloat(double* from, float* to, int n)
{
    for(int i=0; i<n; i++)
    {
        to[i] = from[i];
    }
}

void rotate(model_type_ptr model, double angle, double x, double y, double z)
{
    double** rotMatrix = mat::createMat3x3();
    double vect[3] = {x,y,z};
    mat::FillRotateMatrixFromVectorAndAngle(rotMatrix, vect, angle);

    double vectDouble[3];
    double tempVect[3];
    for(long int i = 0; i<model->vertices_qty; i++)
    {
//        for(int i=0; i<3; i++)
//        {
//            vectDouble[i] = model->vertexList[i].xyz[i];
//        }
        vectFloatToDouble(model->vertexList[i].xyz, vectDouble, 3);
        Matrix_X_Vector(rotMatrix, vectDouble, 3, 3, tempVect);
        vectDoubleToFloat(tempVect, model->vertexList[i].xyz, 3);
    }

//    for(long int i = 0; i<model->polygons_qty; i++)
//    {
//        mat::vectFloatToDouble(model->normalList[i].xyz, vectDouble, 3);
//        mat::Matrix_X_Vector(rotMatrix, vectDouble, 3, 3, tempVect);
//        mat::vectDoubleToFloat(tempVect, model->normalList[i].xyz, 3);
//    }
}

void translate(model_type_ptr model, double x, double y, double z)
{
    for(long int i = 0; i<model->vertices_qty; i++)
    {
        model->vertexList[i].xyz[0] += x;
        model->vertexList[i].xyz[1] += y;
        model->vertexList[i].xyz[2] += z;
    }
}

float FindBeaconAngle(float x, float y, float z)
{
    return atan2(y, x);
}
