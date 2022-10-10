#include "YoutubePlaylist.h"


YoutubePlaylist::YoutubePlaylist(YoutubeApi *newApiObj, const char *newPlaylistId){
    strncpy(playlistId, newPlaylistId, YT_PLAYLISTID_LEN);
    playlistId[YT_PLAYLISTID_LEN] = '\0';

    apiObj = newApiObj;
}


YoutubePlaylist::~YoutubePlaylist(){

    freePlaylistSnippet();
    freePlaylistContentDetails();
    freePlaylistStatus();
}


/**
 * @brief Frees playlistSnippet struct of object and resets flag and pointer.
 * 
 */
void YoutubePlaylist::freePlaylistSnippet(){

    if(!snipSet){
        return;
    }

    free(snip->channelId);
    free(snip->title);
    free(snip->description);
    free(snip->channelTitle);

    free(snip);
    snipSet = false;
    snip = NULL;
}


/**
 * @brief Frees playlistContentDetails struct of object and resets flag and pointer.
 * 
 */
void YoutubePlaylist::freePlaylistContentDetails(){

    if(!contentDetsSet){
        return;
    }

    free(contentDets);
    contentDetsSet = false;
    contentDets = NULL;
}


/**
 * @brief Frees playlistStatus struct of object and resets flag and pointer.
 * 
 */
void YoutubePlaylist::freePlaylistStatus(){

    if(!statusSet){
        return;
    }

    free(status->privacyStatus);

    free(status);
    statusSet = false;
    status = NULL;
}


/**
 * @brief Frees playlistItemsConfiguration struct of object and resets flag and pointer.
 * 
 */
void YoutubePlaylist::freePlaylistItemsConfig(){

    if(!itemsConfigSet){
        return;
    }

    free(playlistItemsConfig);
    itemsConfigSet = false;
    playlistItemsConfig = NULL;
}


/**
 * @brief Returns the value of the playlistStatusSet flag, indicating a valid object.
 * 
 * @return Value of flag.
 */
bool YoutubePlaylist::checkPlaylistStatusSet(){return statusSet;}


/**
 * @brief Returns the value of the playlistSnipSet flag, indicating a valid object.
 * 
 * @return Value of flag.
 */
bool YoutubePlaylist::checkPlaylistSnipSet(){return snipSet;}


/**
 * @brief Returns the value of the playlistContentDets flag, indicating a valid object.
 * 
 * @return Value of flag.
 */
bool YoutubePlaylist::checkPlaylistContentDetsSet(){return contentDetsSet;}


/**
 * @brief Returns the currently set playlist id.
 * 
 * @return const char* playlistId
 */
const char* YoutubePlaylist::getPlaylistId(){return playlistId;}

/**
 * @brief  Fetches playlist status of the set playlist id.
 * 
 * @return true on success, false on error
 */
bool YoutubePlaylist::getPlaylistStatus(){

    freePlaylistStatus();

    char command[150];
    YoutubeApi::createRequestString(playlistListStatus, command, playlistId);
    int httpStatus = apiObj->sendGetToYoutube(command);

    if(httpStatus == 200){
        return parsePlaylistStatus();
    }

    return false;
}

/**
 * @brief Parses the response of the api request to retrieve the playlistStatus.
 * 
 * @return true on success, false on error
 */
bool YoutubePlaylist::parsePlaylistStatus(){

    bool wasSuccessful = false;

	// Get from https://arduinojson.org/v6/assistant/
	const size_t bufferSize = 512;
	DynamicJsonDocument doc(bufferSize);

	// Parse JSON object
	DeserializationError error = deserializeJson(doc, apiObj->client);
	if (!error){

        if(YoutubeApi::checkEmptyResponse(doc)){
            Serial.println("Could not find playlistId!");
            apiObj->closeClient();
	        return wasSuccessful;
        }

        playlistStatus *newplaylistStatus = (playlistStatus*) malloc(sizeof(playlistStatus));

        uint8_t errorCode = 0;

        errorCode += YoutubeApi::allocAndCopy(&newplaylistStatus->privacyStatus, doc["items"][0]["status"]["privacyStatus"].as<const char*>());
        
        if(errorCode){
            Serial.print("Error code: ");
            Serial.print(errorCode);
        }else{
            status = newplaylistStatus;
            statusSet = true;

		    wasSuccessful = true;
        }
    }
	else{
		Serial.print(F("deserializeJson() failed with code "));
		Serial.println(error.c_str());
	}

    apiObj->closeClient();
    return wasSuccessful;
}


