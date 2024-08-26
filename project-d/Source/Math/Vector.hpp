#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

namespace math
{
    constexpr float PI = 3.14159265358979323846f;

    template<typename T>
    union vector3
    {
        T data[3];
        struct
        {
            T x, y, z;
        };

        vector3 operator+(float in) const { return { this->x + in, this->y + in, this->z + in }; }
        vector3 operator+(vector3 in) const { return { this->x + in.x, this->y + in.y, this->z + in.z }; }
        vector3 operator-(float in) const { return { this->x - in, this->y - in, this->z - in }; }
        vector3 operator-(vector3 in) const { return { this->x - in.x, this->y - in.y, this->z - in.z }; }
        vector3 operator*(float in) const { return { this->x * in, this->y * in, this->z * in }; }
        vector3 operator*(vector3 in) const { return { this->x * in.x, this->y * in.y, this->z * in.z }; }
        vector3 operator/(float in) const { return { this->x / in, this->y / in, this->z / in }; }
        vector3 operator/(vector3 in) const { return { this->x / in.x, this->y / in.y, this->z / in.z }; }
        vector3& operator+=(float in) { this->x += in; this->y += in; this->z += in; return *this; }
        vector3& operator+=(vector3 in) { this->x += in.x; this->y += in.y; this->z += in.z; return *this; }
        vector3& operator-=(float in) { this->x -= in; this->y -= in; this->z -= in; return *this; }
        vector3& operator-=(vector3 in) { this->x -= in.x; this->y -= in.y; this->z -= in.z; return *this; }
        vector3& operator*=(float in) { this->x *= in; this->y *= in; this->z *= in; return *this; }
        vector3& operator*=(vector3 in) { this->x *= in.x; this->y *= in.y; this->z *= in.z; return *this; }
        vector3& operator/=(float in) { this->x /= in; this->y /= in; this->z /= in; return *this; }
        vector3& operator/=(vector3 in) { this->x /= in.x; this->y /= in.y; this->z /= in.z; return *this; }

        bool operator==(const vector3& in) const { return this->x == in.x && this->y == in.y && this->z == in.z; }
        bool operator!=(const vector3& in) const { return !(*this == in); }

        vector3 normalize() const
        {
            vector3 out = *this;

            if (out.x > 89)
                out.x = 89;

            if (out.x < -89)
                out.x = -89;

            while (out.y > 180)
                out.y -= 360;

            while (out.y < -180)
                out.y += 360;

            out.z = 0;

            return out;
        }

        vector3 ToAngle() const
        {
            return {
                std::atan2(-z, std::sqrt(x * x + y * y)) * (180.0f / PI),
                std::atan2(y, x) * (180.0f / PI),
                0.0f
            };
        }

        vector3 Normalize() const
        {
            vector3 out = *this;

            if (out.x > 89)
                out.x = 89;

            if (out.x < -89)
                out.x = -89;

            while (out.y > 180)
                out.y -= 360;

            while (out.y < -180)
                out.y += 360;

            out.z = 0;

            return out;
        }

        bool Zero() const
        {
            return (this->x == 0.f && this->y == 0.f && this->z == 0.f);
        }

        vector3 RelativeAngle() const
        {
            return {
                std::atan2(-z, std::hypot(x, y)) * (180.0f / PI),
                std::atan2(y, x) * (180.0f / PI),
                0.0f
            };
        }

        bool invalid() const
        {
            return !x || !y || !z;
        }

        ImVec2 ToImVec2() const
        {
            return ImVec2(x, y);
        }

        std::string ToString() const
        {
            return std::to_string(this->x) + ", " + std::to_string(this->y) + ", " + std::to_string(this->z);
        }
    };

    // Define vector2
    template<typename T>
    union vector2
    {
        T data[2];
        struct
        {
            T x, y;
        };

        vector2 operator+(float in) const { return { this->x + in, this->y + in }; }
        vector2 operator+(vector2 in) const { return { this->x + in.x, this->y + in.y }; }
        vector2 operator-(float in) const { return { this->x - in, this->y - in }; }
        vector2 operator-(vector2 in) const { return { this->x - in.x, this->y - in.y }; }
        vector2 operator*(float in) const { return { this->x * in, this->y * in }; }
        vector2 operator*(vector2 in) const { return { this->x * in.x, this->y * in.y }; }
        vector2 operator/(float in) const { return { this->x / in, this->y / in }; }
        vector2 operator/(vector2 in) const { return { this->x / in.x, this->y / in.y }; }
        vector2& operator+=(float in) { this->x += in; this->y += in; return *this; }
        vector2& operator+=(vector2 in) { this->x += in.x; this->y += in.y; return *this; }
        vector2& operator-=(float in) { this->x -= in; this->y -= in; return *this; }
        vector2& operator-=(vector2 in) { this->x -= in.x; this->y -= in.y; return *this; }
        vector2& operator*=(float in) { this->x *= in; this->y *= in; return *this; }
        vector2& operator*=(vector2 in) { this->x *= in.x; this->y *= in.y; return *this; }
        vector2& operator/=(float in) { this->x /= in; this->y /= in; return *this; }
        vector2& operator/=(vector2 in) { this->x /= in.x; this->y /= in.y; return *this; }

