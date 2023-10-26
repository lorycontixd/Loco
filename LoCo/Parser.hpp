#pragma once
#include <string_view>

namespace Loco {
	struct Token {
		public:
			enum class Type {
				eof = 0, // end of file
				eol = 1, // end of line
				indent = 2, // indentation
				identifier = 3, // identifier (name of variable)
				def = 4,
				symbol = 5,
				string = 6
			};

			Token(Type type, const std::string_view value) {
				
			}

	};
}
