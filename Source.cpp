#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>
#include <OgreTrays.h>
#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include <iostream>
#include <vector>

using namespace std;
using namespace Ogre;
using namespace OgreBites;

#define CAMERA_SHIFT 75, 100, 150

#define KEYSYM_SPACEBAR 32
#define KEYSYM_CTRL 1073742048

bool programStopped = false;

struct roadPosition
{
	double x1;
	double x2;
	double z1;
	double z2;
} roads[256] = { 0 };

struct perchPosition
{
	double x;
	double y;
	double z;
} perches[256] = { 0 };

void createRoads();
void createPerch();

class Player
{
public:
	Vector3 position;
	SceneNode* sphereNode;

	void move();
	void pressedUp();
	void pressedDown();
	void pressedLeft();
	void pressedRight();
	void pressedJump();
	void pressedCTRL();

	bool ballOnRoad();
	void ballFalling();
private:
	void deceletate(double& speed);

	double speedX = 0;
	double speedY = 0;
	double speedZ = 0;
	const double acceleration = 0.4;
	const double deceleration = 0.02;
};

void Player::pressedUp()
{
	speedZ -= acceleration;
}

void Player::pressedDown()
{
	speedZ += acceleration;
}

void Player::pressedLeft()
{
	speedX -= acceleration;
}

void Player::pressedRight()
{
	speedX += acceleration;
}

void Player::pressedJump()
{
	if (ballOnRoad())
	{
		speedY = 8;
	}
}

void Player::pressedCTRL()
{
	if (!ballOnRoad())
	{
		speedX = 0;
		speedZ = 0;
		speedY = -10;
	}
}

bool Player::ballOnRoad()
{
	if (speedY > 0) //Если шар взлетает
	{
		return false;
	}
	if (position.y > 20)
	{
		return false;
	}

	//Участки дороги: (X, Z)
	//([-25, 25], [-125, 25])
	//([-175, -25], [-125, -75])
	//([-175, -125], [-275, -125])
	//([-325, -175], [-275, -225])
	//([-325, -275], [-225, 75])
	//([-275, 25], [25, 75])
	//SpecialWay: ([25, 75], [-25, 25])
	//SpecialPlatform: ([75, 575], [-25, 475])

	double x = position.x;
	double z = position.z;
	
	if (x >= -25 && x <= 25 && z >= -125 && z <= 25)
	{
		return true;
	}
	else if (x >= -175 && x <= -25 && z >= -125 && z <= -75)
	{
		return true;
	}
	else if (x >= -175 && x <= -125 && z >= -275 && z <= -125)
	{
		return true;
	}
	else if (x >= -325 && x <= -175 && z >= -275 && z <= -225)
	{
		return true;
	}
	else if (x >= -325 && x <= -275 && z >= -225 && z <= 75)
	{
		return true;
	}
	else if (x >= -275 && x <= 25 && z >= 25 && z <= 75)
	{
		return true;
	}
	else if (x >= 25 && x <= 75 && z >= -25 && z <= 25)
	{
		return true;
	}
	else if (x >= 75 && x <= 575 && z >= -25 && z <= 475)
	{
		return true;
	}

	return false;
}

void Player::ballFalling()
{
	speedY -= acceleration;
}

void Player::move()
{
	Vector3 newPosition(position.x + speedX, position.y + speedY, position.z + speedZ);

	if (newPosition.y <= -300.0)//Шар уже упал
	{
		newPosition = Vector3(0, 0, 0);
		speedX = 0;
		speedY = 0;
		speedZ = 0;
	}

	if (ballOnRoad() && newPosition.y >= -15)//Заскочили обратно
	{
		speedY = 0;
		newPosition = Vector3(position.x + speedX, 0, position.z + speedZ);
	}

	sphereNode->setPosition(newPosition);
	this->position = newPosition;

	if (speedY < 0 || !ballOnRoad())
	{
		ballFalling();
	}

	deceletate(speedX);
	deceletate(speedZ);
}

void Player::deceletate(double& speed)
{
	if (speed < 0)
	{
		if (speed >= deceleration)
		{
			speed = 0;
		}
		else
		{
			speed += deceleration;
		}
	}
	else
	{
		if (speed <= deceleration)
		{
			speed = 0;
		}
		else
		{
			speed -= deceleration;
		}
	}
}

class OgreProject : public ApplicationContext, public InputListener
{
public:
	OgreProject();

	void setup(void);
	void update(void);

	bool keyPressed(const KeyboardEvent& evt);
	void keyboardHandler();
	bool keyReleased(const KeyboardEvent& evt);

