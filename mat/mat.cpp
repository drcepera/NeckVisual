#include "mat.h"
#include "math.h"

static double prevShXyz[3] = {0,0,0};
static double prevElXzy[3] = {0,0,0};

namespace mat{

void fillRotMatrixX(double** m, double angle);
void fillRotMatrixY(double** m, double angle);
void fillRotMatrixZ(double** m, double angle);

void norm3v(double* vect)
{
    double s = sqrt(vect[0]*vect[0] + vect[1]*vect[1] + vect[2]*vect[2]);
    vect[0] = vect[0]/s;
    vect[1] = vect[1]/s;
    vect[2] = vect[2]/s;
}

// создает матрицу 3x3, заполняет нулями
double** createMat3x3()
{
    double** matrix = new double*[3];
    for (int i=0; i<3; i++)
    {
        matrix[i] = new double[3];
        matrix[i][0] = matrix[i][1] = matrix[i][2] = 0;
    }
    return matrix;
}

double** createMat(int n)
{
    double** matrix = new double*[n];
    for (int i=0; i<n; i++)
    {
        matrix[i] = new double[n];
        for (int j=0; j<n; j++)
        {
            matrix[i][j] = 0;
        }

    }
    return matrix;
}

double** createModuleTransMat(double a, double b, double len)
{
    double ca = cos(a);
    double sa = sin(a);
    double cb = cos(b);
    double sb = sin(b);

    // module translation matrix
    double **moduleTransMat = createMat(4);
    moduleTransMat[0][0] = ca;
    moduleTransMat[0][1] = sa*sb;
    moduleTransMat[0][2] = sa*cb;
    moduleTransMat[0][3] = 0;

    moduleTransMat[1][0] = 0;
    moduleTransMat[1][1] = cb;
    moduleTransMat[1][2] = -sb;
    moduleTransMat[1][3] = 0;

    moduleTransMat[2][0] = -sa;
    moduleTransMat[2][1] = ca*sb;
    moduleTransMat[2][2] = ca*cb;
    moduleTransMat[2][3] = len;

    moduleTransMat[3][0] = 0;
    moduleTransMat[3][1] = 0;
    moduleTransMat[3][2] = 0;
    moduleTransMat[3][3] = 1;

    return moduleTransMat;
}

void deleteMat3x3(double** mat)
{
    for(int i=0; i<3; i++)
    {
        delete[] mat[i];
    }
    delete[] mat;
}

void deleteMat(double** mat, int n)
{
    for(int i=0; i<n; i++)
    {
        delete[] mat[i];
    }
    delete[] mat;
}

void copyMat3x3(double** from, double** to)
{
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            to[i][j] = from[i][j];
        }
    }
}

void copyMat(double** from, double** to, int n)
{
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++)
        {
            to[i][j] = from[i][j];
        }
    }
}

void copyVect3(double* from, double* to)
{
    for(int i=0; i<3; i++)
    {
        to[i] = from[i];
    }
}

void vectFloatToDouble(float* from, double* to, int n)
{
    for(int i=0; i<n; i++)
    {
        to[i] = from[i];
    }
}

void vectDoubleToFloat(double* from, float* to, int n)
{
    for(int i=0; i<n; i++)
    {
        to[i] = from[i];
    }
}

void FillRotateMatrixFromQuat(double** matrix, float quatA, float quatB, float quatC, float quatD)
{
    matrix[0][0] = 1 - 2*quatC*quatC - 2*quatD*quatD;
    matrix[1][1] = 1 - 2*quatB*quatB - 2*quatD*quatD;
    matrix[2][2] = 1 - 2*quatB*quatB - 2*quatC*quatC;

    matrix[0][1] = 2*quatB*quatC - 2*quatD*quatA;
    matrix[1][0] = 2*quatB*quatC + 2*quatD*quatA;

    matrix[0][2] = 2*quatB*quatD + 2*quatC*quatA;
    matrix[2][0] = 2*quatB*quatD - 2*quatC*quatA;

    matrix[1][2] = 2*quatC*quatD - 2*quatB*quatA;
    matrix[2][1] = 2*quatC*quatD + 2*quatB*quatA;

        // обратная
//    matrix[0][1] = 2*quatB*quatC + 2*quatD*quatA;
//    matrix[1][0] = 2*quatB*quatC - 2*quatD*quatA;

//    matrix[0][2] = 2*quatB*quatD - 2*quatC*quatA;
//    matrix[2][0] = 2*quatB*quatD + 2*quatC*quatA;

//    matrix[1][2] = 2*quatC*quatD + 2*quatB*quatA;
//    matrix[2][1] = 2*quatC*quatD - 2*quatB*quatA;
}

