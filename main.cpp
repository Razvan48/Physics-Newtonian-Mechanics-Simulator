#include <iostream>
#include <vector>
#include <cmath>

#include <cstdlib>

#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using namespace std;

const double WINDOW_WIDTH = 768.0;
const double WINDOW_HEIGHT = 768.0;

//const double WINDOW_WIDTH = 400.0;
//const double WINDOW_HEIGHT = 400.0;

const double SCALAR_GRAVITY = 500.0;

double CURRENT_GRAVITY_X = 0.0;
double CURRENT_GRAVITY_Y = -SCALAR_GRAVITY;

const double FRICTION = 0.7;

const double PI = 3.14159265359;

const int NUMBER_OF_SIMULATIONS = 256;

const char* vertexShaderSource =
"#version 330 core \n"
"\n"
"layout (location = 0) in vec2 vertexPosition; \n"
"uniform mat4 ortho; \n"
"\n"
"void main() \n"
"{ \n"
"\n"
"   gl_Position = ortho * vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0); \n"
"\n"
"} \n"
"\0";

const char* fragmentShaderSource =
"#version 330 core \n"
"\n"
"out vec4 vertexColour; \n"
"uniform vec3 colour; \n"
"\n"
"void main() \n"
"{ \n"
"\n"
"   vertexColour = vec4(colour, 1.0); \n"
"\n"
"} \n"
"\0";

int colourPath;

double currentTime;
double previousTime;
double deltaTime;

double simulationDeltaTime;

void updateDeltaTime()
{
    currentTime = glfwGetTime();
    deltaTime = currentTime - previousTime;
    previousTime = currentTime;

    simulationDeltaTime = deltaTime / NUMBER_OF_SIMULATIONS;
}

struct Circle;

vector<Circle*> circles;

struct Circle
{
    double posX;
    double posY;
    double radius;

    double mass;

    double red;
    double green;
    double blue;

    double speedX;
    double speedY;

    unsigned int VAO;
    unsigned int VBO;

    vector<double> drawnPoints;

    bool playerControlled;

    const double angleStep = PI / 16.0;

    Circle() = default;

    Circle(double posX, double posY, double radius, double red = 1.0, double green = 0.0, double blue = 0.0, double mass = 1.0, double speedX = 0.0, double speedY = 0.0)
    {
        this->posX = posX;
        this->posY = posY;
        this->radius = radius;

        this->mass = mass;

        this->speedX = speedX;
        this->speedY = speedY;

        this->red = red;
        this->green = green;
        this->blue = blue;

        this->playerControlled = false;

        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);

        circles.push_back(this);
    }

    void draw()
    {
        this->drawnPoints.clear();

        double currentAngle = 0.0;

        while (currentAngle < 2.0 * PI)
        {
            this->drawnPoints.emplace_back(this->posX + this->radius * cos(currentAngle));
            this->drawnPoints.emplace_back(this->posY + this->radius * sin(currentAngle));

            this->drawnPoints.emplace_back(this->posX);
            this->drawnPoints.emplace_back(this->posY);

            this->drawnPoints.emplace_back(this->posX + this->radius * cos(currentAngle + this->angleStep));
            this->drawnPoints.emplace_back(this->posY + this->radius * sin(currentAngle + this->angleStep));

            currentAngle += this->angleStep;
        }

        glBindVertexArray(this->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

        glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
        glEnableVertexAttribArray(0);

        glBufferData(GL_ARRAY_BUFFER, sizeof(double) * this->drawnPoints.size(), &(this->drawnPoints.front()), GL_DYNAMIC_DRAW);

        glUniform3f(colourPath, this->red, this->green, this->blue);

        glDrawArrays(GL_TRIANGLES, 0, this->drawnPoints.size() / 2);
    }
};

struct Capsule;

vector<Capsule*> capsules;

struct Capsule
{
    double posX[2];
    double posY[2];
    double radius;

    double red;
    double green;
    double blue;

    unsigned int VAO;
    unsigned int VBO;

    bool playerControlled;

    vector<double> drawnPoints;

    const double angleStep = PI / 16.0;

    Capsule() = default;

    Capsule(double pos0X, double pos0Y, double pos1X, double pos1Y, double radius, double red = 1.0, double green = 0.0, double blue = 0.0)
    {
        this->posX[0] = pos0X;
        this->posY[0] = pos0Y;

        this->posX[1] = pos1X;
        this->posY[1] = pos1Y;

        this->radius = radius;

        this->red = red;
        this->green = green;
        this->blue = blue;

        this->playerControlled = false;

        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);

