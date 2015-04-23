#ifndef GRAPHICSENGINE_H
#define GRAPHICSENGINE_H

#include "Engine.hpp"

#include <SFML/Graphics.hpp>

#define WIN_HEIGHT			432
#define WIN_WIDTH			512
#define SIZE_BLOCK			16
#define HEIGHT_IN_BLOCKS	WIN_HEIGHT / SIZE_BLOCK
#define WIDTH_IN_BLOCKS		WIN_WIDTH / SIZE_BLOCK

/*
    Graphics engine: Handles the graphics of the game: loading sprites and displaying them using SFML
*/
class GraphicsEngine : public Engine
{
    public:
        GraphicsEngine(Game *_g);
        ~GraphicsEngine();

        void Frame();
		float GetFramerateLimit();



    private:
		sf::RenderWindow *m_gameWindow;
		static const float FramerateLimit;

		std::map<std::string, sf::Texture> m_textures;

		sf::Sprite* m_tmpSprite;

		// Sprites to draw: 3 levels
		std::vector<sf::Sprite> m_backgroundToDraw;
		std::vector<sf::Sprite> m_levelStructureToDraw;
		std::vector<sf::Sprite> m_displayableObjectsToDraw;
		
		void ProcessEvent(EngineEvent& _event);
		void ProcessWindowEvents();

		void LoadTextures(); // Load all textures at beginning of level

		void ResetSpritesToDraw();

		void DisplayWindow();

		// Add sprites in m_toDraw: the farthest first
		void SetBackgroundToDraw();
		void SetFloorToDraw();
		void SetDisplayableObjectToDraw(InfoForDisplay _info);

		void DrawGame();

		void ResetTmpSprite();

#ifdef DEBUG_MODE
		sf::Clock m_clock;

		sf::Font m_font;
		sf::Text m_debugText;

		sf::Vector2f m_posMario;
		DebugInfo m_debugInfo;

		void StoreDebugInfo(DebugInfo _info) { m_debugInfo = _info; };
		void DrawDebugInfo();
#endif
};

#endif // GRAPHICSENGINE_H