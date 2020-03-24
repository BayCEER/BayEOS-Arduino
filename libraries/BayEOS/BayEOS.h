/** @mainpage BayEOS Arduino Classes
 *
 * S.Holzheu (holzheu@bayceer.uni-bayreuth.de), O.Archner (archner@bayceer.uni-bayreuth.de)
 *
 * Source Code is available on
 * https://github.com/BayCEER/BayEOS-Arduino
 *
 *  @section sec1 Introduction
 *  BayEOS Arduino is developed by BayCEER IT Group. It is a
 *  bunch of classes to facilitate sensor data acquisition with
 *  Arduino based systems.
 *
 *
 *
 *  @section sec2 Classes
 *  @subsection sec2_1 Transport Classes based on BayEOS Class
 *  All transport classes inherit from the BayEOS base class. The constructors
 *  and the initialization functions will vary between the classes. But
 *  constructing the payload and sending will be all the same.
 *
 *  @subsection sec2_2 Buffer Classes based on BayEOSBuffer Class
 *  BayEOS Transport Classes may have a pointer to a BayEOSBuffer instance.
 *  When sending the payload fails the BayEOSBuffer can be used to temporary
 *  store the payload for later resend. Currently there are three Buffer implementations:
 *  BayEOSBufferRAM, BayEOSBufferSD and BayEOSBufferSDFat
 *
 *  @subsection sec2_3 RTC Classes
 *  BayEOSBuffer Class my have a pointer to a RTC instance. With a RTC BayEOSBuffer
 *  uses RTC.get().now() to store absolute or relative timestamps instead
 *  of the less accurate millis() function.
 *  RTClib implements some I2C real time clocks (e.g DS3231).
 *
 *  @subsection sec2_4 Miscellaneous Classes
 *  There are some sensor classes. Most important the DS18B20 (Dallas OneWire
 *  temperatur sensor) and SRF02 (ultrasonic distance sensor). MCP342x is a class to access a I2C 18-bit AD-converter.
 *
 *
 *
 */

#ifndef BayEOS_h
#define BayEOS_h
#define BayEOS_VERSION "1.5"
/*
 *  Frame-Types
 */

#define BayEOS_DataFrame 0x1 /** DataFrame [0x1][TYPE][ARGS...] */
#define BayEOS_Command 0x2 /* [0x2][CMD][ARGS...] */
#define BayEOS_CommandResponse 0x3 /* [0x3][CMD][RESPONSE...] */
#define BayEOS_Message 0x4
#define BayEOS_ErrorMessage 0x5
/*
 * Routed, Delayed, RoutedRSSI, Timestamp Frame:
 * These frames wrap the original frame. In principle this can be
 * used several times. However the effective payload is reduced by
 * five (six with RSSI) byte per hub
 */
#define BayEOS_RoutedFrame 0x6 /* [0x6][MY_ID][PANID][Original Frame] */
#define BayEOS_DelayedFrame 0x7 /* [0x7][(unsigned long) delay (msec)][Original Frame] */
#define BayEOS_RoutedFrameRSSI 0x8 /* [0x9][MY_ID][PANID][RSSI][Original Frame]
Note RSSI is negative but without sign as uint8_t
*/
#define BayEOS_TimestampFrame 0x9 /* [0x9][(unsigned long) timestamp (sec since 2000-01-01 00:00 GMT)][Original Frame] */
#define BayEOS_BinaryFrame 0xa /* [0xa][(unsigned long) pos][binary data] */
#define BayEOS_OriginFrame 0xb /* [0xb][origin_length][ORIGIN][Original Frame] -> Origin replaces current origin*/
#define BayEOS_MillisecondTimestampFrame 0xc /* [0xc][(long long) timestamp (millisec since 1970-01-01 00:00 GMT)][Original Frame] */
#define BayEOS_RoutedOriginFrame 0xd /* [0xd][origin_length][ORIGIN][Original Frame] -> Origin is appended to current origin using "/" */

/*Send commands to the Gateway via frame/save interface */
#define BayEOS_GatewayCommand 0xe /* [0xe][type][ARGS...] */
#define BayEOS_GatewayCommand_SetName 0x1 /* [0xe][0x1][Name]  */
#define BayEOS_GatewayCommand_ApplyTemplate 0x2 /* [0xe][0x2][TemplateName/UUID/URL]  */
#define BayEOS_GatewayCommand_SetSamplingInt 0x3 /* [0xe][0x3][long]  */
#define BayEOS_GatewayCommand_SetCheckDelay 0x4 /* [0xe][0x4][long]  */

