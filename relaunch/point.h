#pragma once

namespace oui
{

	struct Point
	{
		float x;
		float y;
	};

	struct Rectangle
	{
		Point upperLeft;
		Point lowerRight;
	};

}