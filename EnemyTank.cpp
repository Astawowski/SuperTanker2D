#include <SFML/Graphics.hpp>
#include <vector>
#include "EnemyTankType.h"
#include "EnemyTank.h"
#include <iostream>
#include <set>

auto GetEnemyTankTexture (int const Tier, EnemyTankType const tankType, std::vector<sf::Texture>& TankImages) -> sf::Texture {
    if(Tier==1) return TankImages[0];
    if(Tier==2) return TankImages[1];
    if(Tier==3&&tankType==EnemyTankType::MT) return TankImages[2];
    if(Tier==3&&tankType==EnemyTankType::TD) return TankImages[3];
    if(tankType==EnemyTankType::RM){
        if(Tier<=4) return TankImages[16];
        else return TankImages[17];
    }
    auto base = 4+(Tier-4)*3;
    auto TierDependent = int();
    switch (tankType){
        case EnemyTankType::MT:
            TierDependent = 0;
            break;
        case EnemyTankType::HT:
            TierDependent = 1;
            break;
        case EnemyTankType::TD:
            TierDependent = 2;
            break;
    }
    return TankImages[base+TierDependent];
}

auto GenerateEnemies (int const CurrentBattleFieldLevel) -> std::vector<EnemyTank> {


    // ----------------ENEMY TANK ICONS (USSR)---------------
    //--- BT-2 Icon - Tier I - Light Tank (LT)---
    auto BT2TankIcon = sf::Texture();
    if(!BT2TankIcon.loadFromFile("../C++GameData/textures/BT-2TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }

    //--- BT-5 Icon - Tier II - Light Tank (LT)---
    auto BT5TankIcon = sf::Texture();
    if(!BT5TankIcon.loadFromFile("../C++GameData/textures/BT-5TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- T-34-76 Icon - Tier III - Medium Tank (MT)---
    auto T3476TankIcon = sf::Texture();
    if(!T3476TankIcon.loadFromFile("../C++GameData/textures/T-34-76TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- SU-76M Icon - Tier III - Tank Destroyer (TD)---
    auto SU76MTankIcon = sf::Texture();
    if(!SU76MTankIcon.loadFromFile("../C++GameData/textures/SU-76MTankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- T-34-85 Icon - Tier IV - Medium Tank (MT)---
    auto T3485TankIcon = sf::Texture();
    if(!T3485TankIcon.loadFromFile("../C++GameData/textures/T-34-85TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- KV-1 Icon - Tier IV - Heavy Tank (HT)---
    auto KV1TankIcon = sf::Texture();
    if(!KV1TankIcon.loadFromFile("../C++GameData/textures/KV-1TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- SU-85 Icon - Tier IV - Tank Destroyer (TD)---
    auto SU85TankIcon = sf::Texture();
    if(!SU85TankIcon.loadFromFile("../C++GameData/textures/SU-85TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- T-44 Icon - Tier V - Medium Tank (MT)---
    auto T44TankIcon = sf::Texture();
    if(!T44TankIcon.loadFromFile("../C++GameData/textures/T-44TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- IS-1 Icon - Tier V - Heavy Tank (HT)---
    auto IS1TankIcon = sf::Texture();
    if(!IS1TankIcon.loadFromFile("../C++GameData/textures/IS-1TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- SU-122-44 Icon - Tier V - Tank Destroyer (TD)---
    auto SU12244TankIcon = sf::Texture();
    if(!SU12244TankIcon.loadFromFile("../C++GameData/textures/SU-122-44TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- T-54 Icon - Tier VI - Medium Tank (MT)---
    auto T54TankIcon = sf::Texture();
    if(!T54TankIcon.loadFromFile("../C++GameData/textures/T-54TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- IS-3 Icon - Tier VI - Heavy Tank (HT)---
    auto IS3TankIcon = sf::Texture();
    if(!IS3TankIcon.loadFromFile("../C++GameData/textures/IS-3TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- SU-152 Icon - Tier VI - Tank Destroyer (TD)---
    auto SU152TankIcon = sf::Texture();
    if(!SU152TankIcon.loadFromFile("../C++GameData/textures/SU-152TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- Obj. 140 Icon - Tier VII - Medium Tank (MT)---
    auto Obj140TankIcon = sf::Texture();
    if(!Obj140TankIcon.loadFromFile("../C++GameData/textures/Obj140TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- IS-7 Icon - Tier VII - Heavy Tank (HT)---
    auto IS7TankIcon = sf::Texture();
    if(!IS7TankIcon.loadFromFile("../C++GameData/textures/IS-7TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    //--- Obj. 268 Icon - Tier VII - Tank Destroyer (TD)---
    auto Obj268TankIcon = sf::Texture();
    if(!Obj268TankIcon.loadFromFile("../C++GameData/textures/Obj268TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    // --- Rammer 1st. Tank -  Tier I to IV --- (RM)---
    auto Rammer1TankIcon = sf::Texture();
    if(!Rammer1TankIcon.loadFromFile("../C++GameData/textures/Rammer1TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }


    // --- Rammer 2st. Tank -  Tier V to VII --- (RM)---
    auto Rammer2TankIcon = sf::Texture();
    if(!Rammer2TankIcon.loadFromFile("../C++GameData/textures/Rammer2TankIcon.png")){
        std::cout << "Couldn't load game texture";
        return {};
    }




    auto EnemyTankImagesMap = std::vector<sf::Texture>{ BT2TankIcon, BT5TankIcon,
                                                        T3476TankIcon, SU76MTankIcon,
                                                        T3485TankIcon, KV1TankIcon, SU85TankIcon,
                                                        T44TankIcon,IS1TankIcon, SU12244TankIcon,
                                                        T54TankIcon, IS3TankIcon,SU152TankIcon,
                                                        Obj140TankIcon, IS7TankIcon, Obj268TankIcon,
                                                        Rammer1TankIcon, Rammer2TankIcon};



    // --- Tank Type Databases ---
    // --- RM  (Rammer Tank) ---
    auto RM_Speed = sf::Vector2f(0.95,0.95);
    auto RM_HP = float(135);
    auto RM_Damage = float(0);
    auto RM_ShellType = int(1);
    auto RM_ShellRange = float(0);
    auto RM_ShellVelocity = float(0);
    auto RM_ReloadTime = std::chrono::milliseconds(0);


    // --- MT  (Medium Tank) ---
    auto MT_Speed = sf::Vector2f(1,1);
    auto MT_HP = float(75);
    auto MT_Damage = float(20);
    auto MT_ShellType = int(2);
    auto MT_ShellRange = float(800);
    auto MT_ShellVelocity = float(2.25);
    auto MT_ReloadTime = std::chrono::milliseconds(3000);

    // --- HT  (Heavy Tank) ---
    auto HT_Speed = sf::Vector2f(0.5,0.5);
    auto HT_HP = float(200);
    auto HT_Damage = float(30);
    auto HT_ShellType = int(1);
    auto HT_ShellRange = float(800);
    auto HT_ShellVelocity = float(1.5);
    auto HT_ReloadTime = std::chrono::milliseconds(3400);

    // --- TD  (Tank Destroyer) ---
    auto TD_Speed = sf::Vector2f(0,0);
    auto TD_HP = float(100);
    auto TD_Damage = float(40);
    auto TD_ShellType = int(3);
    auto TD_ShellRange = float(1200);
    auto TD_ShellVelocity = float(3.25);
    auto TD_ReloadTime = std::chrono::milliseconds(4000);


    auto EnemyTeam = std::vector<EnemyTank>();
    auto StartPositionsFor1 = std::vector<sf::Vector2f>{ sf::Vector2f(1739,395) };
    auto StartPositionsFor2 = std::vector<sf::Vector2f>{sf::Vector2f(1739,395), sf::Vector2f(1739,695)};
    auto StartPositionsFor3 = std::vector<sf::Vector2f>{sf::Vector2f(1739,395), sf::Vector2f(1739,845), sf::Vector2f(1739,695)};
    auto StartPositionsFor4 = std::vector<sf::Vector2f>{sf::Vector2f(1739,395), sf::Vector2f(1739,245), sf::Vector2f(1739,695), sf::Vector2f(1739,845) };
    auto StartPositionsFor5 = std::vector<sf::Vector2f>{sf::Vector2f(1739,395), sf::Vector2f(1739,245), sf::Vector2f(1739,95), sf::Vector2f(1739,695), sf::Vector2f(1739,845) };
    auto StartPositionsFor6 = std::vector<sf::Vector2f>{sf::Vector2f(1739,395), sf::Vector2f(1739,245), sf::Vector2f(1739,995), sf::Vector2f(1739,695), sf::Vector2f(1739,845), sf::Vector2f(1739,95) };


    // --- Add MT ---
    auto AddMTTank = [&](int Tier, sf::Vector2f StartPosition) -> EnemyTank {
        return (EnemyTank(false,EnemyTankType::MT,true,StartPosition,MT_Speed,MT_HP*Tier,MT_HP*Tier,MT_ShellVelocity,MT_ShellRange,MT_Damage*Tier,
                                      GetEnemyTankTexture(Tier,EnemyTankType::MT,EnemyTankImagesMap),StartPosition,MT_ShellType,true,true,sf::Vector2f(0,0),
                                      sf::Vector2f(0,0),false, MT_ReloadTime, std::chrono::time_point<std::chrono::system_clock>()));
    };

    // --- Add HT ---
    auto AddHTTank = [&](int Tier, sf::Vector2f StartPosition) -> EnemyTank {
        return (EnemyTank(false,EnemyTankType::HT,true,StartPosition,HT_Speed,HT_HP*Tier,HT_HP*Tier,HT_ShellVelocity,HT_ShellRange,HT_Damage*Tier,
                          GetEnemyTankTexture(Tier,EnemyTankType::HT,EnemyTankImagesMap),StartPosition,HT_ShellType,true,true,sf::Vector2f(0,0),
                          sf::Vector2f(0,0),false, HT_ReloadTime, std::chrono::time_point<std::chrono::system_clock>()));
    };

    // --- Add TD ---
    auto AddTDTank = [&](int Tier, sf::Vector2f StartPosition) -> EnemyTank {
        return (EnemyTank(false,EnemyTankType::TD,false,StartPosition,TD_Speed,TD_HP*Tier,TD_HP*Tier,TD_ShellVelocity,TD_ShellRange,TD_Damage*Tier,
                          GetEnemyTankTexture(Tier,EnemyTankType::TD,EnemyTankImagesMap),StartPosition,TD_ShellType,true,true,sf::Vector2f(0,0),
                          sf::Vector2f(0,0),false, TD_ReloadTime, std::chrono::time_point<std::chrono::system_clock>()));
    };

    // --- Add RM ---
    auto AddRMTank = [&](int Tier, sf::Vector2f StartPosition) -> EnemyTank {
        return (EnemyTank(false,EnemyTankType::RM,true,StartPosition,RM_Speed,RM_HP*Tier,RM_HP*Tier,RM_ShellVelocity,RM_ShellRange,RM_Damage*Tier,
                          GetEnemyTankTexture(Tier,EnemyTankType::RM,EnemyTankImagesMap),StartPosition,RM_ShellType,true,false,sf::Vector2f(0,0),
                          sf::Vector2f(0,0),false, RM_ReloadTime, std::chrono::time_point<std::chrono::system_clock>()));
    };






    // C.B.F.L <= 6

    if(CurrentBattleFieldLevel==666){
        EnemyTeam.push_back(AddRMTank(3,StartPositionsFor1[0]));
        return EnemyTeam;
    }
    if(CurrentBattleFieldLevel<=6){
        auto MaxTier = 1+(CurrentBattleFieldLevel-1)/3;
        std::vector<sf::Vector2f> GetPositionsFrom;
        if(CurrentBattleFieldLevel==1||CurrentBattleFieldLevel==4) GetPositionsFrom = StartPositionsFor1;
        else if(CurrentBattleFieldLevel==2||CurrentBattleFieldLevel==5) GetPositionsFrom = StartPositionsFor2;
        else GetPositionsFrom = StartPositionsFor3;

        for( auto i = 0; i<(CurrentBattleFieldLevel-3*((CurrentBattleFieldLevel-1)/3)); i++){
            EnemyTeam.push_back(EnemyTank(false,EnemyTankType::MT,true,GetPositionsFrom[i],MT_Speed,MT_HP*MaxTier,MT_HP*MaxTier,MT_ShellVelocity,MT_ShellRange,MT_Damage*MaxTier,
                                          GetEnemyTankTexture(MaxTier,EnemyTankType::MT,EnemyTankImagesMap),GetPositionsFrom[i],MT_ShellType,true,true,sf::Vector2f(0,0),
                                          sf::Vector2f(0,0),false, MT_ReloadTime, std::chrono::time_point<std::chrono::system_clock>()));
        }
        return EnemyTeam;
    }else if(CurrentBattleFieldLevel<=10){
        switch (CurrentBattleFieldLevel){
            case 7:
                EnemyTeam.push_back(AddMTTank(3,StartPositionsFor3[0]));
                EnemyTeam.push_back(AddMTTank(2,StartPositionsFor3[1]));
                EnemyTeam.push_back(AddRMTank(3,StartPositionsFor3[2]));
                break;
            case 8:
                EnemyTeam.push_back(AddMTTank(3,StartPositionsFor4[0]));
                EnemyTeam.push_back(AddMTTank(2,StartPositionsFor4[1]));
                EnemyTeam.push_back(AddRMTank(3,StartPositionsFor4[2]));
                EnemyTeam.push_back(AddMTTank(3,StartPositionsFor4[3]));
                break;
            case 9:
                EnemyTeam.push_back(AddMTTank(3,StartPositionsFor4[0]));
                EnemyTeam.push_back(AddTDTank(3,StartPositionsFor4[1]));
                EnemyTeam.push_back(AddRMTank(3,StartPositionsFor4[2]));
                EnemyTeam.push_back(AddMTTank(3,StartPositionsFor4[3]));
                break;
            case 10:
                EnemyTeam.push_back(AddMTTank(3,StartPositionsFor5[0]));
                EnemyTeam.push_back(AddTDTank(3,StartPositionsFor5[1]));
                EnemyTeam.push_back(AddRMTank(3,StartPositionsFor5[2]));
                EnemyTeam.push_back(AddMTTank(3,StartPositionsFor5[3]));
                EnemyTeam.push_back(AddTDTank(3,StartPositionsFor5[4]));
                break;
        }
        return EnemyTeam;
    }

    auto Exclusion1 = std::set<int>{15,20,21,26,27,28,33,34,35,36};
    auto Exclusion2 = std::set<int>{15,20,26,33};
    auto Exclusion3 = std::set<int>{21,27,34};
    auto Exclusion4 = std::set<int>{28,35};

    if(Exclusion1.find(CurrentBattleFieldLevel)==Exclusion1.end()){
        auto a = (CurrentBattleFieldLevel-11)/4;
        auto TierMax = a+4;
        auto c = (CurrentBattleFieldLevel-11)%4;
        switch (c) {
            case 0:
                EnemyTeam.push_back(AddMTTank(TierMax,StartPositionsFor3[0]));
                EnemyTeam.push_back(AddMTTank(TierMax-1,StartPositionsFor3[1]));
                EnemyTeam.push_back(AddRMTank(TierMax,StartPositionsFor3[2]));
                break;
            case 1:
                EnemyTeam.push_back(AddMTTank(TierMax,StartPositionsFor4[0]));
                EnemyTeam.push_back(AddMTTank(TierMax-1,StartPositionsFor4[1]));
                EnemyTeam.push_back(AddRMTank(TierMax,StartPositionsFor4[2]));
                EnemyTeam.push_back(AddMTTank(TierMax-1,StartPositionsFor4[3]));
                break;
            case 2:
                EnemyTeam.push_back(AddMTTank(TierMax,StartPositionsFor3[0]));
                EnemyTeam.push_back(AddHTTank(TierMax,StartPositionsFor3[1]));
                EnemyTeam.push_back(AddRMTank(TierMax,StartPositionsFor3[2]));
                break;
            case 3:
                EnemyTeam.push_back(AddMTTank(TierMax,StartPositionsFor4[0]));
                EnemyTeam.push_back(AddHTTank(TierMax,StartPositionsFor4[1]));
                EnemyTeam.push_back(AddRMTank(TierMax,StartPositionsFor4[2]));
                EnemyTeam.push_back(AddMTTank(TierMax-1,StartPositionsFor4[3]));
                break;
        }
        return EnemyTeam;
    }else if(Exclusion2.find(CurrentBattleFieldLevel)!=Exclusion2.end()){
        auto TierMax = int();
        switch (CurrentBattleFieldLevel){
            case 15:
                TierMax=4;
                break;
            case 20:
                TierMax=5;
                break;
            case 26:
                TierMax=6;
                break;
            case 33:
                TierMax=7;
                break;
        }
        EnemyTeam.push_back(AddMTTank(TierMax,StartPositionsFor4[0]));
        EnemyTeam.push_back(AddHTTank(TierMax,StartPositionsFor4[1]));
        EnemyTeam.push_back(AddRMTank(TierMax,StartPositionsFor4[2]));
        EnemyTeam.push_back(AddTDTank(TierMax,StartPositionsFor4[3]));
        return EnemyTeam;
    }else if(Exclusion3.find(CurrentBattleFieldLevel)!=Exclusion3.end()){
        auto TierMax = int();
        switch (CurrentBattleFieldLevel){
            case 21:
                TierMax=5;
                break;
            case 27:
                TierMax=6;
                break;
            case 34:
                TierMax=7;
                break;
        }
        EnemyTeam.push_back(AddMTTank(TierMax,StartPositionsFor5[0]));
        EnemyTeam.push_back(AddHTTank(TierMax,StartPositionsFor5[1]));
        EnemyTeam.push_back(AddRMTank(TierMax,StartPositionsFor5[2]));
        EnemyTeam.push_back(AddTDTank(TierMax,StartPositionsFor5[3]));
        EnemyTeam.push_back(AddMTTank(TierMax,StartPositionsFor5[4]));
        return EnemyTeam;
    }else if(Exclusion4.find(CurrentBattleFieldLevel)!=Exclusion4.end()){
        auto TierMax = int();
        switch (CurrentBattleFieldLevel){
            case 28:
                TierMax=6;
                break;
            case 35:
                TierMax=7;
                break;
        }
        EnemyTeam.push_back(AddMTTank(TierMax,StartPositionsFor6[0]));
        EnemyTeam.push_back(AddHTTank(TierMax,StartPositionsFor6[1]));
        EnemyTeam.push_back(AddRMTank(TierMax,StartPositionsFor6[2]));
        EnemyTeam.push_back(AddTDTank(TierMax,StartPositionsFor6[3]));
        EnemyTeam.push_back(AddTDTank(TierMax,StartPositionsFor6[4]));
        EnemyTeam.push_back(AddMTTank(TierMax,StartPositionsFor6[5]));
        return EnemyTeam;
    }else {
        auto TierMax = 7;
        EnemyTeam.push_back(AddHTTank(TierMax, StartPositionsFor6[0]));
        EnemyTeam.push_back(AddHTTank(TierMax, StartPositionsFor6[1]));
        EnemyTeam.push_back(AddRMTank(TierMax, StartPositionsFor6[2]));
        EnemyTeam.push_back(AddTDTank(TierMax, StartPositionsFor6[3]));
        EnemyTeam.push_back(AddTDTank(TierMax, StartPositionsFor6[4]));
        EnemyTeam.push_back(AddTDTank(TierMax, StartPositionsFor6[5]));
        return EnemyTeam;
    }
}