//#define BayEOS_GatewayCommand_SetChannelName 0x21 /* [0xe][0x2][NR][Name]  */

#define BayEOS_ChecksumFrame 0xf /* [0xf][frame][checksum_16bit] */
#define BayEOS_DelayedSecondFrame 0x10 /* [0x10][(unsigned long) delay (sec)][Original Frame] */
#define BayEOS_RF24Frame 0x11 /* [0x11][uint8_t pipe][Original Frame] */

/* BayEOS Data Frames */
/* [0x1][0x1][offset][[float]]+...  */
#define BayEOS_Float32le 0x1
#define BayEOS_Int32le 0x2
#define BayEOS_Int16le 0x3
#define BayEOS_UInt8 0x4
#define BayEOS_Double64le 0x5
#define BayEOS_String 0x6 /* [length (2b)][string] */
#define BayEOS_Binary 0x7 /* [offset][length (2b)][data] */

/* [0x1][0x21][[float]]+...  */
#define BayEOS_WithoutOffsetFloat32le 0x21
#define BayEOS_WithoutOffsetInt32le 0x22
#define BayEOS_WithoutOffsetInt16le 0x23
#define BayEOS_WithoutOffsetUInt8 0x24
#define BayEOS_WithoutOffsetDouble64le 0x25
/* [0x1][0x41][[Channel number][float]]+...  */
#define BayEOS_ChannelFloat32le 0x41
#define BayEOS_ChannelInt32le 0x42
#define BayEOS_ChannelInt16le 0x43
#define BayEOS_ChannelUInt8 0x44
#define BayEOS_ChannelDouble64le 0x45
 /* [0x1][0x61][[Label length][ChannelLabel][float]]+...  */
#define BayEOS_LabelledChannelFloat32le 0x61
#define BayEOS_LabelledChannelInt32le 0x62
#define BayEOS_LabelledChannelInt16le 0x63
#define BayEOS_LabelledChannelUInt8 0x64
#define BayEOS_LabelledChannelDouble64le 0x65


#define BayEOS_WithoutOffset 0x20
#define BayEOS_ChannelNumber 0x40
#define BayEOS_ChannelLabel 0x60

#define BayEOS_DATATYP_MASK 0x0f
#define BayEOS_OFFSETTYP_MASK 0xf0

/**
 *  BayEOS Commands
 *
 *
 *
 */
#define BayEOS_SetCannelAddress 0x1
#define BayEOS_GetCannelAddress 0x2
#define BayEOS_SetAutoSearch 0x3
#define BayEOS_GetAutoSearch 0x4
#define BayEOS_SetPin 0x5
#define BayEOS_GetPin 0x6
#define BayEOS_GetTime 0x7
#define BayEOS_SetTime 0x8
#define BayEOS_GetName 0x9
#define BayEOS_SetName 0xa
#define BayEOS_StartData 0xb
#define BayEOS_StopData 0xc /** Data 1 = reset buffer, 2 = reset read pointer , 3 = set read pointer to write pointer,
Still working but depreciated!!
*/
#define BayEOS_GetVersion 0xd
#define BayEOS_GetSamplingInt 0xe
#define BayEOS_SetSamplingInt 0xf
#define BayEOS_TimeOfNextFrame 0x10
#define BayEOS_StartLiveData 0x11
#define BayEOS_ModeStop 0x12 /* renamed from StopLiveData - Should be used to stop DUMP/LIVE/SEND-Mode */
#define BayEOS_Seek 0x13
#define BayEOS_StartBinaryDump 0x14 /** [unsigned long start_pos - optional][unsigned long end_pos - optional] */
 /** Buffer Commands
  * 0: save current read pointer to EEPROM
  * 1: erase
  * 2: set read pointer to last EEPROM pos
  * 3: set read pointer to write pointer
  * 4: set read pointer to end pos of binary dump
  * 5: get read pos
  * 6: get write pos
  * 7: get end pos  (v1.5)
  * 8: get length (v1.5)
  * *******************************/