	void createPlayerSphere(Vector3 position);
	void createPlatform(Vector3 position, char* name);
	void createSpecialPlatform();
	void replaceCamToPlayer();
	void replaceLight();
	
	void drawMap();
private:
	bool keyClamped;
	int keyCodes[10] = { 0 };
	int keyCodesIndex = 0;

	TrayManager* mTrayMgr;
	SceneManager* scnMgr;
	Root* root;
	SceneNode* camNode;
	Camera* cam;
	Light* light;
	SceneNode* lightNode;

	Player player;
};

OgreProject::OgreProject() : ApplicationContext("OOP LR 3. Platformer")
{
	
}

void OgreProject::setup(void)
{
	ApplicationContext::setup();
	addInputListener(this);
	root = getRoot();
	scnMgr = root->createSceneManager();
	RTShader::ShaderGenerator* shadergen = RTShader::ShaderGenerator::getSingletonPtr();

	shadergen->addSceneManager(scnMgr);
	scnMgr->addRenderQueueListener(getOverlaySystem());

	mTrayMgr = new TrayManager("OOP LR 3", getRenderWindow());
	mTrayMgr->hideCursor();
	addInputListener(mTrayMgr);

	camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
	cam = scnMgr->createCamera("Cam");
	cam->setNearClipDistance(5);
	cam->setAutoAspectRatio(true);
	camNode->attachObject(cam);
	camNode->setPosition(CAMERA_SHIFT);
	getRenderWindow()->addViewport(cam);
	
	scnMgr->setAmbientLight(ColourValue(0.8, 0.5, 0.1));
	light = scnMgr->createLight("MainLight");
	lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
	lightNode->setPosition(0, 200, 0);
	lightNode->attachObject(light);

	Vector3 spherePos(-30, 0, 0);
	createPlayerSphere(spherePos);

	drawMap();

	keyClamped = false;
}

void OgreProject::createPlayerSphere(Vector3 position)
{
	const float scale = 0.3;
	Vector3 sphereScaleFactor(scale, scale, scale);

	Entity* sphereEntity = scnMgr->createEntity("Sphere", "sphere.mesh");
	player.sphereNode = scnMgr->getRootSceneNode()->createChildSceneNode("sphereNode");

	player.sphereNode->attachObject(sphereEntity);
	player.sphereNode->setPosition(position);
	player.sphereNode->setScale(sphereScaleFactor);
}

void OgreProject::createPlatform(Vector3 position, char* name)
{
	const float scale = 0.5;
	Vector3 cubeScaleFactor(scale, 0.05, scale);
	
	Entity* cubeEntity = scnMgr->createEntity(name, "cube.mesh");
	SceneNode* cubeNode = scnMgr->getRootSceneNode()->createChildSceneNode(name);

	cubeNode->attachObject(cubeEntity);
	cubeNode->setPosition(position);
	cubeNode->setScale(cubeScaleFactor);
}

void OgreProject::drawMap()
{
	createPlatform(Vector3(0, -25, 0), "platform1");
	createPlatform(Vector3(0, -25, -50), "platform2");
	createPlatform(Vector3(0, -25, -100), "platform3");

	createPlatform(Vector3(-50, -25, -100), "platform4");
	createPlatform(Vector3(-100, -25, -100), "platform5");
	createPlatform(Vector3(-150, -25, -100), "platform6");

	createPlatform(Vector3(-150, -25, -150), "platform7");
	createPlatform(Vector3(-150, -25, -200), "platform8");
	createPlatform(Vector3(-150, -25, -250), "platform9");

	createPlatform(Vector3(-200, -25, -250), "platform10");
	createPlatform(Vector3(-250, -25, -250), "platform11");
	createPlatform(Vector3(-300, -25, -250), "platform12");

	createPlatform(Vector3(-300, -25, -200), "platform13");
	createPlatform(Vector3(-300, -25, -150), "platform14");
	createPlatform(Vector3(-300, -25, -100), "platform15");
	createPlatform(Vector3(-300, -25, -50), "platform16");
	createPlatform(Vector3(-300, -25, 0), "platform17");
	createPlatform(Vector3(-300, -25, 50), "platform18");
	
	createPlatform(Vector3(-250, -25, 50), "platform19");
	createPlatform(Vector3(-200, -25, 50), "platform20");
	createPlatform(Vector3(-150, -25, 50), "platform21");
	createPlatform(Vector3(-100, -25, 50), "platform22");
	createPlatform(Vector3(-50, -25, 50), "platform23");
	createPlatform(Vector3(0, -25, 50), "platform24");

	createPlatform(Vector3(50, -25, 0), "SpecialWay");

	createSpecialPlatform();
}

