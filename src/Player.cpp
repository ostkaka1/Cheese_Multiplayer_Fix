#include "player.h"
#include "World.h"
#include "camera.h"
#include "Projectile.h"
#include <tuple>
#include "MessageType.h"
#include "App.h"

Player::Player(float X, float Y, short sizeX, short sizeY, bool IsClientControlling, std::string spriteName, int spriteIndex, std::string Name) 
	: Creature(X, Y, sizeX, sizeY, 4096, 0.875, spriteName, spriteIndex, IsClientControlling)
{
	name = Name;
	cameraDelay = 0;
	right = true;
	down = false;
	left = false;
	up = false;
	lmb = false;

#ifndef _SERVER
	//auto eventUpdate =  [this](App &app, sf::Event &event, World *world, std::queue<sf::Packet> *packetDataList) { EventUpdate(app, event, world, packetDataList); };
	//eventHandler.AddEventCallback(this, eventUpdate);
#endif
}

#ifdef _SERVER
void Player::Update(App &app, World *world, std::queue<sf::Packet> *packetDataList, Camera *camera)
#else
void Player::Update(App &app, World *world, std::queue<sf::Packet> *packetDataList, Camera *camera, EventHandler &eventHandler)
#endif
{
#ifndef _SERVER
	if (isClientControlling)
	{
		if (camera->getEntity() != this)
		{
			if (cameraDelay <= 0)
			{
				camera->setCameraAt(this);
				cameraDelay = 0.5;
			}
			else
			{
				cameraDelay -= app.getFrameTime();
			}
		}
	}
#endif

#ifdef _SERVER
	Creature::Update(app, world, packetDataList, camera);
#else
	Creature::Update(app, world, packetDataList, camera, eventHandler);
#endif
}

#ifndef _SERVER

void Player::EventUpdate(App &app, const sf::Event &event, World *world, std::queue<sf::Packet> *packetDataList)
{
	if (isClientControlling)
	{
		bool
			rDown = right,
			dDown = down,
			lDown = left,
			uDown = up;

		switch(event.type)
		{
		case sf::Event::KeyPressed:
			switch(event.key.code)
			{
			case sf::Keyboard::D:
Right:
				rDown = true;
				break;
			case sf::Keyboard::S:
Down:
				dDown = true;
				break;
			case sf::Keyboard::A:
Left:
				lDown = true;
				break;
			case sf::Keyboard::W:
Up:
				uDown = true;
				break;

			case sf::Keyboard::Right:
				goto Right;

			case sf::Keyboard::Down:
				goto Down;

			case sf::Keyboard::Left:
				goto Left;

			case sf::Keyboard::Up:
				goto Up;
			}
			break;

		case sf::Event::KeyReleased:
			switch(event.key.code)
			{
			case sf::Keyboard::D:
RightR:
				rDown = false;
				break;
			case sf::Keyboard::S:
DownR:
				dDown = false;
				break;
			case sf::Keyboard::A:
LeftR:
				lDown = false;
				break;
			case sf::Keyboard::W:
UpR:
				uDown = false;
				break;

			case sf::Keyboard::Right:
				goto RightR;

			case sf::Keyboard::Down:
				goto DownR;

			case sf::Keyboard::Left:
				goto LeftR;

			case sf::Keyboard::Up:
				goto UpR;
			}
			break;

		case sf::Event::MouseButtonPressed:
			switch(event.key.code)
			{
			case sf::Mouse::Left:
				if(!lmb && (lmb=true))
				{

					//350 �r magiskt nummer av h�jden p� f�nster/2. 576 �r magist nummer f�r h�lften av bredden. De ska fixas sen. 350/2 = 175, 576/2 = 288
					//double angle = atan2((app.getView().GetCenter().y + app.GetInput().GetMouseY() - 256) - (app.getView().getEntityPosition().y+8), (app.getView().GetCenter().x + app.GetInput().GetMouseX() - 384) - (app.getView().getEntityPosition().x+8)) * 180 / 3.1415;
					double angle = atan2((app.getView().getCenter().y + sf::Mouse::getPosition().y - 350- 175) - (y+8), (app.getView().getCenter().x + sf::Mouse::getPosition().x - 576 - 288) - (x+8)) * 180 / 3.1415926536;
					if (angle < 0)
						angle = angle + 360;

					double deltaSpeedX = cos(angle*3.1415926536)*speedX;
					double deltaSpeedY = sin(angle*3.1415926536)*speedY;

					if (angle > 180)
						deltaSpeedX *= -1;

					if (angle < 90 || angle > 270)
						deltaSpeedY *= -1;


					//Projectile *projectile = new Projectile(app.getView().getEntityPosition().x.getEntityPosition().y, 32, 32, -angle, 512, 0, "arrow.png", 0, false);
					Projectile *projectile = new Projectile(x+8, y+8, 32, 32, angle, 2048, 0.03125F, "arrow.png", 0, false);
					world->AddEntity(projectile);//new Projectile(sf::Vector2f(app.getView().getCreaturePosition().x+8.getCreaturePosition().y+8), (float)angle, 500, tC.getTextures("arroaaawb.png")[0]));
					cameraDelay = 0.03125F;
					//app.getView().setCameraAt(*projectile);
				}

				lmb = true;
				break;

			case sf::Mouse::Right:
				break;
			}

		case sf::Event::MouseButtonReleased:
			{
				lmb = false;
			}
			break;
		}
		KeyUpdate(rDown, dDown, lDown, uDown, packetDataList);
	}
}

void Player::Draw(App &app, TextureContainer &tC)
{
	Creature::Draw(app, tC);
}
#endif

void Player::KeyUpdate(bool Right, bool Down, bool Left, bool Up, std::queue<sf::Packet> *packetDataList)
{
	if (Right != right || Down != down || Left != left || Up != up)
	{
		horizontal = 0;
		vertical = 0;

		if (Right)  horizontal += speed;
		if (Down)   vertical += speed;
		if (Left)   horizontal -= speed;
		if (Up)     vertical -= speed;

		//std::cout << "bools: " <<  Right << " " << Down<< " " << Left<< " " << Up<< " " << std::endl;
		//std::cout << "variables: " <<  horizontal << " " << vertical << " " << speedX << " " << speedY << std::endl;

		right = Right;
		down = Down;
		left = Left;
		up = Up;



		if (isClientControlling)
		{
			sf::Packet packet;
			packet << (sf::Uint16)PlayerMove << x << y << speedX << speedY << angle << horizontal << vertical;
			packetDataList->push(packet);
		}
	}
}

void Player::setCameraDelay(float delay)
{
	cameraDelay = delay;
}

std::string Player::getTextureName()
{
	return "graywizard.png";
}

char Player::getTextureId()
{
	return 0;
}