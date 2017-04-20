#include "glwindow.h"
#include "math.h"

QList<GLuint> textureID;

static const quint8 ASCII_NUM_OFFSET = 0x30;

static const float scaleFactor = 0.9;
static const float mouseScaleFactor = 100;

static const float motorModuleCorrection_x = 6.19;
static const float motorModuleCorrection_y = -33.22;
static const float motorModuleCorrection_z = 45.05;
static const float motorModuleCorrection_angle_y = 90.0;

static const float zPostModuleCorrection_x = -11.75;
static const float zPostModuleCorrection_y = -53.23;
static const float zPostModuleCorrection_z = 20.45;
static const float zPostModuleCorrection_angle_x = 90.0;
static const float zPostModuleCorrection_angle_y = 180.0;

static const double moduleShift = 66.0;
static const double moduleSnakeShift = 64.0;
static const float outPostLength = 66.0;

static const float flangeSnakeRadius = 39.0;

static const float boxBottomWidth = 762.32;
static const float furnitureVericalCoords[3][3] = { {-373.75, 0, 380.25},
                                                    {392.25, 0, -398.75},
                                                    {396.25, 0, 380.25}};

static const double angleDelta = 0.2;
static const float beaconRadius = 23.0;

static UINT8 oldLedState = 0x00;
static UINT8 currentLedState = 0x00;
// LEDs are switched on/off only by command from MCP

GLWindow::GLWindow(QWidget *parent)
{
    mode = POSITION;
    modePrev = mode;

    mustOutText = false;
    glText = new Text(QFont("Helvetica", 15));

    neckFilterParam.coordXgoal = 0.0;
    neckFilterParam.coordYgoal = 0.0;
    neckFilterParam.coordXgoalPrev = 0.0;
    neckFilterParam.coordYgoalPrev = 0.0;
    neckFilterParam.coordXout = 0.0;
    neckFilterParam.coordYout = 0.0;
    neckFilterParam.coordXoutPrev = 0.0;
    neckFilterParam.coordYoutPrev = 0.0;
    coordX = 0;
    coordY = 0;
    gl3dParam.filterValue = -0.2;

    printf("GLWindow constructor exited\n");

    // Load 3d models
    Load3DS(&motorModel, "3DS/Motor_Model.3DS");
    Load3DS(&zPostModel, "3DS/Z_post.3DS");

    Load3DS(&motorModelSnake, "3DS/Motor_Module_Snake_Unc.3DS");
    //translate(&motorModelSnake, 0.0, 10.0, -5.0);
    //rotate(&motorModelSnake, M_PI/2, 0, 1, 0);
    //rotate(&motorModelSnake, M_PI, 1, 0, 0);
    //rotate(&motorModelSnake, M_PI/2, 0, 0, 1);
    //translate(&motorModelSnake, 0.0, 6.5, -5.5);
    Load3DS(&postModelSnake, "3DS/Post_Snake_Unc.3DS");

    Copy3DS(&postModelSnake, &postModelSnakeInverted);
    rotate(&postModelSnakeInverted, M_PI/2, 0, 1, 0);
    rotate(&postModelSnakeInverted, M_PI, 1, 0, 0);

    Load3DS(&flangeModelSnake, "3DS/flange.3DS");
    translate(&flangeModelSnake, -flangeSnakeRadius, 0, -flangeSnakeRadius );
    rotate(&flangeModelSnake, M_PI, 0, 1, 0);

    Load3DS(&outPostModel, "3DS/outPostWithMotor.3DS");
    rotate(&outPostModel, -M_PI/2, 0, 1, 0);

    // следует далее привести модельки так, чтобы нулю их соответствовало перекрестие осей
    // и ориентированы они были так, как на модели SolidWorks в начальном положении
    //rotate(&zPostModule, 90*M_PI/180, 0, 1, 0);
    //translate(&motorModule, 0, 0, 100);
    translate(&motorModel, motorModuleCorrection_x, motorModuleCorrection_y, motorModuleCorrection_z);
    rotate(&motorModel, (motorModuleCorrection_angle_y*M_PI/180), 0, 1, 0);

    translate(&zPostModel, zPostModuleCorrection_x, zPostModuleCorrection_y, zPostModuleCorrection_z);
    rotate(&zPostModel, (zPostModuleCorrection_angle_x*M_PI/180), 1, 0, 0);
    rotate(&zPostModel, (zPostModuleCorrection_angle_y*M_PI/180), 0, 1, 0);

    Copy3DS(&zPostModel, &zPostModelInverted);
    rotate(&zPostModelInverted, -M_PI/2, 0, 1, 0);
    rotate(&zPostModelInverted, M_PI, 0, 0, 1);

    Load3DS(&stickModel, "3DS/stick.3DS");
    modelScale(&stickModel, 1.0, 173.0/150.0, 1.0);
    Load3DS(&beaconModelLight, "3DS/beacon.3DS");
    rotate(&beaconModelLight, M_PI/2, 0, 0, 1);
    Load3DS(&beaconModelDark, "3DS/beacon.3DS");
    rotate(&beaconModelDark, M_PI/2, 0, 0, 1);

    // scene models
    Load3DS(&boxBottomModel, "3DS/boxBottom.3DS");
    translate(&boxBottomModel, -392.32, -376, 0);
    rotate(&boxBottomModel, M_PI/2, 1, 0, 0);
    translate(&boxBottomModel, 0, -moduleSnakeShift/2, 0);
    rotate(&boxBottomModel, M_PI, 0, 1, 0);

//    Load3DS(&furnitureHorizontal, "3DS/furnitureHorizontal.3DS");
    Load3DS(&furnitureVerticalModel, "3DS/furnitureVertical.3DS");
    rotate(&furnitureVerticalModel, M_PI/2, 1, 0, 0);
    translate(&furnitureVerticalModel, 0, 687, 0);

    setFocusPolicy(Qt::StrongFocus);    // иначе не реагирует на нажатия

    connect(&dispatcherTimer, SIGNAL(timeout()), SLOT(dispatcherTimerCallback()));
    dispatcherTimer.setSingleShot(false);
    dispatcherTimer.start(dispatcherTimerPeriodMs);

////    qrand();
////    connect(&randomPosGenTimer, SIGNAL(timeout()), SLOT(randomTimerCallback()));
////    randomPosGenTimer.setSingleShot(false);
//    //    randomPosGenTimer.start(randomPosGenTimerPeriodMs);

    demo = false;
}

