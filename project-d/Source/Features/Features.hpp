#include <SDK.hpp>

#include <Aimbot/Aimbot.hpp>
#include <ESP/ESP.hpp>

class Features
{
public:

	void InitAimbot()
	{
		thread([&]()
		{
			while (Globals::Running)
			{
				this_thread::sleep_for(chrono::milliseconds(1));

				aim.Update();
			}
		}).detach();
	}

	static Features& Get()
	{
		static Features instance;
		return instance;
	}

	bool Init()
	{
		//InitAimbot();

		return true;
	}
};

inline Features& features = Features::Get();