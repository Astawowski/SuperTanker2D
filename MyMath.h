#pragma once

auto GetVectorVelocity (float ClickX, float ClickY, float FireX, float FireY, float ShellVelocity) -> sf::Vector2f;
auto SetTimePoints (std::chrono::time_point<std::chrono::system_clock>& end, std::chrono::milliseconds timePeriod) -> void;
auto GetShellRotationAngle (sf::Vector2f shellVelocity) -> float;