#define BayEOS_BufferCommand 0x15
#define BayEOS_GetBatStatus 0x16 /* [0x2][0x16] -> returns [0x3][0x16][uint16_t mV][uint16_t warning limit] */
#define BayEOS_BufferInfo 0x17 /*(v1.5) [0x2][0x17] -> returns [0x3][0x17][uint32_t read][uint32_t write][uint32_t end][uint32_t length] */
#define BayEOS_GetLoggingDisabled 0x18 /*(v1.6) [0x2][0x18] -> returns [0x3][0x18][uint8_t _logging_disabled]*/
#define BayEOS_SetLoggingDisabled 0x19 /*(v1.6) [0x2][0x19][uint8_t 0/1] -> returns [0x3][0x19][uint8_t _logging_disabled]*/

#ifndef BayEOS_MAX_PAYLOAD
#define BayEOS_MAX_PAYLOAD 100
#endif



#include <BayEOSBuffer.h>


class BayEOS {
public:
	/**
	 * Constructor
	 */
	BayEOS(void) {
		_buffer=NULL;
	}
	/**
	 * Send current payload buffer
	 * Has to be overwritten by implementation
	 */
	virtual uint8_t sendPayload(void)=0;

	/**
	 * Read a rx-Packet
	 * Has to be overwritten by implementation
	 */
	virtual uint8_t readIntoPayload(int timeout=5000){return 0;};

	/**
	 * Has RX-data to read
	 * Has to be overwritten by implementation
	 */
	virtual int available(void){ return 0;};

	/**
	 * Send Error Message
	 */
	uint8_t sendError(const String &s);

	/**
	 * Send Message
	 */
	uint8_t sendMessage(const String &s);

	/**
	 * Create a Message in payload
	 */
	uint8_t createMessage(const String &s, uint8_t checksum=0, uint8_t frametype=BayEOS_Message);


	/**
	 * Start a new Frame (set _next to 0)
	 */
	void startFrame(void);

	/**
	 * Set first byte of payload buffer and set _next to 1
	 */
	void startFrame(uint8_t type);

	/**
	 * Set the start of the payload buffer to be a valid origin or routedOrigin frame
	 */
	void startOriginFrame(const String &o, uint8_t routed=0);

	/**
	 * Set first two bytes of payload buffer and set _next to 2
	 */
	void startDataFrame(uint8_t subtype=BayEOS_Float32le,uint8_t checksum=0);

	/**
	 * Starts a Origin Frame and adds header for data frame
	 *
	 * just call addChannelValue() to add values
	 */
	void startDataFrameWithOrigin(uint8_t subtype,const String &o,uint8_t checksum=0,uint8_t routed=0);

	/**
	 * Adds a channel value to the payload
	 * channel_number is optional
	 * will only take effect on certain subtypes
	 * returns 0 on success
	 * returns 1 when no space is left in buffer
	 * returns 2 when startDataFrame was not called
	 * returns 3 when wrong subtype
	 */
	uint8_t addChannelValue(float v, uint8_t channel_number=0);
	uint8_t addChannelValue(double v, uint8_t channel_number=0);
	uint8_t addChannelValue(long v, uint8_t channel_number=0);
	uint8_t addChannelValue(unsigned long v, uint8_t channel_number=0);
	uint8_t addChannelValue(int v, uint8_t channel_number=0);
	uint8_t addChannelValue(unsigned int v, uint8_t channel_number=0);
	uint8_t addChannelValue(int8_t v, uint8_t channel_number=0);
	uint8_t addChannelValue(uint8_t v, uint8_t channel_number=0);

	/**
	 * Adds a channel value to the payload using channel_label type
	 * will only take effect on certain subtypes
	 * returns 0 on success
	 * returns 1 when no space is left in buffer
	 * returns 2 when startDataFrame was not called
	 * returns 3 when wrong subtype
	 */
	uint8_t addChannelValue(float v, const char* channel_label);
	uint8_t addChannelValue(double v, const char* channel_label);
	uint8_t addChannelValue(long v, const char* channel_label);
	uint8_t addChannelValue(unsigned long v, const char* channel_label);
	uint8_t addChannelValue(int v, const char* channel_label);
	uint8_t addChannelValue(unsigned int v, const char* channel_label);
	uint8_t addChannelValue(int8_t v, const char* channel_label);
	uint8_t addChannelValue(uint8_t v, const char* channel_label);

	/*
	 * Add Checksum at end of frame
	 */
	uint8_t addChecksum(void);