/**
 * @brief  Fetches playlist content details of the set playlist id.
 * 
 * @return true on success, false on error
 */
bool YoutubePlaylist::getPlaylistContentDetails(){

    freePlaylistContentDetails();

    char command[150];
    YoutubeApi::createRequestString(playlistListContentDetails, command, playlistId);
    int httpStatus = apiObj->sendGetToYoutube(command);

    if(httpStatus == 200){
        return parsePlaylistContentDetails();
    }

    return false;
}

/**
 * @brief Parses the response of the api request to retrieve the playlist content details.
 * 
 * @return true on success, false on error
 */
bool YoutubePlaylist::parsePlaylistContentDetails(){

    bool wasSuccessful = false;

	// Get from https://arduinojson.org/v6/assistant/
	const size_t bufferSize = 512; // recommended 384, but it throwed errors
	DynamicJsonDocument doc(bufferSize);

	// Parse JSON object
	DeserializationError error = deserializeJson(doc, apiObj->client);
	if (!error){

        if(YoutubeApi::checkEmptyResponse(doc)){
            Serial.println("Could not find playlistId!");
            apiObj->closeClient();
	        return wasSuccessful;
        }

        playlistContentDetails *newPlaylistContentDetails = (playlistContentDetails*) malloc(sizeof(playlistContentDetails));

        newPlaylistContentDetails->itemCount = doc["items"][0]["contentDetails"]["itemCount"].as<uint32_t>();
      
        contentDets = newPlaylistContentDetails;
        contentDetsSet = true;
		wasSuccessful = true;
    }
	else{
		Serial.print(F("deserializeJson() failed with code "));
		Serial.println(error.c_str());
	}

    apiObj->closeClient();
    return wasSuccessful;
}



/**
 * @brief  Fetches playlist snippet of the set playlist id.
 * 
 * @return true on success, false on error
 */
bool YoutubePlaylist::getPlaylistSnippet(){

    freePlaylistSnippet();

    char command[150];
    YoutubeApi::createRequestString(playlistListSnippet, command, playlistId);
    int httpStatus = apiObj->sendGetToYoutube(command);

    if(httpStatus == 200){
        return parsePlaylistSnippet();
    }

    return false;
}

/**
 * @brief Parses the response of the api request to retrieve the playlist content details.
 * 
 * @return true on success, false on error
 */
bool YoutubePlaylist::parsePlaylistSnippet(){

    bool wasSuccessful = false;

	// Get from https://arduinojson.org/v6/assistant/
	const size_t bufferSize = 1600;     // is just enough for upload playlists. It might not work for user made playlists.
                                        // 1600 Bytes is way too large. #TODO: implement filtering to reduce allocated space
	DynamicJsonDocument doc(bufferSize);

	// Parse JSON object
	DeserializationError error = deserializeJson(doc, apiObj->client);
	if (!error){

        if(YoutubeApi::checkEmptyResponse(doc)){
            Serial.println("Could not find playlistId!");
            apiObj->closeClient();
	        return wasSuccessful;
        }

        playlistSnippet *newPlaylistSnippet = (playlistSnippet*) malloc(sizeof(playlistSnippet));

        uint8_t errorCode = 0;

        errorCode += YoutubeApi::allocAndCopy(&newPlaylistSnippet->channelId, doc["items"][0]["snippet"]["channelId"].as<const char*>());
        errorCode += YoutubeApi::allocAndCopy(&newPlaylistSnippet->title, doc["items"][0]["snippet"]["title"].as<const char*>());
        errorCode += YoutubeApi::allocAndCopy(&newPlaylistSnippet->description, doc["items"][0]["snippet"]["description"].as<const char*>());

        errorCode += YoutubeApi::allocAndCopy(&newPlaylistSnippet->channelTitle, doc["items"][0]["snippet"]["channelTitle"].as<const char*>());

        char *ret = strncpy(newPlaylistSnippet->defaultLanguage, doc["items"][0]["snippet"]["defaultLanguage"].as<const char*>(), 3);
        newPlaylistSnippet->defaultLanguage[3]  = '\0';

        newPlaylistSnippet->publishedAt = YoutubeApi::parseUploadDate(doc["items"][0]["snippet"]["publishedAt"].as<const char*>());

        snip = newPlaylistSnippet;
        snipSet = true;
		wasSuccessful = true;
    }
	else{
		Serial.print(F("deserializeJson() failed with code "));
		Serial.println(error.c_str());
	}

    apiObj->closeClient();
    return wasSuccessful;
}