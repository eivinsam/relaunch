#pragma once

#include <memory>
#include <vector>
#include <cassert>
#include <functional>

namespace midi
{
	enum class Status : char 
	{ 
		NoteOff = 0b1000, 
		NoteOn, 
		KeyPressure,
		ControlChange,
		ProgramChange,
		ChannelPressure,
		PitchBendChange
	};

	enum class Channel : char { None = 16 };

	inline constexpr Channel channel(int i) { assert(i >= 1 && i <= 16); return static_cast<Channel>(i - 1); }

	enum class Note : char { };

	class NoteGen
	{
		char _zero;
	public:
		constexpr NoteGen(char zero) : _zero(zero) { }
		constexpr Note operator[](char octave) const { return static_cast<Note>(_zero + 12 * octave); }
	};

	static constexpr NoteGen 
		C(12+0), Cs(12+1), D(12+2), Ds(12+3), E(12+4), 
		F(12+5), Fs(12+6), G(12+7), Gs(12+8), A(12+9), As(12+10), B(12+11);
}

class LaunchpadRouter
{
	int _index(midi::Note) const;
	int _index(int row, int column) const
	{
		assert(0 <= row && row < 8 && 0 <= column && column < 8);
		return column + 8 * row;
	}
public:
	using Output = std::function<void(midi::Status, midi::Channel, midi::Note, float)>;

	struct Destination
	{
		midi::Channel channel = midi::Channel::None;
		midi::Note note = midi::C[4];
		float velocity = 1;
	};

	Destination destinations[65];
	std::vector<Output> outputs;

	      Destination& destination(midi::Note note)           { return destinations[_index(note)]; }
	const Destination& destination(midi::Note note) const     { return destinations[_index(note)]; }
	      Destination& destination(int row, int column)       { return destinations[_index(row, column)]; }
	const Destination& destination(int row, int column) const { return destinations[_index(row, column)]; }

	void send(midi::Status, midi::Note) const;
};