	/*
	 * tries to validate checksum of payload
	 * returns
	 *  0 on success
	 *  1 on checksum error
	 *  2 on no checksum
	 */
	uint8_t validateChecksum(void);

	/**
	 * Set first five (six with rssi) bytes of payload buffer and set _next to 5 (6)
	 */
	void startRoutedFrame(uint16_t sourceMyID,uint16_t sourcePANID,uint8_t rssi=0);

	/**
	 * Set first five bytes of payload buffer
	 */
	void startDelayedFrame(unsigned long delay);

	/**
	 * Set first five bytes of payload buffer
	 */
	void startDelayedSecondFrame(unsigned long delay);
	/**
	 * Set first two bytes of payload buffer
	 */
	void startRF24Frame(uint8_t pipe);

	/**
	 * Set first five bytes of payload buffer
	 */
	void startTimestampFrame(unsigned long timestamp);
	/**
	 * Set first two bytes of payload buffer
	 */
	void startCommand(uint8_t cmd_api);

	/**
	 * Set first two bytes of payload buffer
	 */
	void startCommandResponse(uint8_t cmd_api);

	/**
	 * Add byte to payload buffer
	 * returns 1 on success
	 * returns 0 when no space is left in buffer
	 */
	uint8_t addToPayload(uint8_t b);

	/**
	 * copy a buffer to payload buffer
	 */
	uint8_t addToPayload(const void* p,uint8_t length);

	/**
	 * Add string to payload buffer
	 */
	uint8_t addToPayload(const uint8_t* c);

	/**
	 * Add String to payload buffer
	 */
	uint8_t addToPayload(const String &s);

	/**
	 * Add float (4 byte) to payload buffer
	 */
	uint8_t addToPayload(float f);

	/**
	 * Add unsigned long (4 byte) to payload buffer
	 */
	uint8_t addToPayload(unsigned long l);

	/**
	 * Add long (4 byte) to payload buffer
	 */
	uint8_t addToPayload(long l);

	/**
	 * Add int (2 byte) to payload buffer
	 */
	uint8_t addToPayload(int w);

	/**
	 * Add uint16_t (2 byte) to payload buffer
	 */
	uint8_t addToPayload(uint16_t w);


	/**
	 * returns remaining space in payload buffer
	 */
	uint8_t getPayloadBytesLeft(void) const;

	/**
	 * payload buffer length
	 */
	uint8_t getPayloadLength(void) const;

	/**
	 * packet length in payload
	 */
	uint8_t getPacketLength(void) const;

	/**
	 * get read pointer to payload
	 */
	const uint8_t *getPayload(void) const;

	/*
	 * get a byte from payload
	 */
	uint8_t getPayload(uint8_t index) const;


	/**
	 * Send current payload or write to buffer on error
	 */
	uint8_t sendOrBuffer(void);

	/**
	 * Read packet from buffer and try so send
	 */
	uint8_t sendFromBuffer(void);

	/**
	 * Write current payload to buffer
	 */
	uint8_t writeToBuffer(void);

	/**
	 * read packet from buffer to payload
	 */
	uint8_t readFromBuffer(void);

	/**
	 * read binary packet from buffer to payload
	 */
	uint8_t readBinaryFromBuffer(unsigned long pos);

	/**
	 * read binary packet from buffer to payload - but not overrun end
	 */
	uint8_t readBinaryFromBuffer(unsigned long pos,unsigned long end, unsigned long vpos);

	/**
	 * set buffer pointer to a BayEOSBuffer instance
	 * max_skip defines the maximum number of packets written directly to
	 * buffer without trying to sent...
	 */
	void setBuffer(BayEOSBuffer &buffer, uint16_t max_skip=0) {
		_buffer = &buffer;
		_max_skip=max_skip;
		_skip_counter=0;
		_failure_counter=0;
	}

	uint8_t _payload[BayEOS_MAX_PAYLOAD];
	uint8_t _next;
	uint8_t _success;
	uint8_t _failure_counter; //counts send failures
	uint16_t _max_skip; //maximum number of packets to write to buffer before next try
	uint16_t _skip_counter; //skip counter
	BayEOSBuffer* _buffer;
};


inline uint8_t BayEOS::addToPayload(uint8_t b){
	if(_next<BayEOS_MAX_PAYLOAD){
		_payload[_next]=b;
		_next++;
		return 1;
	}
	return 0;

}
#endif