void OgreProject::createSpecialPlatform()
{
	const float scale = 5;
	Vector3 cubeScaleFactor(scale, 0.05, scale);

	Entity* cubeEntity = scnMgr->createEntity("SpecialPlatform", "cube.mesh");
	SceneNode* cubeNode = scnMgr->getRootSceneNode()->createChildSceneNode("SpecialPlatform");

	cubeNode->attachObject(cubeEntity);
	cubeNode->setPosition(Vector3(325, -25, 225));
	cubeNode->setScale(cubeScaleFactor);
}

void OgreProject::replaceCamToPlayer()
{
	Vector3 cameraShift(CAMERA_SHIFT);
	Vector3 camPosition(player.position + cameraShift);
	cam->setPosition(camPosition);
	cam->lookAt(player.position);

	//light->setPosition(camPosition + Vector3(10, 10, 10));
}

void OgreProject::keyboardHandler()
{
	if (keyClamped)
	{
		for (int i = 0; i < keyCodesIndex; i++)
		{
			int keyCode = keyCodes[i];
			if (keyCode == KEYSYM_SPACEBAR && player.position.y == 0)//Пробел
			{
				player.pressedJump();
			}

			if (keyCode == SDLK_ESCAPE)
			{
				programStopped = true;
			}
			if (keyCode == SDLK_UP)
			{
				player.pressedUp();
			}
			if (keyCode == SDLK_DOWN)
			{
				player.pressedDown();
			}
			if (keyCode == SDLK_LEFT)
			{
				player.pressedLeft();
			}
			if (keyCode == SDLK_RIGHT)
			{
				player.pressedRight();
			}
		}
		replaceCamToPlayer();
	}
}

bool OgreProject::keyReleased(const KeyboardEvent& evt)
{
	for (int i = 0; i < keyCodesIndex; i++)
	{
		if (evt.keysym.sym == keyCodes[i])
		{
			for (int j = i + 1; j < keyCodesIndex; j++)
			{
				keyCodes[j - 1] = keyCodes[j];//Смещаем элементы массива
			}
			keyCodesIndex--;
			break;
		}
	}

	if (keyCodesIndex == 0)
	{
		keyClamped = false;//нет зажатых клавиш
	}

	return true;
}

bool OgreProject::keyPressed(const KeyboardEvent& evt)
{
	if (evt.repeat != 0)
	{
		return true;
	}

	keyClamped = true;
	keyCodes[keyCodesIndex] = evt.keysym.sym;
	keyCodesIndex++;

	//

	if (evt.keysym.sym == KEYSYM_SPACEBAR)//Пробел
	{
		player.pressedJump();
	}

	if (evt.keysym.sym == KEYSYM_CTRL)
	{
		player.pressedCTRL();
	}

	if (evt.keysym.sym == SDLK_ESCAPE)
	{
		programStopped = true;
	}	
	if (evt.keysym.sym == SDLK_UP)
	{
		player.pressedUp();
	}
	if (evt.keysym.sym == SDLK_DOWN)
	{
		player.pressedDown();
	}	
	if (evt.keysym.sym == SDLK_LEFT)
	{
		player.pressedLeft();
	}	
	if (evt.keysym.sym == SDLK_RIGHT)
	{
		player.pressedRight();
	}

	replaceCamToPlayer();
	return true;
}

void OgreProject::update()
{
	keyboardHandler();
	player.move();
	replaceCamToPlayer();
	replaceLight();
}

void OgreProject::replaceLight()
{
	scnMgr->destroyLight(light);
	scnMgr->getRootSceneNode()->removeAndDestroyChild(lightNode);

	/*scnMgr->setAmbientLight(ColourValue(0.8, 0.5, 0.1));*/
	light = scnMgr->createLight("MainLight");
	lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
	lightNode->setPosition(0, 200, 0);
	lightNode->attachObject(light);
}

void createRoads()
{//Заполни дороги
	//roads[] = { x1, x2, z1, z2 };
}

void createPerch()
{//Заполни жердочки
	//perches[] = { x, y, z };
}

int main(int argc, char **argv)
{
	createRoads();
	try
	{
		OgreProject app;
		app.initApp();

		Timer timer;
		while (!programStopped)
		{
			if (timer.getMilliseconds() >= 20)//50 кадров в секунду
			{
				app.update();
				app.getRoot()->renderOneFrame();
				timer.reset();
			}
		}
		app.closeApp();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error! In execution: " << e.what() << std::endl;
		return -1;
	}
	return 0;
}
