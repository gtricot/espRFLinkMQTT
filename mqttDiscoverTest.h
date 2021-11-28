#define MAX_FIELDS 10
#define MAX_DATA_LEN 24
#define BUFFER_SIZE 200
#define MAX_ID_LEN 10 
#define MAX_TOPIC_LEN 100 
#define MAX_MQTT_DISCOVERY_JSON_LEN 400 

#define MQTT_PUBLISH_TOPIC "rflink"	
#define MQTT_DISCOVERY_PREFIX "homeassistant/sensor"	// MQTT discovery topic prefix
#define MQTT_DISCOVERY_SUFFIX "config"					// MQTT discovery topic suffix

typedef struct {
    char fieldName[20];
    char deviceClass[20];
    char unitOfMeasurement[10];
} _discoveryInfo;

_discoveryInfo discoveryInfoDict[] = {
    {"TEMP", "temperature", "°C"},
    {"HUM", "humidity", "%"}
};
const int discoveryInfoSize = (int) (sizeof(discoveryInfoDict) / sizeof(discoveryInfoDict[0])); 

// message builder buffers
char MQTT_NAME[MAX_DATA_LEN];
char MQTT_ID  [MAX_ID_LEN+1];
char MQTT_TOPIC[MAX_TOPIC_LEN];
char FIELD_BUF[MAX_DATA_LEN];

char JSON_FIELDS[MAX_FIELDS][MAX_DATA_LEN];
int JSON_FIELDS_COUNT;

// MQTT discovery variables
char MQTT_DISCOVERY_UID[MAX_DATA_LEN*2+MAX_ID_LEN+1];
char MQTT_DISCOVERY_NAME[MAX_DATA_LEN*2+MAX_ID_LEN+1];
char MQTT_DISCOVERY_TOPIC[MAX_TOPIC_LEN];
char MQTT_DISCOVERY_JSON[MAX_MQTT_DISCOVERY_JSON_LEN];

void readRfLinkPacket(char* line);
void readRfLinkFields(char *fields, int start);
void getDiscoveryInfoFromFieldName(char* fieldName, _discoveryInfo* info);
void publishMqttDiscoveryConfig(char* field);