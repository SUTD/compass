FILE(REMOVE_RECURSE
  "parser.cpp"
  "parser.hpp"
  "lexer.cpp"
  "CMakeFiles/parser.dir/lexer.o"
  "CMakeFiles/parser.dir/parser.o"
  "CMakeFiles/parser.dir/mistral-parser.o"
  "libparser.pdb"
  "libparser.a"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang CXX)
  INCLUDE(CMakeFiles/parser.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
