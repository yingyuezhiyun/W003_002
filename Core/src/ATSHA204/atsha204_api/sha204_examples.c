/*
 * sha204_examples.c
 *
 * Created: 4/9/2024 16:21:16
 *  Author: Student
 */ 


#include <string.h>                   // needed for memset(), memcpy()
#include "../atsha204_lib/sha204_lib_return_codes.h"  // declarations of function return codes
#include "../atsha204_lib/sha204_comm_marshaling.h"   // definitions and declarations for the Command Marshaling module
#include "../atsha204_helper/sha204_helper.h"            // definitions of functions that calculate SHA256 for every command
// #include "../atsha204_examples/atsha204_examples.h"          // definitions and declarations for example functions


/** \brief key values at time of shipping
*/
const uint8_t sha204_default_key[16][SHA204_KEY_SIZE] = {
	{
		0x00, 0x00, 0x00, 0x0A, 0xA1, 0x1A, 0xAC, 0xC5, 0x57, 0x7F, 0xFF, 0xF4, 0x40, 0x04, 0x4E, 0xE4,
		0x45, 0x5D, 0xD4, 0x40, 0x04, 0x40, 0x01, 0x1B, 0xBD, 0xD0, 0x0E, 0xED, 0xD3, 0x3C, 0xC6, 0x67,
	},
	{
		0x11, 0x11, 0x11, 0x12, 0x23, 0x3B, 0xB6, 0x6C, 0xCC, 0xC5, 0x53, 0x3B, 0xB7, 0x7B, 0xB9, 0x9E,
		0xE9, 0x9B, 0xBB, 0xB5, 0x51, 0x1F, 0xFD, 0xD2, 0x2F, 0xF7, 0x74, 0x4C, 0xCD, 0xD0, 0x0E, 0xE9,
	},
	{
		0x22, 0x22, 0x22, 0x2C, 0xC1, 0x17, 0x7C, 0xC1, 0x1C, 0xC4, 0x4D, 0xD5, 0x56, 0x68, 0x89, 0x9A,
		0xAA, 0xA0, 0x00, 0x04, 0x43, 0x3E, 0xE3, 0x39, 0x9C, 0xCF, 0xFB, 0xB6, 0x6B, 0xB0, 0x0B, 0xB6,
	},
	{
		0x33, 0x33, 0x33, 0x33, 0x33, 0x36, 0x61, 0x14, 0x4A, 0xA1, 0x17, 0x79, 0x9A, 0xA2, 0x23, 0x36,
		0x6C, 0xC7, 0x7F, 0xFE, 0xE4, 0x4B, 0xBE, 0xE2, 0x2F, 0xF1, 0x13, 0x32, 0x20, 0x06, 0x67, 0x79,
	},
	{
		0x44, 0x44, 0x44, 0x49, 0x91, 0x11, 0x18, 0x86, 0x68, 0x83, 0x3D, 0xDB, 0xB8, 0x8D, 0xD3, 0x3F,
		0xF8, 0x85, 0x57, 0x70, 0x0C, 0xC7, 0x74, 0x42, 0x2E, 0xED, 0xDA, 0xAD, 0xDA, 0xA5, 0x52, 0x28,
	},
	{
		0x55, 0x55, 0x55, 0x58, 0x86, 0x6F, 0xF2, 0x2B, 0xB3, 0x32, 0x20, 0x09, 0x98, 0x8A, 0xA6, 0x6E,
		0xE1, 0x1E, 0xE6, 0x63, 0x33, 0x37, 0x7A, 0xA5, 0x52, 0x20, 0x01, 0x10, 0x03, 0x36, 0x6A, 0xA0,
	},
	{
		0x66, 0x66, 0x66, 0x6D, 0xD0, 0x04, 0x45, 0x53, 0x3A, 0xAC, 0xC2, 0x22, 0x25, 0x55, 0x57, 0x7F,
		0xF6, 0x6D, 0xD4, 0x46, 0x6B, 0xB7, 0x7D, 0xDD, 0xDF, 0xF9, 0x96, 0x68, 0x89, 0x9D, 0xDA, 0xA2,
	},
	{
		0x77, 0x77, 0x77, 0x72, 0x2F, 0xF4, 0x4A, 0xA9, 0x9C, 0xCC, 0xC0, 0x05, 0x5E, 0xE4, 0x45, 0x59,
		0x99, 0x9B, 0xBD, 0xD2, 0x26, 0x69, 0x96, 0x6D, 0xDD, 0xD4, 0x49, 0x9F, 0xF8, 0x8A, 0xA5, 0x50,
	},
	{
		0x88, 0x88, 0x88, 0x8C, 0xC6, 0x62, 0x2A, 0xAF, 0xFE, 0xE1, 0x1F, 0xF8, 0x82, 0x2D, 0xD4, 0x4E,
		0xE0, 0x08, 0x85, 0x58, 0x85, 0x53, 0x34, 0x44, 0x4D, 0xD7, 0x77, 0x7B, 0xB8, 0x89, 0x9D, 0xDE,
	},
	{
		0x99, 0x99, 0x99, 0x94, 0x4E, 0xE6, 0x6D, 0xD4, 0x4A, 0xAF, 0xF5, 0x59, 0x92, 0x23, 0x30, 0x06,
		0x6B, 0xBD, 0xD2, 0x2D, 0xD5, 0x52, 0x27, 0x77, 0x7D, 0xD7, 0x77, 0x7B, 0xB3, 0x39, 0x95, 0x5E,
	},
	{
		0xAA, 0xAA, 0xAA, 0xA1, 0x15, 0x5A, 0xA2, 0x25, 0x55, 0x50, 0x0B, 0xBD, 0xD2, 0x2E, 0xEA, 0xA9,
		0x9A, 0xAF, 0xF2, 0x29, 0x96, 0x64, 0x46, 0x61, 0x15, 0x56, 0x69, 0x91, 0x11, 0x11, 0x12, 0x29,
	},
	{
		0xBB, 0xBB, 0xBB, 0xB2, 0x24, 0x4D, 0xDB, 0xB7, 0x78, 0x8A, 0xA8, 0x87, 0x70, 0x06, 0x64, 0x4A,
		0xA1, 0x1F, 0xF0, 0x08, 0x8D, 0xDC, 0xC9, 0x91, 0x17, 0x79, 0x96, 0x66, 0x60, 0x00, 0x0A, 0xAF,
	},
	{
		0xCC, 0xCC, 0xCC, 0xCC, 0xC6, 0x61, 0x17, 0x71, 0x1A, 0xA5, 0x52, 0x24, 0x45, 0x5A, 0xAC, 0xCD,
		0xD2, 0x29, 0x92, 0x24, 0x46, 0x62, 0x28, 0x89, 0x90, 0x06, 0x62, 0x24, 0x4C, 0xCA, 0xA5, 0x56,
	},
	{
		0xDD, 0xDD, 0xDD, 0xDB, 0xBF, 0xFA, 0xAC, 0xC1, 0x11, 0x17, 0x70, 0x05, 0x55, 0x59, 0x9C, 0xCC,
		0xC9, 0x9B, 0xB6, 0x62, 0x28, 0x80, 0x0F, 0xF9, 0x92, 0x29, 0x95, 0x5D, 0xDF, 0xF3, 0x30, 0x00,
	},
	{
		0xEE, 0xEE, 0xEE, 0xE0, 0x08, 0x85, 0x55, 0x57, 0x77, 0x7B, 0xBD, 0xDA, 0xA7, 0x7B, 0xB8, 0x8A,
		0xA7, 0x7A, 0xAF, 0xF5, 0x58, 0x8D, 0xD1, 0x18, 0x8B, 0xB9, 0x92, 0x2F, 0xF0, 0x0D, 0xDF, 0xF7,
	},
	{
		0xFF, 0xFF, 0xFF, 0xF6, 0x68, 0x8B, 0xB7, 0x7B, 0xB8, 0x80, 0x01, 0x1B, 0xBE, 0xE6, 0x66, 0x62,
		0x2C, 0xCE, 0xEC, 0xC7, 0x74, 0x46, 0x68, 0x80, 0x0F, 0xFE, 0xE4, 0x47, 0x7D, 0xDC, 0xC1, 0x1C,
	},
};