void GLWindow::dispatcherTimerCallback()
{
//    // test section - fill here your values
//    coordX = 0.5;
//    coordY = -0.0;

    // warning!!! Присваиваются координаты наоборот, чтобы сошлось с физической моделью
    neckFilterParam.coordXgoal = coordY;
    neckFilterParam.coordYgoal = coordX;

    filterCoordinates();

//    // test section - fill here your values
//    neckFilterParam.coordYout = 0.0;
//    neckFilterParam.coordXout = 1.0;

    getMotorCoordsFromXY();

//    double dir = 0;
    for(int i=0; i<jointNum; i++)
    {
        neckRotAngles[i] = neckGoalAngles[i];
//        dir = (neckGoalAngles[i] - neckRotAngles[i]) / abs((neckGoalAngles[i] - neckRotAngles[i]));
//        dir = dir * abs((neckGoalAngles[i] - neckRotAngles[i])) / angleDelta;
//        if(dir == 0)
//        {
//            continue;
//        }
//        if(dir > 0)
//        {
//            if (dir>1)
//                neckRotAngles[i] += angleDelta;
//            else
//                neckRotAngles[i] = neckGoalAngles[i];
//            continue;
//        }
//        if(dir < 0)
//        {
//            if (dir<-1)
//                neckRotAngles[i] -= angleDelta;
//            else
//                neckRotAngles[i] = neckGoalAngles[i];
//        }
    }

    if(demo)
    {
        findIntersectedBeacons();
    }

    updateGL();
}


void GLWindow::randomTimerCallback()
{
//    double x,y;

//    x = qrand()%(61) - 30;
//    y = qrand()%(61) - 30;
//    for(int i=0; i<jointNum/2; i++)
//    {
//        neckGoalAngles[i*2] = x;
//        neckGoalAngles[i*2+1] = y;
//    }

////    for(int i=0; i<jointNum; i++)
////    {
////        neckGoalAngles[i] = qrand()%(91) - 45;
    //    //    }
}

void GLWindow::glOutText(QStringList text)
{
    if (text.size() == 0)
    {
        mustOutText = false;
        textToOutput.clear();
        return;
    }
    mustOutText = true;
    textToOutput = text;
}

