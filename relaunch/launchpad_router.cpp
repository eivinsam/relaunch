#include "launchpad_router.h"

using namespace midi;

int LaunchpadRouter::_index(Note note) const
{
	static constexpr char lookup_size = 90;
	static constexpr int na = 65;
	static constexpr int lookup[lookup_size] =
	{
		na, na, na, na, na,  na, na, na, na, na, // 00 
		na, 56, 57, 58, 59,  60, 61, 62, 63, na, // 10
		na, 48, 49, 50, 51,  52, 53, 54, 55, na, // 20
		na, 40, 41, 42, 43,  44, 45, 46, 47, na, // 30
		na, 32, 33, 34, 35,  36, 37, 38, 39, na, // 40
		na, 24, 25, 26, 27,  28, 29, 30, 31, na, // 50
		na, 16, 17, 18, 19,  20, 21, 22, 23, na, // 60
		na,  8,  9, 10, 11,  12, 13, 14, 15, na, // 70
		na,  0,  1,  2,  3,   4,  5,  6,  7, na  // 80
	};
	const auto code = static_cast<unsigned char>(note);
	return code >= lookup_size ? na : lookup[code];
}

void LaunchpadRouter::send(Status status, Note note_in) const
{
	switch (status)
	{
	case Status::NoteOff:
	case Status::NoteOn:
	{
		const auto& dst = destination(note_in);
		if (dst.channel != midi::Channel::None)
			for (auto&& out : outputs)
				out(status, dst.channel, dst.note, dst.velocity);
		return;
	}
	default:
		return;
	}
}
