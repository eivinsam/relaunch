#pragma once

#include <optional>
#include <chrono>

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

	inline constexpr Color operator+(const Color& a, const Color& b) { return { a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a }; }

	inline constexpr Color operator*(const Color& c, double s) { auto fs = float(s); return { c.r*fs, c.g*fs, c.b*fs, c.a*fs }; }
	inline constexpr Color operator*(double s, const Color& c) { auto fs = float(s); return { c.r*fs, c.g*fs, c.b*fs, c.a*fs }; }
	inline constexpr Color operator/(const Color& c, double s) { auto fs = float(s); return { c.r/fs, c.g/fs, c.b/fs, c.a/fs }; }

	namespace colors
	{
		static constexpr Color black{ 0, 0, 0 };
		static constexpr Color white{ 1, 1, 1 };

		static constexpr Color red{ 1, 0, 0 };
		static constexpr Color green{ 0, 1, 0 };
		static constexpr Color blue{ 0, 0, 1 };

		static constexpr Color yellow{ 1, 1, 0 };
		static constexpr Color cyan{ 0, 1, 1 };
		static constexpr Color magenta{ 1, 0, 1 };
	}


	struct Ratio
	{
		float value;

		Ratio() = default;
		explicit constexpr Ratio(float v) : value(v) { }

		friend constexpr float operator*(float x, Ratio f) { return x*f.value; }
		friend constexpr float operator*(Ratio f, float x) { return f.value*x; }
	};

	struct Vector
	{
		float x;
		float y;
	};

	inline constexpr Vector operator+(Vector a, Vector b) { return { a.x + b.x, a.y + b.y }; }
	inline constexpr Vector operator-(Vector a, Vector b) { return { a.x + b.x, a.y + b.y }; }

	inline constexpr Vector operator*(Vector v, float c) { return { v.x*c, v.y*c }; }
	inline constexpr Vector operator*(float c, Vector v) { return { v.x*c, v.y*c }; }
	inline constexpr Vector operator/(Vector v, float c) { return { v.x/c, v.y/c }; }

	inline constexpr float   dot(Vector a, Vector b) { return a.x*b.x + a.y*b.y; }
	inline constexpr float cross(Vector a, Vector b) { return a.x*b.y - a.y*b.x; }

	inline float angle(Vector a, Vector b) { return acos(dot(a, b) / sqrt(dot(a, a)*dot(b, b))); }

	struct Rectangle;
	struct Point
	{
		float x;
		float y;

		constexpr bool in(const Rectangle&) const;
	};

	inline constexpr Point operator+(Point p, Vector v) { return { p.x + v.x, p.y + v.y }; }
	inline constexpr Point operator-(Point p, Vector v) { return { p.x - v.x, p.y - v.y }; }
	inline constexpr Point operator+(Vector v, Point p) { return { v.x + p.x, v.y + p.y }; }
	inline constexpr Vector operator-(Point a, Point b) { return { a.x - b.x, a.y - b.y }; }
	
	struct Rectangle
	{
		Point min; // Top left corner
		Point max; // Bottom right corner

		constexpr float height() const { return max.y - min.y; }
		constexpr float width() const { return max.x - min.x; }

		constexpr Vector size() const { return max - min; }

		constexpr Point center() const { return min + (max - min) / 2; }

		Rectangle popLeft(float dx)
		{
			dx = oui::min(dx, width());
			const float oldx = min.x;
			min.x += dx;
			return { { oldx, min.y }, { min.x, max.y } };
		}
		Rectangle popRight(float dx)
		{
			dx = oui::min(dx, width());
			const float oldx = max.x;
			max.x -= dx;
			return { { max.x, min.y }, { oldx, max.y } };
		}

		Rectangle popTop(float dy)
		{ 
			dy = oui::min(dy, height());
			const float oldy = min.y;
			min.y += dy; 
			return { { min.x, oldy }, { max.x, min.y } }; 
		}
		Rectangle popBottom(float dy)
		{
			dy = oui::min(dy, height());
			const float oldy = max.y;
			max.y -= dy;
			return { { min.x, max.y }, { max.x, oldy } };
		}

		Rectangle popLeft(Ratio f)   { return popLeft(width()*f); }
		Rectangle popRight(Ratio f)  { return popRight(width()*f); }
		Rectangle popTop(Ratio f)    { return popTop(height()*f); }
		Rectangle popBottom(Ratio f) { return popBottom(height()*f); }

		Rectangle shrink(float trim) const
		{
			Rectangle r = *this;
			r.min.x += trim;
			r.min.y += trim;
			r.max.x -= trim;
			r.max.y -= trim;
			if (r.min.x > r.max.x)
				r.min.x = r.max.x = (r.min.x + r.max.x) / 2;
			if (r.min.y > r.max.y)
				r.min.y = r.max.y = (r.min.y + r.max.y) / 2;
			return *this;
		}
		Rectangle shrink(Ratio f) const
		{
			f.value = oui::max(0, oui::min(f.value, 1));
			const float dx = width()*f;
			const float dy = height()*f;
			return { { min.x + dx, min.y + dy }, { max.x - dx, max.y - dy } };
		}
	};
	void fill(const Rectangle&, const Color&);

	inline constexpr bool Point::in(const Rectangle& area) const
	{
		return
			area.min.x <= x && x < area.max.x &&
			area.min.y <= y && y < area.max.y;
	}

	template <class T>
	using Optional = std::optional<T>;
	using Time = std::chrono::time_point<std::chrono::high_resolution_clock>;

	inline Time now() { return std::chrono::high_resolution_clock::now(); }
	
	inline float duration(Time t0, Time t1) { return std::chrono::duration<float>(t1 - t0).count(); }

	template <class T>
	Optional<T> pop(Optional<T>& op)
	{
		Optional<T> result;
		std::swap(op, result);
		return result;
	}

	class Pointer
	{
		struct Context
		{
			Time time;
			Point position;
		};
		struct Down : Context
		{
			Down(Time time, Point point) : Context{ time, point } { }
			Optional<Context> up;
		};
		Optional<Context> _current;
		Optional<Vector> _delta;
		Optional<Down> _button;
	public:
		void move(const Point& new_position)
		{
			if (_current)
				_delta = _delta.value_or(Vector{ 0, 0 }) + (new_position - _current->position);
			_current = { now(), new_position };
		}
		void press(const Point& position)
		{
			_button = { now(), position };
		}
		void longPress(const Point& position)
		{
			_button = { now() - std::chrono::seconds(1), position };
		}
		void release(const Point& position)
		{
			if (_button)
				_button->up = { now(), position };
		}

		// Reset all complete states
		void takeAll()
		{
			_delta = std::nullopt;
			if (_button && _button->up)
				_button = std::nullopt;
		}


		bool hovering(const Rectangle& area) const { return _current && _current->position.in(area); }
		Optional<Point> holding(const Rectangle& area) const
		{
			if (_button && _button->position.in(area))
				return _current->position;
			return std::nullopt;
		}
		Optional<Vector> dragging(const Rectangle& area)
		{
			if (holding(area))
				return pop(_delta);
			return std::nullopt;
		}

		Optional<Point> pressed(const Rectangle& area)
		{
			if (holding(area) && 
				_button->up && _button->up->position.in(area) &&
				duration(_button->time, _button->up->time) < 1)
			{
				Point result = _button->up->position;
				_button.reset();
				return result;
			}
			return std::nullopt;
		}
		bool longPressed(const Rectangle& area)
		{
			if (holding(area) && 
				_button->up && _button->up->position.in(area) &&
				duration(_button->time, _button->up->time) >= 1)
			{
				_button.reset();
				return true;
			}
			return false;
		}
	};

	class Input
	{
	public:
		Pointer mouse;
	};
}