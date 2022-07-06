/* BingoKit
 * (C) 2022 the BingoKit authors.
 * Please see the README.md and LICENSE files for more information.
 */

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <ctime>
#include "shorthand.h"

/* Mersenne Twister (BSD licensed)
 * Written by Christian Stigen Larsen
 * Class-ified by Jimmio92
 */

class Random {
	public:
		#define M32(x) (0x80000000 & x)
		#define L31(x) (0x7FFFFFFF & x)
		#define UNROLL(expr) \
		y = M32(MT[i]) | L31(MT[i+1]); \
		MT[i] = MT[expr] ^ (y >> 1) ^ (((int32_t(y) << 31) >> 31) & MAGIC); \
		++i;
		
			static const size_t SIZE = 624;
			static const size_t PERIOD = 397;
			static const size_t DIFF = SIZE - PERIOD;
			static const uint32_t MAGIC = 0x9908B0DF;
		
			uint32_t MT[SIZE];
			uint32_t MT_TEMPERED[SIZE];
			size_t index = SIZE;
		
			Random(uint32_t s) {
				seed(s);
			}
		
			void generate_numbers() {
				size_t i = 0;
				uint32_t y;
		
				while(i < DIFF) {
					UNROLL(i + PERIOD);
					//UNROLL(i + PERIOD);
				}
		
				while(i < SIZE - 1) {
					UNROLL(i - DIFF);
					/*
					UNROLL(i-DIFF);
					UNROLL(i-DIFF);
					UNROLL(i-DIFF);
					UNROLL(i-DIFF);
					UNROLL(i-DIFF);
					UNROLL(i-DIFF);
					UNROLL(i-DIFF);
					UNROLL(i-DIFF);
					UNROLL(i-DIFF);
					UNROLL(i-DIFF);
					*/
				}
		
				{
					y = M32(MT[SIZE - 1]) | L31(MT[0]);
					MT[SIZE - 1] = MT[PERIOD - 1] ^ (y >> 1) ^ (((int32_t(y) << 31) >> 31) & MAGIC);
				}
		
				for(size_t i = 0; i < SIZE; ++i) {
					y = MT[i];
					y ^= y >> 11;
					y ^= y <<  7 & 0x9D2C5680;
					y ^= y << 15 & 0xEFC60000;
					y ^= y >> 18;
					MT_TEMPERED[i] = y;
				}
		
				index = 0;
			}
		
			void seed(uint32_t value) {
				MT[0] = value;
				//index = SIZE; this signals to regen numbers... just call it when done
				//seeding
		
				for(uint32_t i = 1; i < SIZE; ++i) {
					MT[i] = 0x6C078965*(MT[i - 1] ^ MT[i - 1] >> 30) + i;
				}
		
				generate_numbers();
			}
		
			size_t operator()() {
				if(index == SIZE) {
					generate_numbers();
					//index = 0; this is done at the end of generate_numbers...
				}
		
				return MT_TEMPERED[index++];
			}
		
		#undef M32
		#undef L31
		#undef UNROLL
};

class Card {
	public:
		s64 faceNumber;
		u8 space[25];
	
		Card(): faceNumber(-1), space{255} { }
	
		Card(size_t c_faceNumber, u8 *c_space): faceNumber(c_faceNumber), space() {
			for(size_t i = 0; i < 25; ++i) {
				space[i] = c_space[i];
			}
		}
	
