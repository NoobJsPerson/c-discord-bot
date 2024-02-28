#include <windows.h>
#include <winhttp.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include "token.h"

HINTERNET hWebsocket;
int intervalDuration;
int sequence = 0;
pthread_mutex_t mutex;

#define HANDLEERR(EXPRESSION, METHOD)                        \
	if (!(EXPRESSION))                                       \
	{                                                        \
		printf("Error %u in %s.\n", GetLastError(), METHOD); \
		exit(1);                                             \
	}

int getSequence(char *json)
{
	regex_t regexCompiled;
	regmatch_t groupArray[2];
	char regexString[] = "\"s\":(.*?),";
	int comp = regcomp(&regexCompiled, regexString, REG_EXTENDED);
	// printf("%d\n",comp);
	HANDLEERR(comp == 0, "regcomp");
	int match = regexec(&regexCompiled, json, 2, groupArray, 0);
	char sourceCopy[strlen(json) + 1];
	strcpy(sourceCopy, json);
	sourceCopy[groupArray[1].rm_eo] = 0;
	if (strcmp(sourceCopy + groupArray[1].rm_so, "null") == 0) return 0;
	regfree(&regexCompiled);
	return atoi(sourceCopy + groupArray[1].rm_so);
};

// void *listenerLoop(void *nothing)
// {
// 	WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
// 	printf("am i even looping\n");
// 	char inLoopJSON[2000];
// 	DWORD beenRead = 0;
// 	pthread_mutex_lock(&mutex);
// 	DWORD Recieved = WinHttpWebSocketReceive(hWebsocket, inLoopJSON, 2000, &beenRead, &bufferType);
// 	pthread_mutex_unlock(&mutex);
// 	if (Recieved != NO_ERROR) return listenerLoop(nothing);
// 	inLoopJSON[beenRead] = 0;
// 	printf("%s\n", inLoopJSON);
// 	Sleep(intervalDuration);
// 	return listenerLoop(nothing);
// }

void *heartbeatLoop(void *nothing)
{
	printf("a heartbeat loop iteration has started\n");
	char templateJSON[] = "{\"op\":1,\"d\":%d}";
	char *usedJSON = calloc(21, sizeof(char));
	char recievedJSON[30];
	sprintf(usedJSON, templateJSON, sequence);
	printf("usedJSON: %s\n", usedJSON);
	WINHTTP_WEB_SOCKET_BUFFER_TYPE sendBuffType = WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE;
	WINHTTP_WEB_SOCKET_BUFFER_TYPE recieveBuffType;
	pthread_mutex_lock(&mutex);
	DWORD sendCode = WinHttpWebSocketSend(hWebsocket, sendBuffType, usedJSON, strlen(usedJSON));
	HANDLEERR(sendCode == NO_ERROR, "WinHttpWebSocketSend");
	// DWORD beenRead;
	// DWORD Recieved = WinHttpWebSocketReceive(hWebsocket, recievedJSON, 531, &beenRead, &recieveBuffType);
	pthread_mutex_unlock(&mutex);
	// printf("beenRead: %d\n", beenRead);
	// recievedJSON[beenRead] = 0;
	// HANDLEERR(Recieved == NO_ERROR, "WinHttpWebSocketReceive");
	// if(Recieved == NO_ERROR) printf("the response has been pick by the main thread (from the heartbeat thread)\n");
	// printf("%s\n", recievedJSON);
	// HANDLEERR(recievedJSON[0] == '{', "hearbeatLoop");
	free(usedJSON);
	Sleep(intervalDuration);
	return heartbeatLoop(nothing);
}

