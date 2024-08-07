#include <bits/stdc++.h>
#include <GL/glut.h>
#include <unistd.h>
using namespace std;

const float pi = 3.14159265;

class displayColor
{
    public:
    float r;
    float g;
    float b;
    displayColor(float x, float y, float z) {
        r = x;
        g = y;
        b = z;
    }
    displayColor() {
        r = 0;
        g = 0;
        b = 0;
    }
};

class Car
{
    public:
    pair <float, float> l;
    pair <float, float> u;
    displayColor body;
    displayColor hood;
    bool s;
    Car(pair <float, float> i, pair <float, float> j, displayColor k, displayColor n, bool x) {
        l = i;
        u = j;
        body = k;
        hood = n;
        s = x;
    }
};

class Obstacle
{
    public:
    pair <float, float> l;
    pair <float, float> u;
    float a;
    float w;
    float h;
    float xMove;
    float yMove;
    float stabilise;
    float stabilisationRate;
    displayColor car;
    displayColor hood;
    bool found;
    Obstacle() {
        this->a = 0;
        this->l = {50, 50};
        this->u = {60, 60};
        this->w = 0;
        this->h = 0;
        this->xMove = 0;
        this->yMove = 0;
        this->found = false;
    }
};

void display();
void reshape(int w, int h);
void timer(int x);
void keyboard(unsigned char k, int x, int y);
void init();

void initBack();

void plotPoint(float x, float y);
void drawQuad(pair <float, float> l, pair <float, float> u);
void drawQuadx(pair <float, float> l, pair <float, float> u, pair <float, float> x, float a);
void drawCircle(pair <float, float> c, float r, displayColor col);
void drawCar(Car x, float a, bool cc);

void gameLogic();
void collisionLogic();

float randGen();
void gameOver();
void gamePause();
void gameResume();
void gameMenu();
void healthBar(float carHealth);

bool polygonCollisionLogic(vector <pair <float, float>> a, vector <pair <float, float>> b);
bool interiorPoint(vector <pair <float, float>> a, pair <float, float> b);
int detSign(pair <float, float> a, pair <float, float> b, pair <float, float> c);

vector <pair < float, float>> genQuad(pair <float, float> l, pair <float, float> u, pair <float, float> x, float a);
pair <float, float> rotatePoint(pair <float, float> p, pair <float, float> x, float a);
vector <vector <float>> matrixMultiply(vector <vector <float>> x, vector <vector <float>> y);

float offSet = 0, carSpeed = 0.3, xMove = 0, yMove = 0, xRelSpeed = 0.4, yRelSpeed = 0.9;
int siren = 0, sirenSpeed = 5;
float copSet = 0, copSpeed = 0.15, copStabilise = 0, stabilisationRate = 1;
float turnFactor = 15;
bool copHere = 1;
float copAngle = 0;
int lCoolDown = 0, rCoolDown = 0, bCoolDown = 50;
float obsSpeed = 0.2, bulSpeed = 0.5;
float carHealth = 10;
float angle = 0;
int lvl = 10;
bool gameStart = 0, gamePaused = 0;
int blink = 0;
int score = 0;

vector <Obstacle> obs;
vector <pair <bool, pair <float, float>>> bul;

int main(int argc, char** argv)
{
    obs.resize(lvl);
    bul.resize(lvl);
    srand(time(0));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(1000, 300);
    glutInitWindowSize(1600, 1600);
    glutCreateWindow("Car Game");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    if(!gamePaused) glutTimerFunc(0, timer, 0);
    init();
    glutMainLoop();
    return 0;
}

void display()
{
    init();
    glLoadIdentity();

    if(gameStart) initBack();

    else gameMenu();

    glutSwapBuffers();

    glutKeyboardFunc(keyboard);
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-50, 50, -50, 50);
    glMatrixMode(GL_MODELVIEW);
}

