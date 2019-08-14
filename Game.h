#pragma once
#include <vector>

class Bullet;
class Spaceship;
class InputManager;
class NetworkManager;

class Game
{
public:
	Game();
	~Game();

	void Init(InputManager* InputManager, NetworkManager* NetworkManager);
	void Update();
	void Draw();
	void DrawRecvData();

private:

	std::unique_ptr<Spaceship>				LocalPlayer;
	std::unique_ptr<Spaceship>				RemotePlayer;
	std::vector<std::unique_ptr<Bullet>>	BulletsPool;

	InputManager*							pInputs;			// Cache only
	NetworkManager*							pNetwork;		// Cache only

	int										BufferLineCounter = 0;
};

