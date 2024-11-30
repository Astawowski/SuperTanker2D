#include <random>
#include "Field.h"
#include <set>
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>

// --- Function for map random generating ---
auto MapGenerator (std::vector<Field>& MapFieldsByType) -> void {

    // Randomizer engine
    auto rd = std::random_device();
    auto gen = std::mt19937(rd());
    auto dis = std::uniform_int_distribution<>();

    // Fields in safe zone (near spawns)
    auto ExcludedForAllFields = std::set<int>{11,23,71,83,24,36,37,48,35,47,46,59};

    // Resetting to default
    MapFieldsByType = std::vector<Field>(84,Field::Standard);

    // Generating one Petrol Station field
    dis = std::uniform_int_distribution<>(0, 31);   // Petrol Station, Ammo Drop and Workshop can only be on one of certain 32 fields
    auto PetrolIndex = dis(gen);                    // Obviously they can not share their field   (Only one of each on one field)
    // Setting Petrol Station Field accordingly
    if(PetrolIndex<8) PetrolIndex+=2;
    else if(PetrolIndex<16) PetrolIndex+=6;
    else if(PetrolIndex<24) PetrolIndex+=46;
    else PetrolIndex+=50;
    MapFieldsByType[PetrolIndex] = Field::PetrolFull;


    // Generating one Ammo Drop field
    auto AmmoDropIndex = dis(gen);
    if(AmmoDropIndex<8) AmmoDropIndex+=2;
    else if(AmmoDropIndex<16) AmmoDropIndex+=6;
    else if(AmmoDropIndex<24) AmmoDropIndex+=46;
    else AmmoDropIndex+=50;
    while(MapFieldsByType[AmmoDropIndex]!=Field::Standard){
        AmmoDropIndex = dis(gen);
        if(AmmoDropIndex<8) AmmoDropIndex+=2;
        else if(AmmoDropIndex<16) AmmoDropIndex+=6;
        else if(AmmoDropIndex<24) AmmoDropIndex+=46;
        else AmmoDropIndex+=50;
    }
    // Setting Ammo Drop Field accordingly
    MapFieldsByType[AmmoDropIndex] = Field::AmmoDropFull;


    // Generating one Workshop field
    auto WorkshopIndex = dis(gen);
    if(WorkshopIndex<8) WorkshopIndex+=2;
    else if(WorkshopIndex<16) WorkshopIndex+=6;
    else if(WorkshopIndex<24) WorkshopIndex+=46;
    else WorkshopIndex+=50;
    while(MapFieldsByType[WorkshopIndex]!=Field::Standard) {
        WorkshopIndex = dis(gen);
        if(WorkshopIndex<8) WorkshopIndex+=2;
        else if(WorkshopIndex<16) WorkshopIndex+=6;
        else if(WorkshopIndex<24) WorkshopIndex+=46;
        else WorkshopIndex+=50;
    }
    // Setting Workshop Field accordingly
    MapFieldsByType[WorkshopIndex] = Field::WorkshopFull;

    // Generating Explosives Field  (2 fields on map, Can not be in some places near spawns)
    dis = std::uniform_int_distribution<>(0, 83);
    auto ExcludedForExplosivesField = std::set<int>{12,25,38,49,60,23,34,45,58,71};
    for(auto i = 0; i<2; i++) {
        auto ExplosiveFieldIndex = dis(gen);
        while( (ExcludedForAllFields.find(ExplosiveFieldIndex)!=ExcludedForAllFields.end()) ||
               (ExcludedForExplosivesField.find(ExplosiveFieldIndex)!=ExcludedForExplosivesField.end()) ||
               MapFieldsByType[ExplosiveFieldIndex]!=Field::Standard){
            ExplosiveFieldIndex = dis(gen);
        }
        MapFieldsByType[ExplosiveFieldIndex] = Field::Explosives;
    }

    // Generating House Obstacles  (There will be max 10 houses)
    for(auto i = 0; i<10; i++){
        auto HouseFieldIndex = dis(gen);
        if( (ExcludedForAllFields.find(HouseFieldIndex)!=ExcludedForAllFields.end()) ||
            MapFieldsByType[HouseFieldIndex]!=Field::Standard ) continue;
        else MapFieldsByType[HouseFieldIndex] = Field::Home;
    }

    // Generating Tank Spikes Obstacles  (There will be max 6 spikes)
    for(auto i = 0; i<6; i++){
        auto SpikesFieldIndex = dis(gen);
        if( (ExcludedForAllFields.find(SpikesFieldIndex)!=ExcludedForAllFields.end()) ||
            MapFieldsByType[SpikesFieldIndex]!=Field::Standard ) continue;
        else MapFieldsByType[SpikesFieldIndex] = Field::Spikes;
    }

    // Generating Forests   (There will be max 5 forests)
    for(auto i = 0; i<5; i++){
        auto ForestFieldIndex = dis(gen);
        if( (ExcludedForAllFields.find(ForestFieldIndex)!=ExcludedForAllFields.end()) ||
            MapFieldsByType[ForestFieldIndex]!=Field::Standard ) continue;
        else MapFieldsByType[ForestFieldIndex] = Field::Forest;
    }

    // Generating Swamps Field  (There will be max 6 swamps)
    for(auto i = 0; i<6; i++){
        auto SwampFieldIndex = dis(gen);
        if( (ExcludedForAllFields.find(SwampFieldIndex)!=ExcludedForAllFields.end()) ||
            MapFieldsByType[SwampFieldIndex]!=Field::Standard ) continue;
        else MapFieldsByType[SwampFieldIndex] = Field::Swamps;
    }

    // Generating Sands Field  (There will be max 6 sands)
    for(auto i = 0; i<6; i++){
        auto SandsFieldIndex = dis(gen);
        if( (ExcludedForAllFields.find(SandsFieldIndex)!=ExcludedForAllFields.end()) ||
            MapFieldsByType[SandsFieldIndex]!=Field::Standard ) continue;
        else MapFieldsByType[SandsFieldIndex] = Field::Sands;
    }

    // Generating Big Mine Instant Death Field (There will be max 2 mines)
    for(auto i = 0; i<2; i++){
        auto BigMineFieldIndex = dis(gen);
        if( (ExcludedForAllFields.find(BigMineFieldIndex)!=ExcludedForAllFields.end()) ||
            MapFieldsByType[BigMineFieldIndex]!=Field::Standard ) continue;
        else MapFieldsByType[BigMineFieldIndex] = Field::BigMine;
    }

    // Generating Big Hole Instant Death Field (There will be max 3 holes)
    for(auto i = 0; i<3; i++){
        auto BigHoleFieldIndex = dis(gen);
        if( (ExcludedForAllFields.find(BigHoleFieldIndex)!=ExcludedForAllFields.end()) ||
            MapFieldsByType[BigHoleFieldIndex]!=Field::Standard ) continue;
        else MapFieldsByType[BigHoleFieldIndex] = Field::Hole;
    }

    // Generating Mine Fields Death Field (There will be max 5 hidden minefields)
    for(auto i = 0; i<5; i++){
        auto MineFieldsFieldIndex = dis(gen);
        if( (ExcludedForAllFields.find(MineFieldsFieldIndex)!=ExcludedForAllFields.end()) ||
            MapFieldsByType[MineFieldsFieldIndex]!=Field::Standard ) continue;
        else MapFieldsByType[MineFieldsFieldIndex] = Field::MineField;
    }

};
//--------------------------