void FillRotateMatrixFromVectorAndAngle(double** matrix, double *vector, double angle)
{
    norm3v(vector);

    double c = cos(angle);
    double s = sin(angle);
    double vers = (1-cos(angle));

    matrix[0][0] = vector[0]*vector[0]*vers + c;
    matrix[0][1] = vector[1]*vector[0]*vers - vector[2]*s;
    matrix[0][2] = vector[2]*vector[0]*vers + vector[1]*s;

    matrix[1][0] = vector[0]*vector[1]*vers + vector[2]*s;
    matrix[1][1] = vector[1]*vector[1]*vers + c;
    matrix[1][2] = vector[2]*vector[1]*vers - vector[0]*s;

    matrix[2][0] = vector[0]*vector[2]*vers - vector[1]*s;
    matrix[2][1] = vector[1]*vector[2]*vers + vector[0]*s;
    matrix[2][2] = vector[2]*vector[2]*vers + c;
}

void FillMatrixWithElem(double** matrix, double elem)
{
    for (int i=0l; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            *(*(matrix+i) + j) = elem;
        }
    }
}

// функция перемножения матриц nxn
int Matrix_X_Matrix(double **M1, double **M2, unsigned short n, double **Result)
{
    double Sum;
    for(int i=0; i<n; i++)
        for(int j=0; j<n; j++)
        {
            Sum=0;
            for(int k=0; k<n; k++)
            {
                Sum += M1[i][k] * M2[k][j];
            }
            Result[i][j] = Sum;
        }

    return 1;
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


int Vector3_Cross_Vector3(double *v1, double *v2, double *Result)
{
    Result[0] = v1[1]*v2[2] - v1[2]*v2[1];
    Result[1] = v1[2]*v2[0] - v1[0]*v2[2];
    Result[2] = v1[0]*v2[1] - v1[1]*v2[0];

    return 1;
}

void FillRabg(double a, double b, double g, double** matrix)
{
    double sinA = sin(a);
    double cosA = cos(a);
    double sinB = sin(b);
    double cosB = cos(b);
    double sinG = sin(g);
    double cosG = cos(g);

    *(*(matrix + 0) + 0) = cosB*cosG;
    *(*(matrix + 0) + 1) = -sinB;
    *(*(matrix + 0) + 2) = cosB*sinG;

    *(*(matrix + 1) + 0) = sinA*sinG + cosA*sinB*cosG;
    *(*(matrix + 1) + 1) = cosA*cosB;
    *(*(matrix + 1) + 2) = cosA*sinB*sinG - sinA*cosG;

    *(*(matrix + 2) + 0) = sinA*sinB*cosG - cosA*sinG;
    *(*(matrix + 2) + 1) = sinA*cosB;
    *(*(matrix + 2) + 2) = cosA*cosG + sinA*sinB*sinG;
}


double FindDifferenceBetweenRotateMatrices(double** M1, double** M2)
{
    double** matResulting = createMat3x3();
    double** invertedM2 = createMat3x3();
    InvertRotateMat(M2,invertedM2);
    Matrix_X_Matrix(M1, invertedM2, 3, matResulting);

    double  tr, s;
    int    i, j, k;

    int nxt[3] = {1, 2, 0};

    tr = matResulting[0][0] + matResulting[1][1] + matResulting[2][2];

    if (tr > 0.0)
    {
        s = sqrt (tr + 1.0);
        s = acos(s/2.0);
    }
    else
    {
        i = 0;
        if (matResulting[1][1] > matResulting[0][0])
            i = 1;
        if (matResulting[2][2] > matResulting[i][i])
            i = 2;
        j = nxt[i];
        k = nxt[j];

        s = sqrt ((matResulting[i][i] - (matResulting[j][j] + matResulting[k][k])) + 1.0);
        s = acos(s/2.0);
    }

    deleteMat3x3(invertedM2);
    deleteMat3x3(matResulting);

    //s = acos(s);
    return (s*s);

//    double columnVectors[3][3];

//    for(int i=0; i<3; i++)
//    {
//        for (int j=0; j<3; j++)
//        {
//            columnVectors[i][j] = M1[j][i] - M2[j][i];
////            columnVectors[i][j] = *(*(M1 + j) + i) - *(*(M2 + j) + i);
//        }
//    }

//    double sumOfSquares = 0.0;
//    for (int i=0; i<3; i++)
//    {
//        sumOfSquares += (columnVectors[i][0] * columnVectors[i][0] +
//                columnVectors[i][1] * columnVectors[i][1] +
//                columnVectors[i][2] * columnVectors[i][2]);
//    }

//    return sqrt(sumOfSquares);
}


void NormalizeVector(double* from, double* to, int n)
{
    double factor = 0;
    for(int i=0; i<n; i++)
    {
        factor += from[i]*from[i];
    }
    factor = 1/sqrt(factor);

    for(int i=0; i<n; i++)
    {
        to[i] = from[i]*factor;
    }
}

void GetQuatFromAngleAndVector(double angle, double* v, double* quat)
{
    double angleBy2 = angle/2;
    quat[0] = cos(angleBy2);

    double vNorm[3];
    NormalizeVector(v, vNorm, 3);

    double sina = sin(angleBy2);
    for(int i=0; i<3; i++)
    {
        quat[i+1] = vNorm[i]*sina;
    }
}

void InvertRotateMat(double **m, double **inv)
{
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            inv[i][j] = m[j][i];
        }
    }
}

