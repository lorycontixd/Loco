#include <fstream>
#include <filesystem>
#include "gtest/gtest.h"
#include "pch.h"
#include "../LoCo/Lexer.hpp"

namespace Loco {
	namespace Tests{
		namespace Lexer {
			////// Useful functions
			void WriteToFile(string filename, string content) {
				ofstream f;
				f.open(filename);
				f << content;
				f.close();
			}

			/*bool DeleteFile(const char* filename) {
				try {
					if (std::filesystem::remove(filename))
						return true;
					else
						return false;
				}
				catch (const std::filesystem::filesystem_error& err) {
					return false;
				}
			}*/


			///// Tests
			TEST(test_lexer, read_chars) {
				//EXPECT_EQ(1, 1);
				//EXPECT_TRUE(true);
				WriteToFile("test1.txt", "Hello World!");

				Loco::Lexer::InputStream strm("test1.txt");
				auto c1 = strm.ReadChar();
				EXPECT_EQ(c1, 'H');
				auto c2 = strm.ReadChar();
				EXPECT_EQ(c2, 'e');

				EXPECT_FALSE(c2 == 'f');

				//DeleteFile("test1.txt");
			}

			TEST(test_lexer, test_locations) {
				WriteToFile("test.txt", "My name is Loco!");
				Loco::Lexer::InputStream strm("test.txt");

				EXPECT_EQ(strm.location.lineNum, 1);
				EXPECT_EQ(strm.location.colNum, 1);

				auto c1 = strm.ReadChar().value();
				EXPECT_EQ(c1, 'M');
				EXPECT_EQ(strm.location.lineNum, 1);
				EXPECT_EQ(strm.location.colNum, 2);

				strm.UnreadChar(c1);
				EXPECT_TRUE(strm.location.lineNum == 1 && strm.location.colNum == 1);
			}

			TEST(test_lexer, test_float) {
				string str = "3.1415";
				WriteToFile("test.txt", str);

				Loco::Lexer::InputStream strm("test.txt");
				auto tk = strm.ReadToken();
				EXPECT_TRUE(tk.type == Loco::Lexer::Token::Type::number);
				EXPECT_TRUE(tk.value->type == Loco::Types::LType::Type::FLOAT);
				EXPECT_TRUE( == 3.1415);
			}
		}
	}
}