		// Outputs a face with some nice formatting.
		void pretty_print() {
			// NOTE: Space #12 is always a free space.
			printf(
				"╔════════════════════════╗\n"
				"║ Card # %-10zu      ║\n"
				"╠════╤════╤════╤════╤════╣\n"
				"║ B  │ I  │ N  │ G  │ O  ║\n"
				"╟────┼────┼────┼────┼────╢\n"
				"║ %-2d │ %2d │ %2d │ %2d │ %2d ║\n"
				"╟────┼────┼────┼────┼────╢\n"
				"║ %-2d │ %2d │ %2d │ %2d │ %2d ║\n"
				"╟────┼────┼────┼────┼────╢\n"
				"║ %-2d │ %2d │%s│ %2d │ %2d ║\n"
				"╟────┼────┼────┼────┼────╢\n"
				"║ %-2d │ %2d │ %2d │ %2d │ %2d ║\n"
				"╟────┼────┼────┼────┼────╢\n"
				"║ %-2d │ %2d │ %2d │ %2d │ %2d ║\n"
				"╚════╧════╧════╧════╧════╝\n",
				faceNumber,
				space[0],space[1],space[2],space[3],space[4],
				space[5],space[6],space[7],space[8],space[9],
				space[10],space[11], "FREE", space[13],space[14],
				space[15],space[16],space[17],space[18],space[19],
				space[20],space[21],space[22],space[23],space[24]
			);
		}
};

class CardGenerator {
	private:
		// Full columns.
		std::vector<u8> all_b;
		std::vector<u8> all_i;
		std::vector<u8> all_n;
		std::vector<u8> all_g;
		std::vector<u8> all_o;

		// Mersenne Twister instance.
		Random r;

	public:
		// Collection of multiple faces, if desired.
		std::vector<Card> faces;
		
		CardGenerator(): all_b(), all_i(), all_n(), all_g(), all_o(),
		faces(), r(0) {
			
			// Gather all the numbers for each column.
			for(u8 i = 1; i <= 15; ++i) {
				all_b.push_back(i);
			}
			for(u8 i = 16; i <= 30; ++i) {
				all_i.push_back(i);
			}
			for(u8 i = 31; i <= 45; ++i) {
				all_n.push_back(i);
			}
			for(u8 i = 46; i <= 60; ++i) {
				all_g.push_back(i);
			}
			for(u8 i = 61; i <= 75; ++i) {
				all_o.push_back(i);
			}
		}
		
		Card generate_face(uint32_t faceNumber) {
			// Initialize the Mersenne Twister with the face number.
			r.seed(faceNumber);

			// Create a Card object to populate.
			Card face;

			// Fill the column bags with copies of the column numbers.
			std::vector<u8> b_bag = all_b;
			std::vector<u8> i_bag = all_i;
			std::vector<u8> n_bag = all_n;
			std::vector<u8> g_bag = all_g;
			std::vector<u8> o_bag = all_o;
	
			// Pull 25 numbers from the bags to populate spaces.
			for(size_t i = 0; i < 25; ++i) {

				// Set up the collector.
				std::vector<u8> *coll = NULL;

				// Pull a number from a given bag based on iterator.
				switch(i % 5) {
				case 0:
					coll = &b_bag;
					break;
				case 1:
					coll = &i_bag;
					break;
				case 2:
					coll = &n_bag;
					break;
				case 3:
					coll = &g_bag;
					break;
				case 4:
					coll = &o_bag;
					break;
				}
	
				// Get the current collection size.
				size_t coll_size = coll->size();

				// Populate the space with the collected number.
				size_t number = r() % coll_size;
				face.space[i] = (*coll)[number];
	
				// Take the number out of the collector (swap and pop).
				(*coll)[number] = (*coll)[coll_size - 1];
				coll->pop_back();
			}

			// Replace space #12's value with 0 (free space).
			face.space[12] = 0;

			// Save the face's number.	
			face.faceNumber = faceNumber;
	
			// Store the face in the collection, if desired.
			faces.push_back(face);

			// Return the generated face.
			return face;
		}
};

int main(int args, char* arg[]) {
	s32 s = time(NULL), t, v;
	Random r(s);
	v = (r() % 65535) + 1;
	CardGenerator cardGenerator;

	if(args == 2) {
		// Try to convert argument 1.
		t = atoi(arg[1]);

		// Conversion failed, generate a random number.
		if(t < 1 || t > 65535) {
			printf( "*** No valid face number (1-65535) specified. Generating random face number instead.  ***\n\n");
		} else {
			v = t;
		}
	} else {
		printf( "*** No value specified. Generating random face number instead.  ***\n\n");
	}

	Card face = cardGenerator.generate_face(v);
 
	face.pretty_print();
	return 0;
}
