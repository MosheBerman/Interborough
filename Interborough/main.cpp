//
//  main.cpp
//  Android
//
//  Created by Moshe Berman on 3/19/13.
//  Copyright (c) 2013 Moshe Berman. All rights reserved.
//

/* GL Related Libraries */

#ifdef __APPLE__
    #include <GLUT/GLUT.h>
    #include <OpenGL/OpenGL.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/freeglut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

/* Standard Libraries */
#include <iostream>
#include <math.h>
#include <vector>

/* Loader Library */

#pragma mark - OpenGL

/* OpenGL/GLUT */

void init();
void display();
void key(unsigned char key, int x, int y);
void reshape(int width, int height);
void timedLoop(int val);

#pragma mark - Scenery

/* Train parts */

void trackWithID(int id);
void wheel();
void car(int carID, int trainID);   //  Which car in which train is it?
void train(int trainID);            //  What train are we rendering?

/* Tracks */

void trackSegmentOfLength(GLfloat length);
void railOfLength(GLfloat length);
void tie();
void track();

/* Platform */
void platform(int platformID);

/* Lighting */
void setLightColor(GLenum light, float *ambientColor, float *specularColor, float *diffuseColor);
void configureSpotlight(GLenum lightID, float *position, float *direction, float angle, float exponent);
void configureAmbientLight(GLenum lightID, float *position, float *direction, float *color);

/* Alternates between two colors */
float *alternatingColor;
void alternateColor(float *firstColor, float *secondColor);

/* Creates vertices of a rect prism with given dimensions */
void rectangularPrism(float width, float height, float length);


#pragma mark - Globals

//  A quadric used for drawing wheels
GLUquadric *quadric;

//  Automatic train animation
bool paused = false;

/* Train Controls */
int traintrackCount = 0;                //  How many tracks are there?
int traintrackUserControlled = 0;       //  Which track is the user controlling?
bool *traintrackShowTrain = new bool;   //  Does this track show a train?
int *traintrackDirection = new int;     //  Is the train going North or South?

/* configures a track */
void installTrack(bool showTrain, int direction);

#pragma mark - Position

// Math Yay
#define DEG2RAD 0.0174532925

//  Used for positioning trainsalong the track
typedef struct { float x, y, z; } Vector3f;

//  Position is per train
std::vector<Vector3f> position;

#define DEFAULT_X_TRANSLATE 0.0f
#define DEFAULT_Y_TRANSLATE -1.1f
#define DEFAULT_Z_TRANSLATE -5.0f

//  World coordinates
float trackRotation[3] =  {0.0f, 0.0f, 0.0f};
float worldRotation[3] =  {0.0f, 0.0f, 0.0f};
float translate[3] = {DEFAULT_X_TRANSLATE ,DEFAULT_Y_TRANSLATE ,DEFAULT_Z_TRANSLATE};


#pragma mark - Color Constants

//  Dark Brown, per http://www.opengl.org/discussion_boards/showthread.php/132502-Color-tables
//  The rest are simple

float black[4] = {0.0f, 0.0f, 0.0f, 1.0f};
float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float darkBrown[4] = {0.35f, 0.16f, 0.14f, 1.0f};
float blueishGray[4] = {0.7f,0.7f,0.73f, 1.0f};
float lightGray[4] = {0.9f,0.9f,0.9f, 1.0f};
float darkGray[4] = {0.3f, 0.3f, 0.3f, 1.0f};
float yellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};
float blue[4] = {0.2f, .2f, 0.6f, 1.0f};

#pragma mark - Viewport Constants

/* Window/Screen */

#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 320
#define FRUSTUM_DEPTH 1000.0
#define SCREEN_FPS 30.0

#pragma mark - Train Constants

/* Track constants */

#define TIE_WIDTH 1.2
#define TIE_HEIGHT 0.02
#define TIE_DEPTH 0.04
#define TRACK_SEGMENT_LENGTH 4.0
#define TRACK_LENGTH 1000.0

