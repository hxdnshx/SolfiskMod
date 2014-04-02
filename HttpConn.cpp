#include "pch.hpp"
#include "HttpConn.h"
#include "Solfisk.hpp"
#include <Winhttp.h>

#pragma comment(lib,"winhttp")

#define MINIMAL_USE_PROCESSHEAPSTRING
#include "MinimalPath.hpp"

#define MINIMAL_USE_PROCESSHEAPARRAY
#include "MinimalArray.hpp"

using Minimal::MinimalStringT;

int HttpRequestGet(Minimal::MinimalStringT<wchar_t> &host,Minimal::MinimalStringT<wchar_t> &addr, Minimal::MinimalStringT<char> &receive)
{
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer;
	int err=0;
    BOOL bResults = FALSE;
    HINTERNET hSession = NULL, 
					   hConnect = NULL, 
                       hRequest = NULL;

	hSession = WinHttpOpen(_T("Solfisk"), 
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 
                           WINHTTP_NO_PROXY_NAME, 
                           WINHTTP_NO_PROXY_BYPASS, 0);

	if (hSession)
        hConnect = WinHttpConnect(hSession, host.GetRaw(), 
                                  INTERNET_DEFAULT_HTTP_PORT, 0);

    // Create an HTTP Request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, _T("GET"), 
                                      addr.GetRaw(), 
                                      NULL, WINHTTP_NO_REFERER, 
                                      WINHTTP_DEFAULT_ACCEPT_TYPES, 
                                      0);

    // Send a Request.
    if (hRequest)
        bResults = WinHttpSendRequest(hRequest, 
                                      WINHTTP_NO_ADDITIONAL_HEADERS, 
                                      0, WINHTTP_NO_REQUEST_DATA, 0, 
                                      0, 0);

    // End the request.
    if (bResults)
        bResults = WinHttpReceiveResponse(hRequest, NULL);


	// Continue to verify data until there is nothing left.
    if (bResults)
    {
        do
        {
            // Verify available data.
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)){
                err=1;
				break;
			}
            
            // Allocate space for the buffer.
            pszOutBuffer = new char[dwSize+1];
            
            // Read the Data.
            ZeroMemory(pszOutBuffer, dwSize+1);

            if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, 
                                 dwSize, &dwDownloaded))
            {
				err=1;break;
			}
            else
                receive+=pszOutBuffer;

            // Free the memory allocated to the buffer.
            delete[] pszOutBuffer;
        } while (dwSize > 0);
    } 

    // Report errors.
    if (!bResults)
        err=1;

    // Close open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
	return err;
}


int HttpRequestPost(Minimal::MinimalStringT<wchar_t> &host,Minimal::MinimalStringT<wchar_t> &addr,Minimal::MinimalStringT<char> &data, Minimal::MinimalStringT<char> &receive)
{
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer;
	int err=0;
    BOOL bResults = FALSE;
    HINTERNET hSession = NULL, 
					   hConnect = NULL, 
                       hRequest = NULL;

	hSession = WinHttpOpen(_T("Solfisk"), 
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 
                           WINHTTP_NO_PROXY_NAME, 
                           WINHTTP_NO_PROXY_BYPASS, 0);

	if (hSession)
        hConnect = WinHttpConnect(hSession, host.GetRaw(), 
                                  INTERNET_DEFAULT_HTTP_PORT, 0);

    // Create an HTTP Request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, _T("POST"), 
                                      addr.GetRaw(), 
                                      NULL, WINHTTP_NO_REFERER, 
                                      WINHTTP_DEFAULT_ACCEPT_TYPES, 
                                      0);

	LPCWSTR header=L"Content-type: application/x-www-form-urlencoded/r/n";
    SIZE_T len = lstrlenW(header);
    WinHttpAddRequestHeaders(hRequest,header,DWORD(len), WINHTTP_ADDREQ_FLAG_ADD);

    // Send a Request.
    if (hRequest)
        bResults = WinHttpSendRequest(hRequest, 
                                      0, 
                                      0, data.GetRaw(), data.GetSize()*sizeof(char), 
                                      data.GetSize()*sizeof(char), 0);

    // End the request.
    if (bResults)
        bResults = WinHttpReceiveResponse(hRequest, NULL);


	// Continue to verify data until there is nothing left.
    if (bResults)
    {
        do
        {
            // Verify available data.
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)){
                err=1;
				break;
			}
            
            // Allocate space for the buffer.
            pszOutBuffer = new char[dwSize+1];
            
            // Read the Data.
            ZeroMemory(pszOutBuffer, dwSize+1);

            if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, 
                                 dwSize, &dwDownloaded))
            {
				err=1;break;
			}
            else
                receive+=pszOutBuffer;

            // Free the memory allocated to the buffer.
            delete[] pszOutBuffer;
        } while (dwSize > 0);
    } 

    // Report errors.
    if (!bResults)
        err=1;

    // Close open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
	return err;
}