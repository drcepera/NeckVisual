#ifndef MAT_H
#define MAT_H

namespace mat{

#define BIG_INF 1e+99

typedef struct
{
    double begin[3];
    double end[3];
} stickCoords;

double **createMat3x3();
double** createMat(int n);
double** createModuleTransMat(double a, double b, double len);

void deleteMat3x3(double** mat);
void deleteMat(double** mat, int n);

void copyMat3x3(double** from, double** to);
void copyMat(double** from, double** to, int n);

void copyVect3(double* from, double* to);
void vectFloatToDouble(float* from, double* to, int n);
void vectDoubleToFloat(double* from, float* to, int n);

void FillRotateMatrixFromQuat(double** matrix, float quatA, float quatB, float quatC, float quatD);
void FillRotateMatrixFromVectorAndAngle(double** matrix, double *vector, double angle);
void FillMatrixWithElem(double** matrix, double elem);

int Matrix_X_Matrix(double **M1, double **M2, unsigned short n, double **Result);
int Matrix_X_Vector(double **M, double *v, unsigned short m, unsigned short n, double *Result);
int Vector3_Cross_Vector3(double *v1, double *v2, double *Result);

void FillRabg(double a, double b, double g, double **matrix);
double FindDifferenceBetweenRotateMatrices(double **M1, double **M2);
void GetQuatFromAngleAndVector(double angle, double* v, double* quat);
void NormalizeVector(double* from, double* to, int n);
void InvertRotateMat(double **m, double **inv);
int GetXyzFromXyzMatrix(double** matrix, double* xyz);
int GetXyzFromQuatMatrix(double** matrix, double* xyz);
int GetXzyFromQuatMatrix(double** matrix, double* xyz);
int GetXzyFromXzyMatrix(double** matrix, double* xzy);
double** CreateRotationMatrix(char* axes, double* angles);  // поиск матрицы вращения, последовательность осей задается axes ("zyx" z.B.),
    // соответствующие углы - angles

stickCoords findStickCoords(double* neckRotAngles, double moduleSnakeShift, double moduleShift, double outPostLength, double stickLength);

}

#endif // MAT_H
