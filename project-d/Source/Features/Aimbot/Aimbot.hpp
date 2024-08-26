#pragma once
#include <Pch.hpp>

class Aimbot
{
public:
	void Update();

	static Aimbot& Get()
	{
		static Aimbot instance;
		return instance;
	}
};

inline Aimbot& aim = Aimbot::Get();