        capsules.push_back(this);
    }

    void draw()
    {
        this->drawnPoints.clear();

        double currentAngle = 0.0;

        while (currentAngle < 2.0 * PI)
        {
            this->drawnPoints.emplace_back(this->posX[0] + this->radius * cos(currentAngle));
            this->drawnPoints.emplace_back(this->posY[0] + this->radius * sin(currentAngle));

            this->drawnPoints.emplace_back(this->posX[0]);
            this->drawnPoints.emplace_back(this->posY[0]);

            this->drawnPoints.emplace_back(this->posX[0] + this->radius * cos(currentAngle + this->angleStep));
            this->drawnPoints.emplace_back(this->posY[0] + this->radius * sin(currentAngle + this->angleStep));

            currentAngle += this->angleStep;
        }

        currentAngle = 0.0;

        while (currentAngle < 2.0 * PI)
        {
            this->drawnPoints.emplace_back(this->posX[1] + this->radius * cos(currentAngle));
            this->drawnPoints.emplace_back(this->posY[1] + this->radius * sin(currentAngle));

            this->drawnPoints.emplace_back(this->posX[1]);
            this->drawnPoints.emplace_back(this->posY[1]);

            this->drawnPoints.emplace_back(this->posX[1] + this->radius * cos(currentAngle + this->angleStep));
            this->drawnPoints.emplace_back(this->posY[1] + this->radius * sin(currentAngle + this->angleStep));

            currentAngle += this->angleStep;
        }

        double deltaX = this->posX[0] - this->posX[1];
        double deltaY = this->posY[0] - this->posY[1];

        double centersDist = sqrt(deltaX * deltaX + deltaY * deltaY);

        deltaX = deltaX / centersDist * this->radius;
        deltaY = deltaY / centersDist * this->radius;

        double aux = deltaX;
        deltaX = deltaY;
        deltaY = -aux;

        this->drawnPoints.emplace_back(this->posX[0] + deltaX);
        this->drawnPoints.emplace_back(this->posY[0] + deltaY);

        this->drawnPoints.emplace_back(this->posX[1] + deltaX);
        this->drawnPoints.emplace_back(this->posY[1] + deltaY);

        this->drawnPoints.emplace_back(this->posX[1] - deltaX);
        this->drawnPoints.emplace_back(this->posY[1] - deltaY);

        this->drawnPoints.emplace_back(this->posX[1] - deltaX);
        this->drawnPoints.emplace_back(this->posY[1] - deltaY);

        this->drawnPoints.emplace_back(this->posX[0] - deltaX);
        this->drawnPoints.emplace_back(this->posY[0] - deltaY);

        this->drawnPoints.emplace_back(this->posX[0] + deltaX);
        this->drawnPoints.emplace_back(this->posY[0] + deltaY);

        glBindVertexArray(this->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

        glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
        glEnableVertexAttribArray(0);

        glBufferData(GL_ARRAY_BUFFER, sizeof(double) * this->drawnPoints.size(), &(this->drawnPoints.front()), GL_DYNAMIC_DRAW);

        glUniform3f(colourPath, this->red, this->green, this->blue);

        glDrawArrays(GL_TRIANGLES, 0, this->drawnPoints.size() / 2);
    }

    void rotate(double angle)
    {
        double middleX = (this->posX[0] + this->posX[1]) / 2.0;
        double middleY = (this->posY[0] + this->posY[1]) / 2.0;

        this->posX[0] -= middleX;
        this->posX[1] -= middleX;

        this->posY[0] -= middleY;
        this->posY[1] -= middleY;

        double new0X = this->posX[0] * cos(angle) - this->posY[0] * sin(angle);
        double new0Y = this->posX[0] * sin(angle) + this->posY[0] * cos(angle);

        double new1X = this->posX[1] * cos(angle) - this->posY[1] * sin(angle);
        double new1Y = this->posX[1] * sin(angle) + this->posY[1] * cos(angle);

        this->posX[0] = new0X;
        this->posY[0] = new0Y;

        this->posX[1] = new1X;
        this->posY[1] = new1Y;

        this->posX[0] += middleX;
        this->posX[1] += middleX;

        this->posY[0] += middleY;
        this->posY[1] += middleY;
    }
};

