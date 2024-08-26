#pragma once
#include <Overlay/Overlay.hpp>

class SDK
{
public:

	bool Init();

	static SDK& Get()
	{
		static SDK instance;
		return instance;
	}

	void InitUpdateSdk()
	{
		thread([&]()
		{
			while (Globals::Running)
			{
				this_thread::sleep_for(chrono::milliseconds(1));
				// Update shit here
			}
		}).detach();
	}

	bool WorldToScreen(const Vector3& WorldPos, Vector2& ScreenPos, const Matrix& Matrix = Globals::ViewMatrix);
};

inline SDK& sdk = SDK::Get();