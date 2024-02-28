#include <windows.h>
#include <stdio.h>
#include <winhttp.h>
#include <string.h>
#include "token.h"
#define HANDLEERR(EXPRESSION, METHOD)                        \
	if (!(EXPRESSION))                                       \
	{                                                        \
		printf("Error %u in %s.\n", GetLastError(), METHOD); \
		exit(1);                                             \
	}

int main(){
	HINTERNET hSession = WinHttpOpen(L"My Discord Bot/1.0 ",
									 WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
									 WINHTTP_NO_PROXY_NAME,
									 WINHTTP_NO_PROXY_BYPASS,
									 0);
	HANDLEERR(hSession, "WinHttpOpen");
	HINTERNET hCommandConnect = WinHttpConnect(hSession, L"discord.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
	HANDLEERR(hCommandConnect, "WinHttpConnect");
	HINTERNET hCommandRequest = WinHttpOpenRequest(hCommandConnect, L"POST", L"/api/v8/channels/777288013981155339/messages", NULL, L"www.discord.com", WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	HANDLEERR(hCommandRequest, "WinHttpOpenRequest");
	LPCWSTR additionalHeaders = L"Content-Type: application/json\r\nAuthorization: Bot " L_BOT_TOKEN L"\r\n" ;
	const char* body = "{\"content\":\"PONG!\"}";
	wprintf(additionalHeaders);
	printf(body);
	BOOL bResults = WinHttpSendRequest(hCommandRequest, additionalHeaders, wcslen(additionalHeaders), (LPVOID)body, strlen(body), strlen(body), 0);
	HANDLEERR(bResults, "WinHttpSendRequest");
	HANDLEERR(WinHttpReceiveResponse(hCommandRequest, NULL), "WinHttpReceiveResponse");
	if (bResults)
    {
    	DWORD dwStatusCode = 0;
		DWORD dwSize = sizeof(dwStatusCode);

	WinHttpQueryHeaders(hCommandRequest, 
    	WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, 
    	WINHTTP_HEADER_NAME_BY_INDEX, 
    	&dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
        if (bResults)
        {
            printf("Status code = %d.\n", dwStatusCode);
        }
		HANDLEERR(bResults,"WinHttpQueryHeaders")
    }

}