void GLWindow::OnCommandReceived(QList<command> *cmdList)
{
    bool beaconCommandReceived = false;

    for(int i=0; i<cmdList->size(); i++)
    {
        // if coord
        if((*cmdList)[i].Key == "X")
        {
            coordX = (*cmdList)[i].Value.toFloat();
//            modePrev = mode;
//            mode = (modeEnum)flag;
        }
        else if((*cmdList)[i].Key == "Y")
        {
            coordY = (*cmdList)[i].Value.toFloat();
//            modePrev = mode;
//            mode = (modeEnum)flag;
        }

        // if LED bitmap
        else if((*cmdList)[i].Key == "LED")
        {
            char led = (*cmdList)[i].Value.toUShort();
            for(int bit=0; bit<beaconNum; bit++)
            {
                beaconList[bit].isShining = (led & (1<<bit));
            }
//            iniFileProc::setBeaconParams(beaconList);

            currentLedState = led;

            // don't send changed beacon state back, said Gook 30.04.2015
//            onBeaconStateChanged();
        }

        // if LED coord
        else if((*cmdList)[i].Key[0] == 'T')
        {
            if(!beaconCommandReceived)
            {
                beaconCommandReceived = true;
                for(int i=0; i<beaconNum; i++)
                {
                    beaconList[i].isShown = false;
                }
            }

            QString numStr = QString((*cmdList)[i].Key[1]);
            int num = numStr.toInt();
            QStringList coordsStr = (*cmdList)[i].Value.split(';');
            if(coordsStr.size() != 3)
                // TODO: error process
                return;
            //
            for(int c=0; c<3; c++)
            {
                coordsStr[c].remove(' ');
            }
            beaconList[num].x = coordsStr[0].toInt();
            beaconList[num].y = coordsStr[1].toInt();
            beaconList[num].z = coordsStr[2].toInt();
            beaconList[num].isShown = true;
//            iniFileProc::setBeaconParams(beaconList);
        }

        // if demo activated
        else if((*cmdList)[i].Key.toUpper() == (QString("Demo")).toUpper())
        {
            demo = (bool) ((*cmdList)[i].Value).toInt();
        }
    }

    // command output to text edit window
    QString outText;
    outText += QTime::currentTime().toString() + "\t";
    for(int i=0; i<cmdList->size(); i++)
    {
        outText += (*cmdList)[i].Key + "=" + (*cmdList)[i].Value + ";  ";
    }
    emit OutText(outText);
}

void GLWindow::OnReloadBeaconList(QList<beacon> beacons)
{
    beaconList = beacons;
}

void GLWindow::OnReloadCameraParam(gl3dParamStruct param)
{
    gl3dParam = param;
}

void GLWindow::SaveParamsToIniFile()
{
    iniFileProc::setCameraParams(gl3dParam);
    iniFileProc::setBeaconParams(beaconList);
}

void GLWindow::initializeGL()
{
    beaconList = iniFileProc::getBeaconParams();
    gl3dParam = iniFileProc::getCameraParams();
    maxAngle = gl3dParam.maxJointAngle;

    currentLedState = 0;
    for(int i=0; i<8; i++)
    {
        if((beaconList[i].isShining) && (beaconList[i].isShown))
            currentLedState |= 1<<i;
    }
    oldLedState = currentLedState;

    // Load textures
    GLuint tex = bindTexture(QPixmap(QString("3DS/Motor_Model_2.bmp")), GL_TEXTURE_2D);
    textureID.append(tex);
    motorModel.id_texture = tex;

    tex = bindTexture(QPixmap(QString("3DS/Z_post.bmp")), GL_TEXTURE_2D);
    textureID.append(tex);
    zPostModel.id_texture = tex;
    zPostModelInverted.id_texture = tex;

    tex = bindTexture(QPixmap(QString("3DS/Motor_Module_Snake_Unc.bmp")), GL_TEXTURE_2D);
    textureID.append(tex);
    motorModelSnake.id_texture = tex;

    tex = bindTexture(QPixmap(QString("3DS/Post_Snake_Unc.bmp")), GL_TEXTURE_2D);
    textureID.append(tex);
    postModelSnake.id_texture = tex;
    postModelSnakeInverted.id_texture = tex;

    tex = bindTexture(QPixmap(QString("3DS/flange.bmp")), GL_TEXTURE_2D);
    textureID.append(tex);
    flangeModelSnake.id_texture = tex;

    tex = bindTexture(QPixmap(QString("3DS/outPostWithMotor.bmp")), GL_TEXTURE_2D);
    textureID.append(tex);
    outPostModel.id_texture = tex;

    tex = bindTexture(QPixmap(QString("3DS/stick.bmp")), GL_TEXTURE_2D);
    textureID.append(tex);
    stickModel.id_texture = tex;

    tex = bindTexture(QPixmap(QString("3DS/beaconLight.bmp")), GL_TEXTURE_2D);
    textureID.append(tex);
    beaconModelLight.id_texture = tex;

    tex = bindTexture(QPixmap(QString("3DS/beaconDark.bmp")), GL_TEXTURE_2D);
    textureID.append(tex);
    beaconModelDark.id_texture = tex;

    tex = bindTexture(QPixmap(QString("3DS/boxBottom.bmp")), GL_TEXTURE_2D);
    textureID.append(tex);
    boxBottomModel.id_texture = tex;

//    tex = bindTexture(QPixmap(QString("3DS/furnitureHorizontal.bmp")), GL_TEXTURE_2D);
//    textureID.append(tex);
//    furnitureHorizontal.id_texture = tex;

    tex = bindTexture(QPixmap(QString("3DS/furnitureVertical.bmp")), GL_TEXTURE_2D);
    textureID.append(tex);
    furnitureVerticalModel.id_texture = tex;

    glClearColor(0.6, 0.6, 0.7, 0.0);

    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH); // Type of shading for the polygons

    glMatrixMode(GL_PROJECTION); // Specifies which matrix stack is the target for matrix operations
    glLoadIdentity(); // We initialize the projection matrix as identity
    glOrtho(widthLeft, widthRight, heightBottom, heightTop, -300, 10000);
