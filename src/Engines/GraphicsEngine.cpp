#include "GraphicsEngine.hpp"
#include "../Graphics/GraphicsEvents.hpp"

const float GraphicsEngine::FramerateLimit = 60;

GraphicsEngine::GraphicsEngine(EventEngine *_eventEngine): Engine (_eventEngine)
{
	m_gameWindow = new sf::RenderWindow(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT, 32), "Super Mario !", sf::Style::Titlebar | sf::Style::Close);

	m_spriteHandler = new SpriteHandler();
	m_spriteHandler->LoadTextures();

	m_tmpSprite = new sf::Sprite();

#ifdef DEBUG_MODE
	m_font.loadFromFile("arial.ttf");
	m_debugText.setFont(m_font);
	m_debugText.setCharacterSize(15);
	m_debugText.setColor(sf::Color::Red);
#endif
}

GraphicsEngine::~GraphicsEngine()
{
	delete m_spriteHandler;
	m_gameWindow->close();
	delete m_gameWindow;
}

void GraphicsEngine::Frame()
{
	m_gameWindow->clear();
	ResetSpritesToDraw();
	if (m_gameWindow->isOpen())
		ProcessWindowEvents();
	ProcessQueue();
	DisplayWindow();
}

// Process a single event, sent by another engine
void GraphicsEngine::ProcessEvent(EngineEvent& _event)
{
	switch (_event.m_type)
	{
		case INFO_LVL:
			StoreLevelInfo(_event.m_levelInfo);
			break;
		case INFO_POS_LVL:
			UpdateForegroundItem(_event.data.m_infoDisplay);
			break;
		case REMOVE_LVL_BLOC:
			m_listForegroundItems.erase(_event.data.m_id);
			break;
		case INFO_POS_CHAR:
			SetDisplayableObjectToDraw(_event.data.m_infoDisplay);
			break;
#ifdef DEBUG_MODE
		case INFO_DEBUG:
			StoreDebugInfo(_event.m_debugInfo);
			break;
#endif
		default:
			break;
	}
}

void GraphicsEngine::ResetSpritesToDraw()
{
	while (!m_backgroundToDraw.empty())
		m_backgroundToDraw.pop_back();
	while (!m_levelStructureToDraw.empty())
		m_levelStructureToDraw.pop_back();
	m_displayableObjectsToDraw.clear();
}

// Process windows events that have happened since the last loop iteration (sent by SFML)
void GraphicsEngine::ProcessWindowEvents()
{
	sf::Event windowEvent;
	EngineEvent engineEvent;

	while (m_gameWindow->pollEvent(windowEvent))
	{
		switch (windowEvent.type)
		{
			case sf::Event::KeyPressed:
				engineEvent.set(KEY_PRESSED, windowEvent.key.code);
				m_engines["g"]->PushEvent(engineEvent);
				break;
			case sf::Event::KeyReleased:
				engineEvent.set(KEY_RELEASED, windowEvent.key.code);
				m_engines["g"]->PushEvent(engineEvent);
				break;
			case sf::Event::Closed:
				Event event;
				m_eventEngine->dispatch(GAME_STOP_REQUEST, &event);
				break;
			default:
				break;
		}
	}
}

void GraphicsEngine::DisplayWindow()
{
	SetBackgroundToDraw();
	SetForegroundToDraw();

	DrawGame();

#ifdef DEBUG_MODE
	DrawDebugInfo();
#endif

	m_gameWindow->display();
}

void GraphicsEngine::SetBackgroundToDraw()
{
	ResetTmpSprite();
	m_tmpSprite->setTexture(m_spriteHandler->GetTexture("background_" + m_currentBackgroundName));
	m_backgroundToDraw.push_back(*m_tmpSprite);
}

void GraphicsEngine::SetForegroundToDraw()
{
	SetListOfDisplayablesToDraw(m_listForegroundItems);
	SetListOfDisplayablesToDraw(m_listPipes);
}

/* Keeping this function even if called only in one place, in case I add another layer between foreground and background */
void GraphicsEngine::SetListOfDisplayablesToDraw(std::map<unsigned int, InfoForDisplay>& _list)
{
	unsigned int id;
	sf::Vector2f tmpCoords;
	std::string spriteName;

	for (std::map<unsigned int, InfoForDisplay>::iterator it = _list.begin(); it != _list.end(); ++it)
	{
		ResetTmpSprite();

		id = it->first;
		tmpCoords.x = it->second.coordinates.left;
		tmpCoords.y = it->second.coordinates.top;

		spriteName = GetTextureName(id, it->second.name, it->second.state);

		m_tmpSprite->setTexture(m_spriteHandler->GetTexture(spriteName));
		m_tmpSprite->setPosition(tmpCoords);
		m_levelStructureToDraw.push_back(*m_tmpSprite);

		// Tell GameEngine what is to be drawn (id and coordinates), so it can handle collisions
		EngineEvent tmpEvent(INFO_POS_LVL, id, m_tmpSprite->getGlobalBounds());
		m_engines["g"]->PushEvent(tmpEvent);
	}
}

void GraphicsEngine::UpdateForegroundItem(InfoForDisplay _info)
{
	if (_info.name.find("pipe_") == std::string::npos)
		m_listForegroundItems[_info.id] = _info;
	else
		m_listPipes[_info.id] = _info;
}

