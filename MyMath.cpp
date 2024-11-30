#include <SFML/Graphics.hpp>
#include <cmath>
#include <chrono>

// --- Function for Calculating the Shell Vector Velocity---
auto GetVectorVelocity (float const ClickX, float const ClickY, float const FireX, float const FireY, float const ShellVelocity) -> sf::Vector2f {
    float deltaX = ClickX - FireX;
    float deltaY = ClickY - FireY;
    // Handling the case when deltaX is very close to zero
    if (fabs(deltaX) < 0.001) {
        // If so - Firing almost straight up or down
        return {0.0f, (deltaY > 0 ? 1.0f : -1.0f) * ShellVelocity};
    }
    float magnitude = sqrt(deltaX * deltaX + deltaY * deltaY);
    float velocityX = ShellVelocity * deltaX / magnitude;
    float velocityY = ShellVelocity * deltaY / magnitude;
    return {velocityX, velocityY};
};

// --- Function for setting time points
auto SetTimePoints (std::chrono::time_point<std::chrono::system_clock>& end, std::chrono::milliseconds const timePeriod) -> void{
    auto start = std::chrono::system_clock::now();
    end = start + timePeriod;
};

// --- Function for Calculating Shell Rotation ---
auto GetShellRotationAngle (sf::Vector2f shellVelocity) -> float {
    float rotationAngle = atan2(shellVelocity.y, shellVelocity.x) * 180 / M_PI;  // Using atan2 to get Angles from Tangens, And M_PI which is a Pi number
    return rotationAngle;
};