//    glFrustum(-100, 100, -100, 100, 100, 600);

    glEnable(GL_DEPTH_TEST); // We enable the depth test (also called z buffer)
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);  //, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode (GL_FRONT, GL_FILL); // Polygon rasterization mode (polygon filled)

    glEnable(GL_TEXTURE_2D); // This Enable the Texture mapping

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    GLfloat lPos1[] = {0,0,800,0.0};
    GLfloat dif1[] = {1.0,1.0,1.0,1.0};
//    GLfloat lDir[] = {-500,-500,-500,0};
    GLfloat amb[] = {0.3,0.3,0.35,1.0};
//    GLfloat spec[] = {0.5,0.5,0.5,1.0};
    glLightfv(GL_LIGHT1, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, dif1);
    glLightfv(GL_LIGHT1, GL_POSITION, lPos1);

    GLfloat lPos2[] = {0,0,400,1.0};
    GLfloat dif2[] = {0.5, 0.5, 0.5, 1.0};
    GLfloat lDir[] = {0,0,-10};
    GLfloat lExp[] = {30, 30, 30};
    glLightfv(GL_LIGHT2, GL_DIFFUSE, dif2);
    glLightfv(GL_LIGHT2, GL_POSITION, lPos2);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, lDir);
    glLightfv(GL_LIGHT2, GL_SPOT_EXPONENT, lExp);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);

    GLfloat lPos3[] = {0,400,11000,1.0};
    GLfloat dif3[] = {0.5, 0.5, 0.5, 1.0};
    GLfloat lDir3[] = {0,0,-1000};
    GLfloat lExp3[] = {50, 50, 50};
    glLightfv(GL_LIGHT3, GL_DIFFUSE, dif3);
    glLightfv(GL_LIGHT3, GL_POSITION, lPos3);
    glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, lDir3);
    glLightfv(GL_LIGHT3, GL_SPOT_EXPONENT, lExp3);
//    glEnable(GL_LIGHT3);

//    GLfloat global_ambient[] = {0.4,0.4,0.4,1.0};
//    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,global_ambient);
}

void GLWindow::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    glColor4f(red, green, blue, alpha);

    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0, -100, 0);
    glTranslatef(gl3dParam.transX, gl3dParam.transY, 0.0);
    glRotatef(gl3dParam.rotX, 1.0, 0.0, 0.0);
    glRotatef(gl3dParam.rotY, 0.0, 1.0, 0.0);
    //glRotatef(90, 0.0, 1.0, 0.0);

    glScalef(gl3dParam.scale, gl3dParam.scale, gl3dParam.scale);

//    glColor4f(1.0, 0.0, 0.0, 1.0);
//    glBegin(GL_TRIANGLES);
//        glVertex3f(-40, 70, -100);
//        glVertex3f(30, -50, -50);
//        glVertex3f(-20, 120, 0);
//    glEnd();

    neckOutput();
    glPopMatrix();

    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, 1000.0);
    textOutput();
    glPopMatrix();

    glFlush();
}

