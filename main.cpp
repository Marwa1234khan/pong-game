#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp> // Include for sound
#include <iostream>
#include <sstream> // Include for std::ostringstream

// Constants
const float WINDOW_WIDTH = 800;
const float WINDOW_HEIGHT = 600;
const float PADDLE_WIDTH = 20;
const float PADDLE_HEIGHT = 100;
const float BALL_RADIUS = 10;
const float PADDLE_SPEED = 400;
const float BALL_SPEED = 300;
const float AI_SPEED = 300; // Speed of the AI paddle

class Paddle {
public:
    sf::RectangleShape shape;
    float speed;

    Paddle(float x, float y) {
        shape.setSize(sf::Vector2f(PADDLE_WIDTH, PADDLE_HEIGHT));
        shape.setPosition(x, y);
        speed = PADDLE_SPEED;
    }

    void moveUp(float deltaTime) {
        if (shape.getPosition().y > 0)
            shape.move(0, -speed * deltaTime);
    }

    void moveDown(float deltaTime) {
        if (shape.getPosition().y < WINDOW_HEIGHT - PADDLE_HEIGHT)
            shape.move(0, speed * deltaTime);
    }

    void moveAI(float deltaTime, const sf::CircleShape& ball) {
        // AI will move based on the ball's position
        if (shape.getPosition().y + PADDLE_HEIGHT / 2 < ball.getPosition().y + BALL_RADIUS) {
            shape.move(0, AI_SPEED * deltaTime); // Move down
        } else if (shape.getPosition().y + PADDLE_HEIGHT / 2 > ball.getPosition().y + BALL_RADIUS) {
            shape.move(0, -AI_SPEED * deltaTime); // Move up
        }
    }
};

class Ball {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity;

    Ball(float x, float y, float speedX, float speedY) {
        shape.setRadius(BALL_RADIUS);
        shape.setPosition(x, y);
        velocity = sf::Vector2f(speedX, speedY);
    }

    void resetPosition() {
        shape.setPosition(WINDOW_WIDTH / 2 - BALL_RADIUS, WINDOW_HEIGHT / 2 - BALL_RADIUS);
        velocity = sf::Vector2f(BALL_SPEED, BALL_SPEED);
    }

    void update(float deltaTime) {
        shape.move(velocity * deltaTime);

        if (shape.getPosition().y <= 0 || shape.getPosition().y + BALL_RADIUS * 2 >= WINDOW_HEIGHT) {
            velocity.y = -velocity.y;
        }
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Pong Game");
    window.setFramerateLimit(60);

    // Load font for score text
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font." << std::endl;
        return -1;
    }

    // Load sound buffers
    sf::SoundBuffer hitBuffer, scoreBuffer;
    if (!hitBuffer.loadFromFile("hit_paddle.wav")) {
        std::cerr << "Failed to load hit_paddle.wav" << std::endl;
        return -1;
    }
    if (!scoreBuffer.loadFromFile("score_point.wav")) {
        std::cerr << "Failed to load score_point.wav" << std::endl;
        return -1;
    }

    // Create sound objects
    sf::Sound hitSound(hitBuffer);
    sf::Sound scoreSound(scoreBuffer);

    // Menu variables
    bool inMenu = true;
    sf::Text menuText;
    menuText.setFont(font);
    menuText.setCharacterSize(30);
    menuText.setFillColor(sf::Color::White);
    menuText.setPosition(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 50);

    sf::Text option1Text;
    option1Text.setFont(font);
    option1Text.setCharacterSize(24);
    option1Text.setFillColor(sf::Color::White);
    option1Text.setPosition(WINDOW_WIDTH / 2 - 75, WINDOW_HEIGHT / 2 + 50);
    option1Text.setString("Press 1: Multiplayer");

    sf::Text option2Text;
    option2Text.setFont(font);
    option2Text.setCharacterSize(24);
    option2Text.setFillColor(sf::Color::White);
    option2Text.setPosition(WINDOW_WIDTH / 2 - 75, WINDOW_HEIGHT / 2 + 100);
    option2Text.setString("Press 2: Play vs CPU");

    // Game mode flag
    bool isCPU = false; // Determines whether it's CPU mode or multiplayer mode

    // Variables for game
    Paddle leftPaddle(10, WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2);
    Paddle rightPaddle(WINDOW_WIDTH - PADDLE_WIDTH - 10, WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2);
    Ball ball(WINDOW_WIDTH / 2 - BALL_RADIUS, WINDOW_HEIGHT / 2 - BALL_RADIUS, BALL_SPEED, BALL_SPEED);

    int leftScore = 0;
    int rightScore = 0;

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(WINDOW_WIDTH / 2 - 50, 10);

    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (inMenu) {
            // Handle menu selection
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                inMenu = false; // Start multiplayer game
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                inMenu = false; // Start game vs CPU
                isCPU = true; // Set flag for CPU mode
            }

            window.clear();
            window.draw(menuText);
            window.draw(option1Text);
            window.draw(option2Text);
            window.display();
            continue;
        }

        float deltaTime = clock.restart().asSeconds();

        // Left paddle movement (User)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            leftPaddle.moveUp(deltaTime);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            leftPaddle.moveDown(deltaTime);
        }

        // Right paddle movement (Multiplayer or AI)
        if (isCPU) {
            // AI controls the right paddle
            rightPaddle.moveAI(deltaTime, ball.shape);
        } else {
            // Multiplayer mode: User controls right paddle
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && rightPaddle.shape.getPosition().y > 0) {
                rightPaddle.moveUp(deltaTime);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && rightPaddle.shape.getPosition().y + PADDLE_HEIGHT < WINDOW_HEIGHT) {
                rightPaddle.moveDown(deltaTime);
            }
        }

        // Ball movement
        ball.update(deltaTime);

        // Ball collision with paddles
        if (ball.shape.getGlobalBounds().intersects(leftPaddle.shape.getGlobalBounds()) ||
            ball.shape.getGlobalBounds().intersects(rightPaddle.shape.getGlobalBounds())) {
            ball.velocity.x = -ball.velocity.x;
            hitSound.play(); // Play hit sound when ball hits paddle
        }

        // Ball out of bounds
        if (ball.shape.getPosition().x < 0) {
            rightScore++;
            ball.resetPosition();
            scoreSound.play(); // Play score sound when right player scores
        }
        if (ball.shape.getPosition().x > WINDOW_WIDTH) {
            leftScore++;
            ball.resetPosition();
            scoreSound.play(); // Play score sound when left player scores
        }

        // Update score text using std::ostringstream
        std::ostringstream scoreStream;
        scoreStream << leftScore << " : " << rightScore;
        scoreText.setString(scoreStream.str());

        // Handle returning to the main menu with Z key
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
            // Reset the game state to return to the menu
            inMenu = true;
            isCPU = false;
            leftScore = 0;
            rightScore = 0;
            ball.resetPosition();
            leftPaddle.shape.setPosition(10, WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2);
            rightPaddle.shape.setPosition(WINDOW_WIDTH - PADDLE_WIDTH - 10, WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2);
        }

        // Rendering
        window.clear();
        window.draw(leftPaddle.shape);
        window.draw(rightPaddle.shape);
        window.draw(ball.shape);
        window.draw(scoreText);
        window.display();
    }

    return 0;
}

