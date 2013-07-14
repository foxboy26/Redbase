CLANG_NO_WARN := -Wno-c++98-compat -Wno-padded

CXX      := clang++ -std=c++11 -stdlib=libc++ 
CXXFLAGS := -Weverything -g $(CLANG_NO_WARN)