void GLWindow::modelOutput(model_type* model)
{
    glColor4f(1.0, 1.0, 1.0, 1.0);

    glBindTexture(GL_TEXTURE_2D, model->id_texture); // We set the active texture

    glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)
    for (long int l_index=0; l_index<model->polygons_qty; l_index++)
    {
        float a[3] = { model->vertexList[model->polygonList[l_index].abc[0]].xyz[0],
                       model->vertexList[model->polygonList[l_index].abc[0]].xyz[1],
                       model->vertexList[model->polygonList[l_index].abc[0]].xyz[2]    };
        float b[3] = { model->vertexList[model->polygonList[l_index].abc[1]].xyz[0],
                       model->vertexList[model->polygonList[l_index].abc[1]].xyz[1],
                       model->vertexList[model->polygonList[l_index].abc[1]].xyz[2]    };
        float c[3] = { model->vertexList[model->polygonList[l_index].abc[2]].xyz[0],
                       model->vertexList[model->polygonList[l_index].abc[2]].xyz[1],
                       model->vertexList[model->polygonList[l_index].abc[2]].xyz[2]    };

        glNormal3fv(model->normalList[l_index].xyz);

        //----------------- FIRST VERTEX -----------------
        // Texture coordinates of the first vertex
        glTexCoord2f(   model->mapcoordList[model->polygonList[l_index].abc[0]].uv[0],
                        model->mapcoordList[model->polygonList[l_index].abc[0]].uv[1]   );
        // Coordinates of the first vertex
        glVertex3f( a[0], a[1], a[2] );

        //----------------- SECOND VERTEX -----------------
        // Texture coordinates of the second vertex
        glTexCoord2f(   model->mapcoordList[model->polygonList[l_index].abc[1]].uv[0],
                        model->mapcoordList[model->polygonList[l_index].abc[1]].uv[1]   );
        // Coordinates of the second vertex
        glVertex3f( b[0], b[1], b[2] );

        //----------------- THIRD VERTEX -----------------
        // Texture coordinates of the third vertex
        glTexCoord2f(   model->mapcoordList[model->polygonList[l_index].abc[2]].uv[0],
                        model->mapcoordList[model->polygonList[l_index].abc[2]].uv[1]   );
        // Coordinates of the Third vertex
        glVertex3f( c[0], c[1], c[2] );
    }
    glEnd();
}

void GLWindow::modelOutput(model_type *model, double red, double green, double blue, double alpha)
{
//    glDisable(GL_TEXTURE_2D);
    glColor4f(red, green, blue, alpha);

    glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)
    for (long int l_index=0; l_index<model->polygons_qty; l_index++)
    {
        float a[3] = { model->vertexList[model->polygonList[l_index].abc[0]].xyz[0],
                       model->vertexList[model->polygonList[l_index].abc[0]].xyz[1],
                       model->vertexList[model->polygonList[l_index].abc[0]].xyz[2]    };
        float b[3] = { model->vertexList[model->polygonList[l_index].abc[1]].xyz[0],
                       model->vertexList[model->polygonList[l_index].abc[1]].xyz[1],
                       model->vertexList[model->polygonList[l_index].abc[1]].xyz[2]    };
        float c[3] = { model->vertexList[model->polygonList[l_index].abc[2]].xyz[0],
                       model->vertexList[model->polygonList[l_index].abc[2]].xyz[1],
                       model->vertexList[model->polygonList[l_index].abc[2]].xyz[2]    };

        glNormal3fv(model->normalList[l_index].xyz);

        // Coordinates of three vertex
        glVertex3f( a[0], a[1], a[2] );
        glVertex3f( b[0], b[1], b[2] );
        glVertex3f( c[0], c[1], c[2] );
    }
    glEnd();

    //    glEnable(GL_TEXTURE_2D);
}

void GLWindow::modelScale(model_type *model, double xs, double ys, double zs)
{
    for(int i=0; i<model->vertexList.size(); i++)
    {
        model->vertexList[i].xyz[0] *= xs;
        model->vertexList[i].xyz[1] *= ys;
        model->vertexList[i].xyz[2] *= zs;
    }
}

void GLWindow::textOutput()
{
    if (!mustOutText)
        return;

    float xPos = widthLeft*0.9; // * ( windowWidth/(widthRight-widthLeft) );
    float yPos = heightTop*0.9; // * ( windowHeight/(heightTop-heightBottom) );

    glColor4f(0.0, 0.0, 0.0, 1.0);
    foreach(QString str, textToOutput)
    {
//        glText->~Text();
//        glText = new Text(QFont( "Helvetica", 15));  //(int)((float)20/windowHeight*(heightTop-heightBottom)) ));
        glText->renderText(xPos, yPos, str);
//        renderText(xPos, yPos, 1000, str);
        yPos -= (int) 20;   // / ( (float)(windowHeight)/(heightTop-heightBottom) );
    }
}

void GLWindow::defaultScene()
{
    gl3dParam.rotX = 15;
    gl3dParam.rotY = 105;
    gl3dParam.scale = 1.0;
    gl3dParam.transX = gl3dParam.transY = 0.0;
//    iniFileProc::setCameraParams(gl3dParam);
    updateGL();
}