void timer(int x)
{
    glutPostRedisplay();
    glutTimerFunc(1000/60, timer, 0);
    if(offSet < 50) offSet += carSpeed;
    else offSet -= 50;
    siren += 1;
    siren %= (120/(sirenSpeed+1));
    if(angle != 0) {
        if(angle > 0) {
            if(angle < 0.3) angle = 0;
            else angle -= 0.3;
        }
        else {
            if(angle > -0.3) angle = 0;
            else angle += 0.3;
        }
    }
    for(auto &i: obs) {
        if(i.found) {
            if(i.yMove < -110) i.found = false;
            else i.yMove -= obsSpeed;
        }
    }
    for(auto &i: bul) {
        if(i.first) {
            if(i.second.second > 90) i.first = false, score += 1;
            else i.second.second += bulSpeed;
        }
    }
    if(carHealth <= 0) gameOver();
    if(!gamePaused && gameStart) {
        if(lCoolDown > 0) lCoolDown -= 1;
        if(rCoolDown > 0) rCoolDown -= 1;
        if(bCoolDown > 0) bCoolDown -= 1;
    }
}

void keyboard(unsigned char k, int x, int y)
{
    switch (k)
    {
    case 'w':
        if(-13 + yMove < 50) yMove += yRelSpeed;
        break;
    case 's':
        if(-20 + yMove > -50) yMove -= yRelSpeed;
        break;
    case 'd':
        if(-8 + xMove < 17) xMove += xRelSpeed;
        angle -= 1;
        break;
    case 'a':
        if(-11 + xMove > -17) xMove -= xRelSpeed;
        angle += 1;
        break;
    case 'p':
        if(carSpeed != 0) gamePaused = 1, gamePause();
        else gamePaused = 0, gameResume();
        break;
    case 'b':
        if(!gameStart) gameStart = true;
        break;
    case 'c':
        string x; cin >> x;
        if(x == "leavemealone") copHere = 0;
        else if(x == "catchmeifyoucan") copHere = 1;
    }
    if(angle < -turnFactor) angle = -turnFactor;
    if(angle > turnFactor) angle = turnFactor;
    if(-8 + xMove > 17) xMove = 25;
    if(-11 + xMove < -17) xMove = -6;
    if(-13 + yMove > 50) yMove = 63;
    if(-20 + yMove < -50) yMove = -30;
}

void init()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.2, 0.2, 0.2, 1);
}

void initBack()
{
    //Grass
    glColor3f(0, 0.8, 0);
    drawQuad({-50, -50}, {-20, 50});
    drawQuad({20, -50}, {50, 50});
    //Health Bar
    healthBar(carHealth);
    //Score
    glColor3f(0, 0, 0);
    glRasterPos2f(33, 20);
    string x = "Score: ";
    x += to_string(score);
    for(int i = 0; i < x.size(); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, x[i]);
    }
    //Pavement
    glColor3f(0.65, 0.65, 0.65);
    for(int i = 0; i < 50; i++) {
        glColor3f(0.5, 0.5, 0.5);
        drawQuad({-20, -100+i*10 - offSet}, {-17, -100+i*10+5 - offSet});
        drawQuad({17, -100+i*10 - offSet}, {20, -100+i*10+5 - offSet});
        
    }
    glColor3f(0.7, 0.7, 0.7);
    for(int i = 0; i < 50; i++) {
        drawQuad({-20, -100+i*10+5 - offSet}, {-17, -100+i*10+10 - offSet});
        drawQuad({17, -100+i*10+5 - offSet}, {20, -100+i*10+10 - offSet});
    }
    //Lane divider
    glColor3f(1, 1, 1);
    for(int i = 0; i < 50; i++) drawQuad({-1, -100+i*10 - offSet}, {1, -100+i*10+5 - offSet});
    //Car
    drawCar(Car({-11 + xMove, -20 + yMove}, {-8 + xMove, -13 + yMove}, displayColor(0.23, 0.45, 0.56), displayColor(0.02, 0.12, 0.23), 0), angle, 1);
    //Game Logic
    gameLogic();
    //Obstacles
    for(auto i: obs) {
        if(i.found) drawCar(Car(i.l, i.u, i.car, i.hood, 0), 0, 0);
    }
    //Bullets
    if(copHere) {
        for(auto i: bul) {
            if(i.first) drawCircle(i.second, 0.5, displayColor(1, 0, 1));
        }
    }
    //Cop car
    if(copHere) drawCar(Car({-12 + copSet, -45}, {-7 + copSet, -35}, displayColor(0, 0, 0), displayColor(0, 0, 0), 1), copAngle, 0);
}

