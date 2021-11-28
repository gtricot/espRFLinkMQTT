#include <stdio.h>
#include <string.h>
#include "mqttDiscoverTest.h"

int main()
{
    printf("\nMQTT Discover test program\n\n");

    char *packet = strdup("20;99;Xiron;ID=5D02;TEMP=005e;HUM=74;BAT=OK;");
    readRfLinkPacket(packet);

    // For each JSON field, build MQTT discovery variables
    for (int i = 0; i < JSON_FIELDS_COUNT; i++)
    {
        printf("MQTT Discovery for field %s\n", JSON_FIELDS[i]);
        publishMqttDiscoveryConfig(JSON_FIELDS[i]);
    }

    return 0;
}

void readRfLinkPacket(char* line) {
        int i = 6; // ignore message type and line number
        int j = 0;
        bool nameHasEq = false;
        bool nameHasDash = false;
		
        // get name : 3rd field (begins at char 6)
        while(line[i] != ';' && i < BUFFER_SIZE && j < MAX_DATA_LEN) {
                if      (line[i]==' ') MQTT_NAME[j] = '_';
                else if (line[i]=='=')  { nameHasEq = true; break; }
                else MQTT_NAME[j] = line[i];
		if (line[i]=='-')  { nameHasDash = true; }
                i++; j++;
        }

        // ends string correctly
        MQTT_NAME[j] = '\0';

        // // if name contains "=", assumes that it's an rflink message, not an RF packet
        // // thus we put a special name and ID=0, then parse to JSON
        // if(nameHasEq==true) {
        //         //Serial.println(F("name contains '=' !"));
        //         i = 6;
        //         strcpy_P(MQTT_NAME,PSTR("message"));
        //         MQTT_ID[0]='0'; MQTT_ID[1]='\0';
        //         readRfLinkFields(line, i);
        //         return;
        // }

        // for all other messages, get MQTT_ID (4th field) then convert to json
        j=0;
        i+=4; // skip ";MQTT_ID="

        while(line[i] != ';' && i < BUFFER_SIZE && j < MAX_ID_LEN) {
                MQTT_ID[j++] = line[i++];
        }
        MQTT_ID[j] = '\0';

        // continue with json convertion
        readRfLinkFields(line, i+1);
}

void readRfLinkFields(char *fields, int start)
{
    int strpos = start;
    int fldpos = 0;
    JSON_FIELDS_COUNT = 0;

    while (strpos < 200 - start && fields[strpos] != '\n' && fields[strpos] != '\0')
    {
        // if current char is "=", we end name parsing and start parsing the field's value
        if (fields[strpos] == '=')
        {
            FIELD_BUF[fldpos] = '\0';
            fldpos = 0;

            // Stock field for MQTT discovery publication
            strcpy(JSON_FIELDS[JSON_FIELDS_COUNT], FIELD_BUF);
            printf("JSON_FIELDS for index %i set to %s = %s \n", JSON_FIELDS_COUNT, FIELD_BUF, JSON_FIELDS[JSON_FIELDS_COUNT]);
            JSON_FIELDS_COUNT++;

            // if current char is ";", we end parsing value and start parsing another field's name
        }
        else if (fields[strpos] == ';')
        {

            FIELD_BUF[fldpos] = '\0';
            fldpos = 0;
        }
        else
        { // default case : copy current char
            FIELD_BUF[fldpos++] = fields[strpos];
        }

        strpos++;
    }
}