        bool operator==(const vector2& in) const { return this->x == in.x && this->y == in.y; }
        bool operator!=(const vector2& in) const { return !(*this == in); }

        bool invalid() const
        {
            return !x || !y;
        }

        bool Zero() const
        {
            return (this->x == 0.f && this->y == 0.f);
        }

        ImVec2 ToImVec2() const
        {
            return ImVec2(x, y);
        }

        std::string ToString() const
        {
            return std::to_string(this->x) + ", " + std::to_string(this->y);
        }

        float Distance(const vector2& in) const
		{
			return std::sqrt(std::pow(in.x - x, 2) + std::pow(in.y - y, 2));
		}
    };

    template<typename T>
    union quaternion
    {
        T data[4];
        struct
        {
            T x, y, z, w;
        };

        quaternion operator+(float in) const { return { this->x + in, this->y + in, this->z + in, this->w + in }; }
        quaternion operator+(quaternion in) const { return { this->x + in.x, this->y + in.y, this->z + in.z, this->w + in.w }; }
        quaternion operator-(float in) const { return { this->x - in, this->y - in, this->z - in, this->w - in }; }
        quaternion operator-(quaternion in) const { return { this->x - in.x, this->y - in.y, this->z - in.z, this->w - in.w }; }
        quaternion operator*(float in) const { return { this->x * in, this->y * in, this->z * in, this->w * in }; }
        quaternion operator*(quaternion in) const { return { this->x * in.x, this->y * in.y, this->z * in.z, this->w * in.w }; }
        quaternion operator/(float in) const { return { this->x / in, this->y / in, this->z / in, this->w / in }; }
        quaternion operator/(quaternion in) const { return { this->x / in.x, this->y / in.y, this->z / in.z, this->w / in.w }; }
        quaternion& operator+=(float in) { this->x += in; this->y += in; this->z += in; this->w += in; return *this; }
        quaternion& operator+=(quaternion in) { this->x += in.x; this->y += in.y; this->z += in.z; this->w += in.w; return *this; }
        quaternion& operator-=(float in) { this->x -= in; this->y -= in; this->z -= in; this->w -= in; return *this; }
        quaternion& operator-=(quaternion in) { this->x -= in.x; this->y -= in.y; this->z -= in.z; this->w -= in.w; return *this; }
        quaternion& operator*=(float in) { this->x *= in; this->y *= in; this->z *= in; this->w *= in; return *this; }
        quaternion& operator*=(quaternion in) { this->x *= in.x; this->y *= in.y; this->z *= in.z; this->w *= in.w; return *this; }
        quaternion& operator/=(float in) { this->x /= in; this->y /= in; this->z /= in; this->w /= in; return *this; }
        quaternion& operator/=(quaternion in) { this->x /= in.x; this->y /= in.y; this->z /= in.z; this->w /= in.w; return *this; }

        bool operator==(const quaternion& in) const { return this->x == in.x && this->y == in.y && this->z == in.z && this->w == in.w; }
        bool operator!=(const quaternion& in) const { return !(*this == in); }

        quaternion normalize() const
        {
            float magnitude = std::sqrt(x * x + y * y + z * z + w * w);
            return { x / magnitude, y / magnitude, z / magnitude, w / magnitude };
        }

        quaternion conjugate() const
        {
            return { -x, -y, -z, w };
        }

        float dot(const quaternion& in) const
        {
            return x * in.x + y * in.y + z * in.z + w * in.w;
        }

        quaternion inverse() const
        {
            float magnitude = x * x + y * y + z * z + w * w;
            return conjugate() / magnitude;
        }

        std::string ToString() const
        {
            return std::to_string(this->x) + ", " + std::to_string(this->y) + ", " + std::to_string(this->z) + ", " + std::to_string(this->w);
        }
    };
}

typedef math::vector2<float> Vector2;
typedef math::vector3<float> Vector3;
typedef math::quaternion<float> Quaternion;
