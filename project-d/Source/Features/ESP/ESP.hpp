#pragma once

class ESP
{
private:
    void Render(ImDrawList* drawList);

public:
    void Update(ImDrawList* drawList)
    {
        TIMER("ESP render");

        Render(drawList);
    }

    static ESP& Get()
    {
        static ESP instance;
        return instance;
    }
};

inline ESP& esp = ESP::Get();