/*  Car constants */
#define CAR_LENGTH 2.0
#define CAR_HEIGHT 1.1
#define CARS_PER_TRAIN 10
#define LIGHTS_PER_TRAIN 2

/* Platform Constants */
#define PLATFORM_LENGTH 30.0f

/* Main Program */

int main(int argc, char ** argv)
{
    //  The glut initialization function
    glutInit(&argc, argv);
    
    //  Set up display settings
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GL_DOUBLE);
    
    //  Configure initial windowing settings
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    //  Create the window with the given title
    glutCreateWindow("Interborough Rapid Transit");
    
    //  This is our own init function, setting up GL
    init();
    
    //  Handle screen resizes.
    glutReshapeFunc(reshape);
    
    //  Register the function that handles rendering each frame
    glutDisplayFunc(display);
    
    //  Register the function that handles key down events
    glutKeyboardFunc(key);
    
    //Set main loop
	glutTimerFunc( 1000 / SCREEN_FPS, timedLoop, 0 );
    
    //  Invoke the main loop
    glutMainLoop();
    
    return 0;
}

/* Set up some initial state */

void init()
{
    //  Clear color
    glClearColor(0.0, 0.0, 0.0, 1.0);
    
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    //  Create a quadric for our glu cylinders
    quadric = gluNewQuadric();
    
    // Ensure that we don't destroy colors with lighting
    glEnable(GL_COLOR_MATERIAL);
    
    //  Turn on lighting
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    
    //Anti-aliasing
    glEnable (GL_POLYGON_SMOOTH);
    glEnable (GL_BLEND);

    //This causes triangles to show.
    //    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
}


void reshape(int width, int height)
{
    
    float ratio = (float)width / (float)height;
    
    // Apply perspective matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(45, ratio, 1.0, FRUSTUM_DEPTH);
    
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Change the camera to a 3D view
    	glViewport(0, 0, width, height);
    glFrustum( -1 * (float) width/2,
              (float) width/2,
              -10.0,
              10.0,
              -FRUSTUM_DEPTH,
              FRUSTUM_DEPTH);
    
}

void displayTrainScene()
{
    
    //  Clear the previous frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Push world matrix
    glPushMatrix();
    {
        
        glTranslated(translate[0], translate[1], translate[2]);
        glRotatef(trackRotation[1], 0, 1, 0);
        glRotatef(trackRotation[0], 1, 0, 0);
        
        glPushMatrix();
        {
            glTranslatef(0.0f, -0.4f, -(PLATFORM_LENGTH*3
                                        ));
            platform(0);
        }
        glPopMatrix();
        
        glPushMatrix();
        {
            glTranslatef(0.0f, -0.4f, -(PLATFORM_LENGTH/2));
            platform(1);
        }
        glPopMatrix();
        
        glPushMatrix();
        {
            glTranslatef(0.0f, -0.4f, (PLATFORM_LENGTH*3));
            platform(2);
        }
        glPopMatrix();
        
        
        glPushMatrix();
        {
            glTranslatef(0.0f, -0.4f, (PLATFORM_LENGTH*3));
            platform(3);
        }
        glPopMatrix();
        
        //  Reset the number of trains
        traintrackCount = 0;
        
        glPushMatrix();
        {
            glTranslatef(1.5f, 0.0f, 0.0f);
            installTrack(1, 0);
        }
        glPopMatrix();
        
        glPushMatrix();
        {
            glTranslatef(-1.5f, 0.0f, 0.0f);
            installTrack(1, 1);
        }
        glPopMatrix();
        
    }
    
    glPopMatrix();
}

/* Test view */

void displayCar()
{
    //  Clear the previous frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    
    // Push world matrix
    glPushMatrix();
    {
        glTranslated(translate[0], translate[1], translate[2]);
        
        glPushMatrix();
        {
            glTranslatef(0, -CAR_HEIGHT, 0);
            glBegin(GL_QUADS);
            glColor4fv(yellow);
            rectangularPrism(100, 1, 100);
            glEnd();
            
            glTranslatef(0, CAR_HEIGHT, 0);
            
            car(0, 0);
            
        }
        glPopMatrix();
        
    }
    glPopMatrix();
    
}