void getXZFromXYZMatrixByY(double** matrix, double* X, double* Y, double* Z)
{
    if(sin(*Y) == 1.0f)
    {
        double xPz = atan2(matrix[1][0],matrix[1][1]);
        *X =  prevShXyz[0];
        *Z = xPz - *X;
    }
    else if(sin(*Y) == -1.0f)
    {
        double xMz = atan2(matrix[2][1], matrix[2][0]);
        *X =  prevShXyz[0];
        *Z = *X - xMz;
    }
    else
    {
        *Z = atan2(-matrix[0][1],matrix[0][0]);
        *X = atan2(-matrix[1][2],matrix[2][2]);
    }

    prevShXyz[0] = *X;
    prevShXyz[1] = *Y;
    prevShXyz[2] = *Z;
//    double cosY = cos(*Y);
//    double sinZ = -matrix[0][1]/cosY;
//    double cosZ = matrix[0][0]/cosY;
//    if(cosZ > 1.0)
//        cosZ = 1.0;
//    if(cosZ < -1.0)
//        cosZ = -1.0;
//    *Z = (sinZ >= 0) ? acos(cosZ) : (-acos(cosZ));

//    double sinX = -matrix[1][2]/cosY;
//    double cosX = matrix[2][2]/cosY;
//    if(cosX > 1.0)
//        cosX = 1.0;
//    if(cosX < -1.0)
//        cosX = -1.0;
//    *X = (sinX > 0) ? acos(cosX) : (-acos(cosX));
}

void getXYFromXZYMatrixByZ(double** matrix, double* X, double* Y, double* Z)
{
    double cosZ = cos(*Z);

    double cosX = matrix[1][1]/cosZ;
    double sinX = matrix[2][1]/cosZ;
    if(cosX > 1.0)
        cosX = 1.0;
    if(cosX < -1.0)
        cosX = -1.0;
    *X = (sinX >= 0) ? acos(cosX) : (-acos(cosX));

    double cosY = matrix[0][0]/cosZ;
    double sinY = matrix[0][2]/cosZ;
    if(cosY > 1.0)
        cosY = 1.0;
    if(cosY < -1.0)
        cosY = -1.0;
    *Y = (sinY >= 0) ? acos(cosY) : (-acos(cosY));
}

