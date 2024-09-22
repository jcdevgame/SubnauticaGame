#include <iostream>
#include <fstream>
#include <vector>
#include <objects.hpp>
#include <nlohmann/json.hpp>

namespace roomLoading {
	struct submarineRoom {
		std::vector <objects::baseHitbox*> RoomWalls;
		std::vector <objects::baseHitbox*> RoomFloors;
		std::vector <objects::gameArtifact*> RoomArtifacts;
        std::vector <objects::gameDoor*> RoomDoors;
	};

    void saveToJson(const submarineRoom& room, const std::string& filename) {
        nlohmann::json jsonData;

        // Save RoomWalls
        for (const auto& wall : room.RoomWalls) {
            nlohmann::json wallJson;
            wallJson["visible"] = wall->visible;
            wallJson["xPos"] = wall->m_xPos;
            wallJson["yPos"] = wall->m_yPos;
            wallJson["zPos"] = wall->m_zPos;
            wallJson["OBJPath"] = wall->OBJFilePath;
            wallJson["Rotation"] = wall->m_uRotate;
            wallJson["ImageIndex"] = wall->texArrayIndex;

            jsonData["RoomWalls"].push_back(wallJson);
        }

        // Save RoomFloors
        for (const auto& floor : room.RoomFloors) {
            nlohmann::json floorJson;
            floorJson["visible"] = floor->visible;
            floorJson["xPos"] = floor->m_xPos;
            floorJson["yPos"] = floor->m_yPos;
            floorJson["zPos"] = floor->m_zPos;
            floorJson["OBJPath"] = floor->OBJFilePath;
            floorJson["Rotation"] = floor->m_uRotate;
            floorJson["ImageIndex"] = floor->texArrayIndex;

            jsonData["RoomFloors"].push_back(floorJson);
        }

        // Save RoomArtifacts
        for (const auto& artifact : room.RoomArtifacts) {
            nlohmann::json artifactJson;
            artifactJson["visible"] = artifact->visible;
            artifactJson["xPos"] = artifact->m_xPos;
            artifactJson["yPos"] = artifact->m_yPos;
            artifactJson["zPos"] = artifact->m_zPos;
            artifactJson["OBJPath"] = artifact->OBJFilePath;
            artifactJson["Rotation"] = artifact->m_uRotate;
            artifactJson["ImageIndex"] = artifact->texArrayIndex;

            jsonData["RoomArtifacts"].push_back(artifactJson);
        }

        for (const auto& door : room.RoomDoors) {
            nlohmann::json doorJson;
            doorJson["visible"] = door->visible;
            doorJson["xPos"] = door->m_xPos;
            doorJson["yPos"] = door->m_yPos;
            doorJson["zPos"] = door->m_zPos;
            doorJson["OBJPath"] = door->OBJFilePath;
            doorJson["Rotation"] = door->m_uRotate;
            doorJson["ImageIndex"] = door->texArrayIndex;
            doorJson["DoorIndex"] = door->DoorIndex;

            jsonData["RoomDoors"].push_back(doorJson);
        }

        std::ofstream file(filename);
        file << jsonData.dump(4); // Pretty print with 4 spaces
    }

    void loadFromJson(submarineRoom& room, const std::string& filename) {
        std::ifstream file(filename);
        nlohmann::json jsonData;
        file >> jsonData;

        // Load RoomWalls
        for (const auto& wallJson : jsonData["RoomWalls"]) {
            auto wall = new objects::baseHitbox();
            wall->visible = wallJson["visible"];
            wall->m_xPos = wallJson["xPos"];
            wall->m_yPos = wallJson["yPos"];
            wall->m_zPos = wallJson["zPos"];
            wall->OBJFilePath = wallJson["OBJPath"];
            wall->m_uRotate = wallJson["Rotation"];
            wall->texArrayIndex = wallJson["ImageIndex"];

            room.RoomWalls.push_back(wall);
        }

        // Load RoomFloors
        for (const auto& floorJson : jsonData["RoomFloors"]) {
            auto floor = new objects::baseHitbox();
            floor->visible = floorJson["visible"];
            floor->m_xPos = floorJson["xPos"];
            floor->m_yPos = floorJson["yPos"];
            floor->m_zPos = floorJson["zPos"];
            floor->OBJFilePath = floorJson["OBJPath"];
            floor->m_uRotate = floorJson["Rotation"];
            floor->texArrayIndex = floorJson["ImageIndex"];

            room.RoomFloors.push_back(floor);
        }

        // Load RoomArtifacts
        for (const auto& artifactJson : jsonData["RoomArtifacts"]) {
            auto artifact = new objects::gameArtifact();
            artifact->visible = artifactJson["visible"];
            artifact->m_xPos = artifactJson["xPos"];
            artifact->m_yPos = artifactJson["yPos"];
            artifact->m_zPos = artifactJson["zPos"];
            artifact->OBJFilePath = artifactJson["OBJPath"];
            artifact->m_uRotate = artifactJson["Rotation"];
            artifact->texArrayIndex = artifactJson["ImageIndex"];

            room.RoomArtifacts.push_back(artifact);
        }
        
        for (const auto& doorJson : jsonData["RoomDoors"]) {
            auto door = new objects::gameDoor();
            door->visible = doorJson["visible"];
            door->m_xPos = doorJson["xPos"];
            door->m_yPos = doorJson["yPos"];
            door->m_zPos = doorJson["zPos"];
            door->OBJFilePath = doorJson["OBJPath"];
            door->m_uRotate = doorJson["Rotation"];
            door->texArrayIndex = doorJson["ImageIndex"];

            room.RoomDoors.push_back(door);
        }
    }
}