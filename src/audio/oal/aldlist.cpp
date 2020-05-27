/*
 * Copyright (c) 2006, Creative Labs Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and
 * 	     the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 * 	     and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of Creative Labs Inc. nor the names of its contributors may be used to endorse or
 * 	     promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "aldlist.h"

#ifndef _WIN32
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#endif

#ifdef AUDIO_OAL
/* 
 * Init call
 */
ALDeviceList::ALDeviceList()
{
	char *devices;
	int index;
	const char *defaultDeviceName;
	const char *actualDeviceName;

	// DeviceInfo vector stores, for each enumerated device, it's device name, selection status, spec version #, and extension support
	nNumOfDevices = 0;

	defaultDeviceIndex = 0;

	if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT")) {
		devices = (char *)alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		defaultDeviceName = (char *)alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
		
		index = 0;
		// go through device list (each device terminated with a single NULL, list terminated with double NULL)
		while (*devices != '\0') {
			if (strcmp(defaultDeviceName, devices) == 0) {
				defaultDeviceIndex = index;
			}
			ALCdevice *device = alcOpenDevice(devices);
			if (device) {
				ALCcontext *context = alcCreateContext(device, NULL);
				if (context) {
					alcMakeContextCurrent(context);
					// if new actual device name isn't already in the list, then add it...
					actualDeviceName = alcGetString(device, ALC_DEVICE_SPECIFIER);
					bool bNewName = true;
					for (unsigned int i = 0; i < GetNumDevices(); i++) {
						if (strcmp(GetDeviceName(i), actualDeviceName) == 0) {
							bNewName = false;
						}
					}
					if ((bNewName) && (actualDeviceName != NULL) && (strlen(actualDeviceName) > 0)) {
						ALDEVICEINFO ALDeviceInfo;
						ALDeviceInfo.bSelected = true;
						ALDeviceInfo.strDeviceName = actualDeviceName;
						alcGetIntegerv(device, ALC_MAJOR_VERSION, sizeof(int), &ALDeviceInfo.iMajorVersion);
						alcGetIntegerv(device, ALC_MINOR_VERSION, sizeof(int), &ALDeviceInfo.iMinorVersion);

						// Check for ALC Extensions
						if (alcIsExtensionPresent(device, "ALC_EXT_CAPTURE") == AL_TRUE)
							ALDeviceInfo.Extensions |= ADEXT_EXT_CAPTURE;
						if (alcIsExtensionPresent(device, "ALC_EXT_EFX") == AL_TRUE)
							ALDeviceInfo.Extensions |= ADEXT_EXT_EFX;

						// Check for AL Extensions
						if (alIsExtensionPresent("AL_EXT_OFFSET") == AL_TRUE)
							ALDeviceInfo.Extensions |= ADEXT_EXT_OFFSET;

						if (alIsExtensionPresent("AL_EXT_LINEAR_DISTANCE") == AL_TRUE)
							ALDeviceInfo.Extensions |= ADEXT_EXT_LINEAR_DISTANCE;
						if (alIsExtensionPresent("AL_EXT_EXPONENT_DISTANCE") == AL_TRUE)
							ALDeviceInfo.Extensions |= ADEXT_EXT_EXPONENT_DISTANCE;
						
						if (alIsExtensionPresent("EAX2.0") == AL_TRUE)
							ALDeviceInfo.Extensions |= ADEXT_EAX2;
						if (alIsExtensionPresent("EAX3.0") == AL_TRUE)
							ALDeviceInfo.Extensions |= ADEXT_EAX3;
						if (alIsExtensionPresent("EAX4.0") == AL_TRUE)
							ALDeviceInfo.Extensions |= ADEXT_EAX4;
						if (alIsExtensionPresent("EAX5.0") == AL_TRUE)
							ALDeviceInfo.Extensions |= ADEXT_EAX5;

						if (alIsExtensionPresent("EAX-RAM") == AL_TRUE)
							ALDeviceInfo.Extensions |= ADEXT_EAX_RAM;

						// Get Source Count
						ALDeviceInfo.uiSourceCount = GetMaxNumSources();

						aDeviceInfo[nNumOfDevices++] = ALDeviceInfo;
					}
					alcMakeContextCurrent(NULL);
					alcDestroyContext(context);
				}
				alcCloseDevice(device);
			}
			devices += strlen(devices) + 1;
			index += 1;
		}
	}

	ResetFilters();
}

/* 
 * Exit call
 */
ALDeviceList::~ALDeviceList()
{
}