// --- Function for map fields building ---
auto MapFieldsBuilder (std::vector<sf::Sprite>& MapFieldsImages, std::map<Field,sf::Sprite>& EnumedImageMap, std::vector<Field>& MapFieldsByType) -> void {
    auto FieldPosition = sf::Vector2f(85,89);
    for(auto row = 0; row<7; row++){
        FieldPosition.x = 85;
        for(auto column = 0; column<12; column++){
            MapFieldsImages[row*12+column] = EnumedImageMap[MapFieldsByType[row*12+column]];
            MapFieldsImages[row*12+column].setPosition(FieldPosition);
            FieldPosition+=sf::Vector2f(150,0);
        }
        FieldPosition+=sf::Vector2f(0,150);
    }
};

//--- Function for Determining index of the field on which thing currently is
auto GetThingCurrentFieldIndex (sf::Sprite thing) -> int {
    auto ThingCurrentRow = int((thing.getPosition().y-13)/150);
    auto ThingCurrentColumn = int((thing.getPosition().x-10)/150);
    auto ThingCurrentField = int(ThingCurrentRow*12+ThingCurrentColumn);
    return ThingCurrentField;
};

//--- Function for Determining the type of the field on which thing currently is
auto GetThingCurrentFieldType (sf::Sprite thing, std::vector<Field>& MapFieldsByType) -> Field {
    return MapFieldsByType[GetThingCurrentFieldIndex(thing)];
};

//--- Function for Getting the field origin point
auto GetThingCurrentFieldPosition (sf::Sprite thing, std::vector<sf::Sprite>& MapFieldsImages) -> sf::Vector2f {
    return MapFieldsImages[GetThingCurrentFieldIndex(thing)].getPosition();
};