/*
 * Header File for MCP Multiplexer Shield
 */

const uint8_t mplex_channel_map[]={ B00001000, B00010000, B00000000, B00011000 ,
									B00010100, B00011100, B00001100, B00000100 };

void mplex_set_channel(uint8_t ch){
	if(ch>7) return;
	DDRD  |= B00011100;
	PORTD &= B11100011;
	PORTD |= mplex_channel_map[ch];
}

#define MPLEX_POWER_PIN 6
