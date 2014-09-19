
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <vector>
#include <crtdbg.h>

static int GetMSBInt(const unsigned char* data)
{
	int value = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
	return value;
}

static int g_TimeDivision = 0; // MIDI time division (e.g. 480 ticks per beat)
static int g_UsecPerBeat = 500000;  // MIDI microseconds per beat
class KeyEvent
{
public:
	float t;
	int note;
	int velocity;

	KeyEvent()
	{
		t = 0.0f;
		note = 0;
		velocity = 0;
	}
};
static std::vector<KeyEvent>* g_Events = 0;

static void AddNote(float t, int note, int velocity)
{
	if (velocity <= 0)
		return;

	KeyEvent ke;
	ke.t = t;
	ke.note = note;
	ke.velocity = velocity;

	if (g_Events)
		g_Events->push_back(ke);
}

static int ReadBlock(FILE* file)
{
	unsigned char data[4] = {0};
	int ret = fread(data, 1, 4, file);
	if (ret != 4)
		return 1; // End of file
	
	const int MThd = 0x4d546864, MTrk = 0x4d54726b; // Block types

	int blockType = GetMSBInt(data);
	if (blockType != MThd && blockType != MTrk)
	{
		printf("Unknown block type 0x%x", blockType);
		return 1; // Unknown block type, stop
	}
		
	fread(data, 1, 4, file);
	int length = GetMSBInt(data);
	if (length <= 0)
		return 1; // Dodgy length, stop
	
	unsigned char* block = new unsigned char[length];
	if (!block)
		return 1; // Memory allocation failed
	
	memset(block, 0, length);
	
	ret = fread(block, 1, length, file);
	if (ret != length)
	{
		printf("ERROR: Could not read a block of length %d\n", length);
		delete[] block;
		return 1;
	}
	
	if (blockType == MThd && length >= 6)
	{
//		int format = block[1];
//		int tracks  = (block[2] << 8) | block[3];
		int timeDiv = (block[4] << 8) | block[5];
		g_TimeDivision = timeDiv;
	}
	
	if (blockType == MTrk)
	{
		int i = 0;
		int evt = 0;
		
		int timePos = 0;
		
		while (i < length)
		{
#define ADV i++; if (i >= length) break;
			// Read time delta value
			int t = 0;
			int b = block[i]; ADV
			if (b & 0x80)
			{
				int c = block[i]; ADV
				if (c & 0x80)
				{
					int d = block[i]; ADV
					t = ((b & 0x7f) << 14) | ((c & 0x7f) << 7) | (d & 0x7f);
				}
				else
				{
					t = ((b & 0x7f) << 7) | (c & 0x7f);
				}
			}
			else
			{
				t = b;
			}
			
			timePos += t;
			int e = block[i]; ADV
			
			if (e & 0x80)
				evt = e; // New event type
			else
				i--;
			
			if ((evt & 0x80) != 0x80)
			{
				printf("ERROR: at i=0x%x evt is 0x%x and doesn't have high bit\n", i, evt);
				return 1;
			}

			if (evt == 0xff)
			{
				// FFxx Meta event
				int metaType = block[i]; ADV
				if (metaType == 0x03)
				{
					int stringLen = block[i]; ADV
					i += stringLen;
				}
				if (metaType == 0x21)
					i += 2;
				if (metaType == 0x51)
				{
					int len = block[i]; ADV
					if (len != 3)
					{
						delete[] block;
						return 1;
					}

					if (i + 3 > length)
						break;
					// Set tempo in usec per beat
					int usec = ( block[i] << 16 ) | ( block[i + 1] << 8 ) | block[i + 2]; 
					g_UsecPerBeat = usec;
					i += 3;
				}

				if (i >= length)
					break;
				else
					continue;
			}
			
			if ((evt & 0xe0) == 0x80)
			{
				int note = block[i]; ADV
				int velocity = block[i]; ADV
				bool on = (evt & 0x10) != 0;
				
				if (!on)
					velocity = 0;

				float tick = (float)g_UsecPerBeat / (float)g_TimeDivision * 0.000001f;
				float seconds = (float)timePos * tick;
				AddNote(seconds, note, velocity);
			}

			if ((evt & 0xf0) == 0xb0)
				i += 2;

			if ((evt & 0xf0) == 0xc0)
				i++;
#undef ADV
		}
	}
		
	delete[] block;
	block = 0;
	
	return 0; // Zero = Still going, Nonzero = stop
}

static void PrintHeader()
{
	printf(
		"// Converted by https://github.com/davehampson/midi2shader\n"
		"\n"
		"vec2 GetPitch(float time)\n"
		"{\n"
	);
}

static void PrintFooter()
{
	printf(
		"	return vec2(0.0, 0.0);\n"
		"}\n"
		"\n"
		"vec2 mainSound(float time)\n"
		"{\n"
		"	vec2 freq = GetPitch(time);\n"
		"	return vec2(sin(6.2831 * freq.x *time) * 0.125 * exp(-3.0 * freq.y));\n"
		"}\n"
	);
}

int main(int argc, char* argv[])
{
	const char* inputMidi = "test3.mid";
	
	if (argc >= 2)
		inputMidi = argv[1];

	int noteLimit = INT_MAX;
	if (argc >= 3)
		 noteLimit = strtol(argv[2], NULL, 0);

	g_Events = new std::vector<KeyEvent>;
	if (!g_Events)
		return 1;

	FILE* file = fopen(inputMidi, "rb");
	if (file == NULL)
	{
		printf("Could not open '%s'\n", inputMidi);
		return 1;
	}
	
	for (;;)
	{
		int status = ReadBlock(file);
		if (status != 0)
			break;
	}
	fclose(file);
	file = NULL;

	PrintHeader();

	int i = 0;
	for ( ; i < (int)g_Events->size() - 1 && i < noteLimit; i++)
	{
		const KeyEvent& curr = (*g_Events)[i];
		const KeyEvent& next = (*g_Events)[i + 1];

		float e = ((float)(curr.note - 69)) / 12.0f;
		float freq = powf(2.0f, e) * 440.0f;

		printf("\tif (time >= %6.2f && time < %6.2f) return vec2(%5.1f, time - %.2f);\n", curr.t, next.t, freq, curr.t);
	}

	PrintFooter();

	delete g_Events;
	g_Events = 0;

	_CrtDumpMemoryLeaks();
	
	return 0;
}