void plotPoint(float x, float y)
{
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

void drawQuad(pair <float, float> l, pair <float, float> u)
{
    glBegin(GL_QUADS);
    glVertex2f(l.first, l.second);
    glVertex2f(u.first, l.second);
    glVertex2f(u.first, u.second);
    glVertex2f(l.first, u.second);
    glEnd();
}

void drawQuadx(pair <float, float> l, pair <float, float> u, pair <float, float> x, float a)
{

    glBegin(GL_QUADS);

    glVertex2f(rotatePoint(x, {l.first, l.second}, a).first, rotatePoint(x, {l.first, l.second}, a).second);
    glVertex2f(rotatePoint(x, {u.first, l.second}, a).first, rotatePoint(x, {u.first, l.second}, a).second);
    glVertex2f(rotatePoint(x, {u.first, u.second}, a).first, rotatePoint(x, {u.first, u.second}, a).second);
    glVertex2f(rotatePoint(x, {l.first, u.second}, a).first, rotatePoint(x, {l.first, u.second}, a).second);

    glEnd();
}

void drawCircle(pair <float, float> c, float r, displayColor col)
{
    glColor3f(col.r, col.g, col.b);
    glBegin(GL_TRIANGLES);
    for(int i = 0; i < 360; i++) {
        glVertex2f(c.first + r*cos(i*(pi/180)), c.second + r*sin(i*(pi/180)));
        glVertex2f(c.first, c.second);
        glVertex2f(c.first + r*cos((i+1)*(pi/180)), c.second + r*sin((i+1)*(pi/180)));
    }
    glEnd();
}

void drawCar(Car x, float a, bool cc)
{
    float d = x.u.second - x.l.second, w = x.u.first - x.l.first, xCen = (x.l.first + x.u.first)/2, yCen = (x.l.second + x.u.second)/2;
    //Body
    glColor3f(x.body.r, x.body.g, x.body.b);
    drawQuadx({xCen - (w/2), yCen - (d/2)}, {xCen + (w/2), yCen + (d/2)}, {xCen, yCen}, a);
    //Windshield
    glColor3f(0, 1, 1);
    drawQuadx({xCen - (w/2)*(0.85), yCen - (d/2)*(0.5 + 0.25)}, {xCen + (w/2)*(0.85), yCen + (d/2)*(0.5 + 0.25)}, {xCen, yCen}, a);
    //Hood
    glColor3f(x.hood.r, x.hood.g, x.hood.b);
    drawQuadx({xCen - (w/2), yCen - (d/2)*(0.5)}, {xCen + (w/2), yCen + (d/2)*(0.5)}, {xCen, yCen}, a);
    //Lights
    glColor3f(1, 1, 0);
    drawQuadx({xCen - (w/2), yCen + (d/2)*(0.8)}, {xCen - (w/2)*(0.8), yCen + (d/2)}, {xCen, yCen}, a);
    drawQuadx({xCen + (w/2)*(0.8), yCen + (d/2)*(0.8)}, {xCen + (w/2), yCen + (d/2)}, {xCen, yCen}, a);
    //Siren
    if(x.s) {
        glColor3f((siren > (60/(sirenSpeed+1)) ? 0 : 1), 0, (siren > (60/(sirenSpeed+1)) ? 1 : 0));
        drawQuadx({xCen - (w/2), yCen + (d/2)*(0.5 - 0.2)}, {xCen, yCen + (d/2)*(0.5)}, {xCen, yCen}, a);
        glColor3f((siren > (60/sirenSpeed) ? 1 : 0), 0, (siren > (60/sirenSpeed) ? 0 : 1));
        drawQuadx({xCen, yCen + (d/2)*(0.5 - 0.2)}, {xCen + (w/2), yCen + (d/2)*(0.5)}, {xCen, yCen}, a);
    }
}

void gameLogic()
{
    //Cop Movement
    if(copStabilise == 0) {
        if(rand()%2 == 0) copStabilise = rand()%200;
        else copStabilise = -rand()%200;
    }
    else {
        if(copStabilise < 0) {
            copSet += copSpeed;
            copStabilise += stabilisationRate;
        }
        else {
            copSet -= copSpeed;
            copStabilise -= stabilisationRate;
        }
    }
    if(copSet < -3) copSet = -3;
    if(copSet > 22) copSet = 22;
    //Obstacle Cars
    //Left Lane
    for(int i = 0; i < (lvl/2); i++) {
        if(obs[i].found == false && lCoolDown == 0) {
            obs[i].a = 0;
            obs[i].found = true;
            lCoolDown = 160*(1.0 + randGen());
            obs[i].xMove = 0, obs[i].yMove = 50;
            obs[i].car = displayColor(randGen()/2, randGen()/2, randGen()/2);
            obs[i].hood = displayColor((obs[i].car.r)/2, (obs[i].car.g)/2, (obs[i].car.b)/2);
            obs[i].w = 3 + randGen(), obs[i].h = 6 + 3*randGen();
            obs[i].l = {-16 + obs[i].xMove, obs[i].yMove};
            obs[i].u = {-16 + obs[i].w + obs[i].xMove, obs[i].h + obs[i].yMove};
        }
        else if(obs[i].found) {
            obs[i].l = {-16 + obs[i].xMove, obs[i].yMove};
            obs[i].u = {-16 + obs[i].w + obs[i].xMove, obs[i].h + obs[i].yMove};
        }
    }
    //Right Lane
    for(int i = (lvl/2); i < lvl; i++) {
        if(obs[i].found == false && rCoolDown == 0) {
            obs[i].found = true;
            rCoolDown = 140*(1.0 + randGen());
            obs[i].xMove = 0, obs[i].yMove = 50;
            obs[i].car = displayColor(randGen()/2, randGen()/2, randGen()/2);
            obs[i].hood = displayColor((obs[i].car.r)/2, (obs[i].car.g)/2, (obs[i].car.b)/2);
            obs[i].w = 3 + randGen(), obs[i].h = 6 + 3*randGen();
            obs[i].l = {2 + obs[i].xMove, obs[i].yMove};
            obs[i].u = {2 + obs[i].w + obs[i].xMove, obs[i].h + obs[i].yMove};
            obs[i].stabilise = 0;
            obs[i].stabilisationRate = 1;
        }
        else if(obs[i].found) {
            obs[i].l = {2 + obs[i].xMove, obs[i].yMove};
            obs[i].u = {2 + obs[i].w + obs[i].xMove, obs[i].h + obs[i].yMove};
        }
    }
    //Obstacle Movement
    for(auto &i: obs) {
        if(i.stabilise == 0) {
            if(i.l.first < 0) {
                if(rand()%3 == 0) i.stabilise = rand()%80;
                else i.stabilise = -rand()%80;
            }
            else {
                if(rand()%3 != 0) i.stabilise = rand()%80;
                else i.stabilise = -rand()%80;
            }
        }
        else {
            if(i.stabilise < 0) {
                i.xMove += copSpeed;
                i.stabilise += i.stabilisationRate;
            }
            else {
                i.xMove -= copSpeed;
                i.stabilise -= i.stabilisationRate;
            }
        }
        if(i.l.first < 0) {
            if(i.l.first < -16) i.xMove += copSpeed, i.xMove += copSpeed, i.stabilise = 0;
            if(i.u.first > -1.2) i.xMove -= copSpeed, i.xMove -= copSpeed, i.stabilise = 0;
        }
        else {
            if(i.l.first < 1.2) i.xMove += copSpeed, i.xMove += copSpeed, i.stabilise = 0;
            if(i.u.first > 16) i.xMove -= copSpeed, i.xMove -= copSpeed, i.stabilise = 0;
        }
    }
    for(auto &i: bul) {
        if(i.first == false && bCoolDown == 0) {
            i.first = true;
            bCoolDown = 100 + 300*randGen();
            i.second = {-9.5 + copSet, -35};
        }
    }
    collisionLogic();
    if(carHealth <= 0) gameOver();
    if(carHealth > 10) carHealth = 10;
}

void collisionLogic()
{
    //Cop Collision
    vector <pair <float, float>> a = genQuad({-11 + xMove, -20 + yMove}, {-8 + xMove, -13 + yMove}, {-9.5 + xMove, -16.5 + yMove}, angle);
    vector <pair <float, float>> b = genQuad({-12 + copSet, -45}, {-7 + copSet, -35}, {-9.5 + copSet, -40}, copAngle);
    if(polygonCollisionLogic(a, b)) {
        gameOver();
    }
    //Obstacle Collision
    for(auto i: obs) {
        vector <pair <float, float>> c = genQuad(i.l, i.u, {0, 0}, 0);
        if(polygonCollisionLogic(a, c)) {
            gameOver();
        }
    }
    //Bullet Collision
    for(auto &i: bul) {
        if(interiorPoint(a, i.second) && i.first) {
            i.first = false;
            i.second.second = -35;
            carHealth -= 1;
        }
    }
}

float randGen()
{
    float x = 0;
    x = (rand()%2) + (rand()%2)/10 + (rand()%2)/100 + (rand()%2)/1000;
    return x;
}

void gameOver()
{
    carSpeed = 0;
    copSpeed = 0;
    xRelSpeed = 0;
    yRelSpeed = 0;
    angle = 0;
    obsSpeed = 0;
    bulSpeed = 0;
    carHealth = 0;
}

void gamePause()
{
    glColor3f(1, 1, 1);
    drawQuad({-40, -40}, {40, 40});
    carSpeed = 0;
    copSpeed = 0;
    xRelSpeed = 0;
    yRelSpeed = 0;
    angle = 0;
    obsSpeed = 0;
    bulSpeed = 0;
}

void gameResume()
{
    carSpeed = 0.3;
    copSpeed = 0.15;
    xRelSpeed = 0.4;
    yRelSpeed = 0.9;
    obsSpeed = 0.2;
    bulSpeed = 0.5;
}

void gameMenu()
{
    blink += 1;
    blink %= 20;
    glColor3f(0.2, 0.5, 0.3);
    drawQuad({-50, -50}, {50, 50});
    glColor3f(0.5, 0.1, 0.1);
    drawQuad({-40, -40}, {40, 40});
    if(blink < 10) glColor3f(1, 1, 1);
    else glColor3f(0, 0.8, 1);

    //C
    drawQuad({-35, 10}, {-33, 30});
    drawQuad({-33, 28}, {-25, 30});
    drawQuad({-33, 10}, {-25, 12});
    //A
    glBegin(GL_QUADS);
    glVertex2f(-23+3, 10);
    glVertex2f(-21+3, 10);
    glVertex2f(-18+3, 28);
    glVertex2f(-18+3, 30);
    glVertex2f(-18+3, 28);
    glVertex2f(-15+3, 10);
    glVertex2f(-13+3, 10);
    glVertex2f(-18+3, 30);
    glEnd();
    drawQuad({-20+3, 19}, {-16+3, 21});
    //R
    drawQuad({-11+6, 10}, {-9+6, 30});
    drawQuad({-3+6, 19}, {-1+6, 30});
    drawQuad({-9+6, 28}, {-3+6, 30});
    drawQuad({-9+6, 19}, {-3+6, 21});
    glBegin(GL_QUADS);
    glVertex2f(-9+6, 19);
    glVertex2f(-3+6, 10);
    glVertex2f(-1+6, 10);
    glVertex2f(-7+6, 19);
    glEnd();
    //X
    glBegin(GL_QUADS);
    glVertex2f(4+15, 10);
    glVertex2f(6+15, 10);
    glVertex2f(15+15, 30);
    glVertex2f(13+15, 30);
    glVertex2f(4+15, 30);
    glVertex2f(13+15, 10);
    glVertex2f(15+15, 10);
    glVertex2f(6+15, 30);
    glEnd();

    if(blink > 10) glColor3f(1, 1, 1);
    else glColor3f(0, 0.8, 1);

    glRasterPos2f(-10, 0);
    char x[] = "Press b to begin            wsad controls";
    for(int i = 0; i < strlen(x); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, x[i]);
    }
    glColor3f(1, 1, 0);
    glRasterPos2f(-9, -30);
    char y[] = "lordmavles a.k.a. kzyzahk   ;3";
    for(int i = 0; i < strlen(y); i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, y[i]);
    }
}