void GraphicsEngine::SetDisplayableObjectToDraw(InfoForDisplay _info)
{
	ResetTmpSprite();

	std::string spriteName = GetTextureName(_info.id, _info.name, _info.state);
	m_tmpSprite->setTexture(m_spriteHandler->GetTexture(spriteName));

	m_tmpSprite->setPosition(sf::Vector2f(_info.coordinates.left, _info.coordinates.top));
	if (_info.reverse)
	{
		float height = m_tmpSprite->getGlobalBounds().height;
		float width = m_tmpSprite->getGlobalBounds().width;
		m_tmpSprite->setTextureRect(sf::IntRect(width, 0, -width, height));
	}

	/*	gfx can receive the information to display a character several times (if it has been hit for exemple, info is sent fron g to gfx right after the hit)
	Only the last one received will be displayed */
	m_displayableObjectsToDraw[_info.id] = *m_tmpSprite;

	// Tell GameEngine what is to be drawn (id and coordinates), so it can handle collisions
	EngineEvent tmpEvent(INFO_POS_LVL, _info.id, m_tmpSprite->getGlobalBounds());
	m_engines["g"]->PushEvent(tmpEvent);

#ifdef DEBUG_MODE
	if (_info.name == "mario")
	{
		m_posMario.x = _info.coordinates.left;
		m_posMario.y = _info.coordinates.top;
	}
#endif
}

/* Figures out which sprite to display, ie the name of the sprite in the RECT file. The name is fetched only if it's an animation or if the state has changed. */
std::string GraphicsEngine::GetTextureName(unsigned int _id, std::string _name, State _state)
{
	std::string newTextureName, oldTextureName;
	std::string fullStateName = m_spriteHandler->GetFullStateName(_name, _state);
	int nbTextures = m_spriteHandler->HowManyLoadedTexturesContainThisName(fullStateName);

	assert(nbTextures > 0);

	if (m_animationStates.find(_id) == m_animationStates.end()) // If id doesn't exist in the map, create entry as UNKNOWN
		m_animationStates[_id] = Sprite::UNKNOWN;

	if (m_spritesCurrentlyDisplayed.find(_id) != m_spritesCurrentlyDisplayed.end())
		oldTextureName = m_spritesCurrentlyDisplayed[_id];
	else
		oldTextureName = "";

	switch (m_animationStates[_id])
	{
		case Sprite::UNKNOWN:
		case Sprite::ANIMATED:
		case Sprite::NEW_STATIC:
			newTextureName = m_spriteHandler->GetTextureNameFromStateName(fullStateName, oldTextureName, nbTextures);
			break;
		case Sprite::STATIC:
			if (fullStateName == m_spritesCurrentlyDisplayed[_id]) // Sprite same as previous
				newTextureName = fullStateName;
			else
				newTextureName = m_spriteHandler->GetTextureNameFromStateName(fullStateName, oldTextureName, nbTextures);
			break;
		default:
			throw std::exception();
	}

	m_spritesCurrentlyDisplayed[_id] = newTextureName;
	return newTextureName;
}

void GraphicsEngine::UpdateAnimationStates(unsigned int _id, std::string _stateFullName, int nbTextures)
{
	if (nbTextures == 1)			// Static
	{
		m_animationStates[_id] = Sprite::STATIC;

		if (_stateFullName != m_spritesCurrentlyDisplayed[_id])
			m_animationStates[_id] = Sprite::NEW_STATIC;
	}
	else							// Animation
	{
		m_animationStates[_id] = Sprite::ANIMATED;
	}
}

// Draw the 3 layers in the correct order
void GraphicsEngine::DrawGame()
{
	for (unsigned int i = 0; i < m_backgroundToDraw.size(); i++)
		m_gameWindow->draw(m_backgroundToDraw[i]);
	for (unsigned int i = 0; i < m_levelStructureToDraw.size(); i++)
		m_gameWindow->draw(m_levelStructureToDraw[i]);
	for (std::map<unsigned int, sf::Sprite>::iterator it = m_displayableObjectsToDraw.begin(); it != m_displayableObjectsToDraw.end(); ++it)
		m_gameWindow->draw(it->second);
}

void GraphicsEngine::StoreLevelInfo(LevelInfo _info)
{
	m_currentBackgroundName = _info.backgroundName;
}

void GraphicsEngine::ResetTmpSprite()
{
	delete m_tmpSprite;
	m_tmpSprite = new sf::Sprite();
}

#ifdef DEBUG_MODE
void GraphicsEngine::DrawDebugInfo()
{
	std::string toWrite;
	sf::Vector2f playerPos = m_posMario;
	sf::Vector2f playerVel = m_debugInfo.velocity;
	sf::Vector2f playerAcc = m_debugInfo.acceleration;

	toWrite += (" Framerate = " + std::to_string(floor((int)1 / m_clock.getElapsedTime().asSeconds())) + "\n");
	toWrite += (" Position: { " + std::to_string(playerPos.x) + "; " + std::to_string(playerPos.y) + " }\n");
	toWrite += (" Velocity: { " + std::to_string(playerVel.x) + "; " + std::to_string(playerVel.y) + " }\n");
	toWrite += (" Acceleration: { " + std::to_string(playerAcc.x) + "; " + std::to_string(playerAcc.y) + " }\n");
	toWrite += (" State: " + Debug::GetTextForState(m_debugInfo.state) + "\n");
	toWrite += (" Jump state: " + Debug::GetTextForJumpState(m_debugInfo.jumpState) + "\n");

	m_clock.restart();
	m_debugText.setString(toWrite);
	m_gameWindow->draw(m_debugText);
}
#endif

float GraphicsEngine::GetFramerateLimit()
{
	return GraphicsEngine::FramerateLimit;
}