int GetXyzFromXyzMatrix(double** matrix, double* xyz)
{
    double X,Y,Z;
    double sinY = matrix[0][2];

    // первый случай - |B|<pi/2
    Y = asin(sinY);
    getXZFromXYZMatrixByY(matrix, &X, &Y, &Z);

    double angles[3] = {X,Y,Z};
    double** tempMatrix = CreateRotationMatrix("xyz", angles);
    if (FindDifferenceBetweenRotateMatrices(matrix, tempMatrix) > 0.1) // больше некоторого мизерного порога
    {
        // считаем |B|>pi/2 и повторяем все те же шаги
        Y = (sinY>0) ? (M_PI - asin(sinY)) : (-M_PI - asin(sinY));
        getXZFromXYZMatrixByY(matrix, &X, &Y, &Z);

        deleteMat3x3(tempMatrix);
        angles[0] = X;
        angles[1] = Y;
        angles[2] = Z;
        tempMatrix = CreateRotationMatrix("xyz", angles);
        if (FindDifferenceBetweenRotateMatrices(matrix, tempMatrix) > 0.01)
        {
            deleteMat3x3(tempMatrix);
            return 0;
        }
    }

    xyz[0] = X;
    xyz[1] = Y;
    xyz[2] = Z;
    deleteMat3x3(tempMatrix);
    return 1;
}

int GetXyzFromQuatMatrix(double** matrix, double* xyz)
{
    xyz[0] = atan2(matrix[2][1], matrix[2][2]);
    xyz[1] = -asin(matrix[2][0]);
    xyz[2] = atan2(matrix[1][0], matrix[0][0]);
}

int GetXzyFromQuatMatrix(double** matrix, double* xyz)
{
    xyz[0] = atan2(-matrix[1][2], matrix[1][1]);
    xyz[1] = asin(matrix[1][0]);
    xyz[2] = atan2(-matrix[2][0], matrix[0][0]);
}


int GetXzyFromXzyMatrix(double** matrix, double* xzy)
{
    double X,Y,Z;
    double sinZ = -matrix[0][1];

    // первый случай - |B|<pi/2
    Z = asin(sinZ);
    getXYFromXZYMatrixByZ(matrix, &X, &Y, &Z);

    double angles[3] = {X,Z,Y};
    double** tempMatrix = CreateRotationMatrix("xzy", angles);
    if (FindDifferenceBetweenRotateMatrices(matrix, tempMatrix) > 0.1) // больше некоторого мизерного порога
    {
        // считаем |B|>pi/2 и повторяем все те же шаги
        Z = (sinZ>0) ? (M_PI - asin(sinZ)) : (-M_PI - asin(sinZ));
        getXYFromXZYMatrixByZ(matrix, &X, &Y, &Z);

        deleteMat3x3(tempMatrix);
        angles[0] = X;
        angles[1] = Z;
        angles[2] = Y;
        tempMatrix = CreateRotationMatrix("xzy", angles);
        if (FindDifferenceBetweenRotateMatrices(matrix, tempMatrix) > 0.01)
        {
            deleteMat3x3(tempMatrix);
            return 0;
        }
    }

    xzy[0] = X;
    xzy[1] = Z;
    xzy[2] = Y;
    deleteMat3x3(tempMatrix);
    return 1;
}