/* This function is run on every runloop. */

void display()
{
//    displayCar();
    
    glPushMatrix();
    {
        glRotated(worldRotation[1], 0, 1, 0);
        displayTrainScene();
    }
    glPopMatrix();
    
    // Flush and swap.
    glutSwapBuffers();
    glFlush();
}



/*
 
 This function handkes keydown events
 
 Controls:
 W - Track Forward
 S - Track backward
 A - Track left
 D - Track right
 E - Track Up
 F - Track Down
 
 X - Train forward
 Z - Train back

 P - Pause
 R - Reset
 
 */

void key(unsigned char key, int x, int y)
{
    float deltaPos = 0.1;
    
    if (traintrackDirection[traintrackUserControlled] == 0) {
        deltaPos *= -1;
    }

    switch (key) {
        case 'a':
            translate[0]++;
            break;
        case 'd':
            translate[0]--;
            break;
        case 'f':
            translate[1]++;
            break;
        case 'e':
            translate[1]--;
            break;
        case 'w':
            translate[2]++;
            break;
        case 's':
            translate[2]--;
            break;
        case 'z':
            position[traintrackUserControlled].x -= sin(DEG2RAD * trackRotation[1]) * deltaPos;
            position[traintrackUserControlled].z += cos(DEG2RAD * trackRotation[1]) * deltaPos;
            break;
        case 'x':
            position[traintrackUserControlled].x += sin(DEG2RAD * trackRotation[1]) * deltaPos;
            position[traintrackUserControlled].z -= cos(DEG2RAD * trackRotation[1]) * deltaPos;
            break;
        case 'l':
        {
            bool light = glIsEnabled(GL_LIGHTING);
            
            if (light) {
                glDisable(GL_LIGHTING);
            }
            else
            {
                
                glEnable(GL_LIGHTING);
            }
        }
            break;
        case 't':
            
            if (traintrackUserControlled < traintrackCount-1) {
                traintrackUserControlled++;
            }
            else{
                traintrackUserControlled = 0;
            }
            
            break;
        case 'r':
            trackRotation[0] = 0;
            trackRotation[1] = 0;
            trackRotation[2] = 0;
            
            worldRotation[0] = 0;
            worldRotation[1] = 0;
            worldRotation[2] = 0;

            for (int i = 0; i < position.size(); i++) {
                position[i].x = 0.0f;
                position[i].y = 0.0f;
                position[i].z = -3.0f;
            }
            
            translate[0]= DEFAULT_X_TRANSLATE;
            translate[1]= DEFAULT_Y_TRANSLATE;
            translate[2]= DEFAULT_Z_TRANSLATE;
            break;
        case 'j':
            worldRotation[1]++;
            break;
        case 'k':
            worldRotation[1]--;
            break;
        case 'p':
            paused = !paused;
            break;
        default:
            break;
    }

    
    glutPostRedisplay();
}

/* Cleans up at the end of our program */

void cleanup()
{
    gluDeleteQuadric(quadric);
}

#pragma mark - Rail Line Drawing

/* configures a track */
void installTrack(bool showTrain, int direction)
{
    traintrackShowTrain[traintrackCount] = showTrain;
    traintrackDirection[traintrackCount] = direction;
    
//    if(position.size() < NUMBER_OF_TRACKS){
        Vector3f vector = {0.0f, 0.0, 3.0f};
        position.push_back(vector);
//    }

    trackWithID(traintrackCount);
    traintrackCount++;
}

void trackWithID(int id)
{
    
    if(traintrackShowTrain[id]){
        //  New matrix allows train movement along the track.
        glPushMatrix();
        {
            //  Controllable translation applies only to the car
            glTranslatef(position[id].x, position[id].y, position[id].z);
        
            // Render the train
            train(id);
        }
        glPopMatrix();
    }
    
    // Render the track
    glPushMatrix();
    {
        //  Move track down
        glTranslatef(0, -0.6, 0);
        
        //  Render the track
        track();
    }
    glPopMatrix();
}