/** 
 * \brief This function wraps \ref sha204p_sleep().
 *
 *        It puts both devices to sleep if two devices (client and host) are used.
 *        This function is also called when a Wakeup did not succeed. 
 *        This would not make sense if a device did not wakeup and it is the only
 *        device on SDA, but if there are two devices (client and host) that
 *        share SDA, the device that is not selected has also woken up.
 */
void sha204e_sleep() 
{
#if defined(SHA204_I2C) && (SHA204_CLIENT_ADDRESS != SHA204_HOST_ADDRESS)
	// Select host device...
	sha204p_set_device_id(SHA204_HOST_ADDRESS);
	// and put it to sleep.
	(void) sha204p_sleep();
	// Select client device...
	sha204p_set_device_id(SHA204_CLIENT_ADDRESS);
	// and put it to sleep.
	(void) sha204p_sleep();
#else	
	(void) sha204p_sleep();
#endif
}


/** \brief This function wakes up two I<SUP>2</SUP>C devices and puts one back to
           sleep, effectively waking up only one device among two that share the bus.
	\param[in] device_id which device to wake up
	\return status of the operation
*/
uint8_t sha204e_wakeup_device(uint8_t device_id)
{
	uint8_t ret_code;
	uint8_t wakeup_response[SHA204_RSP_SIZE_MIN];

	sha204p_set_device_id(device_id);

	// Wake up the devices.
	memset(wakeup_response, 0, sizeof(wakeup_response));
	ret_code = sha204c_wakeup(wakeup_response);
	if (ret_code != SHA204_SUCCESS) {
		sha204e_sleep();
		return ret_code;
	}

#if defined(SHA204_I2C) && (SHA204_CLIENT_ADDRESS != SHA204_HOST_ADDRESS)
	// SHA204 I2C devices can share SDA. We have to put the other device back to sleep.
	// Select other device...
	sha204p_set_device_id(device_id == SHA204_CLIENT_ADDRESS ? SHA204_HOST_ADDRESS : SHA204_CLIENT_ADDRESS);
	// and put it to sleep.
	ret_code = sha204p_sleep();
	
	// Now select the device we want to communicate with.
	sha204p_set_device_id(device_id);
#endif

	return ret_code;	
}


