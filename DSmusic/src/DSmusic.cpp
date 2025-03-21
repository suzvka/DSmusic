#include "DSmusic.h"
#include "DSparser.h"

namespace DS {
	music* get_music(
		const std::string& json, 
		const std::string& language
	){
		return new parser(json, language);
	}
	music* get_music(
		const std::string& language
	){
		return new parser(language);
	}
}

