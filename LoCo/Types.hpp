#pragma once
#include <string>
#include <format>
#include "Lexer.hpp"
using namespace std;

namespace Loco {
	namespace Types {
		
		class LType {
			public:
				enum class Type {
					INT,
					FLOAT,
					STRING,
					CHAR

				};

				virtual string to_string() const = 0;
				virtual ~LType() = default;


				Type type;
		};


		class LInt : public LType {
			public:
				int value;

				LInt(int value)
				{
					this->value = value;
					type = Type::INT;
				}

				int GetValue() { return this->value; }

				string to_string() const override {
					return std::to_string(this->value);
				}
		};

		class LString : public LType{
			public:
				string value;

				LString(string value)
				{
					this->value = value;
					type = Type::STRING;
				}

				string to_string() const override { return value; }
		};

		class LChar : public LType {
		public:
			char value;

			LChar(char value)
			{
				this->value = value;
				type = Type::CHAR;
			}

			string to_string() const override { return std::to_string(this->value); }
		};

		class LFloat : public LType {
			public:
				float value;
				LFloat(float value)
				{
					this->value = value;
					type = Type::FLOAT;
				}

				string to_string() const override { return std::to_string(this->value); }
		};
	}
}