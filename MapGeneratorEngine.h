#include "Field.h"
#pragma once

auto MapGenerator (std::vector<Field>& MapFieldsByType) -> void;
auto MapFieldsBuilder (std::vector<sf::Sprite>& MapFieldsImages, std::map<Field,sf::Sprite>& EnumedImageMap, std::vector<Field>& MapFieldsByType) -> void;
auto GetThingCurrentFieldIndex (sf::Sprite thing) -> int;
auto GetThingCurrentFieldType (sf::Sprite thing, std::vector<Field>& MapFieldsByType) -> Field;
auto GetThingCurrentFieldPosition (sf::Sprite thing, std::vector<sf::Sprite>& MapFieldsImages) -> sf::Vector2f;