/** \brief This function checks the response status byte and puts the device
           to sleep if there was an error.
   \param[in] ret_code return code of function
	\param[in] response pointer to response buffer
	\return status of the operation
*/
uint8_t sha204e_check_response_status(uint8_t ret_code, uint8_t *response)
{
	if (ret_code != SHA204_SUCCESS) {
		sha204p_sleep();
		return ret_code;
	}
	ret_code = response[SHA204_BUFFER_POS_STATUS];
	if (ret_code != SHA204_SUCCESS)
		sha204p_sleep();

	return ret_code;	
}


/** \brief This function reads the serial number from the device.
 *
           The serial number is stored in bytes 0 to 3 and 8 to 12
           of the configuration zone.
   \param[in] tx_buffer pointer to transmit buffer.
	\param[out] sn pointer to nine-byte serial number
	\return status of the operation
*/

uint8_t sha204e_read_serial_number(uint8_t *tx_buffer, uint8_t *sn)
{
	uint8_t rx_buffer[READ_32_RSP_SIZE];
	
	uint8_t status = sha204m_read(tx_buffer, rx_buffer, 
						SHA204_ZONE_COUNT_FLAG | SHA204_ZONE_CONFIG, 0);
	if (status != SHA204_SUCCESS)
		sha204p_sleep();
	
	memcpy(sn, &rx_buffer[SHA204_BUFFER_POS_DATA], 4);
	memcpy(sn + 4, &rx_buffer[SHA204_BUFFER_POS_DATA + 8], 5);
	
	return status;
}