double** CreateRotationMatrix(char* axes, double* angles)
{
    double** resultMatrix = createMat3x3();

    int n;  // число поворотов
    for(n=0; axes[n] != 0; n++);    // определили число поворотов

    // создаем последовательные матрицы элементарных поворотов
    double*** atomicRotMatr = new double**[n];
    for (int i=0; i<n; i++)
    {
        atomicRotMatr[i] = createMat3x3();
        switch(axes[i])
        {
        case('x'):
        {
            fillRotMatrixX(atomicRotMatr[i], angles[i]);
            break;
        }
        case('y'):
        {
            fillRotMatrixY(atomicRotMatr[i], angles[i]);
            break;
        }
        case('z'):
        {
            fillRotMatrixZ(atomicRotMatr[i], angles[i]);
            break;
        }
        default:
        {
            return 0;
        }
        }
    }

    // перемножаем элементарные матрицы
    double** tempMatrix = createMat3x3();   // промежуточный результат умножения
    copyMat3x3(atomicRotMatr[0], tempMatrix);
    for(int i=1; i<n; i++)
    {
        Matrix_X_Matrix(atomicRotMatr[i-1], atomicRotMatr[i], 3, tempMatrix);
        copyMat3x3(tempMatrix, atomicRotMatr[i]);
    }

    copyMat3x3(tempMatrix, resultMatrix);

    // удаляем все вспомогательные матрицы
    deleteMat3x3(tempMatrix);
    for(int i=0; i<n; i++)
    {
        deleteMat3x3(atomicRotMatr[i]);
    }
    delete[] atomicRotMatr;

    return resultMatrix;
}


void fillRotMatrixX(double** m, double angle)
{
    m[0][0] = 1.0;
    m[1][1] = m[2][2] = cos(angle);
    m[1][2] = -sin(angle);
    m[2][1] = sin(angle);
}

void fillRotMatrixY(double** m, double angle)
{
    m[1][1] = 1.0;
    m[0][0] = m[2][2] = cos(angle);
    m[0][2] = sin(angle);
    m[2][0] = -sin(angle);
}

void fillRotMatrixZ(double** m, double angle)
{
    m[2][2] = 1.0;
    m[0][0] = m[1][1] = cos(angle);
    m[0][1] = -sin(angle);
    m[1][0] = sin(angle);
}

stickCoords findStickCoords(double* neckRotAngles, double moduleSnakeShift, double moduleShift, double outPostLength, double stickLength)
{
    stickCoords sc;

    // two meaning angles
    double a = neckRotAngles[0]/180.0*M_PI;
    double b = neckRotAngles[1]/180.0*M_PI;

    // module translation matrix
    double** module0TransMat = createModuleTransMat(a,b,0);
    double** module1TransMat = createModuleTransMat(a,b,(moduleSnakeShift+moduleShift)/2);
    double** module2TransMat = createModuleTransMat(a,b,moduleShift);
    double** module3TransMat = createModuleTransMat(a,b,moduleShift);

    double** resultTransMatrix = createMat(4);
    Matrix_X_Matrix(module0TransMat, module1TransMat, 4, resultTransMatrix);
    copyMat(resultTransMatrix, module0TransMat, 4);
    Matrix_X_Matrix(module0TransMat, module2TransMat, 4, resultTransMatrix);
    copyMat(resultTransMatrix, module0TransMat, 4);
    Matrix_X_Matrix(module0TransMat, module3TransMat, 4, resultTransMatrix);

    // build stickBegin and stickEnd vectors
    double sb4[4] = {0, 0, outPostLength, 1};
    double se4[4] = {0, 0, outPostLength+stickLength, 1};

    double sb0[4];
    double se0[4];

    Matrix_X_Vector(resultTransMatrix, sb4, 4, 4, sb0);
    Matrix_X_Vector(resultTransMatrix, se4, 4, 4, se0);

    // set Z-offset for origin
    sb0[2] += moduleSnakeShift/2;
    se0[2] += moduleSnakeShift/2;

    for(int i=0; i<3; i++)
    {
        sc.begin[i] = sb0[i];
        sc.end[i] = se0[i];
    }

    deleteMat(module0TransMat, 4);
    deleteMat(module1TransMat, 4);
    deleteMat(module2TransMat, 4);
    deleteMat(module3TransMat, 4);
    deleteMat(resultTransMatrix, 4);

    return sc;
}

}   // end of "namespace mat"