#pragma mark - Car Parts

void wheel()
{
    glPushMatrix();
    {
        //  Rotate the wheels to be aligned with the track
        glRotatef(90, 0.0, 1.0, 0);
        
        //  Make them dark gray
        glColor4fv(darkGray);
        gluCylinder(quadric, 0.08, 0.06, 0.05, 32, 32);
    }
    glPopMatrix();
}

void car(int carID, int trainID)
{
    /*  Car */
    
    const float xFromCarCenter = 0.5;
    const float yFromCarCenter = 0.5;
    
    glPushMatrix();
    {
        glTranslatef(0, 0.04, 0);
        
        glBegin(GL_QUADS);
        {
            glColor3f(0.7f, 0.7f, 0.71f);
            rectangularPrism(1.0, CAR_HEIGHT, CAR_LENGTH);
            
        }
        glEnd();
    }
    
    glPopMatrix();
    
    /* 4 Wheels */
    
    glPushMatrix();
    {
        glTranslatef(-xFromCarCenter, -yFromCarCenter, CAR_LENGTH/2.0);
        wheel();
    }
    glPopMatrix();
    
    glPushMatrix();
    {
        glTranslatef(xFromCarCenter, -yFromCarCenter, CAR_LENGTH/2.0);
        wheel();
    }
    glPopMatrix();
    
    glPushMatrix();
    {
        glTranslatef(-xFromCarCenter, -yFromCarCenter, -CAR_LENGTH/2.0);
        wheel();
    }
    glPopMatrix();
    
    glPushMatrix();
    {
        glTranslatef(xFromCarCenter, -yFromCarCenter, -CAR_LENGTH/2.0);
        wheel();
    }
    glPopMatrix();
}


#pragma mark - Train

void train(int trainID)
{
    glPushMatrix();
    {
        
        float numCars = 10;
        
        for (int i = 0; i <numCars; i++) {
            car(i, trainID);
            glTranslatef(0, 0, -CAR_LENGTH*1.2);
        }
    }
    glPopMatrix();
}

#pragma mark - Track

void track()
{
    
    /* Add track in segments */
    
    float backOfTrack = -TRACK_LENGTH;
    
    //  Start from the back and keep adding ties
    while (backOfTrack < TRACK_LENGTH)
    {
        glPushMatrix();
        {
            glTranslatef(0, 0, backOfTrack);
            trackSegmentOfLength(TRACK_SEGMENT_LENGTH);
        }
        glPopMatrix();
        
        backOfTrack += TRACK_SEGMENT_LENGTH;
    }
}

void trackSegmentOfLength(GLfloat length)
{
    
    /* Add railroad ties to the tracks */
    
    float spaceBetweenTies = (float)TIE_DEPTH*2.0;
    float segmentPosition = 0;
    
    //  Start from the back and keep adding ties
    while (segmentPosition < (float)length)
    {
        glPushMatrix();
        {
            glTranslatef(0, 0, segmentPosition);
            tie();
        }
        glPopMatrix();
        
        segmentPosition += spaceBetweenTies + (TIE_DEPTH*5);
    }

    
    /* Now add the rails. */
    
    glPushMatrix();
    {
        glTranslatef(-0.45, 0, 0);
        railOfLength(length);
    }
    glPopMatrix();
    
    glPushMatrix();
    {
        glTranslatef(0.5, 0, 0);
        railOfLength(length);
    }
    glPopMatrix();
    
    /* The third rail - 600V! */
    glPushMatrix();
    {
        glTranslatef(0.2, 0, 0);
        railOfLength(length);
    }
    glPopMatrix();
    
}

void tie()
{
    glBegin(GL_QUADS);
    {
        glColor3fv(darkBrown);
        rectangularPrism(TIE_WIDTH, TIE_HEIGHT, TIE_DEPTH);
    }
    glEnd();
}

void railOfLength(GLfloat length)
{
    glBegin(GL_QUADS);
    {
        glColor3f(0.3, 0.3, 0.3);
        rectangularPrism(0.06, 0.04, length);
    }
    glEnd();
}