/** \brief This function configures a child and parent key for derived key scenarios.
 *
 *         To run this scenario successfully the client device has
 *         to be configured first: We use a key slot in the client device that is already
 *         configured for this purpose, but we need to point to a parent whose
 *         CheckOnly flag is set on the host device. On the client device we have
 *         to reset this bit, otherwise the DeriveKey command would return an error.
 *         Key id 10 is chosen for the child key because only its parent key needs to be changed
 *         from its default configuration. Key id 13 is chosen for the parent key because only
 *         its CheckOnly flag has to be reset compared to its default configuration.
    \return status of the operation
*/
uint8_t sha204e_configure_key()
{
	// declared as "volatile" for easier debugging
	volatile uint8_t ret_code;

	const uint8_t config_child = 0x7D;
	const uint8_t config_parent = 0xCD;
	const uint8_t config_address = 32;
	
	// Make the command buffer the long size (32 bytes, no MAC) of the Write command.
	uint8_t command[WRITE_COUNT_LONG];
	
	uint8_t data_load[SHA204_ZONE_ACCESS_32];

	// Make the response buffer the size of a Read response.
	uint8_t response[READ_32_RSP_SIZE];

	// Wake up the client device.
	ret_code = sha204e_wakeup_device(0);
	if (ret_code != SHA204_SUCCESS)
		return ret_code;
	
	// Read client device configuration for child key.
	memset(response, 0, sizeof(response));
	ret_code = sha204m_read(command, response, SHA204_ZONE_COUNT_FLAG | SHA204_ZONE_CONFIG, config_address);
	if (ret_code != SHA204_SUCCESS) {
		sha204p_sleep();
		return ret_code;
	}

	// Check whether we configured already. If so, exit here.
	if ((response[SHA204_BUFFER_POS_DATA + 9] == config_child)
		&& (response[SHA204_BUFFER_POS_DATA + 14] == config_parent)) {
		sha204p_sleep();
		return ret_code;
	}

	// Write client configuration.
	memcpy(data_load, &response[SHA204_BUFFER_POS_DATA], sizeof(data_load));
	data_load[9] = config_child;
	data_load[14] = config_parent;
	ret_code = sha204m_write(command, response, SHA204_ZONE_COUNT_FLAG | SHA204_ZONE_CONFIG,
							config_address, data_load, NULL);
	if (ret_code != SHA204_SUCCESS) {
		sha204p_sleep();
		return ret_code;
	}

	sha204p_sleep();
	
	return ret_code;
}


/** \brief This function configures the client for the derived key and
 *         diversified key example.
 *
 *         Creating a derived key allows a host device to check a MAC
 *         in a highly secure fashion. No replay attacks are possible
 *         and SHA256 calculation in firmware is not needed.
 * \return status of the operation
 */
uint8_t sha204e_configure_derive_key()
{
	// declared as "volatile" for easier debugging
	volatile uint8_t ret_code;

	// Configure key.
	ret_code = sha204e_configure_key();
	if (ret_code != SHA204_SUCCESS)
		return ret_code;
	
#if (SHA204_EXAMPLE_CONFIG_WITH_LOCK != 0)
	ret_code = sha204e_lock_config_zone(SHA204_HOST_ADDRESS);
#endif

	return ret_code;
}