void GLWindow::filterCoordinates()
{
    if(modePrev == mode)
    {
        neckFilterParam.coordXout = neckFilterParam.coordXgoalPrev*(1+gl3dParam.filterValue)/2 +
                neckFilterParam.coordXgoal*(1+gl3dParam.filterValue)/2 - gl3dParam.filterValue*neckFilterParam.coordXoutPrev;
        neckFilterParam.coordYout = neckFilterParam.coordYgoalPrev*(1+gl3dParam.filterValue)/2 +
                neckFilterParam.coordYgoal*(1+gl3dParam.filterValue)/2 - gl3dParam.filterValue*neckFilterParam.coordYoutPrev;
    }
    else    // eliminating image jitters between mode changes
    {
        neckFilterParam.coordXout = neckFilterParam.coordXgoal;
        neckFilterParam.coordXout = neckFilterParam.coordXgoal;
    }

    // prev state update
    neckFilterParam.coordXgoalPrev = neckFilterParam.coordXgoal;
    neckFilterParam.coordYgoalPrev = neckFilterParam.coordYgoal;
    neckFilterParam.coordXoutPrev = neckFilterParam.coordXout;
    neckFilterParam.coordYoutPrev = neckFilterParam.coordYout;
}

void GLWindow::getMotorCoordsFromXY()
{
    // WARNING! Y axes direction inversed, Y and X swapped are - ??

    if( mode == POSITION)
    {
        for(int i=0; i<jointNum/2; i++)
        {
            neckGoalAngles[i*2] = neckFilterParam.coordYout * maxAngle;
            neckGoalAngles[i*2+1] = neckFilterParam.coordXout * maxAngle;
        }
    }
    else if( (mode>=JOINT_1) && (mode<=JOINT_4))
    {
        int jointToMove = mode - ASCII_NUM_OFFSET - 1;
        neckGoalAngles[jointToMove*2] = neckFilterParam.coordYout * maxAngle;
        neckGoalAngles[jointToMove*2+1] = neckFilterParam.coordXout * maxAngle;
    }
}

void GLWindow::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    windowWidth = width;
    windowHeight = height;

    float rel = (float)width/height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (rel < 1.0)
    {
        glOrtho(widthLeft, widthRight, heightBottom/rel, heightTop/rel, -1000, 10000);
//        glFrustum(-100, 100, -100/rel, 100/rel, 100, 600);
    }
    else
    {
        glOrtho(widthLeft*rel, widthRight*rel, heightBottom, heightTop, -1000, 10000);
//        glFrustum(-100*rel, 100*rel, -100, 100, 100, 600);
    }
}

void GLWindow::mousePressEvent(QMouseEvent *me)
{
    if(me->button() == Qt::LeftButton)
        mousePosPtrLeft = me->pos();
    if(me->button() == Qt::RightButton)
        mousePosPtrRight = me->pos();
//    else if(me->button() == Qt::RightButton)
//        scale /= 0.9;
//    else if (me->button() == Qt::MidButton)
//        scale *= 0.9;

//    updateGL();
}

void GLWindow::mouseMoveEvent(QMouseEvent *me)
{
    if(me->buttons() == Qt::LeftButton)
    {
        gl3dParam.rotY += 180/mouseScaleFactor * (me->x() - mousePosPtrLeft.x());
        gl3dParam.rotX += 180/mouseScaleFactor * (me->y() - mousePosPtrLeft.y());
//        iniFileProc::setCameraParams(gl3dParam);
        mousePosPtrLeft = me->pos();
        updateGL();
    }

    if(me->buttons() == Qt::RightButton)
    {
        gl3dParam.transX += (me->x() - mousePosPtrRight.x());
        gl3dParam.transY -= (me->y() - mousePosPtrRight.y());

        qDebug() << "transX: " << gl3dParam.transX;
        qDebug() << "me->x: " << me->x();
        qDebug() << "mousePosPtr.x: " << mousePosPtrRight.x();

//        iniFileProc::setCameraParams(gl3dParam);
        mousePosPtrRight = me->pos();
        updateGL();
    }
}

void GLWindow::wheelEvent(QWheelEvent *we)
{
    if(we->delta() > 0)
        gl3dParam.scale *= scaleFactor;
    else if (we->delta() < 0)
        gl3dParam.scale /= scaleFactor;
//    iniFileProc::setCameraParams(gl3dParam);
    updateGL();
}

void GLWindow::keyPressEvent(QKeyEvent *ke)
{
    switch(ke->key())
    {
    case(Qt::Key_N):
    {
        defaultScene();
        break;
    }
    case(Qt::Key_Up):
    {
        gl3dParam.rotX -= 15;
        updateGL();
        break;
    }
    case(Qt::Key_Down):
    {
        gl3dParam.rotX += 15;
        updateGL();
        break;
    }
    case(Qt::Key_Right):
    {
        gl3dParam.rotY += 15;
        updateGL();
        break;
    }
    case(Qt::Key_Left):
    {
        gl3dParam.rotY -= 15;
        updateGL();
        break;
    }
    default:
        break;
    }
//    iniFileProc::setCameraParams(gl3dParam);
}

