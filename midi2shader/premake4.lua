-- A solution contains projects, and defines the available configurations
solution "MyApplication"
configurations { "Debug", "Release" }

	-- A project defines one build target
	project "MyApplication"
		kind "ConsoleApp"
		language "C++"
		files { "**.h", "**.cpp" }
		uuid "C49EDC33-2FF9-C449-9E08-6D8A4FC8AA46"

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings" }
			

		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "ExtraWarnings" }    