void drawCircles()
{
    for (int i = 0; i < circles.size(); i++)
        circles[i]->draw();
}

void drawCapsules()
{
    for (int i = 0; i < capsules.size(); i++)
        capsules[i]->draw();
}

bool changeGravitySourceButtonPressed = false;
bool changedGravityActive = false;
int gravitySource;

void handleInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    double playerImpulseX = 1000.0;
    double playerImpulseY = 1000.0;
    double explosionImpulse = 300000.0;

    double playerTranslationX = 300.0;
    double playerTranslationY = 300.0;

    double playerAngle = 5.0;

    for (int i = 0; i < circles.size(); i++)
    {
        if (circles[i]->playerControlled)
        {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                circles[i]->speedY += playerImpulseY * simulationDeltaTime;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                circles[i]->speedY -= playerImpulseY * simulationDeltaTime;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                circles[i]->speedX -= playerImpulseX * simulationDeltaTime;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                circles[i]->speedX += playerImpulseX * simulationDeltaTime;

            if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
            {
                for (int j = 0; j < circles.size(); j++)
                {
                    if (i == j) continue;

                    double deltaX = circles[j]->posX - circles[i]->posX;
                    double deltaY = circles[j]->posY - circles[i]->posY;

                    double centersDist = sqrt(deltaX * deltaX + deltaY * deltaY);

                    circles[j]->speedX += deltaX / centersDist * explosionImpulse / centersDist * simulationDeltaTime;
                    circles[j]->speedY += deltaY / centersDist * explosionImpulse / centersDist * simulationDeltaTime;
                }
            }
            if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
            {
                if (!changeGravitySourceButtonPressed)
                {
                    changeGravitySourceButtonPressed = true;

                    if (changedGravityActive)
                    {
                        changedGravityActive = false;
                        CURRENT_GRAVITY_X = 0.0;
                        CURRENT_GRAVITY_Y = -SCALAR_GRAVITY;
                    }
                    else
                    {
                        changedGravityActive = true;
                        int gravitySource = i;
                    }
                }
            }
            else
            {
                changeGravitySourceButtonPressed = false;
            }
        }
    }

    for (int j = 0; j < capsules.size(); j++)
    {
        if (capsules[j]->playerControlled)
        {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            {
                capsules[j]->posY[0] += playerTranslationY * simulationDeltaTime;
                capsules[j]->posY[1] += playerTranslationY * simulationDeltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            {
                capsules[j]->posY[0] -= playerTranslationY * simulationDeltaTime;
                capsules[j]->posY[1] -= playerTranslationY * simulationDeltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            {
                capsules[j]->posX[0] -= playerTranslationX * simulationDeltaTime;
                capsules[j]->posX[1] -= playerTranslationX * simulationDeltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            {
                capsules[j]->posX[0] += playerTranslationX * simulationDeltaTime;
                capsules[j]->posX[1] += playerTranslationX * simulationDeltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                capsules[j]->rotate(playerAngle * simulationDeltaTime);
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                capsules[j]->rotate(-playerAngle * simulationDeltaTime);
        }
    }
}

void handleCollisions()
{
    for (int i = 0; i < circles.size(); i++)
    {
        if (circles[i]->posX - circles[i]->radius < -WINDOW_WIDTH / 2.0)
        {
            circles[i]->posX += -WINDOW_WIDTH / 2.0 - (circles[i]->posX - circles[i]->radius);
            circles[i]->speedX = -circles[i]->speedX;
        }
        if (circles[i]->posX + circles[i]->radius > WINDOW_WIDTH / 2.0)
        {
            circles[i]->posX -= circles[i]->posX + circles[i]->radius - WINDOW_WIDTH / 2.0;
            circles[i]->speedX = -circles[i]->speedX;
        }
        if (circles[i]->posY - circles[i]->radius < -WINDOW_HEIGHT / 2.0)
        {
            circles[i]->posY += -WINDOW_HEIGHT / 2.0 - (circles[i]->posY - circles[i]->radius);
            circles[i]->speedY = -circles[i]->speedY;
            circles[i]->speedX *= 1.0 - FRICTION * simulationDeltaTime;
        }
        if (circles[i]->posY + circles[i]->radius > WINDOW_HEIGHT / 2.0)
        {
            circles[i]->posY -= circles[i]->posY + circles[i]->radius - WINDOW_HEIGHT / 2.0;
            circles[i]->speedY = -circles[i]->speedY;
        }
    }

    for (int i = 0; i < circles.size(); i++)
    {
        for (int j = i + 1; j < circles.size(); j++)
        {
            double deltaX = circles[i]->posX - circles[j]->posX;
            double deltaY = circles[i]->posY - circles[j]->posY;

            if (deltaX * deltaX + deltaY * deltaY < (circles[i]->radius + circles[j]->radius) * (circles[i]->radius + circles[j]->radius))
            {
                double centersDist = sqrt(deltaX * deltaX + deltaY * deltaY);

                double normDeltaX = deltaX / centersDist;
                double normDeltaY = deltaY / centersDist;

                double overlapDist = circles[i]->radius + circles[j]->radius - centersDist;

                circles[i]->posX += normDeltaX * overlapDist / 2.0;
                circles[i]->posY += normDeltaY * overlapDist / 2.0;

                circles[j]->posX -= normDeltaX * overlapDist / 2.0;
                circles[j]->posY -= normDeltaY * overlapDist / 2.0;

                double collisionInitialSpeedI = circles[i]->speedX * normDeltaX + circles[i]->speedY * normDeltaY;
                double collisionInitialSpeedJ = circles[j]->speedX * normDeltaX + circles[j]->speedY * normDeltaY;

                double collisionFinalSpeedI = (circles[i]->mass - circles[j]->mass) / (circles[i]->mass + circles[j]->mass) * collisionInitialSpeedI + 2.0 * circles[j]->mass / (circles[i]->mass + circles[j]->mass) * collisionInitialSpeedJ;
                double collisionFinalSpeedJ = 2.0 * circles[i]->mass / (circles[i]->mass + circles[j]->mass) * collisionInitialSpeedI + (circles[j]->mass - circles[i]->mass) / (circles[i]->mass + circles[j]->mass) * collisionInitialSpeedJ;

                circles[i]->speedX -= normDeltaX * collisionInitialSpeedI;
                circles[i]->speedY -= normDeltaY * collisionInitialSpeedI;

                circles[j]->speedX -= normDeltaX * collisionInitialSpeedJ;
                circles[j]->speedY -= normDeltaY * collisionInitialSpeedJ;

                circles[i]->speedX += normDeltaX * collisionFinalSpeedI;
                circles[i]->speedY += normDeltaY * collisionFinalSpeedI;

                circles[j]->speedX += normDeltaX * collisionFinalSpeedJ;
                circles[j]->speedY += normDeltaY * collisionFinalSpeedJ;
            }
        }
    }

    for (int i = 0; i < circles.size(); i++)
    {
        for (int j = 0; j < capsules.size(); j++)
        {
            double deltaXCapsule = capsules[j]->posX[0] - capsules[j]->posX[1];
            double deltaYCapsule = capsules[j]->posY[0] - capsules[j]->posY[1];

            double distCentersCapsule = sqrt(deltaXCapsule * deltaXCapsule + deltaYCapsule * deltaYCapsule);

            double normDeltaXCapsule = deltaXCapsule / distCentersCapsule;
            double normDeltaYCapsule = deltaYCapsule / distCentersCapsule;

            double deltaX = circles[i]->posX - capsules[j]->posX[1];
            double deltaY = circles[i]->posY - capsules[j]->posY[1];

            double projection = deltaX * normDeltaXCapsule + deltaY * normDeltaYCapsule;

            if (projection < 0.0)
                projection = 0.0;
            else if (projection > distCentersCapsule)
                projection = distCentersCapsule;

            double nearPointX = capsules[j]->posX[1] + normDeltaXCapsule * projection;
            double nearPointY = capsules[j]->posY[1] + normDeltaYCapsule * projection;

            double deltaXCircleCapsule = nearPointX - circles[i]->posX;
            double deltaYCircleCapsule = nearPointY - circles[i]->posY;

            if (deltaXCircleCapsule * deltaXCircleCapsule + deltaYCircleCapsule * deltaYCircleCapsule < (circles[i]->radius + capsules[j]->radius) * (circles[i]->radius + capsules[j]->radius))
            {
                double distCircleCapsule = sqrt(deltaXCircleCapsule * deltaXCircleCapsule + deltaYCircleCapsule * deltaYCircleCapsule);

                double normDeltaXCircleCapsule = deltaXCircleCapsule / distCircleCapsule;
                double normDeltaYCircleCapsule = deltaYCircleCapsule / distCircleCapsule;

                double overlapDist = circles[i]->radius + capsules[j]->radius - distCircleCapsule;

                circles[i]->posX -= normDeltaXCircleCapsule * overlapDist;
                circles[i]->posY -= normDeltaYCircleCapsule * overlapDist;

                double speedProjection = circles[i]->speedX * normDeltaXCircleCapsule + circles[i]->speedY * normDeltaYCircleCapsule;

                circles[i]->speedX -= normDeltaXCircleCapsule * speedProjection;
                circles[i]->speedY -= normDeltaYCircleCapsule * speedProjection;

                circles[i]->speedX -= (1.0 - FRICTION * simulationDeltaTime) * normDeltaXCircleCapsule * speedProjection;
                circles[i]->speedY -= (1.0 - FRICTION * simulationDeltaTime) * normDeltaYCircleCapsule * speedProjection;
            }
        }
    }
}

void updateCirclesStatuses()
{
    for (int i = 0; i < circles.size(); i++)
    {
        if (changedGravityActive)
        {
            if (i != gravitySource)
            {
                double deltaX = circles[gravitySource]->posX - circles[i]->posX;
                double deltaY = circles[gravitySource]->posY - circles[i]->posY;

                double dist = sqrt(deltaX * deltaX + deltaY * deltaY);

                CURRENT_GRAVITY_X = deltaX / dist * SCALAR_GRAVITY;
                CURRENT_GRAVITY_Y = deltaY / dist * SCALAR_GRAVITY;

                circles[i]->speedX += CURRENT_GRAVITY_X * simulationDeltaTime;
                circles[i]->speedY += CURRENT_GRAVITY_Y * simulationDeltaTime;
            }
            else
            {
                CURRENT_GRAVITY_X = 0.0;
                CURRENT_GRAVITY_Y = 0.0;
            }
        }

        circles[i]->posX += circles[i]->speedX * simulationDeltaTime;
        circles[i]->posY += circles[i]->speedY * simulationDeltaTime;

        circles[i]->speedX += CURRENT_GRAVITY_X * simulationDeltaTime;
        circles[i]->speedY += CURRENT_GRAVITY_Y * simulationDeltaTime;

        circles[i]->speedX *= 1.0 - FRICTION * simulationDeltaTime;
        circles[i]->speedY *= 1.0 - FRICTION * simulationDeltaTime;
    }
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Physics Simulator", 0, 0);
    //glfwGetPrimaryMonitor();

    glfwMakeContextCurrent(window);

    glewInit();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);

    colourPath = glGetUniformLocation(shaderProgram, "colour");
    int orthoPath = glGetUniformLocation(shaderProgram, "ortho");

    glm::mat4 ortho = glm::ortho(-0.5 * WINDOW_WIDTH, 0.5 * WINDOW_WIDTH, -0.5 * WINDOW_HEIGHT, 0.5 * WINDOW_HEIGHT);
    glUniformMatrix4fv(orthoPath, 1, GL_FALSE, glm::value_ptr(ortho));

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    srand(0);

    const int NUM_CIRCLES = 50;

    for (int i = 1; i <= NUM_CIRCLES; i++)
    {
        new Circle(1.0 * rand() / RAND_MAX * WINDOW_WIDTH - WINDOW_WIDTH / 2.0, 1.0 * rand() / RAND_MAX * WINDOW_HEIGHT - WINDOW_HEIGHT / 2.0, 10.0 + 10.0 * rand() / RAND_MAX, 1.0 * rand() / RAND_MAX, 1.0 * rand() / RAND_MAX, 1.0 * rand() / RAND_MAX);
    }

    circles[0]->playerControlled = true;

    new Capsule(10.0, 10.0, 470.0, 425.0, 10.0);

    capsules[0]->playerControlled = true;

    //new Capsule(-100.0, 100.0, 200.0, -100.0, 10.0);

    //capsules[1]->playerControlled = true;

    while (!glfwWindowShouldClose(window))
    {
        updateDeltaTime();

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 1; i <= NUMBER_OF_SIMULATIONS; i++)
        {
            handleInput(window);

            handleCollisions();

            updateCirclesStatuses();
        }

        drawCircles();
        drawCapsules();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    for (int i = 0; i < circles.size(); i++)
        delete circles[i];

    for (int j = 0; j < capsules.size(); j++)
        delete capsules[j];

    return 0;
}

