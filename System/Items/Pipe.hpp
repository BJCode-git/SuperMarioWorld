#ifndef PIPE_H
#define PIPE_H

#include "../DisplayableObject.hpp"
#include "../EventEngine/EventEngine.hpp"
#include "../Characters/Goomba.hpp"
#include <SFML/System/Clock.hpp>

class GameEngine;
class Enemy;

/*
*	A Pipe can be used to travel, to spawn enemies
*/
class Pipe : public DisplayableObject
{
	public:
		Pipe(std::string _name, sf::Vector2f _coord, int _pipeId, PipeType _type, EventEngine *_eventEngine);
		~Pipe();

		void HandleSpawnEnemies(float _dt);

		unsigned int GetPipeId() { return m_pipeId; };
		PipeType GetPipeType() { return m_type; };

		void ToggleSpawn() { m_spawnIsOn = !m_spawnIsOn; };

	protected:
		unsigned int m_pipeId;
		PipeType m_type;

		bool m_spawnIsOn;
		DisplayableObject *m_enemyBeingSpawned; // One enemy at a time can be spawed and controlled by the pipe
		InfoForDisplay *m_enemyInfo;
		bool m_justFinishedSpawn;
		sf::Clock m_spawnTimer;

		void MoveEnemyBeingSpawned(float _dt);
		void SpawnEnemyIfTimeElapsed();
		void SendEnemyBeingSpawnedToGFX();

		void PublishEnemyCreation();
		void RemoveEnemyBeingSpawned();

		bool IsEnemyReadyToLeavePipe();

		static const int milisecondsBetweenSpawns;
};

#endif
