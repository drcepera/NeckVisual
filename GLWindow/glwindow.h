#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QGLWidget>
#include <3dsReader/3dsloader.h>
#include <3dsReader/3dsReader.h>
#include <3dsReader/texture.h>
#include <qevent.h>
#include <GLWindow/modelEdit.h>
#include <QFile>
#include <QTime>
#include <QTimer>

#include <SocketUDP/socketudp.h>
#include <fileProc/iniFileProc.h>

#include <windows.h>
#include <stdio.h>

#include <glText/Text.h>
#include "mat/mat.h"

using namespace mi::opengl;

typedef struct
{
    float coordXgoal;
    float coordYgoal;
    float coordXgoalPrev;
    float coordYgoalPrev;
    float coordXout;
    float coordYout;
    float coordXoutPrev;
    float coordYoutPrev;
} filterParam;

typedef enum
{
    POSITION = 0,
    JOINT_1 = '1',
    JOINT_2 = '2',
    JOINT_3 = '3',
    JOINT_4 = '4'
} modeEnum;

class GLWindow : public QGLWidget
{
    Q_OBJECT

public:
    explicit GLWindow( QWidget * parent = 0);
    ~GLWindow() {   }
    void SaveParamsToIniFile();

signals:
    void OutText(QString str);
    void SendNewBeaconStateToSTC(uint state);

public slots:
    void dispatcherTimerCallback();
    void randomTimerCallback();
    void glOutText(QStringList text);

    void OnCommandReceived(QList<command>* cmdList);

    void OnReloadBeaconList(QList<beacon> beacons);
    void OnReloadCameraParam(gl3dParamStruct param);


protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    // events
    void mousePressEvent(QMouseEvent *me);
    void mouseMoveEvent(QMouseEvent *me);
    void wheelEvent(QWheelEvent *we);
    void keyPressEvent(QKeyEvent *ke);

    void neckOutput();
    void beaconBarOutput( float xBot, float yBot, float height);
    void modelOutput(model_type* model);
    void modelOutput(model_type* model, double red, double green, double blue, double alpha);
    void modelScale(model_type* model, double xs, double ys, double zs);
    void textOutput();

    void defaultScene();

    void filterCoordinates();
    void getMotorCoordsFromXY();

    void onBeaconStateChanged();
    UINT8 findIntersectedBeacons();
    bool isBeaconIntersected(UINT8 beaconNum, mat::stickCoords sc);
    double stickToBeaconDistance(mat::stickCoords sc, double* a);

    bool demo;  // indicates if demo regime is activated

    // gl window geometric params
    static const int widthLeft = -300;
    static const int widthRight = 300;
    static const int heightBottom = -300;
    static const int heightTop = 300;
    int windowWidth;
    int windowHeight;

    // text out param
    QStringList textToOutput;
    bool mustOutText;
    Text* glText;

    // 3DS models
    model_type motorModel;
    model_type zPostModel;
    model_type zPostModelInverted;
    model_type motorModelSnake;
    model_type postModelSnake;
    model_type postModelSnakeInverted;
    model_type flangeModelSnake;
    model_type outPostModel;
    model_type stickModel;
    model_type beaconModelLight;
    model_type beaconModelDark;
    // scene models
    model_type boxBottomModel;
    model_type furnitureHorizontalModel;
    model_type furnitureVerticalModel;

    QPoint mousePosPtrLeft;
    QPoint mousePosPtrRight;

    static const int dispatcherTimerPeriodMs = 20;
    QTimer dispatcherTimer;
    static const int randomPosGenTimerPeriodMs = 4000;
    QTimer randomPosGenTimer;

    gl3dParamStruct gl3dParam;
    filterParam neckFilterParam;

    float coordX;
    float coordY;
    float maxAngle;

    modeEnum mode;
    modeEnum modePrev;


    static const int jointNum = 8;
    double neckRotAngles[jointNum];
    double neckGoalAngles[jointNum];

    QList<beacon> beaconList;
};

#endif // GLWINDOW_H