void GLWindow::neckOutput()
{
    // beacons output
    for(int i=0; i<beaconNum; i++)
    {
        if(beaconList[i].isShown)
        {
            glPushMatrix();
            glTranslatef(beaconList[i].y, beaconList[i].z - moduleSnakeShift/2, beaconList[i].x);
//            glRotatef(FindBeaconAngle(beaconList[i].x, beaconList[i].y, beaconList[i].z)*180/M_PI, 0, -1, 0);
            beaconBarOutput(0.0, 0.0, beaconList[i].z);
            if(beaconList[i].isShining)
            {
                modelOutput(&beaconModelDark, gl3dParam.colorOn.r, gl3dParam.colorOn.g, gl3dParam.colorOn.b, 1.0);
            }
            else
            {
                modelOutput(&beaconModelDark, gl3dParam.colorOff.r, gl3dParam.colorOff.g, gl3dParam.colorOff.b, 1.0);
            }
            glPopMatrix();
        }
    }

    modelOutput(&boxBottomModel);
    // vertical furniture output
    for(int i=0; i<3; i++)
    {
        glPushMatrix();
        glTranslatef(furnitureVericalCoords[i][0], furnitureVericalCoords[i][1], furnitureVericalCoords[i][2]);
        modelOutput(&furnitureVerticalModel);
        glPopMatrix();
    }

    modelOutput(&postModelSnake);
    glRotatef(neckRotAngles[0], 1, 0, 0);
    modelOutput(&motorModelSnake);
    glRotatef(neckRotAngles[1], 0, 0, 1);
    modelOutput(&postModelSnakeInverted);
    glTranslatef(0, moduleSnakeShift/2, 0);

    // snake module flange
    modelOutput(&flangeModelSnake);
    glTranslatef(0, 2, 0);
    modelOutput(&flangeModelSnake);
    glTranslatef(0, 2, 0);
    //modelOutput(&flangeModuleSnake);

    glTranslatef(0, moduleSnakeShift/2 + 3, 0);

    for(int i=1; i<jointNum/2-1; i++)
    {
        modelOutput(&zPostModel);
        glRotatef(neckRotAngles[i*2], 1, 0, 0);
        modelOutput(&motorModel);
        glRotatef(neckRotAngles[i*2+1], 0, 0, 1);
        modelOutput(&zPostModelInverted);

        glTranslatef(0, moduleShift, 0);
    }

    // last module - with different out post
    modelOutput(&zPostModel);
    glRotatef(neckRotAngles[(jointNum/2-1)*2], 1, 0, 0);
    modelOutput(&motorModel);
    glRotatef(neckRotAngles[(jointNum/2-1)*2+1], 0, 0, 1);
    modelOutput(&outPostModel);

    glTranslatef(0, outPostLength, 0);
    modelOutput(&stickModel);
}

#define CIRCLE_APPROX_PIECES_NUMBER 50
#define BEACON_BAR_RADIUS   (float)3