#pragma mark - Subway Station

float platformHeight = 0.4f;
float platformWidth = 1.6f;

float stripWidth = 0.04f;
float stripHeight = 0.02f;

float tileSide = stripWidth*4;
float tileRows = (platformWidth-stripWidth)/tileSide;
float tileColumns = PLATFORM_LENGTH/tileSide;

float pillarHeight = tileSide*6;

/* Yellow safety strips at the edge of the platform */

void safetyStrip()
{
    glBegin(GL_QUADS);
    {
        rectangularPrism(stripWidth, stripHeight, PLATFORM_LENGTH);
    }
    glEnd();
}

/* Draws a square tile on the platform */

void horizontalTile()
{
    glBegin(GL_QUADS);
    {
        rectangularPrism(tileSide, stripHeight, tileSide);
    }
    glEnd();
}

/* Draws a base for the platform */

void platformBase()
{
    glBegin(GL_QUADS);
    {
        glColor4fv(darkGray);
        
        //  Platform surface
        rectangularPrism(platformWidth, platformHeight, PLATFORM_LENGTH);
    }
    glEnd();
}

/* Draws a pillar */
void pillar()
{
    glBegin(GL_QUADS);
    {
        //  Make it blue
        glColor4fv(blue);
        
        //  Platform surface
        rectangularPrism(tileSide, pillarHeight, tileSide);
    }
    glEnd();
}

/*
 
 Draws a train platform, complete with tiles,
 safety strips, and akwardly placed beams.
 
 */
void platform(int platformID)
{
    glPushMatrix();
    {
        //  Draw the base of the platform
        platformBase();
        
        //
        //  Safety Strips
        //
        
        glColor4fv(yellow);  //  Draw em yellow
        
        glPushMatrix();
        {
            glTranslatef(-platformWidth/2-stripWidth, platformHeight/2+stripHeight, 0);
            safetyStrip();
        }
        glPopMatrix();
        
        glPushMatrix();
        {
            glTranslatef(platformWidth/2+stripWidth, platformHeight/2+stripHeight, 0);
            safetyStrip();
        }
        glPopMatrix();
        
        /* Floor Tiles */
        
        glPushMatrix();
        {
            for (int i = 0; i <tileRows;i++) {
                
                alternateColor(lightGray, darkGray);
                
                for (int j = 0; j < tileColumns; j++) {
                    
                    alternateColor(lightGray, darkGray);
                    
                    glPushMatrix();
                    {
                        //
                        //  Adjust for the tile and platform offset
                        //
                        
                        float tileOriginX = (-platformWidth/2)+tileSide/2+tileSide*i;
                        float tileOriginZ = (-PLATFORM_LENGTH/2)+tileSide*j;
                        
                        glTranslatef(tileOriginX, platformHeight/2 + stripHeight, tileOriginZ);
                        horizontalTile();
                    }
                    glPopMatrix();
                }
            }
        }
        glPopMatrix();
        
        /* Pillars */
        
        {
        //  Front left
        glPushMatrix();
        {
            glTranslatef(-platformWidth/2+tileSide, platformHeight/2+(pillarHeight/2), PLATFORM_LENGTH/2 - tileSide);
            pillar();
        }
        glPopMatrix();
        
        //  Front right
        glPushMatrix();
        {
            glTranslatef(platformWidth/2-tileSide, platformHeight/2+(pillarHeight/2), PLATFORM_LENGTH/2 - tileSide);
            pillar();
        }
        glPopMatrix();
        
        //  Middle right
        glPushMatrix();
        {
            glTranslatef(platformWidth/2-tileSide, platformHeight/2+(pillarHeight/2), 0);
            pillar();
        }
        glPopMatrix();
        
        //  Middle Left
        glPushMatrix();
        {
            glTranslatef(-platformWidth/2+tileSide, platformHeight/2+(pillarHeight/2), 0);
            pillar();
        }
        glPopMatrix();
        
        //  Back right
        glPushMatrix();
        {
            glTranslatef(platformWidth/2-tileSide, platformHeight/2+(pillarHeight/2), -PLATFORM_LENGTH/2 + tileSide);
            pillar();
        }
        glPopMatrix();
        
        //  Back left
        glPushMatrix();
        {
            glTranslatef(-platformWidth/2+tileSide, platformHeight/2+(pillarHeight/2), -PLATFORM_LENGTH/2 + tileSide);
            pillar();
        }
        glPopMatrix();
        }
        
        /* Platform spotlight */
        
        float lightID = platformID;
        
        GLfloat position[4] = {0,-(platformHeight/2)+pillarHeight*2,0,1};
        GLfloat direction[4] = {0, 0, 0};
        GLfloat ambient[4] = {0.7, 0.7, 0.7, 0.1};
        GLfloat specular[4] = {0.7, 0.7, 0.7, 0.1};
        GLfloat diffuse[4] = {0.7, 0.7, 0.7, 0.01};
    
        configureSpotlight(lightID, position, direction, 90, 2);
        configureAmbientLight(lightID, position, direction, ambient);
        setLightColor(lightID, ambient, specular, diffuse);
    }    
    glPopMatrix();
    
}