/** \brief This function reads all 88 bytes from the configuration zone.
 *
Obtain the data by putting a breakpoint after every read and inspecting "response".

<b>Factory Defaults of Configuration Zone</b><BR>
01 23 76 ab 00 04 05 00 0c 8f b7 bd ee 55 01 00 c8 00 55 00 8f 80 80 a1 82 e0 a3 60 94 40 a0 85<BR>
86 40 87 07 0f 00 89 f2 8a 7a 0b 8b 0c 4c dd 4d c2 42 af 8f ff 00 ff 00 ff 00 1f 00 ff 00 1f 00<BR>
ff 00 ff 00 1f ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 00 00 55 55<BR>

<b>Slot Summary</b><BR>
Slot 1 is parent key, and slot 1 is child key (DeriveKey-Roll).\n
Slot 2 is parent key, and slot 0 is child key (DeriveKey-Roll).\n
Slot 3 is parent key, and child key has to be given in Param2 (DeriveKey-Roll).\n
Slots 4, 13, and 14 are CheckOnly.\n
Slots 5 and 15 are single use.\n
Slot 8 is plain text.\n
Slot 10 is parent key and slot 10 is child key (DeriveKey-Create).\n
Slot 12 is not allowed as target.\n

<b>Slot Details</b><BR>
Byte # \t          Name    \t\t\t  Value \t\t\t  Description\n
0 - 3 \t   SN[0-3]           \t\t 012376ab   \t part of the serial number\n
4 - 7 \t   RevNum            \t\t 00040500   \t device revision (= 4)\n
8 - 12\t   SN[4-8]           \t\t 0c8fb7bdee \t part of the serial number\n
13    \t\t Reserved        \t\t\t 55       \t\t set by Atmel (55: First 16 bytes are unlocked / special case.)\n
14    \t\t I2C_Enable        \t\t 01       \t\t SWI / I2C (1: I2C)\n
15    \t\t Reserved        \t\t\t 00       \t\t set by Atmel\n
16    \t\t I2C_Address       \t\t c8       \t\t default I2C address\n
17    \t\t RFU         \t\t\t\t\t 00       \t\t reserved for future use; must be 0\n
18    \t\t OTPmode         \t\t\t 55       \t\t 55: consumption mode, not supported at this time\n
19    \t\t SelectorMode      \t\t 00       \t\t 00: Selector can always be written with UpdateExtra command.\n
20    \t\t slot  0, read   \t\t\t 8f       \t\t 8: Secret. f: Does not matter.\n
21    \t\t slot  0, write  \t\t\t 80       \t\t 8: Never write. 0: Does not matter.\n
22    \t\t slot  1, read   \t\t\t 80       \t\t 8: Secret. 0: CheckMac copy\n
23		\t\t slot  1, write  \t\t\t a1       \t\t a: MAC required (roll). 1: key id\n
24		\t\t slot  2, read   \t\t\t 82       \t\t 8: Secret. 2: Does not matter.\n
25		\t\t slot  2, write  \t\t\t e0       \t\t e: MAC required (roll) and write encrypted. 0: key id\n
26		\t\t slot  3, read   \t\t\t a3       \t\t a: Single use. 3: Does not matter.\n
27		\t\t slot  3, write  \t\t\t 60       \t\t 6: Encrypt, MAC not required (roll). 0: Does not matter.\n
28		\t\t slot  4, read   \t\t\t 94       \t\t 9: CheckOnly. 4: Does not matter.\n
29		\t\t slot  4, write  \t\t\t 40       \t\t 4: Encrypt. 0: key id\n
30		\t\t slot  5, read   \t\t\t a0       \t\t a: Single use. 0: key id\n
31		\t\t slot  5, write  \t\t\t 85       \t\t 8: Never write. 5: Does not matter.\n
32		\t\t slot  6, read   \t\t\t 86       \t\t 8: Secret. 6: Does not matter.\n
33		\t\t slot  6, write  \t\t\t 40       \t\t 4: Encrypt. 0: key id\n
34		\t\t slot  7, read   \t\t\t 87       \t\t 8: Secret. 7: Does not matter.\n
35		\t\t slot  7, write  \t\t\t 07       \t\t 0: Write. 7: Does not matter.\n
36		\t\t slot  8, read   \t\t\t 0f       \t\t 0: Read. f: Does not matter.\n
37		\t\t slot  8, write  \t\t\t 00       \t\t 0: Write. 0: Does not matter.\n
38		\t\t slot  9, read   \t\t\t 89       \t\t 8: Secret. 9: Does not matter.\n
39		\t\t slot  9, write  \t\t\t f2       \t\t f: Encrypt, MAC required (create). 2: key id\n
40		\t\t slot 10, read   \t\t\t 8a       \t\t 8: Secret. a: Does not matter.\n
41		\t\t slot 10, write  \t\t\t 7a       \t\t 7: Encrypt, MAC not required (create). a: key id\n
42		\t\t slot 11, read   \t\t\t 0b       \t\t 0: Read. b: Does not matter.\n
43		\t\t slot 11, write  \t\t\t 8b       \t\t 8: Never Write. b: Does not matter.\n
44		\t\t slot 12, read   \t\t\t 0c       \t\t 0: Read. c: Does not matter.\n
45		\t\t slot 12, write  \t\t\t 4c       \t\t 4: Encrypt, not allowed as target. c: key id\n
46		\t\t slot 13, read   \t\t\t dd       \t\t d: CheckOnly. d: key id\n
47		\t\t slot 13, write  \t\t\t 4d       \t\t 4: Encrypt, not allowed as target. d: key id\n
48		\t\t slot 14, read   \t\t\t c2       \t\t c: CheckOnly. 2: key id\n
49		\t\t slot 14, write  \t\t\t 42       \t\t 4: Encrypt. 2: key id\n
50		\t\t slot 15, read   \t\t\t af       \t\t a: Single use. f: Does not matter.\n
51		\t\t slot 15, write  \t\t\t 8f       \t\t 8: Never write. f: Does not matter.\n
52		\t\t UseFlag 0     \t\t\t\t ff       \t\t 8 uses\n
53		\t\t UpdateCount 0     \t\t 00       \t\t count = 0\n
54		\t\t UseFlag 1     \t\t\t\t ff       \t\t 8 uses\n
55		\t\t UpdateCount 1     \t\t 00       \t\t count = 0\n
56		\t\t UseFlag 2     \t\t\t\t ff       \t\t 8 uses\n
57		\t\t UpdateCount 2     \t\t 00       \t\t count = 0\n
58		\t\t UseFlag 3     \t\t\t\t 1f       \t\t 5 uses\n
59		\t\t UpdateCount 3     \t\t 00       \t\t count = 0\n
60		\t\t UseFlag 4     \t\t\t\t ff       \t\t 8 uses\n
61		\t\t UpdateCount 4     \t\t 00       \t\t count = 0\n
62		\t\t UseFlag 5     \t\t\t\t 1f       \t\t 5 uses\n
63		\t\t UpdateCount 5     \t\t 00       \t\t count = 0\n
64		\t\t UseFlag 6     \t\t\t\t ff       \t\t 8 uses\n
65		\t\t UpdateCount 6     \t\t 00       \t\t count = 0\n
66		\t\t UseFlag 7     \t\t\t\t ff       \t\t 8 uses\n
67		\t\t UpdateCount 7     \t\t 00       \t\t count = 0\n
68 - 83 \t LastKeyUse      \t\t\t 1fffffffffffffffffffffffffffffff\n
84		\t\t UserExtra\n
85		\t\t Selector    \t\t\t\t\t 00       \t\t Pause command with chip id 0 leaves this device active.\n
86		\t\t LockValue     \t\t\t\t 55       \t\t OTP and Data zones are not locked.\n
87		\t\t LockConfig    \t\t\t\t 55       \t\t Configuration zone is not locked.\n

 * \param[in]  device_id host or client device
 * \param[out] config_data pointer to all 88 bytes in configuration zone.
               Not used if NULL.
 * \return status of the operation
 */