void healthBar(float carHealth)
{
    glColor3f(0, 0, 0);
    drawQuad({23, 28}, {47, 42});
    glColor3f(0, 1, 0);
    drawQuad({25, 30}, {25 + 2*carHealth, 40});
}

bool polygonCollisionLogic(vector <pair <float, float>> a, vector <pair <float, float>> b)
{
    for(auto i: a) {
        if(interiorPoint(b, i)) return true;
    }
    for(auto i: b) {
        if(interiorPoint(a, i)) return true;
    }
    return false;
}

bool interiorPoint(vector <pair <float, float>> a, pair <float, float> b)
{
    int size = a.size();
    for(int i = 0; i < a.size(); i++) {
        if(detSign(a[i%size], a[(i+1)%size], b) <= 0) return false;
    }
    return true;
}

int detSign(pair <float, float> a, pair <float, float> b, pair <float, float> c)
{
    float f = (a.first == b.first ? 0 : (c.second-b.second)-((b.second-a.second)/(b.first-a.first))*(c.first-b.first));
    if(b.second > a.second && b.first > a.first) {
        if(f > 0) return 1;
        else if(f < 0) return -1;
        return 0;
    }
    else if(b.second > a.second && b.first < a.first) {
        if(f > 0) return -1;
        else if(f < 0) return 1;
        return 0;
    }
    else if(b.second < a.second && b.first < a.first) {
        if(f > 0) return -1;
        else if(f < 0) return 1;
        return 0;
    }
    else if(b.second < a.second && b.first > a.first) {
        if(f > 0) return 1;
        else if(f < 0) return -1;
        return 0;
    }
    else if(b.second == a.second) {
        if(b.first > a.first) {
            if(c.second > b.second) return 1;
            else if(c.second < b.second) return -1;
            return 0;
        }
        else {
            if(c.second > b.second) return -1;
            else if(c.second < b.second) return 1;
            return 0;
        }
    }
    else {
        if(b.second > a.second) {
            if(c.first < b.first) return 1;
            else if(c.first > b.first) return -1;
            return 0;
        }
        else {
            if(c.first < b.first) return -1;
            else if(c.first > b.first) return 1;
            else return 0;
        }
    }
    return 0;
}