#pragma mark - Lighting

void setLightColor(GLenum light, float *ambientColor, float *specularColor, float *diffuseColor)
{
    if(ambientColor)    glLightfv(light, GL_AMBIENT, ambientColor);
    if(specularColor)   glLightfv(light, GL_SPECULAR, specularColor);
    if(diffuseColor)    glLightfv(light, GL_DIFFUSE, diffuseColor);
}

/* Converts an int to a gl #defined light */

int _glLightForInt(int lightID)
{
    int light[8] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};
    
    for (int i = 0; i < 7; i++) {
        if (lightID == light[i]) {
            return lightID;
        }
    }
    
    return light[lightID];
}

/* Configures a light as a spotlight */
void configureSpotlight(GLenum lightID, float *position, float *direction, float angle, float exponent)
{
    
    // "unwrap" the light
    lightID = _glLightForInt(lightID);
    
    glLightfv(lightID, GL_POSITION, position );
    glLightfv(lightID, GL_SPOT_DIRECTION, direction);
    glLightf(lightID, GL_SPOT_CUTOFF, angle); // angle is 0 to 90 or 180
    glLightf(lightID, GL_SPOT_EXPONENT, exponent); // exponent is 0 to 128
    
    glLightf(lightID, GL_LINEAR_ATTENUATION, 1.0);
    
    glEnable(lightID);
}

/* Configures a light as an ambient light */
void configureAmbientLight(GLenum lightID, float *position, float *direction, float *color)
{
    // "unwrap" the light
    lightID = _glLightForInt(lightID);
    
    glLightfv(lightID, GL_POSITION, position );
    glLightfv(lightID, GL_SPOT_DIRECTION, direction);
    glLightfv(lightID, GL_AMBIENT, color);
    
    glEnable(lightID);
}


#pragma mark - Rectangular Prism

/*
 
 Calls the prism function without a texture
 
 */

