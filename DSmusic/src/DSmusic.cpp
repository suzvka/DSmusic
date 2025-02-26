#include "DSmusic.h"
#include "DSparser.h"

namespace DS {
	music* get_music(
		const std::string& json, 
		const std::string& language, 
		const std::unordered_map<std::string, std::string>& ph_map
	){
		return new parser(json, language, ph_map);
	}
	music* get_music(
		const std::string& language, 
		const std::unordered_map<std::string, std::string>& ph_map
	){
		return new parser(language, ph_map);
	}
}

