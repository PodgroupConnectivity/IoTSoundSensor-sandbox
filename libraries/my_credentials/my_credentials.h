// WiFi Credentials
#define WSSID "REPLACE WITH YOUR WIFI SSID" 
#define WPASS "REPLACE WITH YOUR WIFI PASS" 

// Losant IoT Platform credentials
#define DEVICE_ID "REPLACE WITH YOUR DEVICE ID"
#define	ACCESS_KEY "REPLACE WITH YOUR ACCESS KEY"
#define	ACCESS_SECRET "REPLACE WITH YOUR ACCES SECRET"

// -------------------------------
// The Things Network Credentials
// -------------------------------

// Generic ABP authentication method. C-style.
#define NETSKEY	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define	APPSKEY	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define	DEVADDR	0xFFFFFFFF // Replace with your device address

// Generic OTAA authentication method. C-style.
#define DEVEUIS {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} // replace with yours in lsb format
#define APPEUIS {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} // replace with yours in lsb format
#define APPKEYS {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} // replace with yours in msb format

// ABP authentication method just for the Dragino examples.
#define	NETSKEYDRAG "REPLACE WITH YOUR NETWORK SESSION KEY"
#define	APPSKEYDRAG "REPLACE WITH YOUR APPLICATION SESSION KEY"
#define	DEVADDRDRAG "REPLACE WITH YOUR DEVICE ADRESS"