void rectangularPrism(float width, float height, float length){
    
    glEnable(GL_NORMALIZE);
    
    float faceWidth = width/2;
    float faceHeight = height/2;
    float faceLength = length/2;
    
    /* Back Normal */
    glNormal3f(faceWidth, faceHeight, -faceLength);
    glNormal3f(-faceWidth, faceHeight, -faceLength);
    glNormal3f(-faceWidth, faceHeight, faceLength);
    glNormal3f(faceWidth, faceHeight, faceLength);
    
    /* Back Surface */
    glVertex3d(faceWidth, faceHeight, -faceLength);
    glVertex3d(faceWidth, -faceHeight, -faceLength);
    glVertex3d(-faceWidth, -faceHeight, -faceLength);
    glVertex3d(-faceWidth, faceHeight, -faceLength);
    
    /* Front Normal */
    glNormal3f(faceWidth, faceHeight, -faceLength);
    glNormal3f(-faceWidth, faceHeight, -faceLength);
    glNormal3f(-faceWidth, faceHeight, faceLength);
    glNormal3f(faceWidth, faceHeight, faceLength);
    
    /* Front Surface */
    glVertex3d(faceWidth, faceHeight, faceLength);
    glVertex3d(faceWidth, -faceHeight, faceLength);
    glVertex3d(-faceWidth, -faceHeight, faceLength);
    glVertex3d(-faceWidth, faceHeight, faceLength);
    
    /* Top Normal */
    glNormal3f(faceWidth, faceHeight, faceLength);
    glNormal3f(faceWidth, -faceHeight, faceLength);
    glNormal3f(-faceWidth, -faceHeight, faceLength);
    glNormal3f(-faceWidth, faceHeight, faceLength);
    
    /* Top Surface */
    glVertex3d(faceWidth, faceHeight, -faceLength);
    glVertex3d(-faceWidth, faceHeight, -faceLength);
    glVertex3d(-faceWidth, faceHeight, faceLength);
    glVertex3d(faceWidth, faceHeight, faceLength);
    
    
    /* Bottom Normal */
    glNormal3f(faceWidth, faceHeight, faceLength);
    glNormal3f(faceWidth, -faceHeight, faceLength);
    glNormal3f(-faceWidth, -faceHeight, faceLength);
    glNormal3f(-faceWidth, faceHeight, faceLength);
    
    /* Bottom Surface */
    glVertex3d(faceWidth, -faceHeight, -faceLength);
    glVertex3d(-faceWidth, -faceHeight, -faceLength);
    glVertex3d(-faceWidth, -faceHeight, faceLength);
    glVertex3d(faceWidth, -faceHeight, faceLength);
    
    /* Left Normal */
    glNormal3f(faceWidth, faceHeight, -faceLength);
    glNormal3f(-faceWidth, faceHeight, -faceLength);
    glNormal3f(-faceWidth, faceHeight, faceLength);
    glNormal3f(faceWidth, faceHeight, faceLength);
    
    /* Left Surface */
    glVertex3d(-faceWidth, faceHeight, -faceLength);
    glVertex3d(-faceWidth, -faceHeight, -faceLength);
    glVertex3d(-faceWidth, -faceHeight, faceLength);
    glVertex3d(-faceWidth, faceHeight, faceLength);
    
    /* Right Normal */
    glNormal3f(faceWidth, faceHeight, -faceLength);
    glNormal3f(-faceWidth, faceHeight, -faceLength);
    glNormal3f(-faceWidth, faceHeight, faceLength);
    glNormal3f(faceWidth, faceHeight, faceLength);
    
    /* Right Surface */
    glVertex3d(faceWidth, faceHeight, -faceLength);
    glVertex3d(faceWidth, -faceHeight, -faceLength);
    glVertex3d(faceWidth, -faceHeight, faceLength);
    glVertex3d(faceWidth, faceHeight, faceLength);
    
    glDisable(GL_NORMALIZE);
}


#pragma mark - Animation

/* Timer */

void updateTrain(int id)
{
    
    float deltaPos = -0.1;
    
    if(paused)
    {
        return;
    }
    
    if (traintrackDirection[id] == 0) {
        deltaPos *= -1;
    }
    
    position[id].x -= sin(DEG2RAD * trackRotation[1]) * deltaPos;
    position[id].z += cos(DEG2RAD * trackRotation[1]) * deltaPos;
    
    if (position[id].z > FRUSTUM_DEPTH) {
        position[id].z = -FRUSTUM_DEPTH;
    }
}

void timedLoop(int val)
{
    //  Update each train's position
    for (int i=0; i<traintrackCount; i++) {
        updateTrain(i);
    }
    
    display();
    
    //Run frame one more time
    glutTimerFunc( 1000 / SCREEN_FPS, timedLoop, val );
}

#pragma mark - Utility Functions

/* Changes between two colors */

void alternateColor(float *firstColor, float *secondColor)
{
    if (alternatingColor == firstColor) {
        alternatingColor = secondColor;
    }
    else
    {
        alternatingColor = firstColor;
    }
    
    glColor4fv(alternatingColor);
}