vector <pair < float, float>> genQuad(pair <float, float> l, pair <float, float> u, pair <float, float> x, float a)
{
    vector <pair <float, float>> c;
    c.push_back({rotatePoint(x, {l.first, l.second}, a).first, rotatePoint(x, {l.first, l.second}, a).second});
    c.push_back({rotatePoint(x, {u.first, l.second}, a).first, rotatePoint(x, {u.first, l.second}, a).second});
    c.push_back({rotatePoint(x, {u.first, u.second}, a).first, rotatePoint(x, {u.first, u.second}, a).second});
    c.push_back({rotatePoint(x, {l.first, u.second}, a).first, rotatePoint(x, {l.first, u.second}, a).second});
    return c;
}

pair <float, float> rotatePoint(pair <float, float> p, pair <float, float> x, float a)
{
    a *= (pi/180);
    return {p.first + (x.first-p.first)*cos(a)-(x.second-p.second)*sin(a), p.second + (x.first-p.first)*sin(a)+(x.second-p.second)*cos(a)};
}

vector <vector <float>> matrixMultiply(vector <vector <float>> x, vector <vector <float>> y)
{
    pair <int, int> xs = {x.size(), x[0].size()}, ys = {y.size(), y[0].size()};
    if(xs.second != ys.first) return {{0}};
    vector <vector <float>> r (xs.first, vector <float> (ys.second, 0));
    for(int i = 0; i < xs.first; i++) {
        for(int j = 0; j < ys.second; j++) {
            for(int k = 0; k < xs.second; k++) r[i][j] += x[i][k]*y[k][j];
        }
    }
    return r;
}