void GLWindow::beaconBarOutput(float xBot, float yBot, float height)
{
    glColor4f(0.4, 0.4, 0.2, 1.0);

    float dangle = 2*M_PI/CIRCLE_APPROX_PIECES_NUMBER;

    // bottom point 1
    float cylXb1[CIRCLE_APPROX_PIECES_NUMBER];
    float cylYb1[CIRCLE_APPROX_PIECES_NUMBER];
    float cylZb1[CIRCLE_APPROX_PIECES_NUMBER];
    // bottom point 2
    float cylXb2[CIRCLE_APPROX_PIECES_NUMBER];
    float cylYb2[CIRCLE_APPROX_PIECES_NUMBER];
    float cylZb2[CIRCLE_APPROX_PIECES_NUMBER];
    // upper point 1
    float cylXu1[CIRCLE_APPROX_PIECES_NUMBER];
    float cylYu1[CIRCLE_APPROX_PIECES_NUMBER];
    float cylZu1[CIRCLE_APPROX_PIECES_NUMBER];
    // upper point 2
    float cylXu2[CIRCLE_APPROX_PIECES_NUMBER];
    float cylYu2[CIRCLE_APPROX_PIECES_NUMBER];
    float cylZu2[CIRCLE_APPROX_PIECES_NUMBER];

    for(int i=0; i<CIRCLE_APPROX_PIECES_NUMBER; i++)
    {
        cylXb1[i] = sin(i*dangle)*BEACON_BAR_RADIUS + xBot;
        cylXu1[i] = cylXb1[i];

        cylYb1[i] = cos(i*dangle)*BEACON_BAR_RADIUS + yBot;
        cylYu1[i] = cylYb1[i];

        cylZb1[i] = 0;
        cylZu1[i] = height;

        cylXb2[i] = sin((i+1)*dangle)*BEACON_BAR_RADIUS + xBot;
        cylXu2[i] = cylXb2[i];

        cylYb2[i] = cos((i+1)*dangle)*BEACON_BAR_RADIUS + yBot;
        cylYu2[i] = cylYb2[i];

        cylZb2[i] = cylZb1[i];
        cylZu2[i] = cylZu1[i];
    }

    for(int i=0; i<CIRCLE_APPROX_PIECES_NUMBER; i++)
    {
        glBegin(GL_QUADS);
        {
            double v1[3] = {cylXu2[i]-cylXb2[i], -cylZu2[i]+cylZb2[i], cylYu2[i]-cylYb2[i]};
            double v2[3] = {cylXu1[i]-cylXu2[i], -cylZu1[i]+cylZu2[i], cylYu1[i]-cylYu2[i]};
            double norm[3];
            mat::Vector3_Cross_Vector3(v1, v2, norm);

            glNormal3dv(norm);

            glVertex3f(cylXb2[i], -cylZb2[i], cylYb2[i]);
            glVertex3f(cylXu2[i], -cylZu2[i], cylYu2[i]);
            glVertex3f(cylXu1[i], -cylZu1[i], cylYu1[i]);
            glVertex3f(cylXb1[i], -cylZb1[i], cylYb1[i]);
        }
        glEnd();
    }
}


void GLWindow::onBeaconStateChanged()
{
    if(currentLedState != oldLedState)
    {
        if(demo)
        {
            emit(SendNewBeaconStateToSTC(currentLedState));
        }
    }
    oldLedState = currentLedState;
}

UINT8 GLWindow::findIntersectedBeacons()
{
    UINT8 intersect = 0x00;
    UINT8 beaconPos = 1;
    mat::stickCoords sc = mat::findStickCoords(neckRotAngles, moduleSnakeShift, moduleShift, outPostLength, 173);
    for(int i=0; i<8; i++)
    {
        if((int)(beaconPos & currentLedState) != 0)
        {
            if(isBeaconIntersected(i, sc))
            {
                intersect |= beaconPos;
                currentLedState &= ~beaconPos;
                onBeaconStateChanged();

                beaconList[i].isShining = false;
            }
        }
        beaconPos <<= 1;
    }
    oldLedState = currentLedState;
    return intersect;
}

bool GLWindow::isBeaconIntersected(UINT8 beaconNum, mat::stickCoords sc)
{
    if((!beaconList[beaconNum].isShining) || (!beaconList[beaconNum].isShown))  // check only shown shining beacons
    {
        return false;
    }

    double beaconCoords[3];
    beaconCoords[0] = beaconList[beaconNum].x;
    beaconCoords[1] = beaconList[beaconNum].y;
    beaconCoords[2] = beaconList[beaconNum].z;

    double dist = stickToBeaconDistance(sc, beaconCoords);
    if(dist < beaconRadius)
        return true;
    return false;
}


double GLWindow::stickToBeaconDistance(mat::stickCoords sc, double *a)
{
    double *b = sc.begin;
    double *e = sc.end;

    // avoid division by 0, return veri big value
    if ((e[0]+e[1]+e[2]-b[0]-b[1]-b[2]) == 0)
    {
        return 999999;
    }

    double k = ((a[0]-b[0])*(e[0]-b[0]) + (a[1]-b[1])*(e[1]-b[1]) + (a[2]-b[2])*(e[2]-b[2]))/
            ((e[0]-b[0])*(e[0]-b[0]) + (e[1]-b[1])*(e[1]-b[1]) + (e[2]-b[2])*(e[2]-b[2]));

    if(k<0)
    {
        return sqrt((a[0]-b[0])*(a[0]-b[0]) + (a[1]-b[1])*(a[1]-b[1]) + (a[2]-b[2])*(a[2]-b[2]));
    }
    else if (k>1)
    {
        return sqrt((a[0]-e[0])*(a[0]-e[0]) + (a[1]-e[1])*(a[1]-e[1]) + (a[2]-e[2])*(a[2]-e[2]));
    }
    else
    {
        double B[3];
        for(int i=0; i<3; i++)
        {
            B[i] = b[i] + k*(e[i]-b[i]);
        }
        return sqrt((a[0]-B[0])*(a[0]-B[0]) + (a[1]-B[1])*(a[1]-B[1]) + (a[2]-B[2])*(a[2]-B[2]));
    }
}