void publishMqttDiscoveryConfig(char* field) {

	// MQTT Discovery unique ID
        int uid_size = sprintf(MQTT_DISCOVERY_UID, "%s-%s_%s", MQTT_NAME, MQTT_ID, field);
        printf("MQTT_DISCOVERY_UID = '%s' - Size = %i\n", MQTT_DISCOVERY_UID, uid_size);

	// MQTT Discovery name
        int name_size = sprintf(MQTT_DISCOVERY_NAME, "%s %s %s", MQTT_NAME, MQTT_ID, field);
        printf("MQTT_DISCOVERY_NAME = '%s' - Size = %i\n", MQTT_DISCOVERY_NAME, name_size);

	// MQTT Discovery topic
        int topic_size = sprintf(MQTT_DISCOVERY_TOPIC, "%s/%s/%s/%s", MQTT_DISCOVERY_PREFIX, MQTT_PUBLISH_TOPIC, MQTT_DISCOVERY_UID, MQTT_DISCOVERY_SUFFIX);
        printf("MQTT_DISCOVERY_TOPIC = '%s' - Size = %i\n", MQTT_DISCOVERY_TOPIC, topic_size);

	// MQTT Discovery JSON
	MQTT_DISCOVERY_JSON[0] = '\0';
	strcpy(MQTT_DISCOVERY_JSON,"{\"unique_id\":\"");
	strcat(MQTT_DISCOVERY_JSON,MQTT_DISCOVERY_UID);
	strcat(MQTT_DISCOVERY_JSON,"\",\"name\":\"");
	strcat(MQTT_DISCOVERY_JSON,MQTT_DISCOVERY_NAME);
        _discoveryInfo* discoveryInfo;
        getDiscoveryInfoFromFieldName(field, discoveryInfo);
        if (discoveryInfo) {
                strcat(MQTT_DISCOVERY_JSON,"\",\"device_class\":\"");
                strcat(MQTT_DISCOVERY_JSON,discoveryInfo->deviceClass);
                strcat(MQTT_DISCOVERY_JSON,"\",\"unit_of_measurement\":\"");
                strcat(MQTT_DISCOVERY_JSON,discoveryInfo->unitOfMeasurement);
        }
	strcat(MQTT_DISCOVERY_JSON,"\",\"state_topic\":\"");
	strcat(MQTT_DISCOVERY_JSON,MQTT_TOPIC);
	strcat(MQTT_DISCOVERY_JSON,"\",\"value_template\":\"{{ value_json.");
	strcat(MQTT_DISCOVERY_JSON,field);
	strcat(MQTT_DISCOVERY_JSON," }}\",\"device\":{\"identifiers\":[\"");
	strcat(MQTT_DISCOVERY_JSON,MQTT_ID);
	strcat(MQTT_DISCOVERY_JSON,"\"],\"manufacturer\":\"");
	strcat(MQTT_DISCOVERY_JSON,MQTT_NAME);
	strcat(MQTT_DISCOVERY_JSON,"\",\"model\":\"");
	strcat(MQTT_DISCOVERY_JSON,MQTT_NAME);
	strcat(MQTT_DISCOVERY_JSON,"_");
	strcat(MQTT_DISCOVERY_JSON,MQTT_ID);
	strcat(MQTT_DISCOVERY_JSON,"\",\"name\":\"");
	strcat(MQTT_DISCOVERY_JSON,MQTT_NAME);
	strcat(MQTT_DISCOVERY_JSON," ");
	strcat(MQTT_DISCOVERY_JSON,MQTT_ID);
	strcat(MQTT_DISCOVERY_JSON,"\"}}");
        printf("MQTT_DISCOVERY_JSON = %s - Size = %li\n", MQTT_DISCOVERY_JSON, strlen(MQTT_DISCOVERY_JSON));

}

void getDiscoveryInfoFromFieldName(char* fieldName, _discoveryInfo* info) {
        printf("Retrieving discoveryInfo from fieldName '%s'\n", fieldName);
        for (int i = 0; i < (discoveryInfoSize); i++){
                if (strcmp(fieldName,discoveryInfoDict[i].fieldName) == 0) {
                        info = &discoveryInfoDict[i];
                        printf("Found discoveryInfo for fieldName '%s' - deviceClass = '%s' - unitOfMeasurement = '%s'\n", 
                                fieldName, info->deviceClass, info->unitOfMeasurement);
                }
        }
}

void addJSONField(char* jsonBuffer, char* fieldName, char* fieldValue, bool quotedValue) {
        if (quotedValue) {
                sprintf(jsonBuffer, "\"%s\":\"%s\"", fieldName, fieldValue);
        } else {
                sprintf(jsonBuffer, "\"%s\":%s", fieldName, fieldValue);
        }
}