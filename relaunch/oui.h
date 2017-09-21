#pragma once

namespace oui
{
	template <class A, class B>
	constexpr auto min(A a, B b) { return a < b ? a : b; }
	template <class A, class B>
	constexpr auto max(A a, B b) { return a < b ? b : a; }

	struct Color
	{
		float r;
		float g;
		float b;
		float a = 1;
	};

	namespace colors
	{
		static constexpr Color black{ 0, 0, 0 };
		static constexpr Color white{ 1, 1, 1 };

		static constexpr Color red{ 1, 0, 0 };
		static constexpr Color green{ 0, 1, 0 };
		static constexpr Color blue{ 0, 0, 1 };

		static constexpr Color yellow{ 1, 1, 0 };
		static constexpr Color cyan{ 0, 1, 1 };
		static constexpr Color magen{ 1, 0, 1 };
	}


	struct Ratio
	{
		float value;

		Ratio() = default;
		explicit constexpr Ratio(float v) : value(v) { }

		friend constexpr float operator*(float x, Ratio f) { return x*f.value; }
		friend constexpr float operator*(Ratio f, float x) { return f.value*x; }
	};

	struct Point
	{
		float x;
		float y;
	};

	struct Rectangle
	{
		Point upperLeft;
		Point lowerRight;

		float height() const { return lowerRight.y - upperLeft.y; }
		float width() const { return lowerRight.x - upperLeft.x; }

		Rectangle popLeft(float dx)
		{
			dx = min(dx, width());
			const float oldx = upperLeft.x;
			upperLeft.x += dx;
			return { { oldx, upperLeft.y }, { upperLeft.x, lowerRight.y } };
		}
		Rectangle popRight(float dx)
		{
			dx = min(dx, width());
			const float oldx = lowerRight.x;
			lowerRight.x -= dx;
			return { { lowerRight.x, upperLeft.y }, { oldx, lowerRight.y } };
		}

		Rectangle popTop(float dy)
		{ 
			dy = min(dy, height());
			const float oldy = upperLeft.y;
			upperLeft.y += dy; 
			return { { upperLeft.x, oldy }, { lowerRight.x, upperLeft.y } }; 
		}
		Rectangle popBottom(float dy)
		{
			dy = min(dy, height());
			const float oldy = lowerRight.y;
			lowerRight.y -= dy;
			return { { upperLeft.x, lowerRight.y }, { lowerRight.x, oldy } };
		}

		Rectangle popLeft(Ratio f)   { return popLeft(width()*f); }
		Rectangle popRight(Ratio f)  { return popRight(width()*f); }
		Rectangle popTop(Ratio f)    { return popTop(height()*f); }
		Rectangle popBottom(Ratio f) { return popBottom(height()*f); }

		Rectangle& shrink(float trim)
		{
			upperLeft.x += trim;
			upperLeft.y += trim;
			lowerRight.x -= trim;
			lowerRight.y -= trim;
			if (upperLeft.x > lowerRight.x)
				upperLeft.x = lowerRight.x = (upperLeft.x + lowerRight.x) / 2;
			if (upperLeft.y > lowerRight.y)
				upperLeft.y = lowerRight.y = (upperLeft.y + lowerRight.y) / 2;
			return *this;
		}
		Rectangle& shrink(Ratio f)
		{
			f.value = max(0, min(f.value, 1));
			const float dx = width()*f;
			const float dy = height()*f;
			upperLeft.x += dx;
			upperLeft.y += dy;
			lowerRight.x -= dx;
			lowerRight.y -= dy;
			return *this;
		}
	};

	void fill(const Rectangle&, const Color&);
}