int main()
{
	// Initialize the WinHTTP library
	pthread_mutex_init(&mutex, NULL);
	BOOL RequestSent = FALSE;
	char json[201];
	// HANDLEERR(json != NULL, calloc);
	HINTERNET hSession = WinHttpOpen(L"My Discord Bot/1.0",
									 WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
									 WINHTTP_NO_PROXY_NAME,
									 WINHTTP_NO_PROXY_BYPASS,
									 0);
	HANDLEERR(hSession, "WinHttpOpen");
	HINTERNET hConnect = WinHttpConnect(hSession, L"localhost", INTERNET_DEFAULT_HTTPS_PORT, 0);
	HANDLEERR(hConnect, "WinHttpConnect");
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/", NULL, L"localhost", WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_BYPASS_PROXY_CACHE);
	HANDLEERR(hRequest, "WinHttpOpenRequest");
	HANDLEERR(WinHttpSetOption(hRequest, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0), "WinHttpSetOption");
	RequestSent = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	HANDLEERR(RequestSent, "WinHttpSendRequest");
	HANDLEERR(WinHttpReceiveResponse(hRequest, NULL), "WinHttpReceiveResponse");
	DWORD dwStatusCode = 0;
	DWORD dwSize = sizeof(dwStatusCode);
	WinHttpQueryHeaders(hRequest,
						WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
						WINHTTP_HEADER_NAME_BY_INDEX,
						&dwStatusCode,
						&dwSize,
						WINHTTP_NO_HEADER_INDEX);

	// Print the status code to the console.
	printf("Status code: %d\n", dwStatusCode);
	hWebsocket = WinHttpWebSocketCompleteUpgrade(hRequest, 0);
	HANDLEERR(hWebsocket, "WinHttpWebSocketCompleteUpgrade");
	DWORD beenRead;
	WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
	DWORD Recieved = WinHttpWebSocketReceive(hWebsocket, json, 200, &beenRead, &bufferType);
	// printf("%u\n", Recieved);
	HANDLEERR(Recieved == NO_ERROR, "WinHttpWebSocketReceive");
	json[beenRead] = 0;
	printf("%s\n", json);
	regex_t regexCompiled;
	regmatch_t groupArray[2];
	char regexString[] = "\"heartbeat_interval\":(.*),\"";
	int comp = regcomp(&regexCompiled, regexString, REG_EXTENDED);
	// printf("%d\n",comp);
	HANDLEERR(comp == 0, "regcomp");
	int match = regexec(&regexCompiled, json, 2, groupArray, 0);
	char sourceCopy[strlen(json) + 1];
	strcpy(sourceCopy, json);
	sourceCopy[groupArray[1].rm_eo] = 0;
	intervalDuration = atoi(sourceCopy + groupArray[1].rm_so);
	regfree(&regexCompiled);
	printf("interval: %d\n", intervalDuration);
	printf("started sleep\n");
	unsigned int random;
	rand_s(&random);
	int firstDuration = intervalDuration * ((double)random / UINT_MAX); // interval * jitter as mentioned in the discord documentation
	Sleep(firstDuration);
	printf("ended sleep\n");
	pthread_t thread;
	pthread_create(&thread, NULL, heartbeatLoop, NULL);
	// char identifyTemplate[] = "{ \"op\": 2, \"d\": { \"token\": \"%s\", \"intents\": 36864, \"properties\": { \"os\": \"windows\","
	// 						  "\"browser\": \"winhttp\","
	// 						  "\"device\": \"winhttp\"}}}";
	// char identifyBody[200];
	// sprintf(identifyBody, identifyTemplate, BOT_TOKEN);
	// // printf("%s\n",identifyBody);
	// char sentJSON[1000];
	// char recievedJSON[2000];
	// pthread_mutex_lock(&mutex);
	// DWORD sendCode = WinHttpWebSocketSend(hWebsocket, WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE, identifyBody, strlen(identifyBody));
	// HANDLEERR(sendCode == NO_ERROR, "WinHttpWebSocketSend");
	// Recieved = WinHttpWebSocketReceive(hWebsocket, recievedJSON, 2000, &beenRead, &bufferType);
	// pthread_mutex_unlock(&mutex);
	// HANDLEERR(Recieved == NO_ERROR, "WinHttpWebSocketReceive");
	// printf("%s\n", recievedJSON);
	// printf("%u\n", Recieved);
	// sequence = getSequence(recievedJSON);
	// printf("the updated sequence is %d\n", sequence);
	// HINTERNET hHTTPConnect = WinHttpConnect(hSession, )
	for(;;){
		char inLoopJSON[2000];
		Recieved = WinHttpWebSocketReceive(hWebsocket, inLoopJSON, 2000, &beenRead, &bufferType);
		if (Recieved == NO_ERROR) {
			inLoopJSON[beenRead] = 0;
			printf("result of in main thread recieve: %s\n", inLoopJSON);
		}
	}

	// pthread_create(&thread, NULL, listenerLoop, NULL);

	// // Clean up the request handles
	// WinHttpCloseHandle(hWebsocket);
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	pthread_mutex_destroy(&mutex);
	pthread_exit(NULL);

	return 0;
}