/*
 * Returns the number of devices in the complete device list
 */
unsigned int ALDeviceList::GetNumDevices()
{
	return nNumOfDevices;
}

/* 
 * Returns the device name at an index in the complete device list
 */
const char * ALDeviceList::GetDeviceName(unsigned int index)
{
	if (index < GetNumDevices())
		return aDeviceInfo[index].strDeviceName;
	else
		return NULL;
}

/*
 * Returns the major and minor version numbers for a device at a specified index in the complete list
 */
void ALDeviceList::GetDeviceVersion(unsigned int index, int *major, int *minor)
{
	if (index < GetNumDevices()) {
		if (major)
			*major = aDeviceInfo[index].iMajorVersion;
		if (minor)
			*minor = aDeviceInfo[index].iMinorVersion;
	}
	return;
}

/*
 * Returns the maximum number of Sources that can be generate on the given device
 */
unsigned int ALDeviceList::GetMaxNumSources(unsigned int index)
{
	if (index < GetNumDevices())
		return aDeviceInfo[index].uiSourceCount;
	else
		return 0;
}

/*
 * Checks if the extension is supported on the given device
 */
bool ALDeviceList::IsExtensionSupported(int index, unsigned short ext)
{
	return !!(aDeviceInfo[index].Extensions & ext);
}

/*
 * returns the index of the default device in the complete device list
 */
int ALDeviceList::GetDefaultDevice()
{
	return defaultDeviceIndex;
}

/* 
 * Deselects devices which don't have the specified minimum version
 */
void ALDeviceList::FilterDevicesMinVer(int major, int minor)
{
	int dMajor, dMinor;
	for (unsigned int i = 0; i < nNumOfDevices; i++) {
		GetDeviceVersion(i, &dMajor, &dMinor);
		if ((dMajor < major) || ((dMajor == major) && (dMinor < minor))) {
			aDeviceInfo[i].bSelected = false;
		}
	}
}

/* 
 * Deselects devices which don't have the specified maximum version
 */
void ALDeviceList::FilterDevicesMaxVer(int major, int minor)
{
	int dMajor, dMinor;
	for (unsigned int i = 0; i < nNumOfDevices; i++) {
		GetDeviceVersion(i, &dMajor, &dMinor);
		if ((dMajor > major) || ((dMajor == major) && (dMinor > minor))) {
			aDeviceInfo[i].bSelected = false;
		}
	}
}

/*
 * Deselects device which don't support the given extension name
 */
void
ALDeviceList::FilterDevicesExtension(unsigned short ext)
{
	for (unsigned int i = 0; i < nNumOfDevices; i++) {
		if (!IsExtensionSupported(i, ext))
			aDeviceInfo[i].bSelected = false;
	}
}

/*
 * Resets all filtering, such that all devices are in the list
 */
void ALDeviceList::ResetFilters()
{
	for (unsigned int i = 0; i < GetNumDevices(); i++) {
		aDeviceInfo[i].bSelected = true;
	}
	filterIndex = 0;
}

/*
 * Gets index of first filtered device
 */
int ALDeviceList::GetFirstFilteredDevice()
{
	unsigned int i;

	for (i = 0; i < GetNumDevices(); i++) {
		if (aDeviceInfo[i].bSelected == true) {
			break;
		}
	}
	filterIndex = i + 1;
	return i;
}

/*
 * Gets index of next filtered device
 */
int ALDeviceList::GetNextFilteredDevice()
{
	unsigned int i;

	for (i = filterIndex; i < GetNumDevices(); i++) {
		if (aDeviceInfo[i].bSelected == true) {
			break;
		}
	}
	filterIndex = i + 1;
	return i;
}

/*
 * Internal function to detemine max number of Sources that can be generated
 */
unsigned int ALDeviceList::GetMaxNumSources()
{
	ALuint uiSources[256];
	unsigned int iSourceCount = 0;

	// Clear AL Error Code
	alGetError();

	// Generate up to 256 Sources, checking for any errors
	for (iSourceCount = 0; iSourceCount < 256; iSourceCount++)
	{
		alGenSources(1, &uiSources[iSourceCount]);
		if (alGetError() != AL_NO_ERROR)
			break;
	}

	// Release the Sources
	alDeleteSources(iSourceCount, uiSources);
	if (alGetError() != AL_NO_ERROR)
	{
		for (unsigned int i = 0; i < 256; i++)
		{
			alDeleteSources(1, &uiSources[i]);
		}
	}

	return iSourceCount;
}
#endif
