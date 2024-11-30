#pragma once
#include "EnemyTankType.h"
#include <chrono>

struct EnemyTank {
    bool IsDead;
    EnemyTankType TankType;
    bool IsMobile;
    sf::Vector2f TankPosition;
    sf::Vector2f TankSpeed;
    float TankCurrentHP;
    float TankMaxHP;
    float ShellVelocity;
    float ShellRange;
    float ShellDamage;
    sf::Texture TankTexture;
    sf::Vector2f TankShellPosition;
    int shellType;
    bool ShellLoaded;
    bool CanShoot;
    sf::Vector2f shellFiringPoint;
    sf::Vector2f shellVectorVelocity;
    bool IsReloading;
    std::chrono::milliseconds ReloadTime;
    std::chrono::time_point<std::chrono::system_clock> EndOfEnemyReload;
};

auto GetEnemyTankTexture (int Tier, EnemyTankType tankType, std::vector<sf::Texture>& TankImages) -> sf::Texture;
auto GenerateEnemies (int CurrentBattleFieldLevel) -> std::vector<EnemyTank>;