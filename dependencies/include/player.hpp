#ifndef PLAYER_HPP
#define PLAYER_HPP
#include "objects.hpp"

struct player : objects::baseEntity {
    Camera m_PlayerCamera;
    float m_xPos = m_PlayerCamera.mEye.x;
    float m_yPos = m_PlayerCamera.mEye.y;
    float m_zPos = m_PlayerCamera.mEye.z;

    float playerMaxHealth = 100;
    float playerCurrentHealth = 100;
    float playerCurrentOxygen = 100;
    float playerMaxOxygen = 100;
};
#endif PLAYER_HPP