uint8_t sha204e_read_config_zone(uint8_t device_id, uint8_t *config_data)
{
	// declared as "volatile" for easier debugging
	volatile uint8_t ret_code;
	
	uint16_t config_address;
	
	// Make the command buffer the size of the Read command.
	uint8_t command[READ_COUNT];

	// Make the response buffer the size of the maximum Read response.
	uint8_t response[READ_32_RSP_SIZE];
	
	// Use this buffer to read the last 24 bytes in 4-byte junks.
	uint8_t response_read_4[READ_4_RSP_SIZE];
	
	uint8_t *p_response;

	//sha204p_init();

	//sha204p_set_device_id(device_id);

	// Read first 32 bytes. Put a breakpoint after the read and inspect "response" to obtain the data.
	ret_code = sha204c_wakeup(response);
	if (ret_code != SHA204_SUCCESS)
		return ret_code;
		
	memset(response, 0, sizeof(response));
	config_address = 0;
	ret_code = sha204m_read(command, response, SHA204_ZONE_CONFIG | READ_ZONE_MODE_32_BYTES, config_address);
	sha204p_sleep();
	if (ret_code != SHA204_SUCCESS)
		return ret_code;
		
	if (config_data) {
		memcpy(config_data, &response[SHA204_BUFFER_POS_DATA], SHA204_ZONE_ACCESS_32);
		config_data += SHA204_ZONE_ACCESS_32;
	}		
	// Read second 32 bytes. Put a breakpoint after the read and inspect "response" to obtain the data.
	memset(response, 0, sizeof(response));
	ret_code = sha204c_wakeup(response);
	if (ret_code != SHA204_SUCCESS)
		return ret_code;

	config_address += SHA204_ZONE_ACCESS_32;
	memset(response, 0, sizeof(response));
	ret_code = sha204m_read(command, response, SHA204_ZONE_CONFIG | READ_ZONE_MODE_32_BYTES, config_address);
	sha204p_sleep();
	if (ret_code != SHA204_SUCCESS)
		return ret_code;
		
	if (config_data) {
		memcpy(config_data, &response[SHA204_BUFFER_POS_DATA], SHA204_ZONE_ACCESS_32);
		config_data += SHA204_ZONE_ACCESS_32;
	}
		
	// Read last 24 bytes in six four-byte junks.
	memset(response, 0, sizeof(response));
	ret_code = sha204c_wakeup(response);
	if (ret_code != SHA204_SUCCESS)
		return ret_code;
	
	config_address += SHA204_ZONE_ACCESS_32;
	response[SHA204_BUFFER_POS_COUNT] = 0;
	p_response = &response[SHA204_BUFFER_POS_DATA];
	memset(response, 0, sizeof(response));
	while (config_address < SHA204_CONFIG_SIZE) {
		memset(response_read_4, 0, sizeof(response_read_4));
		ret_code = sha204m_read(command, response_read_4, SHA204_ZONE_CONFIG, config_address);
		if (ret_code != SHA204_SUCCESS) {
			sha204p_sleep();
			return ret_code;
		}
		memcpy(p_response, &response_read_4[SHA204_BUFFER_POS_DATA], SHA204_ZONE_ACCESS_4);
		p_response += SHA204_ZONE_ACCESS_4;
		response[SHA204_BUFFER_POS_COUNT] += SHA204_ZONE_ACCESS_4; // Update count byte in virtual response packet.
		config_address += SHA204_ZONE_ACCESS_4;
	}	
	// Put a breakpoint here and inspect "response" to obtain the data.
	sha204p_sleep();
		
	if (ret_code == SHA204_SUCCESS && config_data)
		memcpy(config_data, &response[SHA204_BUFFER_POS_DATA], SHA204_CONFIG_SIZE - 2 * SHA204_ZONE_ACCESS_32);

	return ret_code;
}
