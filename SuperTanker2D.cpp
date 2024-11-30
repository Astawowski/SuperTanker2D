#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread> // for wait command & time management
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <random>
#include "Field.h"
#include "MapGeneratorEngine.h"
#include "MyMath.h"
#include "EnemyTank.h"
#include <fstream>



auto main() -> int {

    // ------------------------------Window Data--------------------------------------
    //  ---- Window -----
    auto window = sf::RenderWindow(
            sf::VideoMode({1920, 1080}), "SuperTanker2D",
            sf::Style::Default, sf::ContextSettings(0, 0, 8)
    );

    auto FPSCap = 60;
    window.setFramerateLimit(FPSCap);
    auto FPSspeedMultiplier = float((float(169)/static_cast<float>(FPSCap)));    // The game was designed for 169 FPS Cap originally

    // --- Original FPS Limit dependent values
    auto OriginalPlayerMovementSpeed = sf::Vector2f(0.7,0.7);
    auto OriginalAPShellVelocity = float(3);
    auto OriginalAPCRShellVelocity = float(3.5);
    auto OriginalHEShellVelocity = float(2);
    auto OriginalFuelBurningRate = float(0.005);
    auto OriginalSpikeDamage = float(0.05);

    //  --- Game Icon ---
    sf::Image icon;
    if (!icon.loadFromFile("../C++GameData/textures/MainIcon.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    // --- Text Arial Font ---
    auto ArialFont = sf::Font();
    if (!ArialFont.loadFromFile("../C++GameData/fonts/arial.ttf")){
        std::cout << "Couldn't load game data - font";
        return -4;
    }

    // --- Universal wait command ---
    auto wait = [](int seconds) -> void {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
    };

    // -------------DEFAULT GAME DATA PART 1--------------------------------------------------------------------

    // --- Player Economic
    auto PlayerCurrentCreditsCount = int(0);
    auto PlayerCurrentXPCount = int(0);

    // --- Current Player Tank tier (1-7) ---
    auto PlayerCurrentTier = int(1);

    // --- Current Tank Modules Level (1-3) ---
    auto PlayerCurrentGunLevel = int(1);
    auto PlayerCurrentTurretLevel = int(1);
    auto PlayerCurrentEngineLevel = int(1);

    //--- Current Battlefield Level ---
    auto CurrentBattlefieldLevel = int(1);

    // --- PlayerTankSpeed ---
    auto DefaultPlayerMovementSpeed = sf::Vector2f(static_cast<float>(0.7)*FPSspeedMultiplier,static_cast<float>(0.7)*FPSspeedMultiplier);
    auto PlayerMovementSpeed = DefaultPlayerMovementSpeed;

    // --- Player Default Start Position ---
    auto DefaultPlayerPosition = sf::Vector2f(91,544);

    // --- Player Current Position ---
    auto PlayerPosition = DefaultPlayerPosition;

    // --- Booling saying whether enemies movement cycle is on
    auto EnemiesMoving = false;

    // --- Player Fuel Burning Rate ---
    auto PlayerDefaultFuelBurning = float(OriginalFuelBurningRate*FPSspeedMultiplier);
    auto PlayerCurrentFuelBurning = PlayerDefaultFuelBurning;

    // --- Player HP ---
    auto PlayerDefaultHP = float(52);
    auto PlayerCurrentHP = PlayerDefaultHP;
    auto PlayerMaxHP = PlayerDefaultHP;

    // --- Player Resistance to Enemies Ratio
    auto DefaultPlayerResistanceRatio = float(1);
    auto CurrentPlayerResistanceRatio = DefaultPlayerResistanceRatio;

    // --- Player Fuel Level ---
    auto PlayerDefaultFuelLevel = float(80);
    auto PlayerCurrentFuelLevel = PlayerDefaultFuelLevel;
    auto PlayerMaxFuelLevel = PlayerDefaultFuelLevel;

    // --- Player Reload Time ---
    auto PlayerDefaultReload =  std::chrono::milliseconds(1500);
    auto PlayerDefaultReloadInSec =  std::chrono::duration<float>(PlayerDefaultReload).count();
                                                                            //  !!  Reload time must be bigger than half of the longest travelling shell travel time.
    auto PlayerCurrentReload = PlayerDefaultReload;              //   Shell Travel time [s] = ShellRange/(ShellVelocity*167)
    auto PlayerCurrentReloadInSec = PlayerDefaultReloadInSec;          // Min. Reload Time = 1/2 * Longest Shell Travel Time
                                                                            //   P.S. it is handled by dedicated function (Throwing Error if incorrect)
    // --- Ammo Count ---
    auto APShellDefaultCount = int(15);
    auto APShellCurrentCount = APShellDefaultCount;
    auto APCRShellDefaultCount = int(15);
    auto APCRShellCurrentCount = APCRShellDefaultCount;
    auto HEShellDefaultCount = int(15);
    auto HEShellCurrentCount = HEShellDefaultCount;

    // --- Repair Kits Count ---
    auto DefaultRepairKitCount = 5;
    auto CurrentRepairKitCount = DefaultRepairKitCount;
    // --- Boolean saying whether player's using repair kit rn
    auto PlayerUsingRepairKit = false;

    //--- Boolean saying whether player's using manual repair kit rn
    auto PlayerUsingManualRepairKit = false;

    // --- Extra Shield Count ---
    auto DefaultExtraShieldCount = 3;
    auto CurrentExtraShieldCount = DefaultExtraShieldCount;
    // --- Boolean saying whether player's using extra shield rn
    auto PlayerUsingExtraShield = false;

    // --- MedKit Count ---
    auto DefaultMedKitCount = 3;
    auto CurrentMedKitCount = DefaultMedKitCount;

    // --- Engine Accelerator Count ---
    auto DefaultEngineAcceleratorCount = 3;
    auto CurrentEngineAcceleratorCount = DefaultEngineAcceleratorCount;
    // --- Boolean saying whether player's using extra shield rn
    auto PlayerUsingEngineAccelerator = false;

    // --- Fuel Producer
    auto PlayerUsingFuelProducer = false;

    // --- Ammo Producer
    auto PlayerUsingAmmoProducer = false;

    // --- Shells Data --------------------------------------------------

    // Reloading flag
    auto PlayerReloading = false;

    // boolean saying whether 1st shell is ready for shot
    auto FirstShellLoaded = true;

    // boolean saying whether 2nd shell is ready for shot
    auto SecondShellLoaded = true;

    // shell type (1-AP, 2-APCR, 3-HE)
    int DefaultShellType = 1;
    int shellType = DefaultShellType;
    auto FirstShellType = int(1);
    auto SecondShellType = int(1);

    // First Shell fired position
    auto FirstShellFirePositionX = PlayerPosition.x;
    auto FirstShellFirePositionY = PlayerPosition.y;

    // Second Shell fired position
    auto SecondShellFirePositionX = PlayerPosition.x;
    auto SecondShellFirePositionY = PlayerPosition.y;

    //First Shell vector velocity (with default value)
    auto FirstshellVectorVelocity = sf::Vector2f(0,0);

    //Second Shell vector velocity (with default value)
    auto SecondshellVectorVelocity = sf::Vector2f(0,0);

    //----------------------  Armor Piercing (AP) ------------------------
    auto APShellTexture = sf::Texture();
    if(!APShellTexture.loadFromFile("../C++GameData/textures/AP-movingTexture.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto APShellTextureImage = sf::Sprite(APShellTexture);
    APShellTextureImage.setOrigin(APShellTextureImage.getLocalBounds().width / 2, APShellTextureImage.getLocalBounds().height / 2);

    auto APShellIconTexture = sf::Texture();
    if(!APShellIconTexture.loadFromFile("../C++GameData/textures/AP-iconTexture.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto APShellIconImage = sf::Sprite(APShellIconTexture);
    APShellIconImage.setPosition(1874, 213);

    //--- Default Stats---
    auto APShellDefaultDamage = float(22);
    auto APShellDefaultPenetration = float(10);
    auto APShellDefaultVelocity = float(2*FPSspeedMultiplier);
    auto APShellDefaultRange = float(750);

    // ---Current Stats---
    auto APShellDamage = APShellDefaultDamage;
    auto APShellPenetration = APShellDefaultPenetration;
    auto APShellVelocity = APShellDefaultVelocity;
    auto APShellRange = APShellDefaultRange;

    // -------------------------Armor Piercing Composite Rigid (APCR)-------------------------
    auto APCRShellTexture = sf::Texture();
    if(!APCRShellTexture.loadFromFile("../C++GameData/textures/APCR-movingTexture.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto APCRShellTextureImage = sf::Sprite(APCRShellTexture);
    APCRShellTextureImage.setOrigin(APCRShellTextureImage.getLocalBounds().width / 2, APCRShellTextureImage.getLocalBounds().height / 2);

    auto APCRShellIconTexture = sf::Texture();
    if(!APCRShellIconTexture.loadFromFile("../C++GameData/textures/APCR-iconTexture.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto APCRShellIconImage = sf::Sprite(APCRShellIconTexture);
    APCRShellIconImage.setPosition(1874, 267);

    //--- Default Stats---
    auto APCRShellDefaultDamage = float(13);
    auto APCRShellDefaultPenetration = float(18);
    auto APCRShellDefaultVelocity = float(3*FPSspeedMultiplier);
    auto APCRShellDefaultRange = float(800);

    // ---Current Stats---
    auto APCRShellDamage = APCRShellDefaultDamage;
    auto APCRShellPenetration = APCRShellDefaultPenetration;
    auto APCRShellVelocity = APCRShellDefaultVelocity;
    auto APCRShellRange = APCRShellDefaultRange;

    //------------------------High Explosive (HE)---------------------------------
    auto HEShellTexture = sf::Texture();
    if(!HEShellTexture.loadFromFile("../C++GameData/textures/HE-movingTexture.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto HEShellTextureImage = sf::Sprite(HEShellTexture);
    HEShellTextureImage.setOrigin(HEShellTextureImage.getLocalBounds().width / 2, HEShellTextureImage.getLocalBounds().height / 2);

    auto HEShellIconTexture = sf::Texture();
    if(!HEShellIconTexture.loadFromFile("../C++GameData/textures/HE-iconTexture.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto HEShellIconImage = sf::Sprite(HEShellIconTexture);
    HEShellIconImage.setPosition(1874, 313);

    //--- Default Stats---
    auto HEShellDefaultDamage = float(30);
    auto HEShellDefaultPenetration = float(2);
    auto HEShellDefaultVelocity = float(1*FPSspeedMultiplier);
    auto HEShellDefaultRange = float(750);

    // ---Current Stats---
    auto HEShellDamage = HEShellDefaultDamage;
    auto HEShellPenetration = HEShellDefaultPenetration;
    auto HEShellVelocity = HEShellDefaultVelocity;
    auto HEShellRange = HEShellDefaultRange;

    // Min. Reload Time Calculator and Validator
    auto MaxShellTravelTime = (APShellRange/(APShellVelocity*169));
    if((APCRShellRange/(APCRShellVelocity*167))>MaxShellTravelTime) MaxShellTravelTime = (APCRShellRange/(APCRShellVelocity*167));
    if((HEShellRange/(HEShellVelocity*167))>MaxShellTravelTime) MaxShellTravelTime = (HEShellRange/(HEShellVelocity*167));
    if(PlayerCurrentReloadInSec<((0.5)*MaxShellTravelTime)) {
        std::cout<<"Error - Player Reload Time is too small compared to shell travel time.";
        return -3;
    }

    //-----------------------------------------------------------------------------------------------

    // ---------------------------------- Consumables Data ----------------------------------
    // -------------------Icon Images----------------------
    //  --- Repair Kit ---
    auto RepairKitIcon = sf::Texture();
    if(!RepairKitIcon.loadFromFile("../C++GameData/textures/Repair_KitConsumableIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto RepairKitIconImage = sf::Sprite(RepairKitIcon);
    RepairKitIconImage.setPosition(1867, 692);
    auto RepairKitCountText = sf::Text(sf::String(std::to_string(CurrentRepairKitCount)), ArialFont, 30);

    // --- Infinity Icon for Infinite Consumables
    auto InfinityIcon = sf::Texture();
    if(!InfinityIcon.loadFromFile("../C++GameData/textures/InfinityIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto InfinityIconImage = sf::Sprite(InfinityIcon);

    //  --- Manual Repair Kit ---
    auto ManualRepairKitIcon = sf::Texture();
    if(!ManualRepairKitIcon.loadFromFile("../C++GameData/textures/ManualRepairKitConsumableIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto ManualRepairKitIconImage = sf::Sprite(ManualRepairKitIcon);
    ManualRepairKitIconImage.setPosition(1867, 743);
    auto ManualRepairKitCountIcon = InfinityIconImage;
    ManualRepairKitCountIcon.setPosition(1829,759);

    //  --- Extra Shield ---
    auto ExtraShieldIcon = sf::Texture();
    if(!ExtraShieldIcon.loadFromFile("../C++GameData/textures/ExtraShieldConsumableIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto ExtraShieldIconImage = sf::Sprite(ExtraShieldIcon);
    ExtraShieldIconImage.setPosition(1867, 794);
    auto ExtraShieldCountText = sf::Text(sf::String(std::to_string(CurrentExtraShieldCount)), ArialFont, 30);

    // --- Med Kit ---
    auto MedKitIcon = sf::Texture();
    if(!MedKitIcon.loadFromFile("../C++GameData/textures/MedkitConsumableIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto MedKitIconImage = sf::Sprite(MedKitIcon);
    MedKitIconImage.setPosition(1867, 845);
    auto MedKitCountText = sf::Text(sf::String(std::to_string(CurrentMedKitCount)), ArialFont, 30);

    // --- Engine Accelerator ---
    auto EngineAcceleratorIcon = sf::Texture();
    if(!EngineAcceleratorIcon.loadFromFile("../C++GameData/textures/EngineAcceleratorConsumableIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto EngineAcceleratorIconImage = sf::Sprite(EngineAcceleratorIcon);
    EngineAcceleratorIconImage.setPosition(1867, 896);
    auto EngineAcceleratorCountText = sf::Text(sf::String(std::to_string(CurrentEngineAcceleratorCount)), ArialFont, 30);

    // --- Extra Fuel ---
    auto ExtraFuelIcon = sf::Texture();
    if(!ExtraFuelIcon.loadFromFile("../C++GameData/textures/ExtraFuelConsumableIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto ExtraFuelIconImage = sf::Sprite(ExtraFuelIcon);
    ExtraFuelIconImage.setPosition(1867, 947);
    auto FuelProducerCountIcon = InfinityIconImage;
    FuelProducerCountIcon.setPosition(1829,963);

    //  --- Ammo Producer ---
    auto AmmoProducerIcon = sf::Texture();
    if(!AmmoProducerIcon.loadFromFile("../C++GameData/textures/AmmoProducerConsumableIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto AmmoProducerIconImage = sf::Sprite(AmmoProducerIcon);
    AmmoProducerIconImage.setPosition(1867, 998);
    auto AmmoProducerCountIcon = InfinityIconImage;
    AmmoProducerCountIcon.setPosition(1829,1014);

    // -------------------- USER INTERFACE ELEMENTS-------------------------

    // --- Credits Current Count Text ---
    auto CurrentCreditsCountText = sf::Text(std::to_string(PlayerCurrentCreditsCount),ArialFont,30);
    CurrentCreditsCountText.setFillColor(sf::Color::White);
    CurrentCreditsCountText.setPosition(1042,22);

    // --- XP Current Count Text ---
    auto CurrentXPCountText = sf::Text(std::to_string(PlayerCurrentXPCount),ArialFont,30);
    CurrentXPCountText.setFillColor(sf::Color::White);
    CurrentXPCountText.setPosition(1292,22);

    auto CreditsCountIcon = sf::Texture();
    if(!CreditsCountIcon.loadFromFile("../C++GameData/textures/Credits_CountIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto CreditsCountIconImage = sf::Sprite(CreditsCountIcon);
    CreditsCountIconImage.setPosition(1000,20);

    auto CreditsLootIcon = sf::Texture();
    if(!CreditsLootIcon.loadFromFile("../C++GameData/textures/Credits_LootIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto CreditsLootIconImage = sf::Sprite(CreditsLootIcon);
    CreditsLootIconImage.setOrigin(CreditsLootIconImage.getLocalBounds().width / 2, CreditsLootIconImage.getLocalBounds().height / 2);

    auto XPCountIcon = sf::Texture();
    if(!XPCountIcon.loadFromFile("../C++GameData/textures/XP_CountIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto XPCountIconImage = sf::Sprite(XPCountIcon);
    XPCountIconImage.setPosition(1250,20);

    auto CurrencyBackground = sf::RectangleShape(sf::Vector2f(445,50));
    CurrencyBackground.setFillColor(sf::Color(0,0,0,128));
    CurrencyBackground.setPosition(985,15);

    // --- Flag for player Tracks being destroyed
    auto PlayerTracksDestroyed = false;

    // --- Texture for tracks destroyed indicator
    auto TracksDestroyedIndicator = sf::Texture();
    if(!TracksDestroyedIndicator.loadFromFile("../C++GameData/textures/TracksDestroyedIndicator.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto TracksDestroyedIndicatorImage = sf::Sprite(TracksDestroyedIndicator);

    // --- Flag for player Engine being destroyed
    auto PlayerEngineDestroyed = false;

    // --- Texture for tracks destroyed indicator
    auto EngineDestroyedIndicator = sf::Texture();
    if(!EngineDestroyedIndicator.loadFromFile("../C++GameData/textures/EngineDestroyedIndicator.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto EngineDestroyedIndicatorImage = sf::Sprite(EngineDestroyedIndicator);

    // --- Texture for gun damaged indicator
    auto GunDamagedIndicator = sf::Texture();
    if(!GunDamagedIndicator.loadFromFile("../C++GameData/textures/GunDamagedIndicator.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto GunDamagedIndicatorImage = sf::Sprite(GunDamagedIndicator);

    // --- Flag for player Gun being damaged
    auto PlayerGunDamaged = false;

    // --- Text for repair in progress indicator
    auto RepairingText = sf::Text(sf::String("Repairing..."),ArialFont,25);
    RepairingText.setFillColor(sf::Color::Black);

    //  --- Resistance Icon ---
    auto PlayerResistanceIcon = sf::Texture();
    if(!PlayerResistanceIcon.loadFromFile("../C++GameData/textures/PlayerResistanceIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto PlayerResistanceIconImage = sf::Sprite(PlayerResistanceIcon);
    PlayerResistanceIconImage.setPosition(1845, 120);

    // --- Info for num keys for consumables&Ammo ---
    // -- 1 --
    auto NumKey1Text = sf::Text(sf::String("1"),ArialFont,15);
    NumKey1Text.setFillColor(sf::Color::White);
    NumKey1Text.setPosition(1870, 210);
    // -- 2 --
    auto NumKey2Text = sf::Text(sf::String("2"),ArialFont,15);
    NumKey2Text.setFillColor(sf::Color::White);
    NumKey2Text.setPosition(1870, 260);
    // -- 3 --
    auto NumKey3Text = sf::Text(sf::String("3"),ArialFont,15);
    NumKey3Text.setFillColor(sf::Color::White);
    NumKey3Text.setPosition(1870, 310);
    // -- 4 --
    auto NumKey4Text = sf::Text(sf::String("4"),ArialFont,15);
    NumKey4Text.setFillColor(sf::Color::White);
    NumKey4Text.setPosition(1856, 692);
    // -- 5 --
    auto NumKey5Text = sf::Text(sf::String("5"),ArialFont,15);
    NumKey5Text.setFillColor(sf::Color::White);
    NumKey5Text.setPosition(1856, 743);
    // -- 6 --
    auto NumKey6Text = sf::Text(sf::String("6"),ArialFont,15);
    NumKey6Text.setFillColor(sf::Color::White);
    NumKey6Text.setPosition(1856, 794);
    // -- 7 --
    auto NumKey7Text = sf::Text(sf::String("7"),ArialFont,15);
    NumKey7Text.setFillColor(sf::Color::White);
    NumKey7Text.setPosition(1856, 845);
    // -- 8 --
    auto NumKey8Text = sf::Text(sf::String("8"),ArialFont,15);
    NumKey8Text.setFillColor(sf::Color::White);
    NumKey8Text.setPosition(1856, 896);
    // -- 9 --
    auto NumKey9Text = sf::Text(sf::String("9"),ArialFont,15);
    NumKey9Text.setFillColor(sf::Color::White);
    NumKey9Text.setPosition(1856, 947);
    // -- 0 --
    auto NumKey0Text = sf::Text(sf::String("0"),ArialFont,15);
    NumKey0Text.setFillColor(sf::Color::White);
    NumKey0Text.setPosition(1856, 998);

    // --- PLAYER TANK ICONS (GERMANY)-----------------------------------------

    //--- Pz. II Icon - Tier I---
    auto PzIITankIcon = sf::Texture();
    if(!PzIITankIcon.loadFromFile("../C++GameData/textures/PzIITankIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto PzIITankIconImage = sf::Sprite(PzIITankIcon);
    PzIITankIconImage.setOrigin(PzIITankIconImage.getLocalBounds().width / 2, PzIITankIconImage.getLocalBounds().height / 2);

    //--- Pz. III Icon - Tier II---
    auto PzIIITankIcon = sf::Texture();
    if(!PzIIITankIcon.loadFromFile("../C++GameData/textures/PzIIITankIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto PzIIITankIconImage = sf::Sprite(PzIIITankIcon);
    PzIIITankIconImage.setOrigin(PzIIITankIconImage.getLocalBounds().width / 2, PzIIITankIconImage.getLocalBounds().height / 2);

    //--- Pz. IV Icon - Tier III---
    auto PzIVTankIcon = sf::Texture();
    if(!PzIVTankIcon.loadFromFile("../C++GameData/textures/PzIVTankIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto PzIVTankIconImage = sf::Sprite(PzIVTankIcon);
    PzIVTankIconImage.setOrigin(PzIVTankIconImage.getLocalBounds().width / 2, PzIVTankIconImage.getLocalBounds().height / 2);

    //--- Panther Icon - Tier IV---
    auto PantherTankIcon = sf::Texture();
    if(!PantherTankIcon.loadFromFile("../C++GameData/textures/PantherTankIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto PantherTankIconImage = sf::Sprite(PantherTankIcon);
    PantherTankIconImage.setOrigin(PantherTankIconImage.getLocalBounds().width / 2, PantherTankIconImage.getLocalBounds().height / 2);

    // --- Tiger 1 Icon - Tier V ---
    auto Tiger1TankIcon = sf::Texture();
    if(!Tiger1TankIcon.loadFromFile("../C++GameData/textures/Tiger1TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Tiger1TankIconImage = sf::Sprite(Tiger1TankIcon);
    Tiger1TankIconImage.setOrigin(Tiger1TankIconImage.getLocalBounds().width / 2, Tiger1TankIconImage.getLocalBounds().height / 2);

    // --- Tiger 2 Icon - Tier VI ---
    auto Tiger2TankIcon = sf::Texture();
    if(!Tiger2TankIcon.loadFromFile("../C++GameData/textures/Tiger2TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Tiger2TankIconImage = sf::Sprite(Tiger2TankIcon);
    Tiger2TankIconImage.setOrigin(Tiger2TankIconImage.getLocalBounds().width / 2, Tiger2TankIconImage.getLocalBounds().height / 2);

    // --- Maus Icon - Tier VII ---
    auto MausTankIcon = sf::Texture();
    if(!MausTankIcon.loadFromFile("../C++GameData/textures/MausTankIcon.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto MausTankIconImage = sf::Sprite(MausTankIcon);
    MausTankIconImage.setOrigin(MausTankIconImage.getLocalBounds().width / 2, MausTankIconImage.getLocalBounds().height / 2);
    //--------------------------------------------------------

    //   ------------------------------DEFAULT GAME DATA PART 2---------------------------------

    // --- Red flag texture for end point flag being unavailable
    auto RedFlagTexture = sf::Texture();
    if(!RedFlagTexture.loadFromFile("../C++GameData/textures/GameEndPointRedFlag.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto RedFlagImage = sf::Sprite(RedFlagTexture);
    RedFlagImage.setPosition(1700,500);

    // --- Green flag texture for end point flag being available
    auto GreenFlagTexture = sf::Texture();
    if(!GreenFlagTexture.loadFromFile("../C++GameData/textures/GameEndPointGreenFlag.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto GreenFlagImage = sf::Sprite(GreenFlagTexture);
    GreenFlagImage.setPosition(1700,500);

    // --- Current Flag image
    auto CurrentEndFlagImage = RedFlagImage;

    // --- Current Flag
    auto CurrentEndFlagGreen = false;

    // --- Player Default Tank Image ---
    auto PlayerTankImage = PzIITankIconImage;
    PlayerTankImage.setPosition(DefaultPlayerPosition);
    auto PlayerHorizontallyRight = true;

    // First Shell Current Position (with default value, when it's not moving)
    auto FirstshellPosition = PlayerTankImage.getPosition();

    // First Shell Current Position (with default value, when it's not moving)
    auto SecondshellPosition = PlayerTankImage.getPosition();

    // Current First Shell type loaded (AP by default)
    auto CurrentFirstShellLoaded = APShellTextureImage;
    auto CurrentFirstShellDamage = APShellDamage;
    auto CurrentFirstShellPenetration = APShellPenetration;
    auto CurrentFirstShellVelocity = APShellVelocity;
    auto CurrentFirstShellRange = APShellRange;

    // Current Second Shell type loaded (AP by default)
    auto CurrentSecondShellLoaded = APShellTextureImage;
    auto CurrentSecondShellDamage = APShellDamage;
    auto CurrentSecondShellPenetration = APShellPenetration;
    auto CurrentSecondShellVelocity = APShellVelocity;
    auto CurrentSecondShellRange = APShellRange;

    // Boolean to flag whether shells effected by forest
    auto ShellsEffectedByForest = false;

    // --- Function to effect shell specs
    auto EffectShellsSpecs = [&](float const rangeRatio, float const damageRatio, float const penRatio) -> void {
        CurrentFirstShellDamage *= damageRatio;
        CurrentSecondShellDamage *= damageRatio;
        APShellDamage *= damageRatio;
        APCRShellDamage *= damageRatio;
        HEShellDamage *= damageRatio;


        CurrentFirstShellRange *= rangeRatio;
        CurrentSecondShellRange *= rangeRatio;
        APShellRange *= rangeRatio;
        APCRShellRange *= rangeRatio;
        HEShellRange *= rangeRatio;

        CurrentFirstShellPenetration *= penRatio;
        CurrentSecondShellPenetration *= penRatio;
        APShellPenetration *= penRatio;
        APCRShellPenetration *= penRatio;
        HEShellPenetration *= penRatio;
    };

    // --- Function for setting current shell type
    auto setCurrentShellType = [&]() -> void {
        if (shellType == 1) {
            if (FirstShellLoaded) {
                CurrentFirstShellLoaded = APShellTextureImage;
                CurrentFirstShellVelocity = APShellVelocity;
                CurrentFirstShellRange = APShellRange;
                CurrentFirstShellDamage = APShellDamage;
                CurrentFirstShellPenetration = APShellPenetration;
                FirstShellType = 1;
            }
            if (SecondShellLoaded) {
                CurrentSecondShellLoaded = APShellTextureImage;
                CurrentSecondShellVelocity = APShellVelocity;
                CurrentSecondShellRange = APShellRange;
                CurrentSecondShellDamage = APShellDamage;
                CurrentSecondShellPenetration = APShellPenetration;
                SecondShellType = 1;
            }
        } else if (shellType == 2) {
            if (FirstShellLoaded) {
                CurrentFirstShellLoaded = APCRShellTextureImage;
                CurrentFirstShellVelocity = APCRShellVelocity;
                CurrentFirstShellRange = APCRShellRange;
                CurrentFirstShellDamage = APCRShellDamage;
                CurrentFirstShellPenetration = APCRShellPenetration;
                FirstShellType = 2;
            }
            if (SecondShellLoaded) {
                CurrentSecondShellLoaded = APCRShellTextureImage;
                CurrentSecondShellVelocity = APCRShellVelocity;
                CurrentSecondShellRange = APCRShellRange;
                CurrentSecondShellDamage = APCRShellDamage;
                CurrentSecondShellPenetration = APCRShellPenetration;
                SecondShellType = 2;
            }
        } else if (shellType == 3) {
            if (FirstShellLoaded) {
                CurrentFirstShellLoaded = HEShellTextureImage;
                CurrentFirstShellVelocity = HEShellVelocity;
                CurrentFirstShellRange = HEShellRange;
                CurrentFirstShellDamage = HEShellDamage;
                CurrentFirstShellPenetration = HEShellPenetration;
                FirstShellType = 3;
            }
            if (SecondShellLoaded) {
                CurrentSecondShellLoaded = HEShellTextureImage;
                CurrentSecondShellVelocity = HEShellVelocity;
                CurrentSecondShellRange = HEShellRange;
                CurrentSecondShellDamage = HEShellDamage;
                CurrentSecondShellPenetration = HEShellPenetration;
                SecondShellType = 3;
            }
        }
    };

    // --- Function to reset shells effects
    auto ResetShellsSpecs = [&]() -> void {
        APShellDamage = APShellDefaultDamage;
        APCRShellDamage = APCRShellDefaultDamage;
        HEShellDamage = HEShellDefaultDamage;

        APShellPenetration = APShellDefaultPenetration;
        APCRShellPenetration = APCRShellDefaultPenetration;
        HEShellPenetration = HEShellDefaultPenetration;

        APShellRange = APShellDefaultRange;
        APCRShellRange = APCRShellDefaultRange;
        HEShellRange = HEShellDefaultRange;

        setCurrentShellType();
    };

    // Flags to track key presses (Tank Movement)
    bool keyPressedW = false;
    bool keyPressedS = false;
    bool keyPressedA = false;
    bool keyPressedD = false;

    // --- Function for player movement&position reset ---
    auto ResetPlayerMovement = [&]() -> void {
        keyPressedW = false;
        keyPressedS = false;
        keyPressedA = false;
        keyPressedD = false;
        PlayerPosition = DefaultPlayerPosition;
    };
    //--------------------------------------------------------------------------------------------------

    // --------------------------- Game Map Fields Textures ---------------------------------------

    //---Standard Ground---
    auto StandardGround_GameField = sf::Texture();
    if(!StandardGround_GameField.loadFromFile("../C++GameData/textures/Standard_Ground_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto StandardGround_GameFieldImage = sf::Sprite(StandardGround_GameField);
    StandardGround_GameFieldImage.setOrigin(75,75);

    //--- Swamps ---
    auto Swamps_GameField = sf::Texture();
    if(!Swamps_GameField.loadFromFile("../C++GameData/textures/Swamps_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Swamps_GameFieldImage = sf::Sprite(Swamps_GameField);
    Swamps_GameFieldImage.setOrigin(75,75);

    //--- Sands ---
    auto Sands_GameField = sf::Texture();
    if(!Sands_GameField.loadFromFile("../C++GameData/textures/Sands_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Sands_GameFieldImage = sf::Sprite(Sands_GameField);
    Sands_GameFieldImage.setOrigin(75,75);

    //--- Home Obstacle ---
    auto HomeObstacle_GameField = sf::Texture();
    if(!HomeObstacle_GameField.loadFromFile("../C++GameData/textures/HomeObstacle_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto HomeObstacle_GameFieldImage = sf::Sprite(HomeObstacle_GameField);
    HomeObstacle_GameFieldImage.setOrigin(75,75);

    // --- Tank Spikes Obstacle---
    auto TankSpikesObstacle_GameField = sf::Texture();
    if(!TankSpikesObstacle_GameField.loadFromFile("../C++GameData/textures/TankSpikesObstacle_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto TankSpikesObstacle_GameFieldImage = sf::Sprite(TankSpikesObstacle_GameField);
    TankSpikesObstacle_GameFieldImage.setOrigin(75,75);

    // --- Big Hole Instant Death---
    auto BigHole_InstantDeath_GameField = sf::Texture();
    if(!BigHole_InstantDeath_GameField.loadFromFile("../C++GameData/textures/BigHole_InstantDeath_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto BigHole_InstantDeath_GameFieldImage = sf::Sprite(BigHole_InstantDeath_GameField);
    BigHole_InstantDeath_GameFieldImage.setOrigin(75,75);

    // --- Forest ---
    auto Forest_GameField = sf::Texture();
    if(!Forest_GameField.loadFromFile("../C++GameData/textures/Forest_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Forest_GameFieldImage = sf::Sprite(Forest_GameField);
    Forest_GameFieldImage.setOrigin(75,75);

    // ---Full Petrol Truck ---
    auto Petrol_Truck_Full_GameField = sf::Texture();
    if(!Petrol_Truck_Full_GameField.loadFromFile("../C++GameData/textures/Petrol_Truck_Full_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Petrol_Truck_Full_GameFieldImage = sf::Sprite(Petrol_Truck_Full_GameField);
    Petrol_Truck_Full_GameFieldImage.setOrigin(75,75);

    // --- Empty Petrol Truck ---
    auto Petrol_Truck_Empty_GameField = sf::Texture();
    if(!Petrol_Truck_Empty_GameField.loadFromFile("../C++GameData/textures/Petrol_Truck_Empty_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Petrol_Truck_Empty_GameFieldImage = sf::Sprite(Petrol_Truck_Empty_GameField);
    Petrol_Truck_Empty_GameFieldImage.setOrigin(75,75);

    // --- Explosives ---
    auto Explosives_GameField = sf::Texture();
    if(!Explosives_GameField.loadFromFile("../C++GameData/textures/Explosives_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Explosives_GameFieldImage = sf::Sprite(Explosives_GameField);
    Explosives_GameFieldImage.setOrigin(75,75);

    // --- Burned Ground ---
    auto BurnedGround_GameField = sf::Texture();
    if(!BurnedGround_GameField.loadFromFile("../C++GameData/textures/BurnedGround_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto BurnedGround_GameFieldImage = sf::Sprite(BurnedGround_GameField);
    BurnedGround_GameFieldImage.setOrigin(75,75);

    // --- Full Workshop ---
    auto Workshop_Full_GameField = sf::Texture();
    if(!Workshop_Full_GameField.loadFromFile("../C++GameData/textures/Workshop_Full_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Workshop_Full_GameFieldImage = sf::Sprite(Workshop_Full_GameField);
    Workshop_Full_GameFieldImage.setOrigin(75,75);

    // --- Empty Workshop ---
    auto Workshop_Empty_GameField = sf::Texture();
    if(!Workshop_Empty_GameField.loadFromFile("../C++GameData/textures/Workshop_Empty_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Workshop_Empty_GameFieldImage = sf::Sprite(Workshop_Empty_GameField);
    Workshop_Empty_GameFieldImage.setOrigin(75,75);

    // --- Full Ammo Drop ---
    auto AmmoDrop_Full_GameField = sf::Texture();
    if(!AmmoDrop_Full_GameField.loadFromFile("../C++GameData/textures/AmmoDrop_Full_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto AmmoDrop_Full_GameFieldImage = sf::Sprite(AmmoDrop_Full_GameField);
    AmmoDrop_Full_GameFieldImage.setOrigin(75,75);

    // --- Empty Ammo Drop ---
    auto AmmoDrop_Empty_GameField = sf::Texture();
    if(!AmmoDrop_Empty_GameField.loadFromFile("../C++GameData/textures/AmmoDrop_Empty_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto AmmoDrop_Empty_GameFieldImage = sf::Sprite(AmmoDrop_Empty_GameField);
    AmmoDrop_Empty_GameFieldImage.setOrigin(75,75);

    // --- Big Mine Instant Death ---
    auto BigMine_InstantDeath_GameField = sf::Texture();
    if(!BigMine_InstantDeath_GameField.loadFromFile("../C++GameData/textures/BigMine_InstantDeath_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto BigMine_InstantDeath_GameFieldImage = sf::Sprite(BigMine_InstantDeath_GameField);
    BigMine_InstantDeath_GameFieldImage.setOrigin(75,75);

    // --- Blowed Mines Game Field after hidden mines exploded on player
    auto BlowedMines_GameField = sf::Texture();
    if(!BlowedMines_GameField.loadFromFile("../C++GameData/textures/BlowedMines_GameField.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto BlowedMines_GameFieldImage = sf::Sprite(BlowedMines_GameField);
    BlowedMines_GameFieldImage.setOrigin(75,75);

    // --- Explosion Radios after Explosion
    auto ExplosionRadios = sf::Texture();
    if(!ExplosionRadios.loadFromFile("../C++GameData/textures/ExplosionRadios.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto ExplosionRadiosImage = sf::Sprite(ExplosionRadios);
    ExplosionRadiosImage.setOrigin(225,225);
    // ----------------------------------------------------------------------------------------------------

    // -------------------------Map Management ---------------------------------

    //  --- Map Consists of 84 fields (12 horizontally x 7 vertically) (150x150 px each)   On default every single one is Standard Ground.
    auto MapFieldsByType = std::vector<Field>(84,Field::Standard);

    // Randomizer engine
    auto rd = std::random_device();
    auto gen = std::mt19937(rd());
    auto dis = std::uniform_int_distribution<>();

    auto MapFieldsImages = std::vector<sf::Sprite>(84);

    auto EnumedImageMap = std::map<Field,sf::Sprite>{
            {Field::Standard, StandardGround_GameFieldImage},
            {Field::MineField, StandardGround_GameFieldImage},
            {Field::Swamps, Swamps_GameFieldImage},
            {Field::Sands, Sands_GameFieldImage},
            {Field::Home, HomeObstacle_GameFieldImage},
            {Field::Spikes, TankSpikesObstacle_GameFieldImage},
            {Field::Hole, BigHole_InstantDeath_GameFieldImage},
            {Field::Forest, Forest_GameFieldImage},
            {Field::PetrolFull, Petrol_Truck_Full_GameFieldImage},
            {Field::PetrolEmpty, Petrol_Truck_Empty_GameFieldImage},
            {Field::Explosives, Explosives_GameFieldImage},
            {Field::Burned, BurnedGround_GameFieldImage},
            {Field::WorkshopFull, Workshop_Full_GameFieldImage},
            {Field::WorkshopEmpty, Workshop_Empty_GameFieldImage},
            {Field::AmmoDropFull, AmmoDrop_Full_GameFieldImage},
            {Field::AmmoDropEmpty, AmmoDrop_Empty_GameFieldImage},
            {Field::BigMine, BigMine_InstantDeath_GameFieldImage},
            {Field::BlowedMines, BlowedMines_GameFieldImage}
    };

    auto PlayerMovementUpBlocked = false;
    auto PlayerMovementDownBlocked = false;
    auto PlayerMovementRightBlocked = false;
    auto PlayerMovementLeftBlocked = false;

    // --- Boolean saying whether explosion radios is visible
    auto ExplosionRadiosVisible = false;

    // ----------------------------------------------------------------------------------------------
    //  ------------------- Main Menu Screen -------------------------------------

    //    --- Title ---
    auto GameTitle = sf::Text("Super Tanker 2D", ArialFont, 45);
    GameTitle.setPosition({750, 40});
    GameTitle.setFillColor(sf::Color::Black);
    GameTitle.setStyle(sf::Text::Bold);

    // --- Game Version ---
    auto GameVersion = sf::Text("v1.0\nOfficial", ArialFont, 15);
    GameVersion.setPosition(1845,3);
    GameVersion.setFillColor(sf::Color::Black);

    // --- Background picture ---
    auto MainMenuBackground = sf::Texture();
    if(!MainMenuBackground.loadFromFile("../C++GameData/textures/MainMenuBackground.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto MainMenuBackgroundImage = sf::Sprite(MainMenuBackground);

    // --- Play Button ---
    auto MenuPlayButton = sf::RectangleShape(sf::Vector2f(300, 75));
    MenuPlayButton.setPosition({300, 300});
    MenuPlayButton.setFillColor(sf::Color(255, 255, 0, 80));
    MenuPlayButton.setOutlineColor(sf::Color::Red);
    MenuPlayButton.setOutlineThickness(2);

    // --- Text for Play Button ---
    auto MenuPlayButtonText = sf::Text(sf::String("Play"), ArialFont, 40);
    MenuPlayButtonText.setPosition({405, 310});
    MenuPlayButtonText.setFillColor(sf::Color::Black);
    MenuPlayButtonText.setStyle(sf::Text::Bold);

    // --- Options Button ---
    auto MenuOptionsButton = MenuPlayButton;
    MenuOptionsButton.setPosition({300, 495});

    // --- Text for Options Button ---
    auto MenuOptionsButtonText = sf::Text(sf::String("Options"), ArialFont, 40);
    MenuOptionsButtonText.setPosition({380, 507});
    MenuOptionsButtonText.setFillColor(sf::Color::Black);
    MenuOptionsButtonText.setStyle(sf::Text::Bold);

    // --- Quit Button ---
    auto MenuQuitButton = MenuPlayButton;
    MenuQuitButton.setPosition({300, 700});

    // --- Text for Quit Button ---
    auto MenuQuitButtonText = sf::Text(sf::String("Quit"), ArialFont, 40);
    MenuQuitButtonText.setPosition({405, 712});
    MenuQuitButtonText.setFillColor(sf::Color::Black);
    MenuQuitButtonText.setStyle(sf::Text::Bold);

    // --- Function for Screen Appearance ---
    auto DrawMainMenuScreen = [&]() -> void {
        window.clear();
        window.draw(MainMenuBackgroundImage);
        window.draw(GameTitle);
        window.draw(GameVersion);
        window.draw(MenuPlayButton);
        window.draw(MenuPlayButtonText);
        window.draw(MenuQuitButton);
        window.draw(MenuQuitButtonText);
        window.draw(MenuOptionsButton);
        window.draw(MenuOptionsButtonText);
    };

    //  --- Boolean saying whether Screen is Appearing ---
    auto MainMenuScreenEnabled = true;
    // ------------------------------------------------------------------------
   // ---------------------Options Screen --------------------

    //  --- Boolean saying whether Screen is Appearing ---
    auto OptionsScreenEnabled = false;

    // --- Text for FPS Cap Prompt ---
    auto FPSCapText = sf::Text(sf::String("FPS Limit"), ArialFont, 40);
    FPSCapText.setPosition({840, 40});
    FPSCapText.setFillColor(sf::Color::Black);
    FPSCapText.setStyle(sf::Text::Bold);

    // --- 30FPS Button ---
    auto FPS30Button = sf::RectangleShape(sf::Vector2f(200, 75));
    FPS30Button.setPosition({460, 200});
    FPS30Button.setFillColor(sf::Color(255, 255, 0, 80));
    FPS30Button.setOutlineColor(sf::Color::Red);
    FPS30Button.setOutlineThickness(2);

    // --- Text for 30 FPS Option ---
    auto FPS30CapText = sf::Text(sf::String("30 FPS"), ArialFont, 40);
    FPS30CapText.setPosition({490, 210});
    FPS30CapText.setFillColor(sf::Color::Black);
    FPS30CapText.setStyle(sf::Text::Bold);

    // --- 60FPS Button ---
    auto FPS60Button = FPS30Button;
    FPS60Button.setPosition(760,200);

    // --- Text for 60 FPS Option ---
    auto FPS60CapText = sf::Text(sf::String("60 FPS"), ArialFont, 40);
    FPS60CapText.setPosition({790, 210});
    FPS60CapText.setFillColor(sf::Color::Black);
    FPS60CapText.setStyle(sf::Text::Bold);

    // --- 120FPS Button ---
    auto FPS120Button = FPS30Button;
    FPS120Button.setPosition(1060,200);

    // --- Text for 120 FPS Option ---
    auto FPS120CapText = sf::Text(sf::String("120 FPS"), ArialFont, 40);
    FPS120CapText.setPosition({1080, 210});
    FPS120CapText.setFillColor(sf::Color::Black);
    FPS120CapText.setStyle(sf::Text::Bold);

    // --- Universal Arrow for Button 'Go back'
    auto GoBackArrow = sf::Texture();
    if(!GoBackArrow.loadFromFile("../C++GameData/textures/GoBackArrow.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto GoBackArrowImage = sf::Sprite(GoBackArrow);
    GoBackArrowImage.setPosition(115,955);

    // --- Button for 'Go Back' ---
    auto BackFromOptionsButton = sf::RectangleShape(sf::Vector2f(200, 80));
    BackFromOptionsButton.setPosition({75, 950});
    BackFromOptionsButton.setFillColor(sf::Color(255, 255, 0, 100));
    BackFromOptionsButton.setOutlineColor(sf::Color::Red);
    BackFromOptionsButton.setOutlineThickness(1);

    // --- Function for drawing Current FPS Limit indicator
    auto CurrentFPSLimitText = sf::Text(sf::String("Currently Chosen"), ArialFont, 20);
    CurrentFPSLimitText.setFillColor(sf::Color::Black);

    auto DrawCurrentFPSLimit = [&]() -> void{
        switch (FPSCap) {
            case 30:
                CurrentFPSLimitText.setPosition(475,275);
                break;
            case 60:
                CurrentFPSLimitText.setPosition(775,275);
                break;
            default:
                CurrentFPSLimitText.setPosition(1075,275);
        }
        window.draw(CurrentFPSLimitText);
    };

    // --- Function for Screen Appearance ---
    auto DrawOptionsScreen = [&]() -> void {
        window.clear();
        window.draw(MainMenuBackgroundImage);
        window.draw(GameVersion);
        window.draw(FPSCapText);
        window.draw(FPS30Button);
        window.draw(FPS60Button);
        window.draw(FPS120Button);
        window.draw(FPS30CapText);
        window.draw(FPS60CapText);
        window.draw(FPS120CapText);
        window.draw(BackFromOptionsButton);
        window.draw(GoBackArrowImage);
        DrawCurrentFPSLimit();
    };
    // --------------------  Quiting menu screen --------------------------------------------------

    // --- Quiting Window ---
    auto QuitingScreen = sf::RectangleShape(sf::Vector2f(900, 600));
    QuitingScreen.setPosition({500, 300});
    QuitingScreen.setFillColor(sf::Color(255, 255, 0, 128));
    QuitingScreen.setOutlineColor(sf::Color::Red);
    QuitingScreen.setOutlineThickness(1);

    //  --- Boolean saying whether Screen is Appearing ---
    auto QuitingScreenEnabled = false;

    // --- Button for 'Yes' Option ---
    auto QuitingYesButton = sf::RectangleShape(sf::Vector2f(180, 80));
    QuitingYesButton.setPosition({630, 770});
    QuitingYesButton.setFillColor(sf::Color(255, 0, 0, 200));
    QuitingYesButton.setOutlineColor(sf::Color::Red);
    QuitingYesButton.setOutlineThickness(1);

    // --- Text for 'Yes' Button ---
    auto QuitingYesButtonText = sf::Text(sf::String("Yes"), ArialFont, 40);
    QuitingYesButtonText.setPosition({685, 785});
    QuitingYesButtonText.setFillColor(sf::Color::Black);

    // --- Button for 'No' Option ---
    auto QuitingNoButton = sf::RectangleShape(sf::Vector2f(180, 80));
    QuitingNoButton.setPosition({1050, 770});
    QuitingNoButton.setFillColor(sf::Color(0, 255, 0, 200));
    QuitingNoButton.setOutlineColor(sf::Color::Green);
    QuitingNoButton.setOutlineThickness(1);

    // --- Text for 'No' Button ---
    auto QuitingNoButtonText = sf::Text(sf::String("No"), ArialFont, 40);
    QuitingNoButtonText.setPosition({1105, 785});
    QuitingNoButtonText.setFillColor(sf::Color::Black);

    // --- Prompt for reassurance ---
    auto QuitingAskingText = sf::Text(sf::String("Are you sure you want to Quit?"), ArialFont, 40);
    QuitingAskingText.setPosition({670, 370});
    QuitingAskingText.setFillColor(sf::Color::Black);

    // --- Function for Quiting screen appearance ---
    auto DrawQuitingScreen = [&]() -> void {
        window.clear();
        window.draw(MainMenuBackgroundImage);
        window.draw(GameTitle);
        window.draw(GameVersion);
        window.draw(QuitingScreen);
        window.draw(QuitingYesButton);
        window.draw(QuitingYesButtonText);
        window.draw(QuitingNoButton);
        window.draw(QuitingNoButtonText);
        window.draw(QuitingAskingText);
    };
    // ----------------------------------------------------------------------------------------------
    // ----------------Play Option Selector Screen ---------------------------------------------------

    // --- Background picture ---
    auto PlayOptionSelectorBackground = sf::Texture();
    if(!PlayOptionSelectorBackground.loadFromFile("../C++GameData/textures/PlayOptionSelectorBackground.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto PlayOptionSelectorBackgroundImage = sf::Sprite(PlayOptionSelectorBackground);

    //  --- Boolean saying whether Screen is Appearing ---
    auto PlayOptionSelectorEnabled = false;

    // --- Button for 'New Game' Option ---
    auto NewGameOptionButton = sf::RectangleShape(sf::Vector2f(400, 100));
    NewGameOptionButton.setPosition({350, 300});
    NewGameOptionButton.setFillColor(sf::Color(255, 255, 0, 128));
    NewGameOptionButton.setOutlineColor(sf::Color::Red);
    NewGameOptionButton.setOutlineThickness(1);


    // --- Button for 'Load Game' Option ---
    auto LoadGameOptionButton = sf::RectangleShape(sf::Vector2f(400, 100));
    LoadGameOptionButton.setPosition({350, 450});
    LoadGameOptionButton.setFillColor(sf::Color(255, 255, 0, 128));
    LoadGameOptionButton.setOutlineColor(sf::Color::Red);
    LoadGameOptionButton.setOutlineThickness(1);

    // --- Button for 'Tutorial' Option ---
    auto TutorialOptionButton = sf::RectangleShape(sf::Vector2f(400, 100));
    TutorialOptionButton.setPosition({350, 600});
    TutorialOptionButton.setFillColor(sf::Color(255, 255, 0, 128));
    TutorialOptionButton.setOutlineColor(sf::Color::Red);
    TutorialOptionButton.setOutlineThickness(1);

    // --- Text For 'New Game' Option ---
    auto NewGameOptionText = sf::Text(sf::String("Start New Game"), ArialFont, 44);
    NewGameOptionText.setPosition({380, 320});
    NewGameOptionText.setFillColor(sf::Color::Black);

    // --- Text For 'Load Game' Option ---
    auto LoadGameOptionText = sf::Text(sf::String("Load Game"), ArialFont, 44);
    LoadGameOptionText.setPosition({430, 470});
    LoadGameOptionText.setFillColor(sf::Color::Black);

    // --- Text For 'Tutorial' Option ---
    auto TutorialOptionText = sf::Text(sf::String("Tutorial"), ArialFont, 44);
    TutorialOptionText.setPosition({450, 620});
    TutorialOptionText.setFillColor(sf::Color::Black);

    auto Back2MainMenuButton = BackFromOptionsButton;

    //  --- Function for Screen Appearance ---
    auto DrawPlayOptionSelector = [&]()->void {
        window.clear();
        window.draw(PlayOptionSelectorBackgroundImage);
        window.draw(NewGameOptionButton);
        window.draw(LoadGameOptionButton);
        window.draw(TutorialOptionButton);
        window.draw(NewGameOptionText);
        window.draw(LoadGameOptionText);
        window.draw(TutorialOptionText);
        window.draw(Back2MainMenuButton);
        window.draw(GoBackArrowImage);
    };
    // --------------------------------------------------------------------------------
    // -----------------Tutorial----------------------

    // Flag for tutorial being enabled
    auto TutorialEnabled = false;
    // Current tutorial Screen
    auto TutorialScreen = 1;

    // --- Tutorial 1 screen ---
    auto Tutorial1Background = sf::Texture();
    if(!Tutorial1Background.loadFromFile("../C++GameData/textures/Tutorial1.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Tutorial1BackgroundImage = sf::Sprite(Tutorial1Background);

    // --- Tutorial 2 screen ---
    auto Tutorial2Background = sf::Texture();
    if(!Tutorial2Background.loadFromFile("../C++GameData/textures/Tutorial2.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Tutorial2BackgroundImage = sf::Sprite(Tutorial2Background);

    // --- Tutorial 3 screen ---
    auto Tutorial3Background = sf::Texture();
    if(!Tutorial3Background.loadFromFile("../C++GameData/textures/Tutorial3.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Tutorial3BackgroundImage = sf::Sprite(Tutorial3Background);

    // --- Tutorial 4 screen ---
    auto Tutorial4Background = sf::Texture();
    if(!Tutorial4Background.loadFromFile("../C++GameData/textures/Tutorial4.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Tutorial4BackgroundImage = sf::Sprite(Tutorial4Background);

    // --- Tutorial 5 screen ---
    auto Tutorial5Background = sf::Texture();
    if(!Tutorial5Background.loadFromFile("../C++GameData/textures/Tutorial5.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto Tutorial5BackgroundImage = sf::Sprite(Tutorial5Background);

    // --- Button to switch to next tutorial screen
    auto NextTutorialButton = Back2MainMenuButton;
    NextTutorialButton.setPosition(950,990);
    // --- Arrow image on this button
    auto ClassicArrow = GoBackArrowImage;
    ClassicArrow.setPosition(705,993);

     // --- Button to switch to previous tutorial screen
    auto PreviousTutorialButton = Back2MainMenuButton;
    PreviousTutorialButton.setPosition(660,990);
    // --- Arrow image on this button
    auto ReverseArrow = GoBackArrowImage;
    ReverseArrow.setScale(-1.f, 1.f);
    ReverseArrow.setPosition(1100,993);

    // --- Function to draw tutorial
    auto DrawTutorial = [&]() -> void {
        switch (TutorialScreen){
            case 1:
                window.draw(Tutorial1BackgroundImage);
                break;
            case 2:
                window.draw(Tutorial2BackgroundImage);
                break;
            case 3:
                window.draw(Tutorial3BackgroundImage);
                break;
            case 4:
                window.draw(Tutorial4BackgroundImage);
                break;
            default:
                window.draw(Tutorial5BackgroundImage);
        }
        window.draw(NextTutorialButton);
        window.draw(PreviousTutorialButton);
        window.draw(ClassicArrow);
        window.draw(ReverseArrow);
    };
    // ---------- Main Fight Preparation Screen --------------------------------------

    // ---  Background image ---
    auto PreFightMenu = sf::Texture();
    if(!PreFightMenu.loadFromFile("../C++GameData/textures/PreFightMenu.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto PreFightMenuImage = sf::Sprite(PreFightMenu);

    //  --- Boolean saying whether Screen is Appearing ---
    auto PreFightMenuEnabled = false;

    //   --- Background for Current Battlefield level ---
    auto PreFightBattlefieldLevelBackground = sf::RectangleShape(sf::Vector2f(600, 400));
    PreFightBattlefieldLevelBackground.setPosition({75, 75});
    PreFightBattlefieldLevelBackground.setFillColor(sf::Color(255, 255, 0, 80));
    PreFightBattlefieldLevelBackground.setOutlineColor(sf::Color::Red);
    PreFightBattlefieldLevelBackground.setOutlineThickness(1);

    // --- Button for 'Go Back' ---
    auto Back2PlayOptionSelectorButton = Back2MainMenuButton;

    // --- Button for 'Fight' option ---
    auto PreFightMenuFightButton = sf::RectangleShape(sf::Vector2f(400, 125));
    PreFightMenuFightButton.setPosition({1000, 125});
    PreFightMenuFightButton.setFillColor(sf::Color(255, 255, 0, 128));
    PreFightMenuFightButton.setOutlineColor(sf::Color::Red);
    PreFightMenuFightButton.setOutlineThickness(1);

    // --- Text for 'Fight' Option
    auto PreFightMenuFightButtonText = sf::Text(sf::String("FIGHT"),ArialFont,100);
    PreFightMenuFightButtonText.setPosition(1050,125);
    PreFightMenuFightButtonText.setFillColor(sf::Color::Black);

    // --- Button for 'Upgrade' option ---
    auto PreFightMenuUpgradeButton = PreFightMenuFightButton;
    PreFightMenuUpgradeButton.setPosition(1000, 325);

    // --- Text for 'Upgrade' Option
    auto PreFightMenuUpgradeButtonText = sf::Text(sf::String("   UPGRADE\n       TANK"),ArialFont,52);
    PreFightMenuUpgradeButtonText.setPosition(1030,325);
    PreFightMenuUpgradeButtonText.setFillColor(sf::Color::Black);

    // --- Button for 'Equipment' option ---
    auto PreFightMenuEquipmentButton = PreFightMenuFightButton;
    PreFightMenuEquipmentButton.setPosition(1000, 525);

    // --- Text for 'Equipment' Option
    auto PreFightMenuEquipmentButtonText = sf::Text(sf::String("EQUIPMENT"),ArialFont,60);
    PreFightMenuEquipmentButtonText.setPosition(1020,550);
    PreFightMenuEquipmentButtonText.setFillColor(sf::Color::Black);

    // --- Button for 'Save Game Progress' option ---
    auto PreFightSaveGameButton = sf::RectangleShape(sf::Vector2f(225, 105));
    PreFightSaveGameButton.setPosition({400, 910});
    PreFightSaveGameButton.setFillColor(sf::Color(255, 255, 0, 128));
    PreFightSaveGameButton.setOutlineColor(sf::Color::Red);
    PreFightSaveGameButton.setOutlineThickness(1);

    // --- Text for 'Save Game Progress' Button ---
    auto PreFightSaveGameButtonText = sf::Text(sf::String("Save Game\n  Progress"), ArialFont, 40);
    PreFightSaveGameButtonText.setPosition({402, 910});
    PreFightSaveGameButtonText.setFillColor(sf::Color::Black);

    // --- Text Before Current Battlefield level---
    auto BattlefieldLevelInfo = sf::Text(sf::String("Battlefield\nLevel:"), ArialFont, 70);
    BattlefieldLevelInfo.setPosition({100, 100});
    BattlefieldLevelInfo.setFillColor(sf::Color::Black);

    // --- Text For Current Battlefield level---
    auto BattlefieldLevelText = sf::Text(std::to_string(CurrentBattlefieldLevel), ArialFont, 200);
    BattlefieldLevelText.setPosition({400, 200});
    BattlefieldLevelText.setFillColor(sf::Color::Black);

    // --- Function to update counters ---
    auto UpdateCounters = [&]() -> void {
        CurrentCreditsCountText = sf::Text(std::to_string(PlayerCurrentCreditsCount),ArialFont,30);
        CurrentCreditsCountText.setFillColor(sf::Color::White);
        CurrentCreditsCountText.setPosition(1042,22);

        CurrentXPCountText = sf::Text(std::to_string(PlayerCurrentXPCount),ArialFont,30);
        CurrentXPCountText.setFillColor(sf::Color::White);
        CurrentXPCountText.setPosition(1292,22);
    };

    //  --- Function for Screen Appearance ---
    auto DrawPreFightMenu = [&]()->void {
        window.clear();
        window.draw(PreFightMenuImage);
        window.draw(PreFightBattlefieldLevelBackground);
        window.draw(BattlefieldLevelInfo);
        window.draw(BattlefieldLevelText);
        window.draw(Back2PlayOptionSelectorButton);
        window.draw(GoBackArrowImage);
        window.draw(PreFightMenuFightButton);
        window.draw(PreFightMenuUpgradeButton);
        window.draw(PreFightMenuEquipmentButton);
        window.draw(PreFightMenuFightButtonText);
        window.draw(PreFightMenuUpgradeButtonText);
        window.draw(PreFightMenuEquipmentButtonText);
        window.draw(PreFightSaveGameButton);
        window.draw(PreFightSaveGameButtonText);
        window.draw(CurrencyBackground);
        UpdateCounters();
        window.draw(XPCountIconImage);
        window.draw(CurrentXPCountText);
        window.draw(CreditsCountIconImage);
        window.draw(CurrentCreditsCountText);
    };
    //-------------------------------------------------------------------------------------------
    // ---- UPGRADE OPTION SELECTOR SCREEN ---

    // --- Flag for screen being enabled
    auto UpgradeOptionsScreenEnabled = false;

    // --- Background screen--
    auto UpgradeOptionsScreenBackground = sf::Texture();
    if(!UpgradeOptionsScreenBackground.loadFromFile("../C++GameData/textures/UpgradeOptionsScreen.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto UpgradeOptionsScreenBackgroundImage = sf::Sprite(UpgradeOptionsScreenBackground);

    // --- Button for "Tech Tree" ---
    auto UpgradeTechTreeButton = sf::RectangleShape(sf::Vector2f(600, 150));
    UpgradeTechTreeButton.setPosition({650, 225});
    UpgradeTechTreeButton.setFillColor(sf::Color(255, 255, 0, 90));
    UpgradeTechTreeButton.setOutlineColor(sf::Color::Red);
    UpgradeTechTreeButton.setOutlineThickness(1);

    // --- Text for "Tech Tree" Option ---
    auto UpgradeTechTreeText = sf::Text(sf::String("TECH TREE"),ArialFont,100);
    UpgradeTechTreeText.setPosition(663, 235);
    UpgradeTechTreeText.setFillColor(sf::Color::Black);

    // --- Button for "Modules" button ---
    auto UpgradeModulesButton = UpgradeTechTreeButton;
    UpgradeModulesButton.setPosition({650, 425});

    // --- Text for "Tech Tree" Option ---
    auto UpgradeModulesText = sf::Text(sf::String("MODULES"),ArialFont,100);
    UpgradeModulesText.setPosition(698, 435);
    UpgradeModulesText.setFillColor(sf::Color::Black);

    // --- Button for 'Go Back' ---
    auto Back2PreFightMenuButton = Back2MainMenuButton;

    // --- Function for screen appearance ---
    auto DrawUpgradeOptionsScreen = [&]() -> void {
        window.clear();
        window.draw(UpgradeOptionsScreenBackgroundImage);
        window.draw(UpgradeTechTreeButton);
        window.draw(UpgradeTechTreeText);
        window.draw(UpgradeModulesButton);
        window.draw(UpgradeModulesText);
        window.draw(Back2PreFightMenuButton);
        window.draw(GoBackArrowImage);
    };
    // ---------------------------------------------------------------
    // --- TECH TREE UPGRADE SCREEN ---

    // --- Boolean for screen appearance ---
    auto TechTreeUpgradeScreenEnabled = false;

    // --- Background screen--
    auto TechTreeScreen = sf::Texture();
    if(!TechTreeScreen.loadFromFile("../C++GameData/textures/TechTreeScreen.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto TechTreeScreenImage = sf::Sprite(TechTreeScreen);

    // --- Button for 'Go Back' ---
    auto Back2UpgradeOptionsButton = Back2MainMenuButton;
    Back2UpgradeOptionsButton.setPosition(465,980);
    // --- Arrow image on this button
    auto UpgradeGoBackArrowImage = GoBackArrowImage;
    UpgradeGoBackArrowImage.setPosition(510,985);

    // -- Current Tank Indicator --
    auto CurrentTankIndicator = sf::Texture();
    if(!CurrentTankIndicator.loadFromFile("../C++GameData/textures/CurrentTankIndicator.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto CurrentTankIndicatorImage = sf::Sprite(CurrentTankIndicator);

    // --- Function to set current Tank Indicator Position ---
    auto TankIndicatorPositions = std::vector<sf::Vector2f>{ {24,452}, {275,451}, {535,452}, {796,452}, {1061,453}, {1322,452}, {1594,449} };
    auto SetCurrentTankIndicatorPosition = [&]() -> void {
        CurrentTankIndicatorImage.setPosition(TankIndicatorPositions[PlayerCurrentTier-1]);
    };

    // --- Function to draw current player tank stats ---
    auto PlayerTanksImages = std::vector<sf::Sprite>{ PzIITankIconImage, PzIIITankIconImage, PzIVTankIconImage, PantherTankIconImage, Tiger1TankIconImage, Tiger2TankIconImage, MausTankIconImage };
    auto PlayerTanks2Choose = std::vector<std::string>{"Pz. II", "Pz. III", "Pz. IV", "Panther", "Tiger 1", "Tiger 2", "Maus"};
    auto DrawPlayerTankStats = [&]() -> void {
        // --- Text for "Tank" value ---
        auto PlayerStatsTank = sf::Text(sf::String(PlayerTanks2Choose[PlayerCurrentTier-1]),ArialFont,22);
        PlayerStatsTank.setPosition(274, 849);
        PlayerStatsTank.setFillColor(sf::Color::Black);
        PlayerStatsTank.setStyle(sf::Text::Bold);

        // --- Text for "HP" value ---
        auto PlayerStatsHP= sf::Text(std::to_string(static_cast<int>(PlayerMaxHP)),ArialFont,22);
        PlayerStatsHP.setPosition(276, 883);
        PlayerStatsHP.setFillColor(sf::Color::Black);
        PlayerStatsHP.setStyle(sf::Text::Bold);

        // --- Text for "AP Damage" value ---
        auto PlayerStatsAPDMG= sf::Text(std::to_string(static_cast<int>(APShellDamage)),ArialFont,22);
        PlayerStatsAPDMG.setPosition(277, 917);
        PlayerStatsAPDMG.setFillColor(sf::Color::Black);
        PlayerStatsAPDMG.setStyle(sf::Text::Bold);

        // --- Text for "APCR Damage" value ---
        auto PlayerStatsAPCRDMG= sf::Text(std::to_string(static_cast<int>(APCRShellDamage)),ArialFont,22);
        PlayerStatsAPCRDMG.setPosition(277, 951);
        PlayerStatsAPCRDMG.setFillColor(sf::Color::Black);
        PlayerStatsAPCRDMG.setStyle(sf::Text::Bold);

        // --- Text for "HE Damage" value ---
        auto PlayerStatsHEDMG= sf::Text(std::to_string(static_cast<int>(HEShellDamage)),ArialFont,22);
        PlayerStatsHEDMG.setPosition(277, 985);
        PlayerStatsHEDMG.setFillColor(sf::Color::Black);
        PlayerStatsHEDMG.setStyle(sf::Text::Bold);

        // --- Text for "Speed" value ---
        auto PlayerStatsSpeed=sf::Text(std::to_string(static_cast<int>(PlayerMovementSpeed.x*40*(1/FPSspeedMultiplier)))+" km/h",ArialFont,22);
        PlayerStatsSpeed.setPosition(277, 1019);
        PlayerStatsSpeed.setFillColor(sf::Color::Black);
        PlayerStatsSpeed.setStyle(sf::Text::Bold);

        // --- Text for "Fuel Capacity" value ---
        auto PlayerStatsFuelCap= sf::Text(std::to_string(static_cast<int>(PlayerMaxFuelLevel))+" L",ArialFont,22);
        PlayerStatsFuelCap.setPosition(277, 1053);
        PlayerStatsFuelCap.setFillColor(sf::Color::Black);
        PlayerStatsFuelCap.setStyle(sf::Text::Bold);

        window.draw(PlayerStatsTank);
        window.draw(PlayerStatsHP);
        window.draw(PlayerStatsAPDMG);
        window.draw(PlayerStatsAPCRDMG);
        window.draw(PlayerStatsHEDMG);
        window.draw(PlayerStatsSpeed);
        window.draw(PlayerStatsFuelCap);
    };
    // --- Total XP Gains per tier ---
    auto TotalXPGainsPerTier = std::vector<int>{1575,3150,6300,10500,15750,22050,29400};

    auto XPRequiredIcon = XPCountIconImage;
    XPRequiredIcon.setPosition(1515, 948);

    // --- Upgrade to next tank button --
    auto Upgrade2NextTankButton = sf::RectangleShape(sf::Vector2f(400, 175));
    Upgrade2NextTankButton.setPosition({1445, 845});
    Upgrade2NextTankButton.setFillColor(sf::Color(255, 255, 0, 80));
    Upgrade2NextTankButton.setOutlineColor(sf::Color::Red);
    Upgrade2NextTankButton.setOutlineThickness(1);

    // --- Upgrade to next tank text --
    auto Upgrade2NextTankText=sf::Text(sf::String("UPGRADE TO\n     NEXT TANK"),ArialFont,40);
    Upgrade2NextTankText.setPosition(1475, 848);
    Upgrade2NextTankText.setFillColor(sf::Color::Black);
    Upgrade2NextTankText.setStyle(sf::Text::Bold);

    // --- XP Required text drawing function--
    auto XPRequired4NextTank = static_cast<int>(TotalXPGainsPerTier[PlayerCurrentTier-1]*0.5);
    auto DrawXPRequiredText = [&]() -> void {
        XPRequired4NextTank = static_cast<int>(TotalXPGainsPerTier[PlayerCurrentTier-1]*0.5);
        auto XPRequired4NextTankText=sf::Text(std::to_string(XPRequired4NextTank),ArialFont,40);
        XPRequired4NextTankText.setPosition(1553, 941);
        if(PlayerCurrentXPCount<XPRequired4NextTank) XPRequired4NextTankText.setFillColor(sf::Color::Red);
        else XPRequired4NextTankText.setFillColor(sf::Color::Black);
        window.draw(XPRequired4NextTankText);
    };

    // --- Function to check if modules are maxed out ---
    auto ModulesMaximized = false;

    // --- Modules not maximized warning ---
    auto ModulesNotMaxedWarning=sf::Text(sf::String("To upgrade to next tank you must \n max out your modules first!"),ArialFont,40);
    ModulesNotMaxedWarning.setPosition(1280, 738);
    ModulesNotMaxedWarning.setFillColor(sf::Color::Red);

    // --- Check if player has upgraded all modules
    auto Check4Modules = [&]() -> void {
        if(PlayerCurrentEngineLevel==3&&PlayerCurrentGunLevel==3&&PlayerCurrentTurretLevel==3) ModulesMaximized = true;
        else ModulesMaximized = false;
        if(!ModulesMaximized){
            window.draw(ModulesNotMaxedWarning);
        }
    };

    // --- Function to update player stats ---
    auto UpgradePlayerStats = [&]() -> void{
        // --- HP ---
        PlayerDefaultHP=static_cast<float>(75*PlayerCurrentTier);
        switch (PlayerCurrentTurretLevel){
            case 1:
                PlayerDefaultHP*=0.7;
                break;
            case 2:
                PlayerDefaultHP*=0.85;
                break;
            default:
                break;
        }
        PlayerCurrentHP=PlayerDefaultHP;
        PlayerMaxHP=PlayerDefaultHP;
        // --- Speed & Fuel ---
        DefaultPlayerMovementSpeed=sf::Vector2f(1*FPSspeedMultiplier,1*FPSspeedMultiplier);
        PlayerDefaultFuelLevel=100;
        switch (PlayerCurrentEngineLevel){
            case 1:
                DefaultPlayerMovementSpeed=sf::Vector2f(static_cast<float>(0.7)*FPSspeedMultiplier,static_cast<float>(0.7)*FPSspeedMultiplier);
                PlayerDefaultFuelLevel=80;
                break;
            case 2:
                DefaultPlayerMovementSpeed=sf::Vector2f(static_cast<float>(0.85)*FPSspeedMultiplier,static_cast<float>(0.85)*FPSspeedMultiplier);
                PlayerDefaultFuelLevel=90;
                break;
            default:
                break;
        }
        OriginalPlayerMovementSpeed=DefaultPlayerMovementSpeed/(1*FPSspeedMultiplier);
        PlayerMovementSpeed=DefaultPlayerMovementSpeed;
        PlayerMaxFuelLevel=PlayerDefaultFuelLevel;
        PlayerCurrentFuelLevel=PlayerMaxFuelLevel;
        // --- Shells Damage ---
        switch (PlayerCurrentGunLevel){
            case 1:
                APShellDefaultDamage=static_cast<float>(22*PlayerCurrentTier);
                APCRShellDefaultDamage=static_cast<float>(13*PlayerCurrentTier);
                HEShellDefaultDamage=static_cast<float>(30*PlayerCurrentTier);
                break;
            case 2:
                APShellDefaultDamage=static_cast<float>(26*PlayerCurrentTier);
                APCRShellDefaultDamage=static_cast<float>(17*PlayerCurrentTier);
                HEShellDefaultDamage=static_cast<float>(35*PlayerCurrentTier);
                break;
            default:
                APShellDefaultDamage=static_cast<float>(30*PlayerCurrentTier);
                APCRShellDefaultDamage=static_cast<float>(20*PlayerCurrentTier);
                HEShellDefaultDamage=static_cast<float>(40*PlayerCurrentTier);
        }
        APShellDamage=APShellDefaultDamage;
        APCRShellDamage=APCRShellDefaultDamage;
        HEShellDamage=HEShellDefaultDamage;
        // --- Tank Image ---
        PlayerTankImage=PlayerTanksImages[PlayerCurrentTier-1];
    };

    // --- Function for screen appearance ---
    auto DrawTechTreeScreen = [&]() -> void {
        window.clear();
        window.draw(TechTreeScreenImage);
        window.draw(Back2UpgradeOptionsButton);
        window.draw(UpgradeGoBackArrowImage);
        window.draw(CurrencyBackground);
        UpdateCounters();
        window.draw(XPCountIconImage);
        window.draw(CurrentXPCountText);
        window.draw(CreditsCountIconImage);
        window.draw(CurrentCreditsCountText);
        SetCurrentTankIndicatorPosition();
        window.draw(CurrentTankIndicatorImage);
        DrawPlayerTankStats();
        if(PlayerCurrentTier!=7) {
            window.draw(Upgrade2NextTankButton);
            window.draw(Upgrade2NextTankText);
            window.draw(XPRequiredIcon);
            DrawXPRequiredText();
            Check4Modules();
        }
    };
    // ------------------ Upgrade Modules Screen ------------------

    // --- Flag for screen Appearance---
    auto UpgradeModulesScreenEnabled = false;

    // --- Module Indicator Image ---
    auto CurrentModuleIndicator = sf::Texture();
    if(!CurrentModuleIndicator.loadFromFile("../C++GameData/textures/CurrentModuleIndicator.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto CurrentModuleIndicatorImage = sf::Sprite(CurrentModuleIndicator);

    // --- Background screen--
    auto ModulesScreen = sf::Texture();
    if(!ModulesScreen.loadFromFile("../C++GameData/textures/UpgradeModulesScreen.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto UpgradeModulesScreen = sf::Sprite(ModulesScreen);

    // --- Positions for current Module Indicator
    auto GunIndicatorPositions = std::vector<sf::Vector2f>{  {540,712}, {540,550}, {539,386}};
    auto TurretIndicatorPositions = std::vector<sf::Vector2f>{ {853,712}, {851,549}, {848,384} };
    auto EngineIndicatorPositions = std::vector<sf::Vector2f>{ {1166,714}, {1164,545}, {1163,385} };

    // --- Current Gun Indicator ---
    auto CurrentGunIndicator = CurrentModuleIndicatorImage;
    // --- Current Turret Indicator ---
    auto CurrentTurretIndicator = CurrentModuleIndicatorImage;
    // --- Current Engine Indicator ---
    auto CurrentEngineIndicator = CurrentModuleIndicatorImage;

    // --- Function to set module indicators ---
    auto SetModulesIndicators = [&]() -> void {
        // --- Current Tank Image ---
        auto ModulesScreenCurrentTank = PlayerTankImage;
        ModulesScreenCurrentTank.setPosition(928,970);

        // --- Next Tank Image ---
        auto ModulesScreenNextTank = sf::Sprite();
        if(PlayerCurrentTier!=7) {
            ModulesScreenNextTank = PlayerTanksImages[PlayerCurrentTier];
            ModulesScreenNextTank.setPosition(918,252);
        }
        window.draw(ModulesScreenCurrentTank);
        if(PlayerCurrentTier!=7) window.draw(ModulesScreenNextTank);
        CurrentGunIndicator.setPosition(GunIndicatorPositions[PlayerCurrentGunLevel-1]);
        CurrentTurretIndicator.setPosition(TurretIndicatorPositions[PlayerCurrentTurretLevel-1]);
        CurrentEngineIndicator.setPosition(EngineIndicatorPositions[PlayerCurrentEngineLevel-1]);
        window.draw(CurrentGunIndicator);
        window.draw(CurrentTurretIndicator);
        window.draw(CurrentEngineIndicator);
    };



    // --- Upgrade Gun Button ---
    auto UpgradeGunButton = sf::RectangleShape(sf::Vector2f(300, 150));
    UpgradeGunButton.setPosition({1570, 315});
    UpgradeGunButton.setFillColor(sf::Color(255, 255, 0, 80));
    UpgradeGunButton.setOutlineColor(sf::Color::Red);
    UpgradeGunButton.setOutlineThickness(1);

    // -- Upgrade Gun Text ---
    auto UpgradeGunText = sf::Text(sf::String("UPGRADE\n     GUN"), ArialFont, 40);
    UpgradeGunText.setFillColor(sf::Color::Black);
    UpgradeGunText.setStyle(sf::Text::Bold);
    UpgradeGunText.setPosition(1580, 325);

    // -- XP Required 4 next Gun Icon--
    auto XPRequired4GunIcon = XPCountIconImage;
    XPRequired4GunIcon.setPosition(1580, 420);

    // --- Function to draw XP required for next gun --
    auto XPRequired4Gun = int();
    auto XPRequired4GunText = sf::Text();
    auto DrawXPRequired4Gun = [&]() -> void {
        if(PlayerCurrentTier!=7) {
            switch (PlayerCurrentGunLevel) {
                case 1:
                    XPRequired4Gun = static_cast<int>(0.08*TotalXPGainsPerTier[PlayerCurrentTier-1]);
                    break;
                default:
                    XPRequired4Gun = static_cast<int>(0.13*TotalXPGainsPerTier[PlayerCurrentTier-1]);
            }
        }else {
            switch (PlayerCurrentGunLevel) {
                case 1:
                    XPRequired4Gun = static_cast<int>(0.16*TotalXPGainsPerTier[PlayerCurrentTier-1]);
                    break;
                default:
                    XPRequired4Gun = static_cast<int>(0.26*TotalXPGainsPerTier[PlayerCurrentTier-1]);
            }
        }
        XPRequired4GunText = sf::Text(std::to_string(XPRequired4Gun), ArialFont,40);
        XPRequired4GunText.setPosition(1622, 414);
        if(PlayerCurrentXPCount<XPRequired4Gun) XPRequired4GunText.setFillColor(sf::Color::Red);
        else XPRequired4GunText.setFillColor(sf::Color::Black);
        window.draw(XPRequired4GunText);
    };

    // --- Upgrade Turret Button ---
    auto UpgradeTurretButton = UpgradeGunButton;
    UpgradeTurretButton.setPosition({1570, 515});

    // -- Upgrade Turret Text ---
    auto UpgradeTurretText = sf::Text(sf::String("UPGRADE\n    TURRET"), ArialFont, 40);
    UpgradeTurretText.setFillColor(sf::Color::Black);
    UpgradeTurretText.setStyle(sf::Text::Bold);
    UpgradeTurretText.setPosition(1580, 525);

    // -- XP Required 4 next Turret Icon--
    auto XPRequired4TurretIcon = XPCountIconImage;
    XPRequired4TurretIcon.setPosition(1580, 620);

    // --- Function to draw XP required for next turret --
    auto XPRequired4Turret = int();
    auto XPRequired4TurretText = sf::Text();
    auto DrawXPRequired4Turret = [&]() -> void {
        if(PlayerCurrentTier!=7) {
            switch (PlayerCurrentTurretLevel) {
                case 1:
                    XPRequired4Turret = static_cast<int>(0.07*TotalXPGainsPerTier[PlayerCurrentTier-1]);
                    break;
                default:
                    XPRequired4Turret = static_cast<int>(0.10*TotalXPGainsPerTier[PlayerCurrentTier-1]);
            }
        }else {
            switch (PlayerCurrentTurretLevel) {
                case 1:
                    XPRequired4Turret = static_cast<int>(0.14*TotalXPGainsPerTier[PlayerCurrentTier-1]);
                    break;
                default:
                    XPRequired4Turret = static_cast<int>(0.2*TotalXPGainsPerTier[PlayerCurrentTier-1]);
            }
        }
        XPRequired4TurretText = sf::Text(std::to_string(XPRequired4Turret), ArialFont,40);
        XPRequired4TurretText.setPosition(1622, 614);
        if(PlayerCurrentXPCount<XPRequired4Turret) XPRequired4TurretText.setFillColor(sf::Color::Red);
        else XPRequired4TurretText.setFillColor(sf::Color::Black);
        window.draw(XPRequired4TurretText);
    };

    // --- Upgrade Engine Button ---
    auto UpgradeEngineButton = UpgradeGunButton;
    UpgradeEngineButton.setPosition({1570, 715});

    // -- Upgrade Engine Text ---
    auto UpgradeEngineText = sf::Text(sf::String("UPGRADE\n    ENGINE"), ArialFont, 40);
    UpgradeEngineText.setFillColor(sf::Color::Black);
    UpgradeEngineText.setStyle(sf::Text::Bold);
    UpgradeEngineText.setPosition(1580, 725);

    // -- XP Required 4 next Engine Icon--
    auto XPRequired4EngineIcon = XPCountIconImage;
    XPRequired4EngineIcon.setPosition(1580, 820);

    // --- Function to draw XP required for next turret --
    auto XPRequired4Engine = int();
    auto XPRequired4EngineText = sf::Text();
    auto DrawXPRequired4Engine = [&]() -> void {
        if(PlayerCurrentTier!=7) {
            switch (PlayerCurrentEngineLevel) {
                case 1:
                    XPRequired4Engine = static_cast<int>(0.05*TotalXPGainsPerTier[PlayerCurrentTier-1]);
                    break;
                default:
                    XPRequired4Engine = static_cast<int>(0.07*TotalXPGainsPerTier[PlayerCurrentTier-1]);
            }
        }else {
            switch (PlayerCurrentEngineLevel) {
                case 1:
                    XPRequired4Engine = static_cast<int>(0.10*TotalXPGainsPerTier[PlayerCurrentTier-1]);
                    break;
                default:
                    XPRequired4Engine = static_cast<int>(0.14*TotalXPGainsPerTier[PlayerCurrentTier-1]);
            }
        }
        XPRequired4EngineText = sf::Text(std::to_string(XPRequired4Engine), ArialFont,40);
        XPRequired4EngineText.setPosition(1622, 814);
        if(PlayerCurrentXPCount<XPRequired4Engine) XPRequired4EngineText.setFillColor(sf::Color::Red);
        else XPRequired4EngineText.setFillColor(sf::Color::Black);
        window.draw(XPRequired4EngineText);
    };

    // --- Function for screen appearance ---
    auto DrawModulesUpgradeScreen = [&]() -> void{
        window.clear();
        window.draw(UpgradeModulesScreen);
        window.draw(Back2UpgradeOptionsButton);
        window.draw(UpgradeGoBackArrowImage);
        window.draw(CurrencyBackground);
        UpdateCounters();
        window.draw(XPCountIconImage);
        window.draw(CurrentXPCountText);
        window.draw(CreditsCountIconImage);
        window.draw(CurrentCreditsCountText);
        DrawPlayerTankStats();
        SetModulesIndicators();
        if(PlayerCurrentGunLevel!=3){
            window.draw(UpgradeGunButton);
            window.draw(UpgradeGunText);
            window.draw(XPRequired4GunIcon);
            DrawXPRequired4Gun();
        }
        if(PlayerCurrentTurretLevel!=3){
            window.draw(UpgradeTurretButton);
            window.draw(UpgradeTurretText);
            window.draw(XPRequired4TurretIcon);
            DrawXPRequired4Turret();
        }
        if(PlayerCurrentEngineLevel!=3){
            window.draw(UpgradeEngineButton);
            window.draw(UpgradeEngineText);
            window.draw(XPRequired4EngineIcon);
            DrawXPRequired4Engine();
        }
    };
    //  ---- Equipment Store Screen ----

    // --- Flag for screen appearance
    auto EquipmentStoreScreenEnabled = false;

    // --- Background screen--
    auto EquipmentStoreScreen = sf::Texture();
    if(!EquipmentStoreScreen.loadFromFile("../C++GameData/textures/EquipmentStoreScreen.jpg")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto EquipmentStoreScreenImage = sf::Sprite(EquipmentStoreScreen);

    // Go back button
    auto Back2UpgradeOptions2Button = Back2MainMenuButton;

    // --- Function for drawing equipment quantity ---
    auto APShellsEQCounter = sf::Text();
    auto APCRShellsEQCounter = sf::Text();
    auto HEShellsEQCounter = sf::Text();
    auto RepairKitEQCounter = sf::Text();
    auto ExtraShieldEQCounter = sf::Text();
    auto MedKitEQCounter = sf::Text();
    auto AcceleratorEQCounter = sf::Text();
    auto DrawEQCounters = [&]() -> void {
        // --- AP ---
        APShellsEQCounter = sf::Text(std::to_string(APShellDefaultCount),ArialFont, 40);
        if(APShellDefaultCount<5) APShellsEQCounter.setFillColor(sf::Color::Red);
        else if(APShellDefaultCount<10) APShellsEQCounter.setFillColor(sf::Color::Yellow);
        else APShellsEQCounter.setFillColor(sf::Color::Green);
        APShellsEQCounter.setPosition(140,405);
        window.draw(APShellsEQCounter);
        // --- APCR ---
        APCRShellsEQCounter = sf::Text(std::to_string(APCRShellDefaultCount),ArialFont, 40);
        if(APCRShellDefaultCount<5) APCRShellsEQCounter.setFillColor(sf::Color::Red);
        else if(APCRShellDefaultCount<10) APCRShellsEQCounter.setFillColor(sf::Color::Yellow);
        else APCRShellsEQCounter.setFillColor(sf::Color::Green);
        APCRShellsEQCounter.setPosition(315,405);
        window.draw(APCRShellsEQCounter);
        // --- HE ---
        HEShellsEQCounter = sf::Text(std::to_string(HEShellDefaultCount),ArialFont, 40);
        if(HEShellDefaultCount<5) HEShellsEQCounter.setFillColor(sf::Color::Red);
        else if(HEShellDefaultCount<10) HEShellsEQCounter.setFillColor(sf::Color::Yellow);
        else HEShellsEQCounter.setFillColor(sf::Color::Green);
        HEShellsEQCounter.setPosition(490,405);
        window.draw(HEShellsEQCounter);
        // --- Repair Kit ---
        RepairKitEQCounter = sf::Text(std::to_string(DefaultRepairKitCount),ArialFont, 40);
        if(DefaultRepairKitCount<2) RepairKitEQCounter.setFillColor(sf::Color::Red);
        else if(DefaultRepairKitCount<4) RepairKitEQCounter.setFillColor(sf::Color::Yellow);
        else RepairKitEQCounter.setFillColor(sf::Color::Green);
        RepairKitEQCounter.setPosition(680,405);
        window.draw(RepairKitEQCounter);
        // --- Extra Shield ---
        ExtraShieldEQCounter = sf::Text(std::to_string(DefaultExtraShieldCount),ArialFont, 40);
        if(DefaultExtraShieldCount<2) ExtraShieldEQCounter.setFillColor(sf::Color::Red);
        else if(DefaultExtraShieldCount<4) ExtraShieldEQCounter.setFillColor(sf::Color::Yellow);
        else ExtraShieldEQCounter.setFillColor(sf::Color::Green);
        ExtraShieldEQCounter.setPosition(855,405);
        window.draw(ExtraShieldEQCounter);
        // --- Med Kit ---
        MedKitEQCounter = sf::Text(std::to_string(DefaultMedKitCount),ArialFont, 40);
        if(DefaultMedKitCount<2) MedKitEQCounter.setFillColor(sf::Color::Red);
        else if(DefaultMedKitCount<4) MedKitEQCounter.setFillColor(sf::Color::Yellow);
        else MedKitEQCounter.setFillColor(sf::Color::Green);
        MedKitEQCounter.setPosition(1030,405);
        window.draw(MedKitEQCounter);
        // --- Accelerator ---
        AcceleratorEQCounter = sf::Text(std::to_string(DefaultEngineAcceleratorCount),ArialFont, 40);
        if(DefaultEngineAcceleratorCount<2) AcceleratorEQCounter.setFillColor(sf::Color::Red);
        else if(DefaultEngineAcceleratorCount<4) AcceleratorEQCounter.setFillColor(sf::Color::Yellow);
        else AcceleratorEQCounter.setFillColor(sf::Color::Green);
        AcceleratorEQCounter.setPosition(1205,405);
        window.draw(AcceleratorEQCounter);
    };

    // --- Total credits Gain per tier
    auto TotalCreditsGainPerTier = std::vector<int>{ 900,1800,2700,3600,4500,5400 };

    // --- Function to update Equipment prices ---
    auto APShellsEQPrice = int();
    auto APCRShellsEQPrice = int();
    auto HEShellsEQPrice = int();
    auto RepairKitEQPrice = int();
    auto ExtraShieldEQPrice = int();
    auto MedKitEQPrice = int();
    auto AcceleratorEQPrice = int();
    auto UpdateEQPrices = [&]() -> void {
        APShellsEQPrice = static_cast<int>(TotalCreditsGainPerTier[PlayerCurrentTier-1]*0.003);
        APCRShellsEQPrice = APShellsEQPrice+1*PlayerCurrentTier;
        HEShellsEQPrice = APShellsEQPrice+1*PlayerCurrentTier;
        RepairKitEQPrice = static_cast<int>(TotalCreditsGainPerTier[PlayerCurrentTier-1]*0.03);
        ExtraShieldEQPrice = RepairKitEQPrice+10*PlayerCurrentTier;
        MedKitEQPrice = RepairKitEQPrice+7*PlayerCurrentTier;
        AcceleratorEQPrice = RepairKitEQPrice-2*PlayerCurrentTier;
    };

    // --- Function to draw Equipment prices ---
    auto APShellsEQPriceText = sf::Text();
    auto APCRShellsEQPriceText = sf::Text();
    auto HEShellsEQPriceText = sf::Text();
    auto RepairKitEQPriceText = sf::Text();
    auto ExtraShieldEQPriceText = sf::Text();
    auto MedKitEQPriceText = sf::Text();
    auto AcceleratorEQPriceText = sf::Text();
    auto DrawEQPrices = [&]() -> void {
        // --- AP ---
        APShellsEQPriceText = sf::Text(std::to_string(APShellsEQPrice),ArialFont, 40);
        if(APShellsEQPrice>PlayerCurrentCreditsCount) APShellsEQPriceText.setFillColor(sf::Color::Red);
        else APShellsEQPriceText.setFillColor(sf::Color::White);
        APShellsEQPriceText.setPosition(147,627);
        window.draw(APShellsEQPriceText);
        // --- APCR ---
        APCRShellsEQPriceText = sf::Text(std::to_string(APCRShellsEQPrice),ArialFont, 40);
        if(APCRShellsEQPrice>PlayerCurrentCreditsCount) APCRShellsEQPriceText.setFillColor(sf::Color::Red);
        else APCRShellsEQPriceText.setFillColor(sf::Color::White);
        APCRShellsEQPriceText.setPosition(318,625);
        window.draw(APCRShellsEQPriceText);
        // --- HE ---
        HEShellsEQPriceText = sf::Text(std::to_string(HEShellsEQPrice),ArialFont, 40);
        if(HEShellsEQPrice>PlayerCurrentCreditsCount) HEShellsEQPriceText.setFillColor(sf::Color::Red);
        else HEShellsEQPriceText.setFillColor(sf::Color::White);
        HEShellsEQPriceText.setPosition(491,624);
        window.draw(HEShellsEQPriceText);
        // --- Repair Kit ---
        RepairKitEQPriceText = sf::Text(std::to_string(RepairKitEQPrice),ArialFont, 40);
        if(RepairKitEQPrice>PlayerCurrentCreditsCount) RepairKitEQPriceText.setFillColor(sf::Color::Red);
        else RepairKitEQPriceText.setFillColor(sf::Color::White);
        RepairKitEQPriceText.setPosition(659,623);
        window.draw(RepairKitEQPriceText);
        // --- Extra Shield ---
        ExtraShieldEQPriceText = sf::Text(std::to_string(ExtraShieldEQPrice),ArialFont, 40);
        if(ExtraShieldEQPrice>PlayerCurrentCreditsCount) ExtraShieldEQPriceText.setFillColor(sf::Color::Red);
        else ExtraShieldEQPriceText.setFillColor(sf::Color::White);
        ExtraShieldEQPriceText.setPosition(827,623);
        window.draw(ExtraShieldEQPriceText);
        // --- Med Kit ---
        MedKitEQPriceText = sf::Text(std::to_string(MedKitEQPrice),ArialFont, 40);
        if(MedKitEQPrice>PlayerCurrentCreditsCount) MedKitEQPriceText.setFillColor(sf::Color::Red);
        else MedKitEQPriceText.setFillColor(sf::Color::White);
        MedKitEQPriceText.setPosition(1015,623);
        window.draw(MedKitEQPriceText);
        // --- Accelerator ---
        AcceleratorEQPriceText = sf::Text(std::to_string(AcceleratorEQPrice),ArialFont, 40);
        if(AcceleratorEQPrice>PlayerCurrentCreditsCount) AcceleratorEQPriceText.setFillColor(sf::Color::Red);
        else AcceleratorEQPriceText.setFillColor(sf::Color::White);
        AcceleratorEQPriceText.setPosition(1192,623);
        window.draw(AcceleratorEQPriceText);

    };

    // --- Invisible Buy Buttons ---
    auto BuyAP_EQButton = sf::RectangleShape(sf::Vector2f(132,67));
    BuyAP_EQButton.setPosition(106,681);

    auto BuyAPCR_EQButton = sf::RectangleShape(sf::Vector2f(132,67));
    BuyAPCR_EQButton.setPosition(280,680);

    auto BuyHE_EQButton = sf::RectangleShape(sf::Vector2f(132,67));
    BuyHE_EQButton.setPosition(452,675);

    auto BuyRepairKit_EQButton = sf::RectangleShape(sf::Vector2f(132,67));
    BuyRepairKit_EQButton.setPosition(630,684);

    auto BuyExtraShield_EQButton = sf::RectangleShape(sf::Vector2f(132,67));
    BuyExtraShield_EQButton.setPosition(798,678);

    auto BuyMedKit_EQButton = sf::RectangleShape(sf::Vector2f(132,67));
    BuyMedKit_EQButton.setPosition(975,679);

    auto BuyAccelerator_EQButton = sf::RectangleShape(sf::Vector2f(132,67));
    BuyAccelerator_EQButton.setPosition(1158,675);

    // --- Function for screen appearance
    auto DrawEquipmentStore = [&]() -> void {
        window.clear();
        window.draw(EquipmentStoreScreenImage);
        window.draw(CurrencyBackground);
        UpdateCounters();
        window.draw(XPCountIconImage);
        window.draw(CurrentXPCountText);
        window.draw(CreditsCountIconImage);
        window.draw(CurrentCreditsCountText);
        window.draw(Back2UpgradeOptions2Button);
        window.draw(GoBackArrowImage);
        DrawEQCounters();
        DrawEQPrices();
    };
    // --------------------------------Main Arena Fight Screen-----------------------------------

    //  --- Boolean saying whether Screen is Appearing ---
    auto ArenaFightScreenEnabled = false;

    // --- Boolean saying whether all enemies are dead
    auto AllEnemiesDead = false;

    // --- Total Credits Looted ---
    auto TotalCreditsLooted = 0;

    // --- Arena Image --------
    auto ArenaTexture = sf::Texture();
    if(!ArenaTexture.loadFromFile("../C++GameData/textures/GamePlate2.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto ArenaTextureImage = sf::Sprite(ArenaTexture);

    // --- Ammo Concept Text
    auto ArenaFightAmmoText = sf::Text(sf::String("Ammo:"), ArialFont, 24);
    ArenaFightAmmoText.setPosition(1842, 170);
    ArenaFightAmmoText.setFillColor(sf::Color::White);

    // --- HP Concept Text
    auto ArenaFightHPText = sf::Text(sf::String("HP:"), ArialFont, 31);
    ArenaFightHPText.setPosition(1845, 7);
    ArenaFightHPText.setFillColor(sf::Color::White);

    // --- Fuel Concept Text
    auto ArenaFightFuelText = sf::Text(sf::String("Fuel:"), ArialFont, 25);
    ArenaFightFuelText.setPosition(1845, 472);
    ArenaFightFuelText.setFillColor(sf::Color::White);

    // --- Tools Concept Text
    auto ArenaFightToolsText = sf::Text(sf::String("Tools:"), ArialFont, 23);
    ArenaFightToolsText.setPosition(1845, 647);
    ArenaFightToolsText.setFillColor(sf::Color::White);

    // --- Enemy Team Pack ---
    auto EnemyTeam = std::vector<EnemyTank>();

    // --- Enemy Team Pack Current Speed Vector ---
    auto EnemyTeamSpeedVectors = std::vector<sf::Vector2f>();

    // --- Enemy Team Pack Tank Images
    auto EnemyTeamTanksImages = std::vector<sf::Sprite>();

    // --- Enemy Team Pack Shells Images
    auto EnemyTeamShellsImages = std::vector<sf::Sprite>();

    // --- Enemy Team Pack HP Bar Texts
    auto EnemyTeamHPBarTexts = std::vector<sf::Text>();

    // --- Enemy Team Pack HP Bar Images
    auto EnemyTeamHPBarImages = std::vector<sf::Sprite>();

    // --- Enemy Team Pack Loots Images
    auto EnemyTeamLootsImages = std::vector<sf::Sprite>();

    // --- Time Points for ends of various actions
    auto EndOfPlayerReload = std::chrono::time_point<std::chrono::system_clock>();
    auto EndOfUsingRepairKit = std::chrono::time_point<std::chrono::system_clock>();
    auto EndOfUsingManualRepairKit = std::chrono::time_point<std::chrono::system_clock>();
    auto EndOfUsingExtraShield = std::chrono::time_point<std::chrono::system_clock>();
    auto EndOfUsingEngineAccelerator = std::chrono::time_point<std::chrono::system_clock>();
    auto EndOfUsingAmmoProducer = std::chrono::time_point<std::chrono::system_clock>();
    auto EndOfUsingFuelProducer = std::chrono::time_point<std::chrono::system_clock>();
    auto EndOfExplosionRadios = std::chrono::time_point<std::chrono::system_clock>();
    auto EndOfEnemiesCycleMovement = std::chrono::time_point<std::chrono::system_clock>();

    // --- Player Current HP Level ---
    auto PlayerHP1String = std::string(std::to_string((int)PlayerCurrentHP)+" /\n "+std::to_string((int)PlayerMaxHP));
    auto PlayerHP1Text = sf::Text(sf::String(PlayerHP1String),ArialFont,20);
    PlayerHP1Text.setPosition(1835, 80);

    auto PlayerHP2String = std::string(std::to_string((int)(PlayerCurrentHP/PlayerMaxHP*100))+"%");
    auto PlayerHP2Text = sf::Text(sf::String(PlayerHP2String),ArialFont,30);
    PlayerHP2Text.setPosition(1835, 40);

    // --- Player Current Resistance Level
    auto CurrentResistanceString = std::string(std::to_string(((1-CurrentPlayerResistanceRatio)*100))+"%");
    auto CurrentResistanceText = sf::Text(sf::String(CurrentResistanceString),ArialFont,30);

    // --- Player Ammo Counter ---
    auto APShellCountText = sf::Text(sf::String(std::to_string(APShellCurrentCount)),ArialFont,30);
    auto APCRShellCountText = sf::Text(sf::String(std::to_string(APCRShellCurrentCount)),ArialFont,30);
    auto HEShellCountText = sf::Text(sf::String(std::to_string(HEShellCurrentCount)),ArialFont,30);

    auto ShellReloadingText = sf::Text(sf::String("Reloading"),ArialFont,18);
    ShellReloadingText.setFillColor(sf::Color::Red);
    ShellReloadingText.setPosition(1835, 350);

    auto ShellReadyText = sf::Text(sf::String("Ready"),ArialFont,28);
    ShellReadyText.setFillColor(sf::Color(0, 100, 0));
    ShellReadyText.setPosition(1835, 350);

    auto NoAmmoText = sf::Text(sf::String("No Ammo"),ArialFont,18);
    NoAmmoText.setFillColor(sf::Color::Red);
    NoAmmoText.setPosition(1835, 350);

    // --- Player Fuel Level Indicator
    auto PlayerFuelString = std::string(std::to_string((int)(PlayerCurrentFuelLevel/PlayerMaxFuelLevel*100))+"%");
    auto PlayerFuelText = sf::Text(sf::String(PlayerFuelString),ArialFont,30);
    PlayerFuelText.setPosition(1835, 550);

    auto PlayerFuel2String = std::string(std::to_string((int)PlayerCurrentFuelLevel)+" /\n "+std::to_string((int)PlayerMaxFuelLevel));
    auto PlayerFuel2Text = sf::Text(sf::String(PlayerFuel2String),ArialFont,20);
    PlayerFuel2Text.setPosition(1835, 600);

    // -- Shell Firing Management Function ---
    auto FireShell = [&]() -> bool {
        if(shellType==1){
            if(APShellCurrentCount==0) return false;
            APShellCurrentCount--;
            return true;
        }else if(shellType==2){
        if(APCRShellCurrentCount==0) return false;
        APCRShellCurrentCount--;
        return true;
        }else {
        if(HEShellCurrentCount==0) return false;
        HEShellCurrentCount--;
        return true;
        }
    };

    // --- Current Shell Icon ---
    auto CurrentShellIcon = sf::Sprite();
    CurrentShellIcon = APShellIconImage;
    CurrentShellIcon.setPosition(1855, 390);

    // --- Function for map fields displaying ---
    auto MapFieldsDisplayer = [&]() -> void {
        for(const auto& fieldImage : MapFieldsImages){
            window.draw(fieldImage);
        }
    };

    //--- Function for ammo info displaying
    auto AmmoInfoDisplayer = [&]() -> void {
        if( (shellType==1&&APShellCurrentCount==0)||(shellType==2&&APCRShellCurrentCount==0)||(shellType==3&&HEShellCurrentCount==0) ){
            window.draw(NoAmmoText);
        }else {
            if (PlayerReloading) window.draw(ShellReloadingText);
            else window.draw(ShellReadyText);
        }
    };

    // --- Function for drawing Tracks Destroyed Indicator
    auto DrawTracksDestroyedIndicator = [&]() -> void {
        if (PlayerTracksDestroyed) {
            TracksDestroyedIndicatorImage.setPosition(PlayerTankImage.getPosition() + sf::Vector2f(-70, -30));
            window.draw(TracksDestroyedIndicatorImage);
        }
    };

    // --- Function for drawing Engine Destroyed Indicator
    auto DrawEngineDestroyedIndicator = [&]() -> void {
        if (PlayerEngineDestroyed) {
            EngineDestroyedIndicatorImage.setPosition(PlayerTankImage.getPosition() + sf::Vector2f(-70, -30));
            window.draw(EngineDestroyedIndicatorImage);
        }
    };


    // --- Function for drawing Gun Damaged Indicator
    auto DrawGunDamagedIndicator = [&]() -> void {
        if (PlayerGunDamaged) {
            GunDamagedIndicatorImage.setPosition(PlayerTankImage.getPosition() + sf::Vector2f(-70, -70));
            window.draw(GunDamagedIndicatorImage);
        }
    };

    // --- Current Thing being used to repair icon ---
    auto CurrentRepairingTool = sf::Sprite();

    // --- Shield Icon appears when shield's activated
    auto ActiveShieldIcon = ExtraShieldIconImage;
    auto DrawExtraShieldInfo = [&]() -> void{
        if(PlayerUsingExtraShield){
           ActiveShieldIcon.setPosition(PlayerTankImage.getPosition() + sf::Vector2f(50, -100));
           window.draw(ActiveShieldIcon);
        }
    };

    // --- Engine Accelerator Icon appears when it's activated
    auto ActiveEngineAcceleratorIcon = EngineAcceleratorIconImage;
    auto DrawEngineAcceleratorInfo = [&]() -> void{
        if(PlayerUsingEngineAccelerator){
            ActiveEngineAcceleratorIcon.setPosition(PlayerTankImage.getPosition() + sf::Vector2f(2, -100));
            window.draw(ActiveEngineAcceleratorIcon);
        }
    };

    // --- Ammo Producer Icon appears when it's activated
    auto ActiveAmmoProducerIcon = AmmoProducerIconImage;
    auto DrawAmmoProducerInfo = [&]() -> void{
        if(PlayerUsingAmmoProducer){
            ActiveAmmoProducerIcon.setPosition(PlayerTankImage.getPosition() + sf::Vector2f(-46, -100));
            window.draw(ActiveAmmoProducerIcon);
        }
    };

    // --- Fuel Producer Icon appears when it's activated
    auto ActiveFuelProducerIcon = ExtraFuelIconImage;
    auto DrawFuelProducerInfo = [&]() -> void{
        if(PlayerUsingFuelProducer){
            ActiveFuelProducerIcon.setPosition(PlayerTankImage.getPosition() + sf::Vector2f(-94, -100));
            window.draw(ActiveFuelProducerIcon);
        }
    };

    // --- Function for drawing Repair in Progress indicator
    auto DrawRepairingText = [&]() -> void {
        if(PlayerUsingRepairKit||PlayerUsingManualRepairKit) {
            RepairingText.setPosition(PlayerTankImage.getPosition() + sf::Vector2f(-70, -70));
            window.draw(RepairingText);
            if (PlayerUsingRepairKit) CurrentRepairingTool = RepairKitIconImage;
            else CurrentRepairingTool = ManualRepairKitIconImage;
            CurrentRepairingTool.setPosition(PlayerTankImage.getPosition() + sf::Vector2f(55, -80));
            window.draw(CurrentRepairingTool);
        }
    };

    // --- Function for drawing Explosion Radios
    auto DrawExplosionRadios = [&]() -> void {
        if(ExplosionRadiosVisible){
            window.draw(ExplosionRadiosImage);
        }
    };

    // --- Enemies killed counter
    auto EnemiesKilledInRound = 0;

    // ---- HP Bars Images ----
    // --- 100% HP ---
    auto EnemyHP100Bar = sf::Texture();
    if(!EnemyHP100Bar.loadFromFile("../C++GameData/textures/EnemyHP100%Bar.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto EnemyHP100BarImage = sf::Sprite(EnemyHP100Bar);
    // --- 80% HP ---
    auto EnemyHP80Bar = sf::Texture();
    if(!EnemyHP80Bar.loadFromFile("../C++GameData/textures/EnemyHP80%Bar.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto EnemyHP80BarImage = sf::Sprite(EnemyHP80Bar);
    // --- 60% HP ---
    auto EnemyHP60Bar = sf::Texture();
    if(!EnemyHP60Bar.loadFromFile("../C++GameData/textures/EnemyHP60%Bar.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto EnemyHP60BarImage = sf::Sprite(EnemyHP60Bar);
    // --- 40% HP ---
    auto EnemyHP40Bar = sf::Texture();
    if(!EnemyHP40Bar.loadFromFile("../C++GameData/textures/EnemyHP40%Bar.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto EnemyHP40BarImage = sf::Sprite(EnemyHP40Bar);
    // --- 20% HP ---
    auto EnemyHP20Bar = sf::Texture();
    if(!EnemyHP20Bar.loadFromFile("../C++GameData/textures/EnemyHP20%Bar.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto EnemyHP20BarImage = sf::Sprite(EnemyHP20Bar);

    // --- Function for drawing Enemies & Their stuff ---
    auto DrawEnemies = [&]() -> void {
        for(auto const& ShellImage : EnemyTeamShellsImages){
            window.draw(ShellImage);
        }
        for(auto const& EnemyImage : EnemyTeamTanksImages){
            window.draw(EnemyImage);
        }
        for(auto const& EnemyHPBarImage : EnemyTeamHPBarImages){
            window.draw(EnemyHPBarImage);
        }
        for(auto const& EnemyHPBarText : EnemyTeamHPBarTexts){
            window.draw(EnemyHPBarText);
        }
        for(auto const& EnemyLootImage2 : EnemyTeamLootsImages){
            window.draw(EnemyLootImage2);
        }
    };

    //  --- Function for resetting player consumables after death ---
    auto ResetPlayerItems = [&]() -> void {
        APShellCurrentCount = APShellDefaultCount;
        APCRShellCurrentCount = APCRShellDefaultCount;
        HEShellCurrentCount = HEShellDefaultCount;
        PlayerCurrentFuelLevel = PlayerDefaultFuelLevel;
        PlayerCurrentHP = PlayerDefaultHP;
        CurrentRepairKitCount = DefaultRepairKitCount;
        CurrentMedKitCount = DefaultMedKitCount;
        CurrentEngineAcceleratorCount = DefaultEngineAcceleratorCount;
        CurrentExtraShieldCount = DefaultExtraShieldCount;
        // Resetting Enemies
        EnemyTeam = std::vector<EnemyTank>();
        EnemyTeamTanksImages = std::vector<sf::Sprite>();
        EnemyTeamShellsImages = std::vector<sf::Sprite>();
        EnemyTeamSpeedVectors = std::vector<sf::Vector2f>();
        EnemyTeamHPBarTexts = std::vector<sf::Text>();
        EnemyTeamHPBarImages = std::vector<sf::Sprite>();
        EnemyTeamLootsImages = std::vector<sf::Sprite>();
        // Resetting Consumables Using Info
        CurrentPlayerResistanceRatio=DefaultPlayerResistanceRatio;
        PlayerTracksDestroyed = false;
        PlayerEngineDestroyed = false;
        PlayerUsingRepairKit = false;
        PlayerUsingManualRepairKit = false;
        PlayerUsingFuelProducer = false;
        PlayerUsingEngineAccelerator = false;
        PlayerUsingAmmoProducer = false;
        PlayerUsingExtraShield = false;
    };

    //  --- Function for updating player consumables after win ---
    auto UpdatePlayerItems = [&]() -> void {
        APShellDefaultCount = APShellCurrentCount;
        APCRShellDefaultCount = APCRShellCurrentCount;
        HEShellDefaultCount = HEShellCurrentCount;
        PlayerCurrentFuelLevel = PlayerDefaultFuelLevel;
        PlayerCurrentHP = PlayerDefaultHP;
        DefaultRepairKitCount = CurrentRepairKitCount;
        DefaultMedKitCount = CurrentMedKitCount;
        DefaultEngineAcceleratorCount = CurrentEngineAcceleratorCount;
        DefaultExtraShieldCount = CurrentExtraShieldCount;
        // Resetting Enemies
        EnemyTeam = std::vector<EnemyTank>();
        EnemyTeamTanksImages = std::vector<sf::Sprite>();
        EnemyTeamShellsImages = std::vector<sf::Sprite>();
        EnemyTeamSpeedVectors = std::vector<sf::Vector2f>();
        EnemyTeamHPBarTexts = std::vector<sf::Text>();
        EnemyTeamHPBarImages = std::vector<sf::Sprite>();
        EnemyTeamLootsImages = std::vector<sf::Sprite>();
        // Resetting Consumables Using Info
        PlayerTracksDestroyed = false;
        PlayerEngineDestroyed = false;
        PlayerUsingRepairKit = false;
        PlayerUsingManualRepairKit = false;
        PlayerUsingFuelProducer = false;
        PlayerUsingEngineAccelerator = false;
        PlayerUsingAmmoProducer = false;
        PlayerUsingExtraShield = false;
    };

    // --- Function for Screen Appearance ---
    auto DrawArenaFight = [&]() -> void {
        window.clear();
        window.draw(ArenaTextureImage);
        MapFieldsDisplayer();
        window.draw(ArenaFightAmmoText);
        window.draw(ArenaFightHPText);
        window.draw(ArenaFightFuelText);
        window.draw(ArenaFightToolsText);
        window.draw(CurrentFirstShellLoaded);
        window.draw(CurrentSecondShellLoaded);
        window.draw(PlayerTankImage);
        window.draw(PlayerHP1Text);
        window.draw(PlayerHP2Text);
        window.draw(APShellCountText);
        window.draw(APCRShellCountText);
        window.draw(HEShellCountText);
        AmmoInfoDisplayer();
        window.draw(APShellIconImage);
        window.draw(APCRShellIconImage);
        window.draw(HEShellIconImage);
        window.draw(CurrentShellIcon);
        window.draw(PlayerFuelText);
        window.draw(PlayerFuel2Text);
        window.draw(RepairKitIconImage);
        window.draw(RepairKitCountText);
        window.draw(ManualRepairKitIconImage);
        window.draw(ManualRepairKitCountIcon);
        window.draw(ExtraShieldIconImage);
        window.draw(ExtraShieldCountText);
        window.draw(MedKitIconImage);
        window.draw(MedKitCountText);
        window.draw(EngineAcceleratorIconImage);
        window.draw(EngineAcceleratorCountText);
        window.draw(ExtraFuelIconImage);
        window.draw(FuelProducerCountIcon);
        window.draw(AmmoProducerIconImage);
        window.draw(AmmoProducerCountIcon);
        DrawTracksDestroyedIndicator();
        DrawEngineDestroyedIndicator();
        DrawGunDamagedIndicator();
        DrawRepairingText();
        window.draw(PlayerResistanceIconImage);
        window.draw(CurrentResistanceText);
        DrawExtraShieldInfo();
        DrawEngineAcceleratorInfo();
        DrawAmmoProducerInfo();
        DrawFuelProducerInfo();
        window.draw(NumKey1Text);
        window.draw(NumKey2Text);
        window.draw(NumKey3Text);
        window.draw(NumKey4Text);
        window.draw(NumKey5Text);
        window.draw(NumKey6Text);
        window.draw(NumKey7Text);
        window.draw(NumKey8Text);
        window.draw(NumKey9Text);
        window.draw(NumKey0Text);
        window.draw(CurrentEndFlagImage);
        DrawExplosionRadios();
        DrawEnemies();
        // ----------
    };

   // ------------ Player Died Screen ---------

   // --- Boolean saying whether the screen is showing
   auto PlayerDeathScreenEnabled = false;

   // --- Death Info Background
   auto YouAreDeadBackground = sf::RectangleShape(sf::Vector2f(1000, 800));
   YouAreDeadBackground.setPosition({400, 125});
   YouAreDeadBackground.setFillColor(sf::Color(150, 0, 0, 40));
   YouAreDeadBackground.setOutlineColor(sf::Color::Red);
   YouAreDeadBackground.setOutlineThickness(1);

   // --- 'OK' Button in Death info Screen
   auto OkFromDeathButton = sf::RectangleShape(sf::Vector2f(175, 100));
   OkFromDeathButton.setPosition({1100, 725});
   OkFromDeathButton.setFillColor(sf::Color(150, 150, 0, 40));
   OkFromDeathButton.setOutlineColor(sf::Color::Red);
   OkFromDeathButton.setOutlineThickness(1);

   // --- 'OK' Text on Button
   auto OkFromDeathText = sf::Text(sf::String("Ok..."), ArialFont, 50);
   OkFromDeathText.setFillColor(sf::Color::Black);
   OkFromDeathText.setPosition(1130,745);

   // --- Text informing that player's dead
   auto YouAreDeadText = sf::Text(sf::String("You are dead."), ArialFont, 70);
   YouAreDeadText.setFillColor(sf::Color::Black);
   YouAreDeadText.setPosition(700,400);

   // --- Death Skull Image
   auto DeathSkull = sf::Texture();
   if(!DeathSkull.loadFromFile("../C++GameData/textures/DeathSkull.png")){
       std::cout << "Couldn't load game texture";
       return -4;
   }
   auto DeathSkullImage = sf::Sprite(DeathSkull);
   DeathSkullImage.setPosition(770,150);

    // --- XP Gained Icon ---
    auto XPGainedIcon = XPCountIconImage;
    XPGainedIcon.setPosition(600,600);

    // --- Credits Gained Icon ---
    auto CreditsGainedIcon = CreditsCountIconImage;
    CreditsGainedIcon.setPosition(600,680);

    // --- XP Gained Count Text ---
    auto XPGainedCount=int(0);
    auto XPGainedCountText = sf::Text(std::to_string(XPGainedCount), ArialFont, 40);
    XPGainedCountText.setPosition(622,600);

    // --- Credits Gained Count Text ---
    auto CreditsGainedCountText = sf::Text(std::to_string(TotalCreditsLooted), ArialFont, 40);
    CreditsGainedCountText.setPosition(582,677);

    auto setDeathXPGainedText = [&]() -> void {
        dis = std::uniform_int_distribution<>(1, 5);
        auto XPBonus = dis(gen);
        XPGainedCount=int(35*EnemiesKilledInRound*PlayerCurrentTier+XPBonus*PlayerCurrentTier);
        XPGainedCountText = sf::Text("+    "+std::to_string(XPGainedCount), ArialFont, 35);
        XPGainedCountText.setPosition(582,597);
        PlayerCurrentXPCount+=XPGainedCount;
    };

    auto setCreditsGainedText = [&]() -> void {
        CreditsGainedCountText = sf::Text("+    "+std::to_string(TotalCreditsLooted), ArialFont, 40);
        CreditsGainedCountText.setPosition(582,677);
        PlayerCurrentCreditsCount+=TotalCreditsLooted;
    };

    // --- Function for screen appearance ---
    auto DrawPlayerDeathScreen = [&]() -> void {
        window.draw(YouAreDeadBackground);
        window.draw(YouAreDeadText);
        window.draw(DeathSkullImage);
        window.draw(XPGainedIcon);
        window.draw(XPGainedCountText);
        window.draw(CreditsGainedIcon);
        window.draw(CreditsGainedCountText);
        window.draw(OkFromDeathButton);
        window.draw(OkFromDeathText);
    };
    //----------------------------------------------------------------------
    // ------------------- Player has won Screen --------------------
    // --- Booling saying whether screen is appearing
    auto PlayerWinScreenEnabled = false;

    // --- Won Info Background
    auto YouWinBackground = sf::RectangleShape(sf::Vector2f(1000, 800));
    YouWinBackground.setPosition({400, 125});
    YouWinBackground.setFillColor(sf::Color(0, 170, 0, 40));
    YouWinBackground.setOutlineColor(sf::Color::Blue);
    YouWinBackground.setOutlineThickness(1);

    // --- 'Essa' Button in Won info Screen
    auto EssaWinButton = sf::RectangleShape(sf::Vector2f(175, 100));
    EssaWinButton.setPosition({1100, 725});
    EssaWinButton.setFillColor(sf::Color(50, 150, 0, 40));
    EssaWinButton.setOutlineColor(sf::Color::Blue);
    EssaWinButton.setOutlineThickness(1);

    // --- 'Essa' Text on Button
    auto EssaWinText = sf::Text(sf::String("Essa."), ArialFont, 50);
    EssaWinText.setFillColor(sf::Color::Black);
    EssaWinText.setPosition(1130,745);

    // --- Text informing that player's won
    auto YouWonText = sf::Text(sf::String("You have won!"), ArialFont, 70);
    YouWonText.setFillColor(sf::Color::Black);
    YouWonText.setPosition(700,400);

    // --- Won Price Image
    auto WinStar = sf::Texture();
    if(!WinStar.loadFromFile("../C++GameData/textures/BigStarWin.png")){
        std::cout << "Couldn't load game texture";
        return -4;
    }
    auto WinStarImage = sf::Sprite(WinStar);
    WinStarImage.setPosition(770,150);

    // --- XP Gained Count Text ---
    auto setXPGainedText = [&]() -> void {
        dis = std::uniform_int_distribution<>(1, 49);
        auto XPBonus = dis(gen);
        XPGainedCount=int((500+XPBonus)*PlayerCurrentTier);
        XPGainedCountText = sf::Text("+    "+std::to_string(XPGainedCount), ArialFont, 35);
        XPGainedCountText.setPosition(582,597);
        PlayerCurrentXPCount+=XPGainedCount;
    };

    // --- Function for screen appearance ---
    auto DrawPlayerWonScreen = [&]() -> void {
        window.draw(YouWinBackground);
        window.draw(YouWonText);
        window.draw(XPGainedIcon);
        window.draw(XPGainedCountText);
        window.draw(CreditsGainedIcon);
        window.draw(CreditsGainedCountText);
        window.draw(WinStarImage);
        window.draw(EssaWinButton);
        window.draw(EssaWinText);
    };

    // -------------SAVED GAME PROGRESS READER ---------
    auto ReadGameProgress = [&]() -> void {
        auto GameProgressFile = std::fstream("data.st2d.txt");
        auto WordRead = std::string();
        auto WordsRead = std::vector<std::string>();
        while(GameProgressFile>>WordRead){
            WordsRead.push_back(WordRead);
        }
        PlayerCurrentCreditsCount=std::stoi(WordsRead[2]);
        PlayerCurrentXPCount=std::stoi(WordsRead[4]);
        CurrentBattlefieldLevel=std::stoi(WordsRead[6]);
        PlayerCurrentTier=std::stoi(WordsRead[8]);
        PlayerCurrentGunLevel=std::stoi(WordsRead[10]);
        PlayerCurrentTurretLevel=std::stoi(WordsRead[12]);
        PlayerCurrentEngineLevel=std::stoi(WordsRead[14]);
        APShellDefaultCount=std::stoi(WordsRead[16]);
        APCRShellDefaultCount=std::stoi(WordsRead[18]);
        HEShellDefaultCount=std::stoi(WordsRead[20]);
        DefaultRepairKitCount=std::stoi(WordsRead[22]);
        DefaultExtraShieldCount=std::stoi(WordsRead[24]);
        DefaultMedKitCount=std::stoi(WordsRead[26]);
        DefaultEngineAcceleratorCount=std::stoi(WordsRead[28]);
        BattlefieldLevelText = sf::Text(std::to_string(CurrentBattlefieldLevel), ArialFont, 200);
        BattlefieldLevelText.setPosition({400, 200});
        BattlefieldLevelText.setFillColor(sf::Color::Black);
        UpgradePlayerStats();
    };


    // ---- Reset Game Progress ---
    auto ResetGameProgress = [&]() -> void {
        PlayerCurrentCreditsCount=int(0);
        PlayerCurrentXPCount=int(0);
        PlayerCurrentTier = int(1);
        PlayerCurrentGunLevel = int(1);
        PlayerCurrentTurretLevel = int(1);
        PlayerCurrentEngineLevel = int(1);
        CurrentBattlefieldLevel = int(1);
        APShellDefaultCount = int(15);
        APShellCurrentCount = APShellDefaultCount;
        APCRShellDefaultCount = int(15);
        APCRShellCurrentCount = APCRShellDefaultCount;
        HEShellDefaultCount = int(15);
        HEShellCurrentCount = HEShellDefaultCount;
        DefaultRepairKitCount = int(5);
        CurrentRepairKitCount = DefaultRepairKitCount;
        DefaultExtraShieldCount = int(3);
        CurrentExtraShieldCount = DefaultExtraShieldCount;
        DefaultMedKitCount = int(3);
        CurrentMedKitCount = DefaultMedKitCount;
        DefaultEngineAcceleratorCount = int(3);
        BattlefieldLevelText = sf::Text(std::to_string(CurrentBattlefieldLevel), ArialFont, 200);
        BattlefieldLevelText.setPosition({400, 200});
        BattlefieldLevelText.setFillColor(sf::Color::Black);
        UpgradePlayerStats();
    };



    // ------------ Warning BEFORE SAVING SCREEN ----------
    // --- Flag for warning enabled ---
    auto WarningEnabled = false;

    auto WarningBackground = sf::RectangleShape(sf::Vector2f(1193,758));
    WarningBackground.setPosition(430,160);
    WarningBackground.setFillColor(sf::Color(0, 240, 240, 250));
    WarningBackground.setOutlineColor(sf::Color::Red);
    WarningBackground.setOutlineThickness(1);

    auto WarningInfo = sf::Text(sf::String("                               WARNING!!!\nSaving game data will overwrite previous save.\nAre you sure you want to proceed?"),ArialFont,50);
    WarningInfo.setPosition(450,180);
    WarningInfo.setFillColor(sf::Color::Black);

    auto YesSaveButton = sf::RectangleShape(sf::Vector2f(274,130));
    YesSaveButton.setPosition(1270,730);
    YesSaveButton.setFillColor(sf::Color(200, 0, 0, 128));
    YesSaveButton.setOutlineColor(sf::Color::Red);
    YesSaveButton.setOutlineThickness(1);

    auto YesSaveText = sf::Text(sf::String("Save"), ArialFont, 50);
    YesSaveText.setFillColor(sf::Color::Black);
    YesSaveText.setPosition(1335,762);

    auto NoCancelText = sf::Text(sf::String("Cancel"), ArialFont, 50);
    NoCancelText.setFillColor(sf::Color::Black);
    NoCancelText.setPosition(555,765);

    auto NoCancelButton = sf::RectangleShape(sf::Vector2f(274,130));
    NoCancelButton.setPosition(505,735);
    NoCancelButton.setFillColor(sf::Color(0, 200, 0, 128));
    NoCancelButton.setOutlineColor(sf::Color::Red);
    NoCancelButton.setOutlineThickness(1);

    auto DrawWarning = [&]() -> void {
        window.draw(WarningBackground);
        window.draw(WarningInfo);
        window.draw(NoCancelButton);
        window.draw(YesSaveButton);
        window.draw(NoCancelText);
        window.draw(YesSaveText);
    };

    // ---- Save Game Data Writer ----
    auto SaveGameData = [&]() -> void {
        auto DataSaveFile = std::fstream("data.st2d.txt", std::ios::out | std::ios::trunc);
        DataSaveFile << "<This_is_a_SuperTanker2D_Game_Data_Progress_File>\n";
        DataSaveFile << "PlayerCurrentCreditsCount " << PlayerCurrentCreditsCount <<'\n';
        DataSaveFile << "PlayerCurrentXPCount " << PlayerCurrentXPCount <<'\n';
        DataSaveFile << "CurrentBattlefieldLevel " << CurrentBattlefieldLevel <<'\n';
        DataSaveFile << "PlayerCurrentTier " << PlayerCurrentTier <<'\n';
        DataSaveFile << "PlayerCurrentGunLevel " << PlayerCurrentGunLevel <<'\n';
        DataSaveFile << "PlayerCurrentTurretLevel " << PlayerCurrentTurretLevel <<'\n';
        DataSaveFile << "PlayerCurrentEngineLevel " << PlayerCurrentEngineLevel <<'\n';
        DataSaveFile << "APShellDefaultCount " << APShellDefaultCount <<'\n';
        DataSaveFile << "APCRShellDefaultCount " << APCRShellDefaultCount <<'\n';
        DataSaveFile << "HEShellDefaultCount " << HEShellDefaultCount <<'\n';
        DataSaveFile << "DefaultRepairKitCount " << DefaultRepairKitCount <<'\n';
        DataSaveFile << "DefaultExtraShieldCount " << DefaultExtraShieldCount <<'\n';
        DataSaveFile << "DefaultMedKitCount " << DefaultMedKitCount <<'\n';
        DataSaveFile << "DefaultEngineAcceleratorCount " << DefaultEngineAcceleratorCount <<'\n';
    };

    // --- Function to set current frame rate & variables dependent to it
    auto SetFPScap = [&](int FPSLimit) -> void{
        window.setFramerateLimit(FPSLimit);
        FPSCap = FPSLimit;
        FPSspeedMultiplier = float((float(169)/static_cast<float>(FPSCap)));
        DefaultPlayerMovementSpeed = sf::Vector2f(OriginalPlayerMovementSpeed.x*FPSspeedMultiplier,OriginalPlayerMovementSpeed.y*FPSspeedMultiplier);
        PlayerMovementSpeed = DefaultPlayerMovementSpeed;
        APShellDefaultVelocity = float(OriginalAPShellVelocity*FPSspeedMultiplier);
        APShellVelocity = APShellDefaultVelocity;
        APCRShellDefaultVelocity = float(OriginalAPCRShellVelocity*FPSspeedMultiplier);
        APCRShellVelocity = APCRShellDefaultVelocity;
        HEShellDefaultVelocity = float(OriginalHEShellVelocity*FPSspeedMultiplier);
        HEShellVelocity = HEShellDefaultVelocity;
        PlayerDefaultFuelBurning = float(OriginalFuelBurningRate*FPSspeedMultiplier);
        PlayerCurrentFuelBurning = PlayerDefaultFuelBurning;
        setCurrentShellType();
    };
    //-------------------------------WINDOW------------------------------------------------------------
    while (window.isOpen()) {
        // --------------------EVENT HANDLING-------------------------------
        auto event = sf::Event();
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    auto mousePos = sf::Mouse::getPosition(window);
                    if (MainMenuScreenEnabled && MenuPlayButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        //  --- Main Screen - Option chosen -> 'Play' ---
                        MainMenuScreenEnabled = false;
                        PlayOptionSelectorEnabled = true;

                    } else if (MainMenuScreenEnabled && MenuQuitButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        //  --- Main Screen - Option chosen -> 'Quit' ---
                        QuitingScreenEnabled = true;
                        MainMenuScreenEnabled = false;

                    }else if (MainMenuScreenEnabled && MenuOptionsButton.getGlobalBounds().contains(
                                {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                            //  --- Main Screen - Option chosen -> 'Options' ---
                            OptionsScreenEnabled = true;
                            MainMenuScreenEnabled = false;

                    }else if (OptionsScreenEnabled && BackFromOptionsButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        //  --- Options screen - Option chosen -> 'Go Back' ---
                        OptionsScreenEnabled = false;
                        MainMenuScreenEnabled = true;

                    }else if (OptionsScreenEnabled && (FPSCap!=30) && FPS30Button.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        //  --- Options screen - Option chosen -> '30 FPS' ---
                        SetFPScap(30);
                    }else if (OptionsScreenEnabled && (FPSCap!=60) && FPS60Button.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        //  --- Options screen - Option chosen -> '60 FPS' ---
                        SetFPScap(60);
                    }else if (OptionsScreenEnabled && (FPSCap!=120) && FPS120Button.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        //  --- Options screen - Option chosen -> '120 FPS' ---
                        SetFPScap(120);
                    } else if (QuitingScreenEnabled && QuitingNoButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        //  --- Quiting Screen - Option chosen -> 'No' ---
                        QuitingScreenEnabled = false;
                        MainMenuScreenEnabled = true;

                    } else if (QuitingScreenEnabled && QuitingYesButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        //  --- Quiting Screen - Option chosen -> 'Yes' ---
                        window.close();

                    } else if (PlayOptionSelectorEnabled && NewGameOptionButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Play Option Selector Screen - Option chosen -> 'New Game' ---
                        ResetGameProgress();
                        PreFightMenuEnabled = true;
                        PlayOptionSelectorEnabled = false;
                    } else if (PlayOptionSelectorEnabled && LoadGameOptionButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Play Option Selector Screen - Option chosen -> 'Load Game' ---
                        ReadGameProgress();
                        PreFightMenuEnabled = true;
                        PlayOptionSelectorEnabled = false;
                    } else if (PlayOptionSelectorEnabled && Back2MainMenuButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Go Back Button clicked - in Play Option Selector Screen ---
                        MainMenuScreenEnabled = true;
                        PlayOptionSelectorEnabled = false;
                    } else if (PlayOptionSelectorEnabled && TutorialOptionButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Tutorial option clicked - in Play Option Selector Screen ---
                        TutorialEnabled = true;
                        PlayOptionSelectorEnabled = false;
                    } else if (TutorialEnabled && NextTutorialButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Next Tutorial option clicked - in Tutorial Screen ---
                        if(TutorialScreen!=5) TutorialScreen++;
                        else {
                            TutorialEnabled = false;
                            PlayOptionSelectorEnabled = true;
                        }
                    } else if (TutorialEnabled && PreviousTutorialButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Previous Tutorial option clicked - in Tutorial Screen ---
                        if(TutorialScreen!=1) TutorialScreen--;
                        else {
                            TutorialEnabled = false;
                            PlayOptionSelectorEnabled = true;
                        }
                    } else if (PreFightMenuEnabled && Back2PlayOptionSelectorButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Go Back Button clicked - in Pre Fight Menu Screen ---
                        PlayOptionSelectorEnabled = true;
                        PreFightMenuEnabled = false;
                    } else if (PreFightMenuEnabled && !WarningEnabled && PreFightSaveGameButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Save Game button clicked - in Pre Fight Menu Screen ---
                        WarningEnabled = true;
                        PreFightMenuEnabled = false;
                    } else if (WarningEnabled && NoCancelButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Cancel button clicked - in Warning Screen ---
                        WarningEnabled = false;
                        PreFightMenuEnabled = true;
                    } else if (WarningEnabled && YesSaveButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Save button clicked - in Warning Screen ---
                        SaveGameData();
                        WarningEnabled = false;
                        PreFightMenuEnabled = true;
                    } else if (PreFightMenuEnabled && PreFightMenuUpgradeButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Upgrade Button clicked - in Pre Fight Menu Screen ---
                        UpgradeOptionsScreenEnabled = true;
                        PreFightMenuEnabled = false;
                    } else if (PreFightMenuEnabled && PreFightMenuEquipmentButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Equipment button clicked - in Pre Fight Menu Screen ---
                        EquipmentStoreScreenEnabled = true;
                        PreFightMenuEnabled = false;
                        UpdateEQPrices();
                    } else if (EquipmentStoreScreenEnabled && Back2UpgradeOptions2Button.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Go back button clicked - in Equipment store Screen ---
                        EquipmentStoreScreenEnabled = false;
                        PreFightMenuEnabled = true;
                    } else if (EquipmentStoreScreenEnabled && (PlayerCurrentCreditsCount>APShellsEQPrice) && (APShellDefaultCount<50) && BuyAP_EQButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Buy AP button clicked - in Equipment store Screen ---
                        APShellDefaultCount++;
                        APShellCurrentCount=APShellDefaultCount;
                        PlayerCurrentCreditsCount-=APShellsEQPrice;
                    } else if (EquipmentStoreScreenEnabled && (PlayerCurrentCreditsCount>APCRShellsEQPrice) && (APCRShellDefaultCount<50) && BuyAPCR_EQButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Buy APCR button clicked - in Equipment store Screen ---
                        APCRShellDefaultCount++;
                        APCRShellCurrentCount=APCRShellDefaultCount;
                        PlayerCurrentCreditsCount-=APCRShellsEQPrice;
                    } else if (EquipmentStoreScreenEnabled && (PlayerCurrentCreditsCount>HEShellsEQPrice) && (HEShellDefaultCount<50) && BuyHE_EQButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Buy HE button clicked - in Equipment store Screen ---
                        HEShellDefaultCount++;
                        HEShellCurrentCount=HEShellDefaultCount;
                        PlayerCurrentCreditsCount-=HEShellsEQPrice;
                    } else if (EquipmentStoreScreenEnabled && (PlayerCurrentCreditsCount>RepairKitEQPrice) && (DefaultRepairKitCount<15) && BuyRepairKit_EQButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Buy Repair Kit button clicked - in Equipment store Screen ---
                        DefaultRepairKitCount++;
                        CurrentRepairKitCount=DefaultRepairKitCount;
                        PlayerCurrentCreditsCount-=RepairKitEQPrice;
                    } else if (EquipmentStoreScreenEnabled && (PlayerCurrentCreditsCount>ExtraShieldEQPrice) && (DefaultExtraShieldCount<15) && BuyExtraShield_EQButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Buy ExtraShield button clicked - in Equipment store Screen ---
                        DefaultExtraShieldCount++;
                        CurrentExtraShieldCount=DefaultExtraShieldCount;
                        PlayerCurrentCreditsCount-=ExtraShieldEQPrice;
                    } else if (EquipmentStoreScreenEnabled && (PlayerCurrentCreditsCount>MedKitEQPrice) && (DefaultMedKitCount<15) && BuyMedKit_EQButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Buy MedKit button clicked - in Equipment store Screen ---
                        DefaultMedKitCount++;
                        CurrentMedKitCount=DefaultMedKitCount;
                        PlayerCurrentCreditsCount-=MedKitEQPrice;
                    } else if (EquipmentStoreScreenEnabled && (PlayerCurrentCreditsCount>AcceleratorEQPrice) && (DefaultEngineAcceleratorCount<15) && BuyAccelerator_EQButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Buy Accelerator button clicked - in Equipment store Screen ---
                        DefaultEngineAcceleratorCount++;
                        CurrentEngineAcceleratorCount=DefaultEngineAcceleratorCount;
                        PlayerCurrentCreditsCount-=AcceleratorEQPrice;
                    } else if (UpgradeOptionsScreenEnabled && Back2PreFightMenuButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Go Back Button clicked - in Upgrade Options Screen ---
                        UpgradeOptionsScreenEnabled = false;
                        PreFightMenuEnabled = true;

                    } else if (UpgradeOptionsScreenEnabled && UpgradeTechTreeButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Tech Tree button clicked - in Upgrade Options Screen ---
                        UpgradeOptionsScreenEnabled = false;
                        TechTreeUpgradeScreenEnabled = true;
                    } else if (UpgradeOptionsScreenEnabled && UpgradeModulesButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Modules button clicked - in Upgrade Options Screen ---
                        UpgradeOptionsScreenEnabled = false;
                        UpgradeModulesScreenEnabled = true;

                    } else if (TechTreeUpgradeScreenEnabled && Back2UpgradeOptionsButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Go Back Button clicked - in Tech Tree Screen ---
                        UpgradeOptionsScreenEnabled = true;
                        TechTreeUpgradeScreenEnabled = false;
                    } else if (UpgradeModulesScreenEnabled && Back2UpgradeOptionsButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Go Back Button clicked - in Modules Screen ---
                        UpgradeOptionsScreenEnabled = true;
                        UpgradeModulesScreenEnabled = false;
                    } else if (UpgradeModulesScreenEnabled && (PlayerCurrentGunLevel<3) && (PlayerCurrentXPCount>XPRequired4Gun) && UpgradeGunButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Upgrade Gun Button clicked - in Modules Screen ---
                        PlayerCurrentGunLevel++;
                        PlayerCurrentXPCount-=XPRequired4Gun;
                        UpgradePlayerStats();
                    } else if (UpgradeModulesScreenEnabled && (PlayerCurrentTurretLevel<3) && (PlayerCurrentXPCount>XPRequired4Turret) && UpgradeTurretButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Upgrade Turret Button clicked - in Modules Screen ---
                        PlayerCurrentTurretLevel++;
                        PlayerCurrentXPCount-=XPRequired4Turret;
                        UpgradePlayerStats();
                    } else if (UpgradeModulesScreenEnabled && (PlayerCurrentEngineLevel<3) && (PlayerCurrentXPCount>XPRequired4Engine) && UpgradeEngineButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Upgrade Engine Button clicked - in Modules Screen ---
                        PlayerCurrentEngineLevel++;
                        PlayerCurrentXPCount-=XPRequired4Engine;
                        UpgradePlayerStats();
                    } else if (TechTreeUpgradeScreenEnabled && ModulesMaximized && (XPRequired4NextTank<PlayerCurrentXPCount) && Upgrade2NextTankButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- Upgrade to next tank button clicked - in Tech Tree Screen ---
                        PlayerCurrentTier++;
                        PlayerCurrentXPCount-=XPRequired4NextTank;
                        PlayerCurrentTurretLevel=1;
                        PlayerCurrentEngineLevel=1;
                        PlayerCurrentGunLevel=1;
                        UpgradePlayerStats();

                    } else if (PreFightMenuEnabled && PreFightMenuFightButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // --- 'Fight' Button clicked - in Pre Fight Menu Screen ---
                        PreFightMenuEnabled = false;
                        wait(1);
                        TotalCreditsLooted=0;
                        EnemiesKilledInRound=0;
                        CurrentPlayerResistanceRatio=DefaultPlayerResistanceRatio;
                        MapGenerator(MapFieldsByType);
                        MapFieldsBuilder(MapFieldsImages,EnumedImageMap,MapFieldsByType);
                        EnemyTeam = GenerateEnemies(CurrentBattlefieldLevel);
                        // --- Creating Sprites for each enemy ---
                        for(auto const& enemy : EnemyTeam){
                            auto EnemyImage = sf::Sprite(enemy.TankTexture);
                            EnemyImage.setPosition(enemy.TankPosition);
                            EnemyImage.setOrigin(EnemyImage.getLocalBounds().width / 2, EnemyImage.getLocalBounds().height / 2);
                            EnemyTeamTanksImages.push_back(EnemyImage);
                            auto EnemyShellImage = sf::Sprite();
                            switch (enemy.shellType) {
                                case 1:
                                    EnemyShellImage = APShellTextureImage;
                                    break;
                                case 2:
                                    EnemyShellImage = APCRShellTextureImage;
                                    break;
                                case 3:
                                    EnemyShellImage = HEShellTextureImage;
                                    break;
                            }
                            EnemyShellImage.setPosition(enemy.TankPosition);
                            EnemyTeamShellsImages.push_back(EnemyShellImage);
                            CurrentEndFlagImage=RedFlagImage;
                            AllEnemiesDead=false;
                            EnemyTeamSpeedVectors.push_back(sf::Vector2f(0,0));
                            auto HPBarText = sf::Text(std::to_string(enemy.TankCurrentHP), ArialFont, 15);
                            HPBarText.setFillColor(sf::Color::Black);
                            EnemyTeamHPBarTexts.push_back(HPBarText);
                            auto EnemyLoot = CreditsLootIconImage;
                            EnemyLoot.setPosition(2000,2000);
                            EnemyTeamLootsImages.push_back(EnemyLoot);
                            const auto& EnemyHPBarImage = EnemyHP100BarImage;
                            EnemyTeamHPBarImages.push_back(EnemyHPBarImage);
                        }
                        ArenaFightScreenEnabled = true;
                    } else if (ArenaFightScreenEnabled && FirstShellLoaded && !PlayerReloading&&FireShell()) {
                        // Firing if 1st shell Ready & Player not Reloading
                        PlayerReloading = true;  // RELOAD TIMER START
                        SetTimePoints(EndOfPlayerReload, PlayerCurrentReload);
                        FirstShellFirePositionX = PlayerPosition.x;
                        FirstShellFirePositionY = PlayerPosition.y;
                        FirstshellVectorVelocity = GetVectorVelocity(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y),FirstShellFirePositionX, FirstShellFirePositionY,CurrentFirstShellVelocity);
                        CurrentFirstShellLoaded.setRotation(GetShellRotationAngle(FirstshellVectorVelocity));
                        FirstShellLoaded = false;
                    } else if (ArenaFightScreenEnabled && !FirstShellLoaded && SecondShellLoaded &&!PlayerReloading&&FireShell()) {
                        // Firing if 1st shell Absent & 2nd shell Ready & Player not Reloading
                        PlayerReloading = true;  // RELOAD TIMER START
                        SetTimePoints(EndOfPlayerReload, PlayerCurrentReload);
                        SecondShellFirePositionX = PlayerPosition.x;
                        SecondShellFirePositionY = PlayerPosition.y;
                        SecondshellVectorVelocity = GetVectorVelocity(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y),SecondShellFirePositionX,SecondShellFirePositionY,CurrentSecondShellVelocity);
                        CurrentSecondShellLoaded.setRotation(GetShellRotationAngle(SecondshellVectorVelocity));
                        SecondShellLoaded = false;

                    } else if (PlayerDeathScreenEnabled && OkFromDeathButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // Player Clicking "OK" button in Death Screen
                        ResetPlayerItems();
                        ResetPlayerMovement();
                        PlayerDeathScreenEnabled = false;
                        PreFightMenuEnabled = true;
                    } else if (PlayerWinScreenEnabled && EssaWinButton.getGlobalBounds().contains(
                            {static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                        // Player Clicking "Essa" button in Win Screen
                        ResetPlayerMovement();
                        PlayerWinScreenEnabled = false;
                        PreFightMenuEnabled = true;
                    }
                }

            }
            // --- Tracking for WASD and Num keys pressing ---
            if (ArenaFightScreenEnabled) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::W)
                        keyPressedW = true;
                    else if (event.key.code == sf::Keyboard::S)
                        keyPressedS = true;
                    else if (event.key.code == sf::Keyboard::A)
                        keyPressedA = true;
                    else if (event.key.code == sf::Keyboard::D)
                        keyPressedD = true;
                    else if (event.key.code == sf::Keyboard::Num1) {
                        shellType = 1;
                        setCurrentShellType();
                        CurrentShellIcon = APShellIconImage;
                        CurrentShellIcon.setPosition(1855, 387);
                    }
                    else if (event.key.code == sf::Keyboard::Num2) {
                        shellType = 2;
                        setCurrentShellType();
                        CurrentShellIcon = APCRShellIconImage;
                        CurrentShellIcon.setPosition(1855, 387);
                    }
                    else if (event.key.code == sf::Keyboard::Num3) {
                        shellType = 3;
                        setCurrentShellType();
                        CurrentShellIcon = HEShellIconImage;
                        CurrentShellIcon.setPosition(1855, 387);
                    }
                    else if (event.key.code == sf::Keyboard::Num4&&(CurrentRepairKitCount>0)&&(PlayerTracksDestroyed||PlayerEngineDestroyed)&&!PlayerUsingRepairKit) {
                        PlayerUsingRepairKit = true;
                        SetTimePoints(EndOfUsingRepairKit, std::chrono::milliseconds(1000));
                        CurrentRepairKitCount--;
                    }
                    else if(event.key.code == sf::Keyboard::Num5&&!PlayerUsingManualRepairKit&&!PlayerUsingRepairKit&&(PlayerTracksDestroyed||PlayerEngineDestroyed)){
                        PlayerUsingManualRepairKit = true;
                        SetTimePoints(EndOfUsingManualRepairKit, std::chrono::milliseconds(5000));
                    }
                    else if(event.key.code == sf::Keyboard::Num6&&!PlayerUsingExtraShield&&(CurrentExtraShieldCount>0)){
                        PlayerUsingExtraShield = true;
                        SetTimePoints(EndOfUsingExtraShield,std::chrono::milliseconds(15000));
                        CurrentPlayerResistanceRatio -= 0.5;
                        CurrentExtraShieldCount--;
                    }
                    else if(event.key.code == sf::Keyboard::Num7&&(CurrentMedKitCount>0)&&((PlayerCurrentHP/PlayerMaxHP)<0.75)){
                        PlayerCurrentHP+=(static_cast<float>(PlayerMaxHP*0.25));
                        if(PlayerCurrentHP>PlayerMaxHP) PlayerCurrentHP = PlayerMaxHP;
                        CurrentMedKitCount--;
                    }
                    else if(event.key.code == sf::Keyboard::Num8&&(CurrentEngineAcceleratorCount>0)&&!PlayerUsingEngineAccelerator){
                        PlayerUsingEngineAccelerator = true;
                        SetTimePoints(EndOfUsingEngineAccelerator, std::chrono::milliseconds(15000));
                        CurrentEngineAcceleratorCount--;
                    }
                    else if(event.key.code == sf::Keyboard::Num9&&(!PlayerUsingFuelProducer)&&((PlayerCurrentFuelLevel/PlayerMaxFuelLevel)<=0.01)){
                        PlayerUsingFuelProducer = true;
                        SetTimePoints(EndOfUsingFuelProducer, std::chrono::milliseconds(10000));
                    }
                    else if(event.key.code == sf::Keyboard::Num0&&(APShellCurrentCount==0)&&(APCRShellCurrentCount==0)&&(HEShellCurrentCount==0)&&!PlayerUsingAmmoProducer)
                        PlayerUsingAmmoProducer = true;
                        SetTimePoints(EndOfUsingAmmoProducer, std::chrono::milliseconds(10000));
                }
                // --- Tracking for WASD keys realising ---
                if (event.type == sf::Event::KeyReleased) {
                    if (event.key.code == sf::Keyboard::W)
                        keyPressedW = false;
                    if (event.key.code == sf::Keyboard::S)
                        keyPressedS = false;
                    if (event.key.code == sf::Keyboard::A)
                        keyPressedA = false;
                    if (event.key.code == sf::Keyboard::D)
                        keyPressedD = false;
                }
            }
        }






        // ------------------ Updating Player Position------------------------------
        if(ArenaFightScreenEnabled){

        // --- Player Tank Movement By WASD Keys --
        if(!PlayerTracksDestroyed&&!PlayerEngineDestroyed) {
            if ((PlayerCurrentFuelLevel > 0) && keyPressedW && !PlayerMovementUpBlocked) {
                PlayerPosition.y -= PlayerMovementSpeed.y;
            }
            if ((PlayerCurrentFuelLevel > 0) && keyPressedS && !PlayerMovementDownBlocked) {
                PlayerPosition.y += PlayerMovementSpeed.y;
            }
            if ((PlayerCurrentFuelLevel > 0) && keyPressedA && !PlayerMovementLeftBlocked) {
                PlayerPosition.x -= PlayerMovementSpeed.x;
                if (PlayerHorizontallyRight) {
                    // --- Rotating along Y axis ---
                    PlayerHorizontallyRight = false;
                    PlayerTankImage.setScale(-1.f, 1.f);
                }
            }
            if ((PlayerCurrentFuelLevel > 0) && keyPressedD && !PlayerMovementRightBlocked) {
                PlayerPosition.x += PlayerMovementSpeed.x;
                if (!PlayerHorizontallyRight) {
                    // --- Rotating along Y axis ---
                    PlayerHorizontallyRight = true;
                    PlayerTankImage.setScale(1.f, 1.f);
                }
            }
        }
            // --- Reducing boosted player speed when going in 2 directions
            if(PlayerCurrentFuelLevel>=0.01&&!PlayerEngineDestroyed&&!PlayerTracksDestroyed) {
                if (keyPressedW && (keyPressedD || keyPressedA)) PlayerPosition.y += (PlayerMovementSpeed.y -
                                                                                      (PlayerMovementSpeed.y) *
                                                                                      static_cast<float>(1 / sqrt(2)));
                if (keyPressedS && (keyPressedD || keyPressedA)) PlayerPosition.y -= (PlayerMovementSpeed.y -
                                                                                      (PlayerMovementSpeed.y) *
                                                                                      static_cast<float>(1 / sqrt(2)));
                if (keyPressedA && (keyPressedW || keyPressedS)) PlayerPosition.x += (PlayerMovementSpeed.x -
                                                                                      (PlayerMovementSpeed.x) *
                                                                                      static_cast<float>(1 / sqrt(2)));
                if (keyPressedD && (keyPressedW || keyPressedS)) PlayerPosition.x -= (PlayerMovementSpeed.x -
                                                                                      (PlayerMovementSpeed.x) *
                                                                                      static_cast<float>(1 / sqrt(2)));
            }

            // --- Resetting Player Movement Block Flags & Speed ---
            PlayerMovementLeftBlocked = false;
            PlayerMovementUpBlocked = false;
            PlayerMovementRightBlocked = false;
            PlayerMovementDownBlocked = false;
            PlayerMovementSpeed = DefaultPlayerMovementSpeed;

            // --- Player Engine Acceleration ---
            if(PlayerUsingEngineAccelerator) {
                PlayerMovementSpeed.x*=1.50;
                PlayerMovementSpeed.y*=1.50;
            }

            // --- FUEL Deduction caused by movement ---
            if((PlayerCurrentFuelLevel>0)&&!PlayerTracksDestroyed&&!PlayerEngineDestroyed&&(keyPressedW||keyPressedA||keyPressedD||keyPressedS)) PlayerCurrentFuelLevel -= PlayerCurrentFuelBurning;

        // --- Player Tank Hitting Arena Barrier ---
            if(PlayerPosition.y >= 1027)
                PlayerPosition.y -= PlayerMovementSpeed.y;
            if(PlayerPosition.y <= 54)
                PlayerPosition.y += PlayerMovementSpeed.y;
            if(PlayerPosition.x <= 65)
                PlayerPosition.x += PlayerMovementSpeed.x;
            if(PlayerPosition.x >= 1765)
                PlayerPosition.x -= PlayerMovementSpeed.x;

            PlayerTankImage.setPosition(PlayerPosition);     // <- Updating player Position
        // --------------------------------------------------------------

        // -------------- Updating player shells Position ------------------
            if (FirstShellLoaded) {
                FirstshellPosition = PlayerTankImage.getPosition();
            } else {
                FirstshellPosition += FirstshellVectorVelocity;
            }
            CurrentFirstShellLoaded.setPosition(FirstshellPosition);
            if (SecondShellLoaded) {
                SecondshellPosition = PlayerTankImage.getPosition();
            } else {
                SecondshellPosition += SecondshellVectorVelocity;
            }
            CurrentSecondShellLoaded.setPosition(SecondshellPosition);

        // Shells return to the tank, when they exceeded their range
        if(!FirstShellLoaded) {
            if (sqrt(pow(FirstShellFirePositionX - FirstshellPosition.x, 2) + pow(FirstShellFirePositionY - FirstshellPosition.y, 2)) >
                CurrentFirstShellRange) {
                FirstShellLoaded = true;
                setCurrentShellType();
            }
        }
        if(!SecondShellLoaded) {
            if (sqrt(pow(SecondShellFirePositionX - SecondshellPosition.x, 2) + pow(SecondShellFirePositionY - SecondshellPosition.y, 2)) >
                CurrentSecondShellRange) {
                SecondShellLoaded = true;
                setCurrentShellType();
            }
        }
        //---------------------------------------
        //-------------------------------TIME MANAGEMENT------------------------------
        // --- RELOAD TIMER - //
           if(PlayerReloading) {
                if(std::chrono::system_clock::now()>=EndOfPlayerReload) PlayerReloading = false;
           }
        // --- Repair Kit Usage Timer ---
            if(PlayerUsingRepairKit){
                if(std::chrono::system_clock::now()>=EndOfUsingRepairKit) {
                    PlayerUsingRepairKit = false;
                    PlayerTracksDestroyed = false;
                    PlayerEngineDestroyed = false;
                }
            }
        // --- Manual Repair Kit Usage Timer ---
            if(PlayerUsingManualRepairKit){
                if(std::chrono::system_clock::now()>=EndOfUsingManualRepairKit) {
                    PlayerUsingManualRepairKit = false;
                    PlayerTracksDestroyed = false;
                    PlayerEngineDestroyed = false;
                }
            }
            //---Extra Shield Timer ---
            if(PlayerUsingExtraShield){
                if(std::chrono::system_clock::now()>=EndOfUsingExtraShield){
                    PlayerUsingExtraShield = false;
                    CurrentPlayerResistanceRatio += 0.5;
                }
            }
            // --- Engine Accelerator Timer ---
            if(PlayerUsingEngineAccelerator){
                if(std::chrono::system_clock::now()>=EndOfUsingEngineAccelerator){
                    PlayerUsingEngineAccelerator = false;
                }
            }
            // --- Ammo Producer Timer ---
            if(PlayerUsingAmmoProducer){
                if(std::chrono::system_clock::now()>=EndOfUsingAmmoProducer){
                    PlayerUsingAmmoProducer = false;
                    APShellCurrentCount+=5;
                    APCRShellCurrentCount+=5;
                    HEShellCurrentCount+=5;
                }
            }
            // --- Fuel Producer Timer ---
            if(PlayerUsingFuelProducer){
                if(std::chrono::system_clock::now()>=EndOfUsingFuelProducer){
                    PlayerUsingFuelProducer = false;
                    PlayerCurrentFuelLevel+=static_cast<float>(0.45*PlayerMaxFuelLevel);
                }
            }
            // --- Explosion Radios Timer ---
            if(ExplosionRadiosVisible){
                if(std::chrono::system_clock::now()>=EndOfExplosionRadios){
                    ExplosionRadiosVisible = false;
                }
            }
            // --- Enemies Movement Cycle Timer ---
            if(EnemiesMoving){
                if(std::chrono::system_clock::now()>=EndOfEnemiesCycleMovement){
                    EnemiesMoving = false;
                }
            }
            // ---------- Checking if player's dead ------------
            if(PlayerCurrentHP<=0){
                wait(1);
                ArenaFightScreenEnabled = false;
                PlayerDeathScreenEnabled = true;
                ResetPlayerItems();
                setDeathXPGainedText();
                setCreditsGainedText();
            }
            // ---------------------------- Enemies Management ------------------------------------

            // --------- Function to pick a direction vector towards player (8 directions in total) -------------
            auto PickADirection2Player = [&](float EnemyTankSpeed, sf::Vector2f EnemyPosition) -> sf::Vector2f {
                return GetVectorVelocity(PlayerTankImage.getPosition().x,PlayerTankImage.getPosition().y,EnemyPosition.x,EnemyPosition.y,EnemyTankSpeed);
            };

            // --------- Function to pick a direction vector (8 directions in total) -------------
            auto PickADirectionVector = [&](float EnemyTankSpeed, sf::Vector2f EnemyPosition) -> sf::Vector2f {
                dis = std::uniform_int_distribution<>(1, 18);
                auto DirectionPicked = dis(gen);
                switch (DirectionPicked){
                    case 1:
                    case 2:
                        return {-EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2)),-EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2))};
                        break;
                    case 3:
                    case 4:
                        return {-EnemyTankSpeed*FPSspeedMultiplier,0};
                        break;
                    case 5:
                    case 6:
                        return {0,EnemyTankSpeed*FPSspeedMultiplier};
                        break;
                    case 7:
                    case 8:
                        return {0,-EnemyTankSpeed*FPSspeedMultiplier};
                        break;
                    case 9:
                    case 10:
                        return {EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2)),EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2))};
                        break;
                    case 11:
                    case 12:
                        return {-EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2)),EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2))};
                        break;
                    case 13:
                    case 14:
                        return {EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2)),-EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2))};
                        break;
                    case 15:
                        return {EnemyTankSpeed*FPSspeedMultiplier,0};
                    default:
                        return PickADirection2Player(EnemyTankSpeed,EnemyPosition);
                }
            };

            // --------- Function to pick an evasive direction vector (4 directions in total) -------------
            auto PickEvasiveDirectionVector = [&](float EnemyTankSpeed) -> sf::Vector2f {
                dis = std::uniform_int_distribution<>(1, 4);
                auto DirectionPicked = dis(gen);
                switch (DirectionPicked){
                    case 1:
                        return {-EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2)),-EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2))};
                        break;
                    case 2:
                        return {EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2)),-EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2))};
                        break;
                    case 3:
                        return {EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2)),EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2))};
                        break;
                    default:
                        return {-EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2)),EnemyTankSpeed*FPSspeedMultiplier*static_cast<float>(1/sqrt(2))};
                        break;
                } return {0,0};
            };

            // Enemies Movement // // Bots picks a new movement direction every new cycle, or if they hit an obstacle (They Move in cycles)
            if(!EnemiesMoving) {
                for(auto i = 0; i<EnemyTeam.size(); i++) {
                    if(!EnemyTeam[i].IsDead&&EnemyTeam[i].IsMobile&&EnemyTeam[i].CanShoot)
                    EnemyTeamSpeedVectors[i]=PickADirectionVector(EnemyTeam[i].TankSpeed.x,EnemyTeam[i].TankPosition);
                    else if(!EnemyTeam[i].IsDead&&EnemyTeam[i].IsMobile&&!EnemyTeam[i].CanShoot)
                        EnemyTeamSpeedVectors[i]=PickADirection2Player(EnemyTeam[i].TankSpeed.x, EnemyTeam[i].TankPosition);
                }
                EnemiesMoving=true;
                SetTimePoints(EndOfEnemiesCycleMovement, std::chrono::milliseconds(750));
            }

            AllEnemiesDead = true;
            for(auto i = 0; i<EnemyTeam.size(); i++){
                if(!EnemyTeam[i].IsDead) {
                    if(EnemyTeam[i].IsMobile) {
                        // --- Enemy Tank Hitting Arena Barrier ---
                        if (EnemyTeam[i].TankPosition.y >= 1027) {
                            EnemyTeam[i].TankPosition.y -= 3;
                            EnemyTeamSpeedVectors[i] = PickADirectionVector(EnemyTeam[i].TankSpeed.x,EnemyTeam[i].TankPosition);
                        } else if (EnemyTeam[i].TankPosition.y <= 54) {
                            EnemyTeam[i].TankPosition.y += 3;
                            EnemyTeamSpeedVectors[i] = PickADirectionVector(EnemyTeam[i].TankSpeed.x,EnemyTeam[i].TankPosition);
                        } else if (EnemyTeam[i].TankPosition.x <= 65) {
                            EnemyTeam[i].TankPosition.x += 3;
                            EnemyTeamSpeedVectors[i] = PickADirectionVector(EnemyTeam[i].TankSpeed.x,EnemyTeam[i].TankPosition);
                        } else if (EnemyTeam[i].TankPosition.x >= 1765) {
                            EnemyTeam[i].TankPosition.x -= 3;
                            EnemyTeamSpeedVectors[i] = PickADirectionVector(EnemyTeam[i].TankSpeed.x,EnemyTeam[i].TankPosition);
                        }

                        // --- Enemy Tanks Hiting obstacle fields ---
                        auto EnemyCurrentFieldType = GetThingCurrentFieldType(EnemyTeamTanksImages[i], MapFieldsByType);
                        if (EnemyCurrentFieldType == Field::Home || EnemyTeam[i].CanShoot&&EnemyCurrentFieldType == Field::Spikes ||
                                EnemyTeam[i].CanShoot&&EnemyCurrentFieldType == Field::Hole) {
                            auto ObstaclePosition = GetThingCurrentFieldPosition(EnemyTeamTanksImages[i],
                                                                                 MapFieldsImages);
                            if (EnemyTeam[i].TankPosition.x > ObstaclePosition.x) {
                                EnemyTeam[i].TankPosition.x += 3;
                            } else {
                                EnemyTeam[i].TankPosition.x -= 3;
                            }
                            if (EnemyTeam[i].TankPosition.y > ObstaclePosition.y) {
                                EnemyTeam[i].TankPosition.y += 3;
                            } else {
                                EnemyTeam[i].TankPosition.y -= 3;
                            }
                            if(EnemyTeam[i].CanShoot)
                            EnemyTeamSpeedVectors[i] = PickADirectionVector(EnemyTeam[i].TankSpeed.x,EnemyTeam[i].TankPosition);
                            else EnemyTeamSpeedVectors[i] = PickEvasiveDirectionVector(EnemyTeam[i].TankSpeed.x);
                        }
                    }

                    // --- Player Shell hitting enemy tank ---
                    if (!FirstShellLoaded) {
                        auto deltaX = CurrentFirstShellLoaded.getPosition().x - EnemyTeam[i].TankPosition.x;
                        auto deltaY = CurrentFirstShellLoaded.getPosition().y - EnemyTeam[i].TankPosition.y;
                        if (sqrt(deltaX * deltaX + deltaY * deltaY) <
                            (EnemyTeamTanksImages[i].getLocalBounds().height / 2)) {
                            EnemyTeam[i].TankCurrentHP -= CurrentFirstShellDamage;
                            FirstShellLoaded = true;
                        }
                    }
                    if (!SecondShellLoaded) {
                        auto deltaX = CurrentSecondShellLoaded.getPosition().x - EnemyTeam[i].TankPosition.x;
                        auto deltaY = CurrentSecondShellLoaded.getPosition().y - EnemyTeam[i].TankPosition.y;
                        if (sqrt(deltaX * deltaX + deltaY * deltaY) <
                            (EnemyTeamTanksImages[i].getLocalBounds().height / 2)) {
                            EnemyTeam[i].TankCurrentHP -= CurrentSecondShellDamage;
                            SecondShellLoaded = true;
                        }
                    }

                    // --- Updating Enemies HP Bar Texts ---
                    auto HPBarText = sf::Text(std::to_string(static_cast<int>(EnemyTeam[i].TankCurrentHP)), ArialFont,
                                              19);
                    HPBarText.setFillColor(sf::Color::White);
                    HPBarText.setStyle(sf::Text::Bold);
                    if(EnemyTeam[i].TankCurrentHP<10) HPBarText.setPosition(EnemyTeam[i].TankPosition + sf::Vector2f(-5, -60));
                    else if(EnemyTeam[i].TankCurrentHP<100) HPBarText.setPosition(EnemyTeam[i].TankPosition + sf::Vector2f(-15, -60));
                    else if(EnemyTeam[i].TankCurrentHP<1000) HPBarText.setPosition(EnemyTeam[i].TankPosition + sf::Vector2f(-18, -60));
                    else HPBarText.setPosition(EnemyTeam[i].TankPosition + sf::Vector2f(-25, -60));
                    EnemyTeamHPBarTexts[i] = HPBarText;

                    // --- Updating Enemies HP Bar Images ---
                    auto HPBarImage = sf::Sprite();
                    if((EnemyTeam[i].TankCurrentHP/EnemyTeam[i].TankMaxHP)<=0.21){
                        EnemyTeamHPBarImages[i] = EnemyHP20BarImage;
                    }else if((EnemyTeam[i].TankCurrentHP/EnemyTeam[i].TankMaxHP)<=0.41){
                        EnemyTeamHPBarImages[i] = EnemyHP40BarImage;
                    }else if((EnemyTeam[i].TankCurrentHP/EnemyTeam[i].TankMaxHP)<=0.61){
                        EnemyTeamHPBarImages[i] = EnemyHP60BarImage;
                    }else if((EnemyTeam[i].TankCurrentHP/EnemyTeam[i].TankMaxHP)<=0.81){
                        EnemyTeamHPBarImages[i] = EnemyHP80BarImage;
                    }
                    EnemyTeamHPBarImages[i].setPosition(EnemyTeam[i].TankPosition+sf::Vector2f(-38,-60));

                    if(EnemyTeam[i].CanShoot) {
                        // --- Enemies Firing ---
                        if ((EnemyTeam[i].ShellLoaded)&&!EnemyTeam[i].IsReloading) {
                            auto deltaX = PlayerTankImage.getPosition().x - EnemyTeam[i].TankPosition.x;
                            auto deltaY = PlayerTankImage.getPosition().y - EnemyTeam[i].TankPosition.y;
                            if (sqrt(deltaX * deltaX + deltaY * deltaY) - 50 < EnemyTeam[i].ShellRange) {
                                EnemyTeam[i].shellFiringPoint = EnemyTeam[i].TankPosition;
                                EnemyTeam[i].shellVectorVelocity = GetVectorVelocity(PlayerTankImage.getPosition().x,
                                                                                     PlayerTankImage.getPosition().y,
                                                                                     EnemyTeam[i].shellFiringPoint.x,
                                                                                     EnemyTeam[i].shellFiringPoint.y,
                                                                                     EnemyTeam[i].ShellVelocity *
                                                                                     FPSspeedMultiplier);
                                EnemyTeamShellsImages[i].setRotation(
                                        GetShellRotationAngle(EnemyTeam[i].shellVectorVelocity));
                                EnemyTeam[i].ShellLoaded = false;
                                SetTimePoints(EnemyTeam[i].EndOfEnemyReload, EnemyTeam[i].ReloadTime);
                                EnemyTeam[i].IsReloading = true;
                            }
                        }
                        // Applying Enemy Shell Vector Velocity
                        if (EnemyTeam[i].ShellLoaded) {
                            EnemyTeam[i].TankShellPosition = EnemyTeam[i].TankPosition;
                        } else {
                            EnemyTeam[i].TankShellPosition += EnemyTeam[i].shellVectorVelocity;
                        }

                        // Enemy shell returns to his tank when it has exceeded its range
                        if (!EnemyTeam[i].ShellLoaded) {
                            if (sqrt(pow(EnemyTeam[i].shellFiringPoint.x - EnemyTeam[i].TankShellPosition.x, 2) +
                                     pow(EnemyTeam[i].shellFiringPoint.y - EnemyTeam[i].TankShellPosition.y, 2)) >
                                EnemyTeam[i].ShellRange) {
                                EnemyTeam[i].ShellLoaded = true;
                                EnemyTeam[i].TankShellPosition = EnemyTeam[i].TankPosition;
                                EnemyTeam[i].shellVectorVelocity = sf::Vector2f(0, 0);
                            }
                        }

                        // --- Checking if enemy shell has hit player ---
                        if (!EnemyTeam[i].ShellLoaded) {
                            auto deltaX = EnemyTeamShellsImages[i].getPosition().x - PlayerTankImage.getPosition().x;
                            auto deltaY = EnemyTeamShellsImages[i].getPosition().y - PlayerTankImage.getPosition().y;
                            if (sqrt(deltaX * deltaX + deltaY * deltaY) <
                                (PlayerTankImage.getLocalBounds().height / 2)) {
                                PlayerCurrentHP -= EnemyTeam[i].ShellDamage*CurrentPlayerResistanceRatio;
                                EnemyTeam[i].ShellLoaded = true;
                                EnemyTeam[i].TankShellPosition = EnemyTeam[i].TankPosition;
                                EnemyTeam[i].shellVectorVelocity = sf::Vector2f(0, 0);
                            }
                        }

                        // -------Enemy Reload Timer ------
                        if(EnemyTeam[i].IsReloading){
                            if(std::chrono::system_clock::now()>=EnemyTeam[i].EndOfEnemyReload){
                                EnemyTeam[i].IsReloading = false;
                            }
                        }

                    }else EnemyTeam[i].TankShellPosition = EnemyTeam[i].TankPosition;

                    // --- Checking if rammer has rammed player ---
                    if(!EnemyTeam[i].CanShoot&&EnemyTeamTanksImages[i].getGlobalBounds().intersects(PlayerTankImage.getGlobalBounds())){
                        PlayerCurrentHP = 0;
                    }

                    // Enemy Tank Image Speed Vector Applied
                    EnemyTeam[i].TankPosition += EnemyTeamSpeedVectors[i];
                    // Tank Image Position Assignment
                    EnemyTeamTanksImages[i].setPosition(EnemyTeam[i].TankPosition);
                    // Enemy Shell Image Position Assignment
                    EnemyTeamShellsImages[i].setPosition(EnemyTeam[i].TankShellPosition);
                    // Checking if anybody is alive
                    AllEnemiesDead = false;

                    // --- Checking if enemy has died ---
                    if (EnemyTeam[i].TankCurrentHP <= 0) {
                        EnemiesKilledInRound++;
                        EnemyTeam[i].IsDead = true;
                    }

                    // --- Hiding enemy if he has died ---
                    if(EnemyTeam[i].IsDead) {
                        EnemyTeamLootsImages[i].setPosition(EnemyTeam[i].TankPosition.x,EnemyTeam[i].TankPosition.y);
                        EnemyTeam[i].ShellLoaded = true;
                        EnemyTeam[i].TankPosition = sf::Vector2f(2000,2000);
                        EnemyTeamTanksImages[i].setPosition(sf::Vector2f(2000,2000));
                        EnemyTeamHPBarTexts[i].setPosition(sf::Vector2f(2000,2000));
                        EnemyTeamShellsImages[i].setPosition(sf::Vector2f(2000,2000));
                        EnemyTeamHPBarImages[i].setPosition(sf::Vector2f(2000,2000));
                    }
                }
                // -- Checking if player has picked up loot ---
                if(PlayerTankImage.getGlobalBounds().intersects(EnemyTeamLootsImages[i].getGlobalBounds())){
                    TotalCreditsLooted+=static_cast<int>(EnemyTeam[i].TankMaxHP);
                    EnemyTeamLootsImages[i].setPosition(2000,2000);
                }
            }

            // ----------- End Point flag management ---------------
            if(AllEnemiesDead) CurrentEndFlagImage=GreenFlagImage;

            // --- Checking if player on green flag -> Player Has Won ---
            if(!PlayerDeathScreenEnabled&&AllEnemiesDead&&(PlayerTankImage.getGlobalBounds().intersects(CurrentEndFlagImage.getGlobalBounds()))){
                ArenaFightScreenEnabled = false;
                PlayerWinScreenEnabled = true;
                setCreditsGainedText();
                setXPGainedText();
                CurrentBattlefieldLevel++;
                UpdatePlayerItems();
                BattlefieldLevelText = sf::Text(std::to_string(CurrentBattlefieldLevel), ArialFont, 200);
                BattlefieldLevelText.setPosition({400, 200});
                BattlefieldLevelText.setFillColor(sf::Color::Black);
            }

            // ----------------- Handling Field Effects triggered by player ------------------------
            auto PlayerCurrentFieldType = GetThingCurrentFieldType(PlayerTankImage, MapFieldsByType);
            // --- Blocking Player Movement if he's on Home Obstacle
            if(PlayerCurrentFieldType==Field::Home){
                auto ObstaclePosition = GetThingCurrentFieldPosition(PlayerTankImage, MapFieldsImages);
                if(PlayerPosition.x>ObstaclePosition.x){
                    PlayerMovementLeftBlocked = true;
                }
                else{
                    PlayerMovementRightBlocked = true;
                }
                if(PlayerPosition.y>ObstaclePosition.y) {
                    PlayerMovementUpBlocked = true;
                }
                else{
                    PlayerMovementDownBlocked = true;
                }
            } // --- Player's Movement is Blocked while on Spikes and Damage is Dealt ---
            else if(PlayerCurrentFieldType==Field::Spikes){
                auto ObstaclePosition = GetThingCurrentFieldPosition(PlayerTankImage, MapFieldsImages);
                if(PlayerPosition.x>ObstaclePosition.x){
                    PlayerMovementLeftBlocked = true;
                }
                else{
                    PlayerMovementRightBlocked = true;
                }
                if(PlayerPosition.y>ObstaclePosition.y) {
                    PlayerMovementUpBlocked = true;
                }
                else{
                    PlayerMovementDownBlocked = true;
                }
                PlayerCurrentHP-=OriginalSpikeDamage*FPSspeedMultiplier;
            }
            // --- Player dies immediately if he's on hole ----
            else if(PlayerCurrentFieldType==Field::Hole){
                // --- Moving player's stuff outside game window to imitate that he has fallen into hole
                PlayerTankImage.setPosition(3000,3000);
                CurrentFirstShellLoaded.setPosition(3000,3000);
                CurrentSecondShellLoaded.setPosition(3000,3000);
                PlayerCurrentHP = 0;
            }
                // --- Player dies immediately if he's on mine ----
            else if(PlayerCurrentFieldType==Field::BigMine){
                // --- Moving player's stuff outside game window to imitate that he has been blew up
                PlayerCurrentHP = 0;
                MapFieldsByType[GetThingCurrentFieldIndex(PlayerTankImage)] = Field::Burned;
                MapFieldsBuilder(MapFieldsImages,EnumedImageMap,MapFieldsByType);
                window.clear();
                MapFieldsDisplayer();
                PlayerTankImage.setPosition(3000,3000);
                CurrentFirstShellLoaded.setPosition(3000,3000);
                CurrentSecondShellLoaded.setPosition(3000,3000);
            }
            // --- Player's movement speed is decreased by 75% while on sands
            else if(PlayerCurrentFieldType==Field::Sands){
                //auto ObstaclePosition = GetThingCurrentFieldPosition(PlayerTankImage);
                PlayerMovementSpeed.x *= 0.25;
                PlayerMovementSpeed.y *= 0.25;
            }
            // --- Player's movement speed is decreased by 50% while on swamps & his engine can die
            else if(PlayerCurrentFieldType==Field::Swamps){
                //auto ObstaclePosition = GetThingCurrentFieldPosition(PlayerTankImage);
                PlayerMovementSpeed.x *= 0.5;
                PlayerMovementSpeed.y *= 0.5;
                if(!PlayerEngineDestroyed&&(PlayerCurrentFuelLevel>0)&&(keyPressedW||keyPressedS||keyPressedA||keyPressedD)){
                    dis = std::uniform_int_distribution<>(0, static_cast<int>(800*(1/FPSspeedMultiplier)));
                    int numberPicked = dis(gen);
                    if(numberPicked==0){
                        PlayerEngineDestroyed = true;
                    }
                }
            }
            // --- Gives Some Ammo and turns into useless field
            else if(PlayerCurrentFieldType==Field::AmmoDropFull){
                APShellCurrentCount+=7;
                APCRShellCurrentCount+=7;
                HEShellCurrentCount+=7;
                CurrentRepairKitCount+=2;
                CurrentExtraShieldCount++;
                CurrentMedKitCount++;
                CurrentEngineAcceleratorCount++;
                MapFieldsByType[GetThingCurrentFieldIndex(PlayerTankImage)] = Field::AmmoDropEmpty;
                MapFieldsBuilder(MapFieldsImages,EnumedImageMap,MapFieldsByType);
            }
            // --- Gives Some HP and turns into useless field
            else if(PlayerCurrentFieldType==Field::WorkshopFull&&((PlayerCurrentHP/PlayerMaxHP)<1)){
                PlayerCurrentHP+=(static_cast<float>(PlayerMaxHP*0.5));
                if(PlayerCurrentHP>PlayerMaxHP) PlayerCurrentHP = PlayerMaxHP;
                MapFieldsByType[GetThingCurrentFieldIndex(PlayerTankImage)] = Field::WorkshopEmpty;
                MapFieldsBuilder(MapFieldsImages,EnumedImageMap,MapFieldsByType);
            }
            // --- Gives Some Fuel and turns into useless field
            else if(PlayerCurrentFieldType==Field::PetrolFull&&((PlayerCurrentFuelLevel/PlayerMaxFuelLevel)<1)){
                PlayerCurrentFuelLevel+=(static_cast<float>(PlayerMaxFuelLevel*0.5));
                if(PlayerCurrentFuelLevel>PlayerMaxFuelLevel) PlayerCurrentFuelLevel=PlayerMaxFuelLevel;
                MapFieldsByType[GetThingCurrentFieldIndex(PlayerTankImage)] = Field::PetrolEmpty;
                MapFieldsBuilder(MapFieldsImages,EnumedImageMap,MapFieldsByType);
            }
            // ---  Blowes Player Tracks up.
            else if(PlayerCurrentFieldType==Field::MineField){
                PlayerTracksDestroyed = true;
                MapFieldsByType[GetThingCurrentFieldIndex(PlayerTankImage)] = Field::BlowedMines;
                MapFieldsBuilder(MapFieldsImages,EnumedImageMap,MapFieldsByType);
            }
            // --- Forest gives player 50% damage taken reduction, but also damages his gun, making their range 50% worse and damage 30% worse and penetration 15% worse
            else if(PlayerCurrentFieldType==Field::Forest) {
                if(!ShellsEffectedByForest) {
                    CurrentPlayerResistanceRatio -= 0.5;
                    ShellsEffectedByForest = true;
                    PlayerGunDamaged = true;
                    EffectShellsSpecs(0.5,0.5,0.85);
                }
            }
            if(PlayerCurrentFieldType!=Field::Forest&&ShellsEffectedByForest) {
                    ShellsEffectedByForest = false;
                    PlayerGunDamaged = false;
                    ResetShellsSpecs();
                    CurrentPlayerResistanceRatio += 0.5;
            }

            // --------------------- Handling Fields triggered by shells---------------------------
            auto FirstShellCurrentFieldType = GetThingCurrentFieldType(CurrentFirstShellLoaded, MapFieldsByType);
            auto SecondShellCurrentFieldType = GetThingCurrentFieldType(CurrentSecondShellLoaded, MapFieldsByType);

            // Shells disappear when hitting home
            if(FirstShellCurrentFieldType==Field::Home){
                auto deltaX = CurrentFirstShellLoaded.getPosition().x - MapFieldsImages[GetThingCurrentFieldIndex(CurrentFirstShellLoaded)].getPosition().x;
                auto deltaY = CurrentFirstShellLoaded.getPosition().y - MapFieldsImages[GetThingCurrentFieldIndex(CurrentFirstShellLoaded)].getPosition().y;
                if(sqrt(deltaX * deltaX + deltaY * deltaY)<=63) FirstShellLoaded = true;
            }
            if(SecondShellCurrentFieldType==Field::Home){
                auto deltaX = CurrentSecondShellLoaded.getPosition().x - MapFieldsImages[GetThingCurrentFieldIndex(CurrentSecondShellLoaded)].getPosition().x;
                auto deltaY = CurrentSecondShellLoaded.getPosition().y - MapFieldsImages[GetThingCurrentFieldIndex(CurrentSecondShellLoaded)].getPosition().y;
                if(sqrt(deltaX * deltaX + deltaY * deltaY)<=63) SecondShellLoaded = true;
            }

            for(auto i = 0; i<EnemyTeam.size(); i++){
                if(!EnemyTeam[i].IsDead){
                    auto EnemyShellCurrentFieldType = GetThingCurrentFieldType(EnemyTeamShellsImages[i], MapFieldsByType);
                    if(EnemyShellCurrentFieldType==Field::Home){
                        auto deltaX = EnemyTeam[i].TankShellPosition.x - MapFieldsImages[GetThingCurrentFieldIndex(EnemyTeamShellsImages[i])].getPosition().x;
                        auto deltaY = EnemyTeam[i].TankShellPosition.y - MapFieldsImages[GetThingCurrentFieldIndex(EnemyTeamShellsImages[i])].getPosition().y;
                        if(sqrt(deltaX * deltaX + deltaY * deltaY)<=63) {
                            EnemyTeam[i].ShellLoaded = true;
                            EnemyTeam[i].TankShellPosition = EnemyTeam[i].TankPosition;
                            EnemyTeam[i].shellVectorVelocity = sf::Vector2f(0, 0);
                        }
                    }
                }
            }

            // Shells explode the Explosive Field turning it burned & disappear    !! Dealing damage to nearby
            if(FirstShellCurrentFieldType==Field::Explosives&&!FirstShellLoaded){
                MapFieldsByType[GetThingCurrentFieldIndex(CurrentFirstShellLoaded)] = Field::Burned;
                auto ExplosiveCenter = MapFieldsImages[GetThingCurrentFieldIndex(CurrentFirstShellLoaded)].getPosition();
                auto deltaX = PlayerTankImage.getPosition().x - ExplosiveCenter.x;
                auto deltaY = PlayerTankImage.getPosition().y - ExplosiveCenter.y;
                if(sqrt(deltaX * deltaX + deltaY * deltaY)<=250){
                    PlayerCurrentHP=0;
                }
                // --- Checking if enemy is within explosion radios ---
                for(auto i = 0; i<EnemyTeam.size(); i++){
                    if(!EnemyTeam[i].IsDead){
                        deltaX = EnemyTeam[i].TankPosition.x - ExplosiveCenter.x;
                        deltaY = EnemyTeam[i].TankPosition.y - ExplosiveCenter.y;
                        if(sqrt(deltaX * deltaX + deltaY * deltaY)<=250){
                            EnemyTeam[i].TankCurrentHP = 0;
                        }
                    }
                }
                CurrentFirstShellLoaded.setPosition(PlayerTankImage.getPosition());
                ExplosionRadiosImage.setPosition(ExplosiveCenter);
                SetTimePoints(EndOfExplosionRadios, std::chrono::milliseconds(1500));
                ExplosionRadiosVisible = true;
                MapFieldsBuilder(MapFieldsImages,EnumedImageMap,MapFieldsByType);
                FirstShellLoaded = true;
            }
            if(SecondShellCurrentFieldType==Field::Explosives&&!SecondShellLoaded){
                MapFieldsByType[GetThingCurrentFieldIndex(CurrentSecondShellLoaded)] = Field::Burned;
                auto ExplosiveCenter = MapFieldsImages[GetThingCurrentFieldIndex(CurrentSecondShellLoaded)].getPosition();
                auto deltaX = PlayerTankImage.getPosition().x - ExplosiveCenter.x;
                auto deltaY = PlayerTankImage.getPosition().y - ExplosiveCenter.y;
                if(sqrt(deltaX * deltaX + deltaY * deltaY)<=250){
                    PlayerCurrentHP=0;
                }
                // --- Checking if enemy is within explosion radios ---
                for(auto i = 0; i<EnemyTeam.size(); i++){
                    if(!EnemyTeam[i].IsDead){
                        deltaX = EnemyTeam[i].TankPosition.x - ExplosiveCenter.x;
                        deltaY = EnemyTeam[i].TankPosition.y - ExplosiveCenter.y;
                        if(sqrt(deltaX * deltaX + deltaY * deltaY)<=250){
                            EnemyTeam[i].TankCurrentHP = 0;
                        }
                    }
                }

                ExplosionRadiosImage.setPosition(ExplosiveCenter);
                SetTimePoints(EndOfExplosionRadios, std::chrono::milliseconds(1500));
                ExplosionRadiosVisible = true;
                CurrentSecondShellLoaded.setPosition(PlayerTankImage.getPosition());
                MapFieldsBuilder(MapFieldsImages,EnumedImageMap,MapFieldsByType);
                SecondShellLoaded = true;
            }

            //  -----------------------Updating Player Interface (HP,FUEL,SHELL COUNT,CONSUMABLES) ----------------------
            // --- HP Bar ---
            PlayerHP1String = std::string(std::to_string((int) PlayerCurrentHP) + " /\n " + std::to_string((int) PlayerMaxHP));
            PlayerHP1Text = sf::Text(sf::String(PlayerHP1String), ArialFont, 20);
            if (PlayerCurrentHP / PlayerMaxHP > 0.9) PlayerHP1Text.setFillColor(sf::Color(0, 100, 0));
            else if (PlayerCurrentHP / PlayerMaxHP > 0.70) PlayerHP1Text.setFillColor(sf::Color(144, 238, 144));
            else if (PlayerCurrentHP / PlayerMaxHP > 0.5) PlayerHP1Text.setFillColor(sf::Color(255, 255, 0));
            else if (PlayerCurrentHP / PlayerMaxHP > 0.25) PlayerHP1Text.setFillColor(sf::Color(255, 165, 0));
            else PlayerHP1Text.setFillColor(sf::Color(255, 0, 0));
            PlayerHP1Text.setPosition(1845, 70);

            PlayerHP2String = std::string(std::to_string((int) (PlayerCurrentHP / PlayerMaxHP * 100)) + "%");
            PlayerHP2Text = sf::Text(sf::String(PlayerHP2String), ArialFont, 30);
            if (PlayerCurrentHP / PlayerMaxHP > 0.9) PlayerHP2Text.setFillColor(sf::Color(0, 100, 0));
            else if (PlayerCurrentHP / PlayerMaxHP > 0.70) PlayerHP2Text.setFillColor(sf::Color(144, 238, 144));
            else if (PlayerCurrentHP / PlayerMaxHP > 0.5) PlayerHP2Text.setFillColor(sf::Color(255, 255, 0));
            else if (PlayerCurrentHP / PlayerMaxHP > 0.25) PlayerHP2Text.setFillColor(sf::Color(255, 165, 0));
            else PlayerHP2Text.setFillColor(sf::Color(255, 0, 0));
            if(PlayerCurrentHP / PlayerMaxHP >= 0.999) PlayerHP2Text.setPosition(1835, 40);
            else if(PlayerCurrentHP / PlayerMaxHP >= 0.1) PlayerHP2Text.setPosition(1842, 40);
            else PlayerHP2Text.setPosition(1847, 40);

            // --- Resistance Bar ---
            CurrentResistanceString = std::string(std::to_string((int)((1-CurrentPlayerResistanceRatio)*100)));
            CurrentResistanceText = sf::Text(sf::String(CurrentResistanceString),ArialFont,25);
            if(1-CurrentPlayerResistanceRatio>0.01) {
                CurrentResistanceText.setFillColor(sf::Color::Green);
                CurrentResistanceText.setPosition(1858, 130);
            }
            else {
                CurrentResistanceText.setFillColor(sf::Color::White);
                CurrentResistanceText.setPosition(1866, 130);
            }

            //  --- Fuel Bar---
            PlayerFuelString = std::string(std::to_string((int)(PlayerCurrentFuelLevel/PlayerMaxFuelLevel*100))+"%");
            PlayerFuelText = sf::Text(sf::String(PlayerFuelString),ArialFont,30);
            if (PlayerCurrentFuelLevel/PlayerMaxFuelLevel > 0.9) PlayerFuelText.setFillColor(sf::Color(0, 100, 0));
            else if (PlayerCurrentFuelLevel/PlayerMaxFuelLevel > 0.70) PlayerFuelText.setFillColor(sf::Color(144, 238, 144));
            else if (PlayerCurrentFuelLevel/PlayerMaxFuelLevel > 0.5) PlayerFuelText.setFillColor(sf::Color(255, 255, 0));
            else if (PlayerCurrentFuelLevel/PlayerMaxFuelLevel > 0.25) PlayerFuelText.setFillColor(sf::Color(255, 165, 0));
            else PlayerFuelText.setFillColor(sf::Color(255, 0, 0));
            if(PlayerCurrentFuelLevel/PlayerMaxFuelLevel >= 0.999) PlayerFuelText.setPosition(1835, 500);
            else if(PlayerCurrentFuelLevel/PlayerMaxFuelLevel >= 0.1) PlayerFuelText.setPosition(1842, 500);
            else PlayerFuelText.setPosition(1847, 500);

            PlayerFuel2String = std::string(std::to_string((int) PlayerCurrentFuelLevel) + " /\n " + std::to_string((int) PlayerMaxFuelLevel));
            PlayerFuel2Text = sf::Text(sf::String(PlayerFuel2String), ArialFont, 20);
            if (PlayerCurrentFuelLevel/PlayerMaxFuelLevel > 0.9) PlayerFuel2Text.setFillColor(sf::Color(0, 100, 0));
            else if (PlayerCurrentFuelLevel/PlayerMaxFuelLevel > 0.70) PlayerFuel2Text.setFillColor(sf::Color(144, 238, 144));
            else if (PlayerCurrentFuelLevel/PlayerMaxFuelLevel > 0.5) PlayerFuel2Text.setFillColor(sf::Color(255, 255, 0));
            else if (PlayerCurrentFuelLevel/PlayerMaxFuelLevel > 0.25) PlayerFuel2Text.setFillColor(sf::Color(255, 165, 0));
            else PlayerFuel2Text.setFillColor(sf::Color(255, 0, 0));
            PlayerFuel2Text.setPosition(1845, 550);

            //--- Ammo Bar------
            // -- AP --
            APShellCountText = sf::Text(sf::String(std::to_string(APShellCurrentCount)), ArialFont, 30);
            if(APShellCurrentCount==0){
                APShellCountText.setFillColor(sf::Color::Red);
                APShellCountText.setPosition(1845, 210);
            }
            else if (APShellCurrentCount <= 5) {
                APShellCountText.setFillColor(sf::Color(255, 165, 0));
                APShellCountText.setPosition(1845, 210);
            }
            else if (APShellCurrentCount <= 9) {
                APShellCountText.setFillColor(sf::Color(sf::Color::Yellow));
                APShellCountText.setPosition(1845, 210);
            }
            else {
                APShellCountText.setFillColor(sf::Color(0, 100, 0));
                APShellCountText.setPosition(1835, 210);
            }

            //-- APCR --
            APCRShellCountText = sf::Text(sf::String(std::to_string(APCRShellCurrentCount)), ArialFont, 30);
            if(APCRShellCurrentCount==0){
                APCRShellCountText.setFillColor(sf::Color::Red);
                APCRShellCountText.setPosition(1845, 260);
            }
            else if (APCRShellCurrentCount <= 5) {
                APCRShellCountText.setFillColor(sf::Color(255, 165, 0));
                APCRShellCountText.setPosition(1845, 260);
            }
            else if (APCRShellCurrentCount <= 9) {
                APCRShellCountText.setFillColor(sf::Color(sf::Color::Yellow));
                APCRShellCountText.setPosition(1845, 260);
            }
            else {
                APCRShellCountText.setFillColor(sf::Color(0, 100, 0));
                APCRShellCountText.setPosition(1835, 260);
            }

            // -- HE --
            HEShellCountText = sf::Text(sf::String(std::to_string(HEShellCurrentCount)), ArialFont, 30);
            if(HEShellCurrentCount==0){
                HEShellCountText.setFillColor(sf::Color::Red);
                HEShellCountText.setPosition(1845, 310);
            }
            else if (HEShellCurrentCount <= 5) {
                HEShellCountText.setFillColor(sf::Color(255, 165, 0));
                HEShellCountText.setPosition(1845, 310);
            }
            else if (HEShellCurrentCount <= 9) {
                HEShellCountText.setFillColor(sf::Color(sf::Color::Yellow));
                HEShellCountText.setPosition(1845, 310);
            }
            else {
                HEShellCountText.setFillColor(sf::Color(0, 100, 0));
                HEShellCountText.setPosition(1835, 310);
            }

            // --- Repair Kit ---
            RepairKitCountText = sf::Text(sf::String(std::to_string(CurrentRepairKitCount)), ArialFont, 30);
            if(CurrentRepairKitCount==0){
                RepairKitCountText.setFillColor(sf::Color::Red);
            }
            else if (CurrentRepairKitCount <= 3) {
                RepairKitCountText.setFillColor(sf::Color(sf::Color::Yellow));
            }
            else {
                RepairKitCountText.setFillColor(sf::Color(0, 100, 0));
            }
            if(CurrentRepairKitCount<=9){
                RepairKitCountText.setPosition(1835, 695);
            }else RepairKitCountText.setPosition(1827, 695);

            // --- Extra Shield ---
            ExtraShieldCountText = sf::Text(sf::String(std::to_string(CurrentExtraShieldCount)), ArialFont, 30);
            if(CurrentExtraShieldCount==0){
                ExtraShieldCountText.setFillColor(sf::Color::Red);
            }
            else if (CurrentExtraShieldCount <= 2) {
                ExtraShieldCountText.setFillColor(sf::Color(sf::Color::Yellow));
            }
            else {
                ExtraShieldCountText.setFillColor(sf::Color(0, 100, 0));
            }
            if(CurrentExtraShieldCount<=9){
                ExtraShieldCountText.setPosition(1835, 796);
            }else ExtraShieldCountText.setPosition(1827, 796);

            // --- Med Kit ---
            MedKitCountText = sf::Text(sf::String(std::to_string(CurrentMedKitCount)), ArialFont, 30);
            if(CurrentMedKitCount==0){
                MedKitCountText.setFillColor(sf::Color::Red);
            }
            else if (CurrentMedKitCount <= 2) {
                MedKitCountText.setFillColor(sf::Color(sf::Color::Yellow));
            }
            else {
                MedKitCountText.setFillColor(sf::Color(0, 100, 0));
            }
            if(CurrentMedKitCount<=9){
                MedKitCountText.setPosition(1835, 847);
            }else MedKitCountText.setPosition(1827, 847);

            // --- Engine Accelerator ---
            EngineAcceleratorCountText = sf::Text(sf::String(std::to_string(CurrentEngineAcceleratorCount)), ArialFont, 30);
            if(CurrentEngineAcceleratorCount==0){
                EngineAcceleratorCountText.setFillColor(sf::Color::Red);
            }
            else if (CurrentEngineAcceleratorCount <= 2) {
                EngineAcceleratorCountText.setFillColor(sf::Color(sf::Color::Yellow));
            }
            else {
                EngineAcceleratorCountText.setFillColor(sf::Color(0, 100, 0));
            }
            if(CurrentEngineAcceleratorCount<=9){
                EngineAcceleratorCountText.setPosition(1835, 899);
            }else EngineAcceleratorCountText.setPosition(1827, 899);
        }

        if(MainMenuScreenEnabled) DrawMainMenuScreen();
        if(OptionsScreenEnabled) DrawOptionsScreen();
        if(QuitingScreenEnabled) DrawQuitingScreen();
        if(PlayOptionSelectorEnabled) DrawPlayOptionSelector();
        if(PreFightMenuEnabled) DrawPreFightMenu();
        if(UpgradeOptionsScreenEnabled) DrawUpgradeOptionsScreen();
        if(ArenaFightScreenEnabled) DrawArenaFight();
        if(PlayerWinScreenEnabled) DrawPlayerWonScreen();
        if(PlayerDeathScreenEnabled) DrawPlayerDeathScreen();
        if(TechTreeUpgradeScreenEnabled) DrawTechTreeScreen();
        if(UpgradeModulesScreenEnabled) DrawModulesUpgradeScreen();
        if(EquipmentStoreScreenEnabled) DrawEquipmentStore();
        if(WarningEnabled) DrawWarning();
        if(TutorialEnabled) DrawTutorial();
        window.display();
    }
}