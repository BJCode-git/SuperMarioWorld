#ifndef CHARACTER_H
#define CHARACTER_H

#include "../DisplayableObject.hpp"

/*
*	A MovingObject can be a player, an enemy, or a moving item, such as a power-up
*/
class MovingObject : public DisplayableObject
{
	public:
		MovingObject(EventEngine *_eventEngine, std::string _name, sf::Vector2f _coord, State _state = UNKNOWN);
		MovingObject(EventEngine *_eventEngine, std::string _name, float _x, float _y, State _state = UNKNOWN);
		~MovingObject();

		void Init();

		virtual InfoForDisplay GetInfoForDisplay();

		void UpdatePosition(float _dt);
		virtual void UpdateAfterCollision(CollisionDirection _dir, ObjectClass _classOfOtherObject) = 0;
		virtual void UpdateAfterCollisionWithMapEdge(CollisionDirection _dir, float _gap);

		void Kill();

		void SetVelX(float _x) { m_velocity.x = _x; };
		void SetVelY(float _y) { m_velocity.y = _y; }
		void SetJumpState(JumpState _state) { m_jumpState = _state; };
		bool IsInTheAir() { return m_jumpState != ONFLOOR; };
		bool CanCollide() { return !m_noCollision; };
		bool HasBeenHit() { return m_noCollision; }; // m_noCollision has been set to true exactly when character has been hit

		void MarkAsDead() { m_isDead = true; };
		bool IsDead() { return m_isDead; };

#ifdef DEBUG_MODE
		DebugInfo GetDebugInfo();
#endif


	protected:
		Direction m_facing;

		JumpState m_jumpState;
		State m_previousState; // When in the air, store the previous state for acceleration calculation

		void UpdateAcceleration();
		void UpdateVelocity(float _dt);

		virtual float GetMaxAbsVelocity_X() = 0;
		virtual void AddOwnAcceleration() = 0;
		virtual void Move(Instruction _inst) = 0;

		int m_maxSpeed;
		sf::Vector2f m_velocity;
		sf::Vector2f m_acceleration;

		bool m_isRunning;

		bool m_noCollision; // Character is not affected by any collision (will die when hitting the bottom of the screen)
		bool m_isDead; // Flag to set when caracter dies so all the dead are deleted at the end of GameEngineFrame()
};

#endif
